set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -pthread")

add_library( revca pattern.cpp field.cpp random_search.cpp analyze.cpp rule.cpp tree_pattern.cpp )

add_library( pattern_enum pattern_enumerator.cpp zigzag.cpp )


#add_executable( cpp-revca cpp-revca.cpp )
add_executable( bulk_analyzer bulk_analyzer.cpp streamed_analysys.cpp file_pattern_source.cpp)
add_executable( bruteforce bruteforce.cpp) 

target_link_libraries( bruteforce pattern_enum revca )
target_link_libraries( bulk_analyzer revca )
#target_link_libraries( cpp-revca revca )
