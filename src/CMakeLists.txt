cmake_minimum_required(VERSION 3.5)

project(${project} LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(coderdbc
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/c-main-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/mon-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/config-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/c-util-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/conditional-tree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/c-sigprinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/filewriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/fs-creator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers/formatter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/dbclineparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/dbcscanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/options-parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/maincli.cpp
)


include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  cctest
  ${CMAKE_CURRENT_SOURCE_DIR}/helpers/formatter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/options-parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/parser/dbclineparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/args-test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/bitext-test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/dbcline-test.cpp
)

target_link_libraries(
  cctest
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cctest)


add_library(
  dummy OBJECT 
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/gencode/usr/testdb-fmon.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/gencode/lib/testdb.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/gencode/butl/bms_testdb-binutil.c
)

target_include_directories(
  dummy PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/gencode/conf
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/gencode/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/gencode/inc
)


# Exercise actual generated conversions, not a copy of the generator's formula.
set(rounding_dir "${CMAKE_CURRENT_BINARY_DIR}/rounding")
file(MAKE_DIRECTORY "${rounding_dir}")
configure_file(tests/rounding-config.hpp "${rounding_dir}/rounding-config.hpp" COPYONLY)
add_custom_command(
  OUTPUT "${rounding_dir}/rounding.cpp" "${rounding_dir}/rounding.hpp"
  COMMAND $<TARGET_FILE:coderdbc>
    -dbc "${CMAKE_CURRENT_SOURCE_DIR}/../test/rounding.dbc"
    -out "${rounding_dir}" -drvname rounding -rw -noconfig -noinc -nofmon
  DEPENDS coderdbc "${CMAKE_CURRENT_SOURCE_DIR}/../test/rounding.dbc"
  VERBATIM
)
add_custom_target(rounding_codegen
  DEPENDS "${rounding_dir}/rounding.cpp" "${rounding_dir}/rounding.hpp")
foreach(mode float double raw)
  add_executable(rounding_${mode}_test
    tests/rounding-test.cpp "${rounding_dir}/rounding.cpp")
  add_dependencies(rounding_${mode}_test rounding_codegen)
  target_include_directories(rounding_${mode}_test PRIVATE "${rounding_dir}")
  target_link_libraries(rounding_${mode}_test PRIVATE GTest::gtest_main)
  if(mode STREQUAL "raw")
    target_compile_definitions(rounding_${mode}_test PRIVATE ROUNDING_TEST_TYPE=float)
  else()
    target_compile_definitions(rounding_${mode}_test PRIVATE
      ROUNDING_USE_SIGFLOAT ROUNDING_TEST_TYPE=${mode})
  endif()
  gtest_discover_tests(rounding_${mode}_test TEST_PREFIX "${mode}.")
endforeach()
