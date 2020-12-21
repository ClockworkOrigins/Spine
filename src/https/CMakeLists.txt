SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/https)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineHttpsSrc ${srcdir}/*.cpp)
FILE(GLOB SpineHttpsHeader ${includedir}/*.h)

ADD_LIBRARY(SpineHttps STATIC ${SpineHttpsSrc} ${SpineHttpsHeader})

target_link_libraries(SpineHttps ${OPENSSL_LIBRARIES})

set_target_properties(
	SpineHttps PROPERTIES FOLDER Libraries
)
