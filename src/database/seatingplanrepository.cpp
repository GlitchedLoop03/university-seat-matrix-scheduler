#include "seatingplanrepository.h"
#include <sqlite3.h>

SeatingPlanRepository::SeatingPlanRepository(sqlite3 *database) : m_db(database) {}
QString SeatingPlanRepository::lastError() const { return m_lastError; }

QVector<SeatingExamination> SeatingPlanRepository::getExaminations()
{
    QVector<SeatingExamination> out; m_lastError.clear();
    if(!m_db){m_lastError="Database connection is not available.";return out;}
    sqlite3_stmt *stmt=nullptr;
    const char *sql="SELECT id,subject,exam_date,exam_time FROM examinations ORDER BY exam_date,exam_time,subject;";
    if(sqlite3_prepare_v2(m_db,sql,-1,&stmt,nullptr)!=SQLITE_OK){m_lastError=QString::fromUtf8(sqlite3_errmsg(m_db));return out;}
    while(sqlite3_step(stmt)==SQLITE_ROW){
        auto txt=[stmt](int i){const auto*p=reinterpret_cast<const char*>(sqlite3_column_text(stmt,i));return QString::fromUtf8(p?p:"");};
        SeatingExamination e; e.id=sqlite3_column_int(stmt,0); e.label=txt(1)+" — "+txt(2)+" "+txt(3); out.append(e);
    }
    sqlite3_finalize(stmt);return out;
}

QVector<SeatingAllocation> SeatingPlanRepository::getAllocations(int examinationId)
{
    QVector<SeatingAllocation> out; m_lastError.clear();
    if(!m_db||examinationId<=0)return out;
    const char *sql=R"SQL(
      SELECT r.room_number,r.rows,r.columns,a.seat_number,s.registration_number,s.name,s.department,e.subject
      FROM seat_allocations a
      JOIN examinations chosen ON chosen.id=?
      JOIN examinations e ON e.id=a.examination_id AND e.exam_date=chosen.exam_date AND e.exam_time=chosen.exam_time
      JOIN rooms r ON r.id=a.room_id JOIN students s ON s.id=a.student_id
      ORDER BY r.room_number COLLATE NOCASE,a.seat_number
    )SQL";
    sqlite3_stmt *stmt=nullptr;
    if(sqlite3_prepare_v2(m_db,sql,-1,&stmt,nullptr)!=SQLITE_OK){m_lastError=QString::fromUtf8(sqlite3_errmsg(m_db));return out;}
    sqlite3_bind_int(stmt,1,examinationId);int rc=SQLITE_OK;
    while((rc=sqlite3_step(stmt))==SQLITE_ROW){
        auto txt=[stmt](int i){const auto*p=reinterpret_cast<const char*>(sqlite3_column_text(stmt,i));return QString::fromUtf8(p?p:"");};
        SeatingAllocation a;a.roomNumber=txt(0);a.roomRows=sqlite3_column_int(stmt,1);a.roomColumns=sqlite3_column_int(stmt,2);
        a.seatNumber=sqlite3_column_int(stmt,3);a.registrationNumber=txt(4);a.studentName=txt(5);a.department=txt(6);a.subject=txt(7);out.append(a);
    }
    if(rc!=SQLITE_DONE)m_lastError=QString::fromUtf8(sqlite3_errmsg(m_db));
    sqlite3_finalize(stmt);return out;
}

