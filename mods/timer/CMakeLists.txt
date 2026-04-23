project(DoDoeb.timer_mngr)

add_library($(PROJECT_NAME) MODULE src/timer.cpp timer.hpp)
target_link_libraries($(PROJECT_NAME) PRIVATE modlib)
set_target_properties($(PROJECT_NAME) PROPERTIES OUTPUT_NAME "DoDoeb.timer_mngr")
set_target_properties($(PROJECT_NAME) PROPERTIES PREFIX "")
set_target_properties($(PROJECT_NAME) PROPERTIES SUFFIX ".mod")