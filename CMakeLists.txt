
cmake_minimum_required( VERSION 2.8 )

find_package( Qt4 REQUIRED )
include( "${QT_USE_FILE}" )

add_executable( nv2rgb main.cpp )
target_link_libraries( nv2rgb ${QT_LIBRARIES} )
