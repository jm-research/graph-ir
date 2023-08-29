include(CTest)
enable_testing()
find_package(GTest REQUIRED CONFIG)

add_subdirectory(Graph)
add_subdirectory(Frontend)
add_subdirectory(CodeGen)