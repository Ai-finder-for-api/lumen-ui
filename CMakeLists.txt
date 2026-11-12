cmake_minimum_required(VERSION 3.20)
project(LumenUI VERSION 0.1.0 LANGUAGES CXX)

option(LUMEN_BUILD_TESTS "Build Lumen tests" ON)
option(LUMEN_BUILD_EXAMPLES "Build Lumen examples" ON)

add_library(lumen
    src/application.cpp
    src/widget.cpp
    src/canvas.cpp
)
add_library(Lumen::Lumen ALIAS lumen)
find_package(Threads REQUIRED)
target_link_libraries(lumen PUBLIC Threads::Threads)
target_compile_features(lumen PUBLIC cxx_std_20)
target_include_directories(lumen PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(lumen PROPERTIES CXX_EXTENSIONS OFF EXPORT_NAME Lumen)
if(MSVC)
  target_compile_options(lumen PRIVATE /W4 /permissive-)
else()
  target_compile_options(lumen PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(LUMEN_BUILD_TESTS)
  enable_testing()
  add_executable(lumen_tests tests/test_main.cpp)
  target_link_libraries(lumen_tests PRIVATE Lumen::Lumen)
  add_test(NAME lumen_tests COMMAND lumen_tests)
endif()

if(LUMEN_BUILD_EXAMPLES)
  add_executable(lumen_demo examples/hello.cpp)
  target_link_libraries(lumen_demo PRIVATE Lumen::Lumen)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/LumenConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/LumenConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Lumen)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/LumenConfigVersion.cmake
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(TARGETS lumen EXPORT LumenTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT LumenTargets NAMESPACE Lumen:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Lumen)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LumenConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/LumenConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Lumen)
