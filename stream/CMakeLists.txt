find_package(Threads REQUIRED)

add_executable(server server.cpp)
target_link_libraries(server Threads::Threads)