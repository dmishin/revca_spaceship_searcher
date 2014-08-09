set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -pthread")

add_library( revca 
	     pattern.cpp field.cpp random_search.cpp analyze.cpp 
	     rule.cpp tree_pattern.cpp singlerot.cpp )

add_library( bruteforcing 
	     pattern_enumerator.cpp zigzag.cpp bruteforce_pattern_source.cpp )


add_executable( bulk_analyzer 
		bulk_analyzer.cpp streamed_analysys.cpp file_pattern_source.cpp )
target_link_libraries( bulk_analyzer
		       bruteforcing revca )

#add_executable( sample_random_seed_search sample_random_seed_search.cpp )
#target_link_libraries( sample_random_seed_search revca )
#add_executable( sample_bruteforce_patterns sample_bruteforce_patterns.cpp) 
