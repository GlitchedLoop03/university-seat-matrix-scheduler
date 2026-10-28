cmake_minimum_required(VERSION 3.21)

project(UniversitySeatMatrixScheduler
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(SQLite3 REQUIRED)

qt_standard_project_setup()

qt_add_executable(university-seat-matrix-scheduler
    src/database/roomrepository.cpp
    src/ui/roomspage.h
    src/ui/roomspage.cpp
    src/ui/examinationspage.cpp
    src/database/registrationrepository.h
    src/database/registrationrepository.cpp
    src/ui/registrationspage.h
    src/ui/registrationspage.cpp
    src/database/seatingplanrepository.h
    src/database/seatingplanrepository.cpp
    src/ui/seatingplanspage.h
    src/ui/seatingplanspage.cpp
    src/ui/examinationspage.h
    src/database/examinationrepository.cpp
    src/database/examinationrepository.h
    src/models/examination.h
    src/database/roomrepository.h
    src/models/room.h
    src/ui/studentspage.cpp
    src/ui/studentspage.h
    main.cpp
    src/database/database.h
    src/database/database.cpp
    src/database/studentrepository.h
    src/database/studentrepository.cpp
    src/models/student.h
)

target_link_libraries(university-seat-matrix-scheduler
    PRIVATE
        Qt6::Widgets
        SQLite::SQLite3
)
