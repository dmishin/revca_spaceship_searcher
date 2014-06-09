#add_executable( unit_network unit_network.cpp )
#target_link_libraries( unit_network spiking gtest gtest_main )
#add_test( Network unit_network )

function(add_cppgtest TITLE NAME LIBS)
  add_executable(${NAME} ${NAME}.cpp)
  target_link_libraries(${NAME} gtest gtest_main ${LIBS})
  add_test(${TITLE} ${NAME} )
endfunction(add_cppgtest)

add_cppgtest(Analyze test_analyze_pattern revca )
