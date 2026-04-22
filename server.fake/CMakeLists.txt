project(server)

add_executable(server main.cpp)
target_link_libraries(server PUBLIC modlib bardak-ifc)