bool SeatingPlanRepository::generatePlan(int examinationId)
{
    m_lastError.clear();
    auto fail=[this](const QString&m){m_lastError=m;return false;};
    if(!m_db||examinationId<=0)return fail("Select a valid examination.");
    if(sqlite3_exec(m_db,"BEGIN IMMEDIATE;",nullptr,nullptr,nullptr)!=SQLITE_OK)return fail(QString::fromUtf8(sqlite3_errmsg(m_db)));
    auto rollback=[this,&fail](const QString&m){sqlite3_exec(m_db,"ROLLBACK;",nullptr,nullptr,nullptr);return fail(m);};

    struct Exam{int id;QString subject;QVector<int> students;}; QVector<Exam> exams;
    sqlite3_stmt *stmt=nullptr;
    const char *examSql="SELECT id,subject FROM examinations WHERE (exam_date,exam_time)=(SELECT exam_date,exam_time FROM examinations WHERE id=?) ORDER BY CASE upper(subject) WHEN 'IOOP' THEN 0 WHEN 'CN' THEN 1 ELSE 2 END, subject COLLATE NOCASE, id;";
    if(sqlite3_prepare_v2(m_db,examSql,-1,&stmt,nullptr)!=SQLITE_OK)return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    sqlite3_bind_int(stmt,1,examinationId);int rc=SQLITE_OK;
    while((rc=sqlite3_step(stmt))==SQLITE_ROW){const auto*p=reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));exams.append({sqlite3_column_int(stmt,0),QString::fromUtf8(p?p:""),{}});}
    sqlite3_finalize(stmt);if(rc!=SQLITE_DONE)return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    if(exams.isEmpty())return rollback("The selected examination no longer exists.");

    for(auto &exam:exams){
        if(sqlite3_prepare_v2(m_db,"SELECT reg.student_id FROM registrations reg JOIN students s ON s.id=reg.student_id WHERE reg.examination_id=? ORDER BY s.registration_number COLLATE NOCASE,s.id;",-1,&stmt,nullptr)!=SQLITE_OK)return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
        sqlite3_bind_int(stmt,1,exam.id);rc=SQLITE_OK;
        while((rc=sqlite3_step(stmt))==SQLITE_ROW)exam.students.append(sqlite3_column_int(stmt,0));
        sqlite3_finalize(stmt);if(rc!=SQLITE_DONE)return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    }
    int totalStudents=0;for(const auto&e:exams)totalStudents+=e.students.size();
    if(totalStudents==0)return rollback("No students are registered for any examination in this time slot.");

    struct Room{int id;QString number;int rows;int columns;};QVector<Room> rooms;
    if(sqlite3_prepare_v2(m_db,"SELECT id,room_number,rows,columns FROM rooms ORDER BY room_number COLLATE NOCASE;",-1,&stmt,nullptr)!=SQLITE_OK)return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    while((rc=sqlite3_step(stmt))==SQLITE_ROW){
        const auto*p=reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
        Room r{sqlite3_column_int(stmt,0),QString::fromUtf8(p?p:""),sqlite3_column_int(stmt,2),sqlite3_column_int(stmt,3)};
        if(r.rows<=0||r.columns<=0||qint64(r.rows)*r.columns>100000){sqlite3_finalize(stmt);return rollback("A room has invalid dimensions. Edit that room before generating a plan.");}
        rooms.append(r);
    }
    sqlite3_finalize(stmt);if(rc!=SQLITE_DONE)return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    if(rooms.isEmpty())return rollback("Add at least one room before generating a seating plan.");

    QVector<int> subjectCapacity(exams.size(),0);int totalCapacity=0;
    for(const auto&r:rooms){for(int col=0;col<r.columns;++col)subjectCapacity[col%exams.size()]+=r.rows;totalCapacity+=r.rows*r.columns;}
    if(totalCapacity<totalStudents)return rollback(QString("Insufficient physical seats. Registered students: %1; available seats: %2.").arg(totalStudents).arg(totalCapacity));
    for(int i=0;i<exams.size();++i)if(exams[i].students.size()>subjectCapacity[i])
        return rollback(QString("Not enough alternating-column seats for %1. Registered: %2; available in its assigned columns: %3. Add another room or adjust room dimensions.").arg(exams[i].subject).arg(exams[i].students.size()).arg(subjectCapacity[i]));

    if(sqlite3_prepare_v2(m_db,"DELETE FROM seat_allocations WHERE examination_id IN (SELECT id FROM examinations WHERE (exam_date,exam_time)=(SELECT exam_date,exam_time FROM examinations WHERE id=?));",-1,&stmt,nullptr)!=SQLITE_OK)return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    sqlite3_bind_int(stmt,1,examinationId);rc=sqlite3_step(stmt);sqlite3_finalize(stmt);if(rc!=SQLITE_DONE)return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));

    sqlite3_stmt *insert=nullptr;
    if(sqlite3_prepare_v2(m_db,"INSERT INTO seat_allocations(examination_id,room_id,student_id,seat_number) VALUES(?,?,?,?);",-1,&insert,nullptr)!=SQLITE_OK)return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    QVector<int> next(exams.size(),0);
    for(const auto&r:rooms)for(int row=0;row<r.rows;++row)for(int col=0;col<r.columns;++col){
        int e=col%exams.size();if(next[e]>=exams[e].students.size())continue;
        int seat=col*r.rows+row+1;
        sqlite3_reset(insert);sqlite3_clear_bindings(insert);sqlite3_bind_int(insert,1,exams[e].id);sqlite3_bind_int(insert,2,r.id);
        sqlite3_bind_int(insert,3,exams[e].students[next[e]++]);sqlite3_bind_int(insert,4,seat);
        if(sqlite3_step(insert)!=SQLITE_DONE){QString err=QString::fromUtf8(sqlite3_errmsg(m_db));sqlite3_finalize(insert);return rollback(err);}
    }
    sqlite3_finalize(insert);
    if(sqlite3_exec(m_db,"COMMIT;",nullptr,nullptr,nullptr)!=SQLITE_OK){QString err=QString::fromUtf8(sqlite3_errmsg(m_db));sqlite3_exec(m_db,"ROLLBACK;",nullptr,nullptr,nullptr);return fail(err);}
    return true;
}
