set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

function(add_cppgtest TITLE NAME LIBS)
  add_executable(${NAME} ${NAME}.cpp)
  target_link_libraries(${NAME} gtest gtest_main ${LIBS})
  add_test(${TITLE} ${NAME} )
endfunction(add_cppgtest)

add_cppgtest(Analyze test_analyze_pattern revca )
add_cppgtest(Transform test_transform revca )
