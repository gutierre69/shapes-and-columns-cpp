cmake_minimum_required(VERSION 3.16)
project(Columns CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(raylib/include)
link_directories(raylib/lib)

file(GLOB SOURCES
    src/*.cpp
)

add_executable(Columns ${SOURCES})

target_link_libraries(Columns raylib m dl pthread GL X11)

# if(UNIX)
#     target_link_libraries(Columns raylib m dl pthread GL X11)
# elseif(WIN32)
#     # Linka bibliotecas alternativas no Windows, se necessário
#     target_link_libraries(Columns PRIVATE raylib opengl32)
# endif()

# Copy the resources
file(GLOB resources resources/*)
set(test_resources)
list(APPEND test_resources ${resources})
file(COPY ${test_resources} DESTINATION "resources/")
