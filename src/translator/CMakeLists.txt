SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/translator)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineTranslatorSrc ${srcdir}/*.cpp)
FILE(GLOB SpineTranslatorHeader ${includedir}/*.h)

ADD_LIBRARY(SpineTranslator STATIC ${SpineTranslatorSrc} ${SpineTranslatorHeader})

TARGET_LINK_LIBRARIES(SpineTranslator SpineCommon SpineGui SpineUtils)

set_target_properties(
	SpineTranslator PROPERTIES FOLDER Libraries
)
