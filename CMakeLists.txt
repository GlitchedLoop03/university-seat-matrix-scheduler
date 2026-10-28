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
