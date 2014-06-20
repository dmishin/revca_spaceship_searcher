set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -pthread")

add_library( revca pattern.cpp field.cpp random_search.cpp analyze.cpp rule.cpp tree_pattern.cpp )

#add_executable( cpp-revca cpp-revca.cpp )
add_executable( bulk_analyzer bulk_analyzer.cpp )

target_link_libraries( bulk_analyzer revca )
#target_link_libraries( cpp-revca revca )
