
add_library( revca pattern.cpp field.cpp random_search.cpp analyze.cpp rule.cpp )
#add_library( gason gason.cpp )

add_executable( cpp-revca cpp-revca.cpp )
add_executable( bulk_analyzer bulk_analyzer.cpp )

target_link_libraries( bulk_analyzer revca )
target_link_libraries( cpp-revca revca )
