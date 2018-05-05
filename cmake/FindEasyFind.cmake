# easyFind version 1.2 - 21.10.2016 - # 011

# Find $LIBNAME includes and library
#
# --------------------
# This modules works correctly with these find_package parameters
# --------------------
#	QUIET			no messages are print
#	REQUIRED [comp...]	cmake fails if not found
#	COMPONENTS [comp...]	only look for the compoents comp. if not specified, look for all
#
# --------------------
# This modules uses the following variables
# --------------------
#	${LIBNAME}			Name of the Libray Tag to be used
#	${LIBNAME}_HEADER	This file is used to find the include directory
#	${LIBNAME}_ROOT		Look here first for library / headers
#
# --------------------
# This modules defines the following variables
# --------------------
#	${LIBNAME}_INCLUDE_DIR	Path to the include files
#	${LIBNAME}_LIBRARIES	List of all libraries
#	${LIBNAME}_LIB_DIR	location of the libraries
#	${LIBNAME}_FOUND	TRUE if ${LIBNAME} was found, FALSE otherwise

# sets all components needed
IF (EasyFind_FIND_COMPONENTS)
	SET (COMPONENTS ${EasyFind_FIND_COMPONENTS})
ENDIF (EasyFind_FIND_COMPONENTS)

IF (NOT EasyFind_FIND_QUIETLY)
	message(STATUS "Looking for ${LIBNAME}:")
ENDIF (NOT EasyFind_FIND_QUIETLY)

# find the libraries
	FOREACH (COMPONENT ${COMPONENTS})
		string(TOUPPER ${COMPONENT} UPPERCOMPONENT)

		find_library(
			${LIBNAME}_${UPPERCOMPONENT}_LIBRARY	# return value
			NAMES ${COMPONENT}						# list of possible names, currently only exactly the name
			HINTS ${${LIBNAME}_ROOT}/lib			# first search here
			NO_CMAKE_FIND_ROOT_PATH					# otherwise will break with the android toolchain
		)

		IF (${LIBNAME}_${UPPERCOMPONENT}_LIBRARY)
			SET(${LIBNAME}_LIBRARIES ${${LIBNAME}_LIBRARIES} ${${LIBNAME}_${UPPERCOMPONENT}_LIBRARY})
			GET_FILENAME_COMPONENT(${LIBNAME}_LIB_DIR ${${LIBNAME}_${UPPERCOMPONENT}_LIBRARY} PATH)
			message(STATUS "	${COMPONENT}	found")
		ELSE (${LIBNAME}_${UPPERCOMPONENT}_LIBRARY)
			SET (${LIBNAME}_notFoundList ${${LIBNAME}_notFoundList} ${COMPONENT})
			message(STATUS "	${COMPONENT}	not found")
		ENDIF (${LIBNAME}_${UPPERCOMPONENT}_LIBRARY)
	ENDFOREACH (COMPONENT)

	IF (${LIBNAME}_notFoundList)
		SET (${LIBNAME}_FOUND FALSE)
		IF (${EasyFind_FIND_REQUIRED})
			message(FATAL_ERROR "${LIBNAME} marked as required, but not all components were found.")
		ELSE (${EasyFind_FIND_REQUIRED})
			message(WARNING "Not all components were found. Continuing anyways")
		ENDIF (${EasyFind_FIND_REQUIRED})
	ELSEIF (${LIBNAME}_notFoundList)
		SET (${LIBNAME}_FOUND TRUE)
	ENDIF (${LIBNAME}_notFoundList)

# find the header
	find_path(
		${LIBNAME}_HEADER					# return value
		NAMES ${LIBHEADER}					# file to find
		HINTS ${${LIBNAME}_ROOT}			# first search here
		${${LIBNAME}_ROOT}/include			# first search here
		${${LIBNAME}_ROOT}/include/*		# first search here
		NO_CMAKE_FIND_ROOT_PATH				# otherwise will break with the android toolchain
	)

	IF (${LIBNAME}_HEADER)
		SET (${LIBNAME}_INCLUDE_DIR ${${LIBNAME}_HEADER})
	ELSE (${LIBNAME}_HEADER)
		SET (${LIBNAME}_FOUND FALSE)
		IF (${EasyFind_FIND_REQUIRED})
			message(FATAL_ERROR "${LIBNAME} marked as required but header not found")
		ELSE (${EasyFind_FIND_REQUIRED})
			IF (NOT ${LIBNAME}_FIND_QUIETLY)
				message(WARNING "${LIBNAME} header not found")
			ENDIF (NOT ${LIBNAME}_FIND_QUIETLY)
		ENDIF (${EasyFind_FIND_REQUIRED})
	ENDIF (${LIBNAME}_HEADER)
