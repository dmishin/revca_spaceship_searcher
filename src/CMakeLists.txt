set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -pthread")

add_library( revca 
	     pattern.cpp field.cpp random_search.cpp analyze.cpp 
	     rule.cpp tree_pattern.cpp )

add_library( bruteforcing 
	     pattern_enumerator.cpp zigzag.cpp bruteforce_pattern_source.cpp )


#add_executable( cpp-revca cpp-revca.cpp )

add_executable( bulk_analyzer1 
		bulk_analyzer.cpp streamed_analysys.cpp file_pattern_source.cpp
		singlerot.cpp)
target_link_libraries( bulk_analyzer1
		       bruteforcing revca )

#add_executable( bruteforce bruteforce.cpp) 

#target_link_libraries( bruteforce bruteforcing revca )

#target_link_libraries( cpp-revca revca )
