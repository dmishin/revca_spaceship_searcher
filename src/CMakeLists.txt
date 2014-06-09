
add_library( revca pattern.cpp field.cpp random_search.cpp analyze.cpp rule.cpp )
add_executable( cpp-revca cpp-revca.cpp )

target_link_libraries( cpp-revca revca )
