project(template_test LANGUAGES CXX)

link_libraries(SceSystemService SceUserService SceSysmodule SceNpMatching2)

create_pkg("TMAT00100" 1 00 "code/main.cpp;code/test.cpp")
