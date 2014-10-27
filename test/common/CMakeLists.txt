set(TEST_COMMON_SOURCES
    test_map.cpp
    test_map_rgba.cpp
    test_mathutils.cpp
    test_region.cpp
)

add_library(test_common ${TEST_COMMON_SOURCES})
target_link_libraries(test_common common common_rgba ${BOOST_FILESYSTEM_LIBRARY})
