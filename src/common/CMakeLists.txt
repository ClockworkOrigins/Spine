SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/common)

INCLUDE_DIRECTORIES(${includedir})

SET(CommonSrc
	${srcdir}/Encryption.cpp
	${srcdir}/MessageStructs.cpp
)

FILE(GLOB CommonHeader ${includedir}/*.h)

ADD_LIBRARY(SpineCommon STATIC ${CommonSrc} ${CommonHeader})

target_link_libraries(SpineCommon ${BOOST_LIBRARIES} ${CLOCKUTILS_LIBRARIES} ${OPENSSL_LIBRARIES} ${TINYXML_LIBRARIES})

set_target_properties(
	SpineCommon PROPERTIES FOLDER Libraries
)
