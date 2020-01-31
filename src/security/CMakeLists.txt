SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/security)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineSecuritySrc ${srcdir}/*.cpp)
FILE(GLOB SpineSecurityHeader ${includedir}/*.h)

ADD_LIBRARY(SpineSecurity STATIC ${SpineSecuritySrc} ${SpineSecurityHeader})

set_target_properties(
	SpineSecurity PROPERTIES FOLDER Libraries
)
