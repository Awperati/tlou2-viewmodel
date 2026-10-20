cmake_minimum_required(VERSION 3.20)
project(tlou2_viewmodel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# TLOU2Viewmodel.dll: the viewmodel and head lock, injected into the running game.
add_library(tlou2_vm SHARED src/vm/vm_main.cpp src/mod/log.cpp)
set_target_properties(tlou2_vm PROPERTIES OUTPUT_NAME "TLOU2Viewmodel")
target_include_directories(tlou2_vm PRIVATE src)
target_compile_definitions(tlou2_vm PRIVATE UNICODE _UNICODE NOMINMAX)

# fp_launcher.exe: loads one or more DLLs/ASIs into the running game from their own folder.
add_executable(fp_launcher src/launcher/fp_launcher.cpp)
target_compile_definitions(fp_launcher PRIVATE UNICODE _UNICODE NOMINMAX)

enable_testing()
add_executable(viewmodel_tests tests/viewmodel_tests.cpp)
target_include_directories(viewmodel_tests PRIVATE src)
add_test(NAME viewmodel_tests COMMAND viewmodel_tests)
