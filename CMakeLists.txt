cmake_minimum_required(VERSION 3.10)

project(MedicalScheduler)

set(CMAKE_CXX_STANDARD 11)

# Statically link the C/C++ runtime for all targets (so .exe works standalone)
if(MSVC)
    foreach(flag_var CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
        set(${flag_var} "${${flag_var}} /MT")
    endforeach()
endif()

# Ensure all executables go to build/Debug or build/Release, even with Visual Studio
if(CMAKE_CONFIGURATION_TYPES)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG})
    endforeach()
endif()

# Set the output directory for all executables (for single-config generators)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

# Include directories
include_directories(src/common)
include_directories(src/utils)
include_directories(src/client)
include_directories(src/server)

# Client executable
add_executable(client
    src/client/main.cpp
    src/client/Patient.cpp
    src/client/VigenereCipher.cpp
    src/client/NetworkClient.cpp
)

# Server executable
add_executable(server
    src/server/main.cpp
    src/server/Server.cpp
    src/server/Admin.cpp
    src/server/Appointment.cpp
)

# Common source files
set(COMMON_SOURCES
    src/common/User.cpp
    src/common/Protocol.cpp
)

# Link common sources to both client and server
target_sources(client PRIVATE ${COMMON_SOURCES})
target_sources(server PRIVATE ${COMMON_SOURCES})

# Approved Appointments Viewer executable
add_executable(approved_viewer
    src/viewer/main.cpp
    src/viewer/ApprovedAppointmentsViewer.cpp
    src/client/NetworkClient.cpp  # Reusing NetworkClient from client directory
)

# Link common sources to the viewer
target_sources(approved_viewer PRIVATE ${COMMON_SOURCES})

# Add include directories for the viewer
# It needs access to common, client (for NetworkClient.h), and its own viewer directory
target_include_directories(approved_viewer PRIVATE
    src/common
    src/client 
    src/viewer
)

# Link libraries (e.g., ws2_32 on Windows for socket programming)
if(WIN32)
    target_link_libraries(client ws2_32)
    target_link_libraries(server ws2_32)
    target_link_libraries(approved_viewer ws2_32)
endif()