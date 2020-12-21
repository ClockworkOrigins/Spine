SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/common)

INCLUDE_DIRECTORIES(${includedir})

SET(CommonSrc
	${srcdir}/Encryption.cpp
	${srcdir}/MessageStructs.cpp
)

FILE(GLOB CommonHeader ${includedir}/*.h)

ADD_LIBRARY(SpineCommon STATIC ${CommonSrc} ${CommonHeader})

target_link_libraries(SpineCommon ${OPENSSL_LIBRARIES})
target_link_libraries(SpineCommon debug ${BOOST_DEBUG_BOOST_SERIALIZATION_LIBRARY} optimized ${BOOST_RELEASE_BOOST_SERIALIZATION_LIBRARY})
target_link_libraries(SpineCommon debug ${CLOCKUTILS_DEBUG_CLOCK_SOCKETS_LIBRARY} optimized ${CLOCKUTILS_RELEASE_CLOCK_SOCKETS_LIBRARY})

set_target_properties(
	SpineCommon PROPERTIES FOLDER Libraries
)
