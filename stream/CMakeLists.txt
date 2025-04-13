find_package(Threads REQUIRED)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(server Threads::Threads)