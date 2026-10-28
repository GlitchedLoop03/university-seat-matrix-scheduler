# University Seat Matrix Scheduler

A desktop application for managing university examination seating. It provides student, room, examination, and registration management, along with room-wise seating-plan generation.

Built with **C++17, Qt 6 Widgets, SQLite, and CMake** on Linux.

## Features

### Student Management
- Add, view, search, edit, and delete student records.
- Store registration numbers, names, and departments.
- Validate required fields and reject duplicate registration numbers.

### Room Management
- Create and manage examination rooms.
- Store room numbers and seating capacities.
- Search, edit, and delete rooms.
- Validate room capacity and unique room numbers.

### Examination Management
- Schedule examinations by subject, date, and start time.
- Search, edit, and delete examination records.

### Registration Management
- Register existing students for examinations.
- View registrations for a selected examination.
- Prevent duplicate registrations for the same examination.
- Remove a registration without deleting the student record.

### Seating Plans
- Generate room-wise seating allocations for registered students.
- Display room, seat number, registration number, student name, and department.
- Check available room capacity before generating a plan.
- Regenerate seating plans when needed.

### Dashboard and Persistence
- Navigate between the main application modules.
- View summary counts for students, rooms, and examinations.
- Store application records in an SQLite database.

## Technology Stack

| Technology | Purpose |
|---|---|
| C++17 | Application logic |
| Qt 6 Widgets | Desktop user interface |
| SQLite | Persistent relational database |
| CMake | Build configuration |
| Git | Version control |

## Requirements

- A C++17-compatible compiler
- CMake
- Qt 6 Widgets
- SQLite development libraries
- Git

### Install dependencies on Arch Linux

```bash
sudo pacman -S --needed base-devel cmake ninja git qt6-base sqlite
```

## Build and Run

Clone the repository:

```bash
git clone https://github.com/GlitchedLoop03/university-seat-matrix-scheduler.git
cd university-seat-matrix-scheduler
```

Configure and build:

```bash
cmake -S . -B build
cmake --build build
```

Launch the application:

```bash
./build/university-seat-matrix-scheduler
```

## Typical Workflow

1. Add student records.
2. Create examination rooms and set their capacities.
3. Schedule an examination.
4. Register students for that examination.
5. Open **Seating Plans** and generate the seating allocation.
6. Review the room-wise seating plan.

The total available room capacity must be sufficient for the number of registered students.

## Project Structure

```text
university-seat-matrix-scheduler/
├── CMakeLists.txt
├── main.cpp
└── src/
    ├── database/
    │   ├── database.h
    │   ├── database.cpp
    │   ├── studentrepository.h
    │   ├── studentrepository.cpp
    │   ├── roomrepository.h
    │   ├── roomrepository.cpp
    │   ├── examinationrepository.h
    │   ├── examinationrepository.cpp
    │   ├── registrationrepository.h
    │   ├── registrationrepository.cpp
    │   ├── seatingplanrepository.h
    │   └── seatingplanrepository.cpp
    ├── models/
    └── ui/
```

The `models` directory contains data structures, while `database` contains persistence and repository logic. The `ui` directory contains the application pages.

## Database

The application initializes its SQLite schema when it starts and stores data in the operating system's application-data directory. The database is not intended to be committed to the source repository.

Back up important records before making changes to the application or its database.

## Testing Notes

The application has been built and exercised manually during development. The main workflows include student management, room management, examination scheduling, student registration, and seating-plan generation.

A complete automated regression test suite has not been established. Edge cases and behavior across all modules should be verified before using the application for real examination administration.

## Current Status

**Development prototype.** The main application modules are implemented. Further testing, accessibility checks, and release packaging may be needed before production use.

## Author

Developed as a C++ desktop application project.
