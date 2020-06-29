SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/discord)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB DiscordHeader ${includedir}/*.h)
FILE(GLOB DiscordSrc ${srcdir}/*.cpp)
FILE(GLOB DiscordDepSrc ${DISCORD_ROOT}/src/*.cpp)

ADD_LIBRARY(SpineDiscord STATIC ${DiscordSrc} ${DiscordDepSrc} ${DiscordHeader})

target_link_libraries(SpineDiscord ${DISCORD_LIBRARIES})

set_target_properties(
	SpineDiscord PROPERTIES FOLDER Libraries
)
