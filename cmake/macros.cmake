include(CMakeParseArguments)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckCSourceCompiles)

#-------------------------------------------------------------------------------
# macros
#-------------------------------------------------------------------------------

set(GAME_BASE_DIR "base")

if (NOT WIN32)
	string(ASCII 27 Esc)
	set(ColorReset  "${Esc}[m")
	set(ColorRed    "${Esc}[31m")
	set(ColorGreen  "${Esc}[32m")
	set(ColorYellow "${Esc}[33m")
	set(ColorBlue   "${Esc}[34m")
endif()

if(NOT COMMAND find_host_package)
	macro(find_host_package)
		find_package(${ARGN})
	endmacro()
endif()

if(NOT COMMAND find_host_program)
	macro(find_host_program)
		find_program(${ARGN})
	endmacro()
endif()

#function(message)
#	list(GET ARGV 0 TYPE)
#	if (TYPE STREQUAL FATAL_ERROR)
#		list(REMOVE_AT ARGV 0)
#		_message(${TYPE} "${ColorRed}${ARGV}${ColorReset}")
#	elseif(TYPE STREQUAL WARNING)
#		list(REMOVE_AT ARGV 0)
#		_message(${TYPE} "${ColorYellow}${ARGV}${ColorReset}")
#	elseif(TYPE STREQUAL STATUS)
#		list(REMOVE_AT ARGV 0)
#		_message(${TYPE} "${ColorGreen}${ARGV}${ColorReset}")
#	elseif (ARGV)
#		_message("${ARGV}")
#	endif()
#endfunction()

macro(cp_message MSG)
	if (VERBOSE)
		message("${ColorBlue}${MSG}${ColorReset}")
	endif()
endmacro()

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
	set(CP_GCC 1)
	cp_message("C++ Compiler: GCC")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	set(CP_CLANG 1)
	cp_message("C++ Compiler: Clang")
elseif (MSVC)
	set(CP_MSVC 1)
	cp_message("C++ Compiler: MSVC")
else()
	message(WARNING "C++ Compiler: Unknown")
endif()

if (CP_GCC OR CP_CLANG)
	check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
	check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
	if (COMPILER_SUPPORTS_CXX11)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	elseif (COMPILER_SUPPORTS_CXX0X)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
		cp_message(STATUS "Assume that -std=c++11 works")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
	if (RELEASE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
	else()
		# cp_cast needs this in debug builds
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
	endif()
endif()

cp_message("Target processor: ${CMAKE_SYSTEM_PROCESSOR}")
cp_message("Host processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")

macro(texture_file_write TARGET_FILE FILEENTRY)
	file(READ ${FILEENTRY} CONTENTS)
	string(REGEX REPLACE ";" "\\\\;" CONTENTS "${CONTENTS}")
	string(REGEX REPLACE "\n" ";" CONTENTS "${CONTENTS}")
	list(REMOVE_AT CONTENTS 0 -2)
	foreach(LINE ${CONTENTS})
		file(APPEND ${TARGET_FILE} "${LINE}\n")
	endforeach()
endmacro()

macro(get_subdirs RESULT DIR)
	file(GLOB SUBDIRS RELATIVE ${DIR} ${DIR}/*)
	set(DIRLIST "")
	foreach(CHILD ${SUBDIRS})
		if (IS_DIRECTORY ${DIR}/${CHILD})
			list(APPEND DIRLIST ${CHILD})
		endif()
	endforeach()
	if (DIRLIST)
		set(${RESULT} ${DIRLIST})
		list(SORT ${RESULT})
	endif()
endmacro()

macro(create_dir_header PROJECTNAME)
	set(TARGET_DIR ${CMAKE_BINARY_DIR})
	file(MAKE_DIRECTORY ${TARGET_DIR})
	set(TARGET_FILE ${TARGET_DIR}/${PROJECTNAME}-files.h.in)
	set(BASEDIR ${ROOT_DIR}/${GAME_BASE_DIR}/${PROJECTNAME})
	set(FINAL_TARGET_FILE ${TARGET_DIR}/${PROJECTNAME}-files.h)

	set(SUBDIRS)
	get_subdirs(SUBDIRS ${BASEDIR})
	file(WRITE ${TARGET_FILE} "")
	foreach(SUBDIR ${SUBDIRS})
		file(GLOB FILESINDIR RELATIVE ${BASEDIR}/${SUBDIR} ${BASEDIR}/${SUBDIR}/*)
		list(LENGTH FILESINDIR LISTENTRIES)
		file(APPEND ${TARGET_FILE} "if (basedir == \"${SUBDIR}/\") {\n")
		file(APPEND ${TARGET_FILE} "\tentriesAll.reserve(${LISTENTRIES});\n")
		set(RESULT ${FILESINDIR})
		if (RESULT)
			list(SORT RESULT)
		endif()
		foreach(FILEINDIR ${RESULT})
			string(COMPARE EQUAL "${FILEINDIR}" ".gitignore" IGNOREFILE)
			if (NOT IGNOREFILE)
				file(APPEND ${TARGET_FILE} "\tentriesAll.push_back(\"${FILEINDIR}\");\n")
			endif()
		endforeach()
		file(APPEND ${TARGET_FILE} "\treturn entriesAll;\n}\n")
	endforeach()
	configure_file(${TARGET_FILE} ${FINAL_TARGET_FILE})
	message(STATUS "wrote ${TARGET_FILE}")
endmacro()

macro(texture_merge TARGET_FILE FILELIST_BIG FILELIST_SMALL)
	file(WRITE ${TARGET_FILE} "")

	file(APPEND ${TARGET_FILE} "texturesbig = {\n")
	foreach(FILEENTRY ${FILELIST_BIG})
		texture_file_write(${TARGET_FILE} ${FILEENTRY})
	endforeach()
	file(APPEND ${TARGET_FILE} "}\n")

	file(APPEND ${TARGET_FILE} "texturessmall = {\n")
	foreach(FILEENTRY ${FILELIST_SMALL})
		texture_file_write(${TARGET_FILE} ${FILEENTRY})
	endforeach()
	file(APPEND ${TARGET_FILE} "}\n")
endmacro()

macro(textures PROJECTNAME)
	file(GLOB FILELIST_BIG ${ROOT_DIR}/contrib/assets/png-packed/${PROJECTNAME}-*big.lua)
	file(GLOB FILELIST_SMALL ${ROOT_DIR}/contrib/assets/png-packed/${PROJECTNAME}-*small.lua)
	message(STATUS "build complete.lua: ${ROOT_DIR}/contrib/assets/png-packed/${PROJECTNAME}")
	texture_merge(${PROJECT_BINARY_DIR}/complete.lua.in "${FILELIST_BIG}" "${FILELIST_SMALL}")
	configure_file(${PROJECT_BINARY_DIR}/complete.lua.in ${ROOT_DIR}/${GAME_BASE_DIR}/${PROJECTNAME}/textures/complete.lua COPYONLY)
endmacro()

macro(texturepacker)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS PROJECTNAME)
	set(_MULTI_VALUE_ARGS FILELIST)

	cmake_parse_arguments(_TP "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	if (NOT _TP_PROJECTNAME)
		message(FATAL_ERROR "texturepacker requires the PROJECTNAME argument")
	endif()
	if (NOT _TP_FILELIST)
		message(FATAL_ERROR "texturepacker requires the FILELIST argument")
	endif()

	if (TEXTUREPACKER_BIN)
		set(DEPENDENCIES)
		foreach(FILEENTRY ${_TP_FILELIST})
			string(REPLACE "{n}" "0" FILEENTRY_N "${FILEENTRY}")
			set(PNGBIG ${ROOT_DIR}/${GAME_BASE_DIR}/${_TP_PROJECTNAME}/pics/${FILEENTRY_N}-big.png)
			set(PNGSMALL ${ROOT_DIR}/${GAME_BASE_DIR}/${_TP_PROJECTNAME}/pics/${FILEENTRY_N}-small.png)
			string(REPLACE "-{n}" "" FILEENTRY_CLEAN "${FILEENTRY}")
			set(TPS ${ROOT_DIR}/contrib/assets/png-packed/${FILEENTRY_CLEAN}.tps)
			add_custom_command(OUTPUT ${PNGBIG} ${PNGSMALL} COMMAND ${TEXTUREPACKER_BIN} ARGS ${TPS} DEPENDS ${TPS} VERBATIM)
			list(APPEND DEPENDENCIES ${PNGBIG} ${PNGSMALL})
		endforeach()
		if (DEPENDENCIES)
			add_custom_target(${_TP_PROJECTNAME}_texturepacker DEPENDS ${DEPENDENCIES})
			add_dependencies(${_TP_PROJECTNAME} ${_TP_PROJECTNAME}_texturepacker)
		endif()
		set(DEPENDENCIES)
	endif()
endmacro()

macro(check_lua_files TARGET FILES)
	find_host_program(LUAC_EXECUTABLE NAMES ${DEFAULT_LUAC_EXECUTABLE})
	if (LUAC_EXECUTABLE)
		message(STATUS "${LUAC_EXECUTABLE} found")
		foreach(_FILE ${FILES})
			string(REPLACE "/" "-" TARGETNAME ${_FILE})
			add_custom_target(
				${TARGET}-${TARGETNAME}
				COMMENT "checking lua file ${_FILE}"
				COMMAND ${LUAC_EXECUTABLE} -p ${_FILE}
				WORKING_DIRECTORY ${ROOT_DIR}/${GAME_BASE_DIR}/${TARGET}
			)
			add_dependencies(${TARGET} ${TARGET}-${TARGETNAME})
		endforeach()
	else()
		message(STATUS "No lua compiler (${DEFAULT_LUAC_EXECUTABLE}) found")
	endif()
endmacro()

#
# Install android packages
#
# parameters:
# PACKAGE The package id that you need to install
#
macro(cp_android_package PACKAGE)
	message(STATUS "install android sdk package ${PACKAGE}")
	file(WRITE ${CMAKE_BINARY_DIR}/yes.txt "y")
	execute_process(
		COMMAND ${ANDROID_SDK_TOOL} list target -c
		OUTPUT_VARIABLE TARGETS_LIST
	)
	if (${TARGETS_LIST} MATCHES ${PACKAGE})
		message(STATUS "${PACKAGE} is already installed")
	else()
		execute_process(
			COMMAND ${ANDROID_SDK_TOOL} update sdk -a -u -s -t ${PACKAGE}
			INPUT_FILE ${CMAKE_BINARY_DIR}/yes.txt
		)
	endif()
endmacro()

#
# Prepare android workspace with assets and sdk/ndk commands.
#
# Also adds some helper targets:
# * android-PROJECTNAME-uninstall uninstalls the application
# * android-PROJECTNAME-install installs the application
# * android-PROJECTNAME-start starts the application
# * android-PROJECTNAME-backtrace creates a backtrace from a crash
#
# parameters:
# PROJECTNAME: the project name in lower case letters - used for e.g. resolving the java classes and icons
#              if this is a test project for a specific game, it should start with 'tests_' - as we then just
#              reuse some of the game settings and assets for the tests.
# APPNAME: the normal app name, must not contain whitespaces, but can contain upper case letters.
# VERSION: the version code, e.g. 1.0
# VERSION_CODE: the android version code needed for google play store
#
macro(cp_android_prepare PROJECTNAME APPNAME VERSION VERSION_CODE)
	# TODO: add java activity classes to dependencies to recompile target on java class changes
	message(STATUS "prepare java code for ${PROJECTNAME}")
	file(COPY ${ANDROID_ROOT} DESTINATION ${CMAKE_BINARY_DIR}/android-${PROJECTNAME})
	if (HD_VERSION)
		set(PACKAGENAME ${PROJECTNAME}hd)
	else()
		set(PACKAGENAME ${PROJECTNAME})
	endif()
	set(APPCLASS ${APPNAME})
	if (${PACKAGENAME} MATCHES "tests")
		set(PACKAGENAME "tests")
		set(APPCLASS "TestsApp")
	endif()
	set(WHITELIST ${GAME_BASE_DIR} libsdl ${PACKAGENAME})
	set(ANDROID_BIN_ROOT ${CMAKE_BINARY_DIR}/android-${PROJECTNAME})
	get_subdirs(SUBDIRS ${ANDROID_BIN_ROOT}/src/org)
	list(REMOVE_ITEM SUBDIRS ${WHITELIST})
	foreach(DIR ${SUBDIRS})
		file(REMOVE_RECURSE ${ANDROID_BIN_ROOT}/src/org/${DIR})
	endforeach()
	configure_file(${ANDROID_BIN_ROOT}/AndroidManifest.xml.in ${ANDROID_BIN_ROOT}/AndroidManifest.xml @ONLY)
	configure_file(${ANDROID_BIN_ROOT}/strings.xml.in ${ANDROID_BIN_ROOT}/res/values/strings.xml @ONLY)
	configure_file(${ANDROID_BIN_ROOT}/default.properties.in ${ANDROID_BIN_ROOT}/default.properties @ONLY)
	configure_file(${ANDROID_BIN_ROOT}/project.in ${ANDROID_BIN_ROOT}/.project @ONLY)
	configure_file(${ANDROID_BIN_ROOT}/project.properties.in ${ANDROID_BIN_ROOT}/project.properties @ONLY)
	configure_file(${ANDROID_BIN_ROOT}/google-play-services_lib/project.properties.in ${ANDROID_BIN_ROOT}/google-play-services_lib/project.properties @ONLY)
	configure_file(${ANDROID_BIN_ROOT}/build.gradle.in ${ANDROID_BIN_ROOT}/build.gradle @ONLY)
	add_custom_target(android-${PROJECTNAME}-backtrace ${ANDROID_ADB} logcat | ${ANDROID_NDK_STACK} -sym ${ANDROID_BIN_ROOT}/obj/local/${CMAKE_ANDROID_ARCH_ABI} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	add_custom_target(android-${PROJECTNAME}-install ${ANDROID_ANT} ${ANT_INSTALL_TARGET} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	add_custom_target(android-${PROJECTNAME}-uninstall ${ANDROID_ANT} uninstall WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	set(APP_PACKAGENAME "org.${PACKAGENAME}")
	add_custom_target(android-${PROJECTNAME}-start ${ANDROID_ADB} shell am start -n ${APP_PACKAGENAME}/${APP_PACKAGENAME}.${APPCLASS} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	string(REPLACE "tests_" "" CLEAN_PROJECTNAME "${PROJECTNAME}")
	if (EXISTS ${ROOT_DIR}/contrib/installer/android/${CLEAN_PROJECTNAME}/)
		file(COPY ${ROOT_DIR}/contrib/installer/android/${CLEAN_PROJECTNAME}/ DESTINATION ${ANDROID_BIN_ROOT})
	endif()
	message(STATUS "copy files from ${ROOT_DIR}/${GAME_BASE_DIR}/${CLEAN_PROJECTNAME} to ${ANDROID_BIN_ROOT}/assets/${GAME_BASE_DIR}/${CLEAN_PROJECTNAME}")
	file(COPY ${ROOT_DIR}/${GAME_BASE_DIR}/${CLEAN_PROJECTNAME} DESTINATION ${ANDROID_BIN_ROOT}/assets/${GAME_BASE_DIR})
	#install(DIRECTORY ${ROOT_DIR}/${GAME_BASE_DIR}/${CLEAN_PROJECTNAME} DESTINATION ${ANDROID_BIN_ROOT}/assets/${GAME_BASE_DIR})
	set(RESOLUTIONS hdpi ldpi mdpi xhdpi)
	set(ICON "${PACKAGENAME}-icon.png")
	if (NOT EXISTS ${ROOT_DIR}/contrib/${ICON})
		set(ICON "${CLEAN_PROJECTNAME}-icon.png")
	endif()
	if (EXISTS ${ROOT_DIR}/contrib/${ICON})
		foreach(RES ${RESOLUTIONS})
			file(MAKE_DIRECTORY ${ANDROID_BIN_ROOT}/res/drawable-${RES})
			configure_file(${ROOT_DIR}/contrib/${ICON} ${ANDROID_BIN_ROOT}/res/drawable-${RES}/icon.png COPYONLY)
		endforeach()
	endif()
	set(ANDROID_SO_OUTDIR ${ANDROID_BIN_ROOT}/libs/${CMAKE_ANDROID_ARCH_ABI})
	set_target_properties(${PROJECTNAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${ANDROID_SO_OUTDIR})
	set_target_properties(${PROJECTNAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${ANDROID_SO_OUTDIR})
	set_target_properties(${PROJECTNAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${ANDROID_SO_OUTDIR})
	if (NOT EXISTS ${ANDROID_BIN_ROOT}/local.properties)
		message(STATUS "=> create Android SDK project: ${PROJECTNAME}")
		execute_process(COMMAND ${ANDROID_SDK_TOOL} --silent update project
				--path .
				${ANDROID_TOOL_FLAGS}
				--name ${APPNAME}
				--target ${ANDROID_API}
				WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
		execute_process(COMMAND ${ANDROID_SDK_TOOL} --silent update lib-project
				${ANDROID_TOOL_FLAGS}
				--path google-play-services_lib
				--target ${ANDROID_API}
				WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	endif()
	if (RELEASE)
		set(APK_NAME ${APPNAME}-${ANT_TARGET}-unsigned.apk)
	else()
		set(APK_NAME ${APPNAME}-${ANT_TARGET}.apk)
	endif()
	set(TMP_APK_NAME ${APPNAME}-${ANT_TARGET}-tmp.apk)
	set(FINAL_APK_NAME ${APPNAME}-${ANT_TARGET}.apk)

	add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E rename ${ANDROID_SO_OUTDIR}/lib${PROJECTNAME}.so ${ANDROID_SO_OUTDIR}/lib${PACKAGENAME}.so)
	message(STATUS "Android settings: keystore=${ANDROID_KEYSTORE} apkname=${APK_NAME} alias=${ANDROID_ALIAS} (apk /${TMP_APK_NAME})")
	add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${ANDROID_ANT} ${ANT_FLAGS} ${ANT_TARGET} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E rename ${ANDROID_BIN_ROOT}/bin/${APK_NAME} ${ANDROID_BIN_ROOT}/bin/${TMP_APK_NAME})
	if (RELEASE)
		add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${JARSIGNER} -sigalg SHA1withRSA -digestalg SHA1 -keypass ${ANDROID_KEYPASS} -storepass ${ANDROID_STOREPASS} -keystore ${ANDROID_KEYSTORE} ${TMP_APK_NAME} ${ANDROID_ALIAS} WORKING_DIRECTORY ${ANDROID_BIN_ROOT}/bin/)
		add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${JARSIGNER} -verify -certs ${TMP_APK_NAME} WORKING_DIRECTORY ${ANDROID_BIN_ROOT}/bin/)
	endif()
	add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${ANDROID_ZIPALIGN} -f ${ZIPALIGN_FLAGS} 4 ${TMP_APK_NAME} ${FINAL_APK_NAME} WORKING_DIRECTORY ${ANDROID_BIN_ROOT}/bin/)
	#add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${ANDROID_ANT} uninstall ${ANT_INSTALL_TARGET} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
	#add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${ANDROID_ADB} shell am start -n org.${PROJECTNAME}/org.${PROJECTNAME}.${APPNAME} WORKING_DIRECTORY ${ANDROID_BIN_ROOT})
endmacro()

#
# put a variable into the global namespace
#
macro(var_global VARIABLES)
	foreach(VAR ${VARIABLES})
		cp_message("${VAR} => ${${VAR}}")
		set(${VAR} ${${VAR}} CACHE STRING "" FORCE)
		mark_as_advanced(${VAR})
	endforeach()
endmacro()

macro(package_global LIB)
	find_package(${LIB})
endmacro()

#
# Add external dependency. It will trigger a find_package and use the system wide install if found, otherwise the bundled version
# If you set USE_BUILTIN the system wide is ignored.
#
# parameters:
# LIB:
# CFLAGS:
# LINKERFLAGS:
# SRCS: the list of source files for the bundled lib
# DEFINES: a list of defines (without -D or /D)
#
macro(cp_add_library)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS LIB PACKAGE CFLAGS LINKERFLAGS)
	set(_MULTI_VALUE_ARGS SRCS DEFINES)

	cmake_parse_arguments(_ADDLIB "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	if (NOT _ADDLIB_LIB)
		message(FATAL_ERROR "cp_add_library requires the LIB argument")
	endif()
	if (NOT _ADDLIB_SRCS)
		message(FATAL_ERROR "cp_add_library requires the SRCS argument")
	endif()

	if (_ADDLIB_PACKAGE)
		set(FIND_PACKAGE_NAME ${_ADDLIB_PACKAGE})
	else()
		set(FIND_PACKAGE_NAME ${_ADDLIB_LIB})
	endif()

	string(TOUPPER ${_ADDLIB_LIB} PREFIX)
	find_package(${FIND_PACKAGE_NAME})
	find_package(${FIND_PACKAGE_NAME})

	string(TOUPPER ${_ADDLIB_LIB} PREFIX)
	string(TOUPPER ${FIND_PACKAGE_NAME} PKG_PREFIX)
	if (NOT ${PREFIX} STREQUAL ${PKG_PREFIX})
		if (${PKG_PREFIX}_INCLUDE_DIRS)
			set(${PREFIX}_INCLUDE_DIRS ${${PKG_PREFIX}_INCLUDE_DIRS})
		elseif(${PKG_PREFIX}_INCLUDE_DIR)
			set(${PREFIX}_INCLUDE_DIRS ${${PKG_PREFIX}_INCLUDE_DIR})
		else()
			set(${PREFIX}_INCLUDE_DIRS ${${PKG_PREFIX}_INCLUDEDIR})
		endif()
		if (NOT ${PREFIX}_LIBRARIES AND ${PKG_PREFIX}_LIBRARIES)
			set(${PREFIX}_LIBRARIES ${${PKG_PREFIX}_LIBRARIES})
		elseif(NOT ${PREFIX}_LIBRARIES AND ${PKG_PREFIX}_LIBRARY)
			set(${PREFIX}_LIBRARIES ${${PKG_PREFIX}_LIBRARY})
		else()
			message(WARN "Could not find libs for ${PREFIX}")
		endif()
		set(${PREFIX}_FOUND ${${PKG_PREFIX}_FOUND})
		message(STATUS "Found ${PKG_PREFIX} => ${${PREFIX}_FOUND}")
	endif()
	var_global(${PREFIX}_INCLUDE_DIRS ${PREFIX}_LIBRARIES ${PREFIX}_FOUND)
	if (NOT ${PREFIX}_FOUND)
		message(STATUS "No system wide installation found, use built-in version of ${_ADDLIB_LIB}")
		set(${PREFIX}_LIBRARIES ${_ADDLIB_LIB})
		set(${PREFIX}_LINKERFLAGS ${_ADDLIB_LINKERFLAGS})
		#set(${PREFIX}_COMPILERFLAGS "${_ADDLIB_DEFINES} ${_ADDLIB_CFLAGS}")
		set(${PREFIX}_DEFINITIONS ${_ADDLIB_DEFINES})
		set(${PREFIX}_INCLUDE_DIRS ${ROOT_DIR}/src/libs/${_ADDLIB_LIB})
		add_library(${_ADDLIB_LIB} STATIC ${_ADDLIB_SRCS})
		add_library(${PKG_PREFIX}::${PKG_PREFIX} ALIAS ${_ADDLIB_LIB})
		target_compile_options(${_ADDLIB_LIB} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-undef>)
		target_include_directories(${_ADDLIB_LIB} PUBLIC ${${PREFIX}_INCLUDE_DIRS})
		set_target_properties(${_ADDLIB_LIB} PROPERTIES COMPILE_DEFINITIONS "${${PREFIX}_DEFINITIONS}")
		if (NOT CP_MSVC)
			set_target_properties(${_ADDLIB_LIB} PROPERTIES COMPILE_FLAGS "${_ADDLIB_CFLAGS}")
		endif()
		set_target_properties(${_ADDLIB_LIB} PROPERTIES FOLDER ${_ADDLIB_LIB})
		#var_global(${PREFIX}_COMPILERFLAGS)
		var_global(${PREFIX}_LINKERFLAGS)
		var_global(${PREFIX}_DEFINITIONS)
	endif()
	# mark this library as an external dependency.
	set(${PREFIX}_EXTERNAL ON)
	var_global(${PREFIX}_EXTERNAL)
endmacro()

#
# macro for the FindLibName.cmake files. If USE_BUILTIN is set we don't search for system wide installs at all.
#
# parameters:
# LIB: the library we are trying to find
# HEADER: the header we are trying to find
# SUFFIX: suffix for the include dir
# VERSION: the operator and version that is given to the pkg-config call (e.g. ">=1.0")
#
# Example: cp_find(SDL2_image SDL_image.h SDL2 FALSE)
#
macro(cp_find LIB HEADER SUFFIX VERSION)
	string(TOUPPER ${LIB} PREFIX)
	if (DEFINED ${PREFIX}_FOUND)
		return()
	endif()
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_PROCESSOR_ARCH "x64")
	else()
		set(_PROCESSOR_ARCH "x86")
	endif()
	set(_SEARCH_PATHS
		~/Library/Frameworks
		/Library/Frameworks
		/usr/local
		/usr
		/sw # Fink
		/opt/local # DarwinPorts
		/opt/csw # Blastwave
		/opt
		/usr/local/opt
		/usr/local/opt/${LIB}
		$ENV{VCPKG_ROOT}/installed/${_PROCESSOR_ARCH}-windows
		C:/Tools/vcpkg/installed/${_PROCESSOR_ARCH}-windows
		C:/vcpkg/installed/${_PROCESSOR_ARCH}-windows
	)
	find_package(PkgConfig QUIET)
	if (PKG_CONFIG_FOUND)
		pkg_check_modules(_${PREFIX} "${LIB}${VERSION}")
	endif()
	if (NOT ${PREFIX}_FOUND)
		find_path(${PREFIX}_INCLUDE_DIRS
			NAMES ${HEADER}
			HINTS ENV ${PREFIX}DIR
			PATH_SUFFIXES include include/${SUFFIX} ${SUFFIX}
			PATHS
				${_${PREFIX}_INCLUDE_DIRS}
				${_SEARCH_PATHS}
		)
		find_library(${PREFIX}_LIBRARIES
			NAMES ${LIB} ${PREFIX} ${_${PREFIX}_LIBRARIES}
			HINTS ENV ${PREFIX}DIR
			PATH_SUFFIXES lib64 lib lib/${_PROCESSOR_ARCH}
			PATHS
				${_${PREFIX}_LIBRARY_DIRS}
				${_SEARCH_PATHS}
		)
	else()
		set(${PREFIX}_INCLUDE_DIRS ${_${PREFIX}_INCLUDE_DIRS})
		list(APPEND ${PREFIX}_INCLUDE_DIRS ${_${PREFIX}_INCLUDEDIR})
		set(${PREFIX}_LIBRARIES ${_${PREFIX}_LIBRARIES})
		list(APPEND ${PREFIX}_LIBRARIES ${_${PREFIX}_LIBRARY})
	endif()
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(${LIB} FOUND_VAR ${PREFIX}_FOUND REQUIRED_VARS ${PREFIX}_INCLUDE_DIRS ${PREFIX}_LIBRARIES)
	mark_as_advanced(${PREFIX}_INCLUDE_DIRS ${PREFIX}_LIBRARIES ${PREFIX}_FOUND)
	if (${PREFIX}_FOUND)
		add_library(${LIB} INTERFACE)
		if (${PREFIX}_INCLUDE_DIRS)
			target_include_directories(${LIB} INTERFACE ${${PREFIX}_INCLUDE_DIRS})
			message(STATUS "${LIB}: ${PREFIX}_INCLUDE_DIRS: ${${PREFIX}_INCLUDE_DIRS}")
		endif()
		if (${PREFIX}_LIBRARIES)
			target_link_libraries(${LIB} INTERFACE ${${PREFIX}_LIBRARIES})
			message(STATUS "${LIB}: ${PREFIX}_LIBRARIES: ${${PREFIX}_LIBRARIES}")
		endif()
	endif()
	unset(PREFIX)
	unset(_SEARCH_PATHS)
	unset(_PROCESSOR_ARCH)
endmacro()

macro(cp_find_header_only LIB HEADER SUFFIX VERSION)
	string(TOUPPER ${LIB} PREFIX)
	if (DEFINED ${PREFIX}_FOUND)
		return()
	endif()
	set(_SEARCH_PATHS
		~/Library/Frameworks
		/Library/Frameworks
		/usr/local
		/usr/local/opt
		/usr/local/opt/${LIB}
		/usr
		/sw # Fink
		/opt/local # DarwinPorts
		/opt/csw # Blastwave
		/opt
	)
	find_package(PkgConfig QUIET)
	if (PKG_CONFIG_FOUND)
		pkg_check_modules(_${PREFIX} "${LIB}${VERSION}")
	endif()
	find_path(${PREFIX}_INCLUDE_DIRS
		NAMES ${HEADER}
		HINTS ENV ${PREFIX}DIR
		PATH_SUFFIXES include include/${SUFFIX} ${SUFFIX}
		PATHS
			${_${PREFIX}_INCLUDE_DIRS}
			${_SEARCH_PATHS}
	)
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(${LIB} FOUND_VAR ${PREFIX}_FOUND REQUIRED_VARS ${PREFIX}_INCLUDE_DIRS)
	mark_as_advanced(${PREFIX}_INCLUDE_DIRS ${PREFIX}_FOUND)
	if (${PREFIX}_FOUND)
		add_library(${LIB} INTERFACE)
		if (${PREFIX}_INCLUDE_DIRS)
			target_include_directories(${LIB} INTERFACE ${${PREFIX}_INCLUDE_DIRS})
			message(STATUS "${LIB}: ${PREFIX}_INCLUDE_DIRS: ${${PREFIX}_INCLUDE_DIRS}")
		endif()
		if (${PREFIX}_LIBRARIES)
			target_link_libraries(${LIB} INTERFACE ${${PREFIX}_LIBRARIES})
			message(STATUS "${LIB}: ${PREFIX}_LIBRARIES: ${${PREFIX}_LIBRARIES}")
		endif()
	endif()
	unset(PREFIX)
	unset(_SEARCH_PATHS)
endmacro()

macro(cp_recurse_resolve_dependencies LIB DEPS)
	list(APPEND ${DEPS} ${LIB})
	get_property(_DEPS GLOBAL PROPERTY ${LIB}_DEPS)
	foreach(DEP ${_DEPS})
		#cp_message("=> resolved dependency ${DEP} for ${LIB}")
		cp_recurse_resolve_dependencies(${DEP} ${DEPS})
	endforeach()
endmacro()

macro(cp_resolve_dependencies LIB DEPS)
	get_property(_DEPS GLOBAL PROPERTY ${LIB}_DEPS)
	list(APPEND ${DEPS} ${LIB})
	foreach(DEP ${_DEPS})
		#cp_message("=> resolved dependency ${DEP} for ${LIB}")
		cp_recurse_resolve_dependencies(${DEP} ${DEPS})
	endforeach()
endmacro()

macro(cp_unique INPUTSTR OUT)
	string(REPLACE " " ";" TMP "${INPUTSTR}")
	list(REVERSE TMP)
	list(REMOVE_DUPLICATES TMP)
	list(REVERSE TMP)
	string(REPLACE ";" " " ${OUT} "${TMP}")
endmacro()

macro(cp_target_link_libraries)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS TARGET)
	set(_MULTI_VALUE_ARGS LIBS)

	cmake_parse_arguments(_LINKLIBS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	target_link_libraries(${_LINKLIBS_TARGET} PUBLIC ${_LINKLIBS_LIBS})
endmacro()

macro(cp_set_properties TARGET VARNAME VALUE)
	set(${VARNAME} "${VALUE}")
	var_global(${VARNAME})
	set_target_properties(${TARGET} PROPERTIES ${VARNAME} "${VALUE}")
endmacro()

function(cp_add_debugger TARGET)
	if (${DEBUGGER} MATCHES "gdb")
		add_custom_target(${TARGET}-debug)
		add_custom_command(TARGET ${TARGET}-debug
			POST_BUILD
			COMMAND ${GDB_EXECUTABLE} -ex run --args $<TARGET_FILE:${TARGET}>
			COMMENT "Starting debugger session for ${TARGET}"
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${TARGET}
		)
	elseif (${DEBUGGER} MATCHES "lldb")
		add_custom_target(${TARGET}-debug)
		add_custom_command(TARGET ${TARGET}-debug
			POST_BUILD
			COMMAND CG_CONTEXT_SHOW_BACKTRACE=1 ${LLDB_EXECUTABLE} -b -o run $<TARGET_FILE:${TARGET}>
			COMMENT "Starting debugger session for ${TARGET}"
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${TARGET}
		)
	endif()
endfunction()

#
# set up the binary for the application. This will also set up platform specific stuff for you
#
# Example: cp_add_executable(TARGET SomeTargetName SRCS Source.cpp Main.cpp WINDOWED APPNAME "Some App Name" VERSION 1.0 VERSION_CODE 1)
#
macro(cp_add_executable)
	set(_OPTIONS_ARGS WINDOWED)
	set(_ONE_VALUE_ARGS TARGET VERSION VERSION_CODE APPNAME CATEGORY)
	set(_MULTI_VALUE_ARGS SRCS TPS)

	cmake_parse_arguments(_EXE "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	# tests have different install behaviour
	if (${_EXE_TARGET} MATCHES "tests")
		set(TESTS TRUE)
	else()
		set(TESTS FALSE)
	endif()
	string(REGEX REPLACE "tests_" "" BASEDIR ${_EXE_TARGET})
	set(TARGET_ASSETS "")
	set(RESOURCE_DIR ${ROOT_DIR}/${GAME_BASE_DIR}/${BASEDIR})
	file(GLOB CHILDREN RELATIVE ${ROOT_DIR}/${GAME_BASE_DIR}/ ${ROOT_DIR}/${GAME_BASE_DIR}/${BASEDIR}/*)
	foreach (CHILD ${CHILDREN})
		# TODO: broken
		if (IS_DIRECTORY "${ROOT_DIR}/${GAME_BASE_DIR}/${CHILD}")
			cp_message("Asset dir: ${ROOT_DIR}/${GAME_BASE_DIR}/${CHILD}")
			file(GLOB RESOURCE_DIR_FILES RELATIVE "${ROOT_DIR}" "${RESOURCE_DIR}/${CHILD}/[A-Za-z]*.*")
			set_source_files_properties(${RESOURCE_DIR_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${RESOURCE_DIR_NAME}/${CHILD})
			list(APPEND TARGET_ASSETS ${RESOURCE_DIR_FILES})
		endif()
	endforeach()

	set(ABOUT_FILE ${ROOT_DIR}/docs/${_EXE_TARGET}/ABOUT.en)
	if (EXISTS ${ABOUT_FILE})
		file(READ ${ABOUT_FILE} DESCRIPTION_RAW)
		string(REPLACE "\n" "\\n" DESCRIPTION ${DESCRIPTION_RAW})
	endif()
	set(KEYWORDS_FILE ${ROOT_DIR}/docs/${_EXE_TARGET}/KEYWORDS)
	if (EXISTS ${KEYWORDS_FILE})
		file(READ ${KEYWORDS_FILE} KEYWORDS)
	endif()

	set(GUI_IDENTIFIER org.${_EXE_TARGET})
	set(APPNAME ${_EXE_APPNAME})
	set(APP ${_EXE_TARGET})
	set(VERSION ${_EXE_VERSION})
	if ("${_EXE_CATEGORY}" STREQUAL "")
		set(APPCATEGORIES "Game;")
	else()
		set(APPCATEGORIES "Game;${_EXE_CATEGORY};")
	endif()
	set(VERSION_CODE ${_EXE_VERSION_CODE})
	if (VERBOSE)
		message(STATUS "Prepare build with settings:")
		message(STATUS "- APPNAME:        '${APPNAME}'")
		message(STATUS "- VERSION:        '${VERSION}'")
		message(STATUS "- VERSION_CODE:   '${VERSION_CODE}'")
		message(STATUS "- GUI_IDENTIFIER: '${GUI_IDENTIFIER}'")
	endif()

	set(CPACK_COMPONENT_${_EXE_TARGET}_DISPLAY_NAME ${APPNAME})

	# by default, put system related files into the current binary dir on install
	set(SHARE_DIR ".")
	# by default, put data files into the current binary dir on install
	set(GAMES_DIR "${_EXE_TARGET}")
	# by default, put the binary into a subdir with the target name
	set(BIN_DIR "${_EXE_TARGET}")
	set(ICON_DIR ".")

	create_dir_header(${_EXE_TARGET})
	if (ANDROID)
		add_library(${_EXE_TARGET} SHARED ${_EXE_SRCS} ${TARGET_ASSETS})
		if (_EXE_TPS)
			texturepacker(PROJECTNAME ${_EXE_TARGET} FILELIST ${_EXE_TPS})
			textures(${_EXE_TARGET})
		endif()
		cp_android_prepare(${_EXE_TARGET} ${APPNAME} ${VERSION} ${VERSION_CODE})
		set_target_properties(${_EXE_TARGET} PROPERTIES LINK_FLAGS "-Wl,--undefined=Java_org_base_BaseActivity_onPaymentDone,--undefined=Java_org_base_BaseActivity_isDebug,--undefined=Java_org_base_BaseActivity_isTrackingOptOut,--undefined=Java_org_base_BaseActivity_isHD,--undefined=Java_org_base_BaseActivity_onPersisterConnectFailed,--undefined=Java_org_base_BaseActivity_onPersisterConnectSuccess,--undefined=Java_org_base_BaseActivity_onPersisterDisconnect")
	else()
		if (LINUX AND NOT TESTS AND NOT STEAMLINK)
			set(SHARE_DIR "share")
			set(GAMES_DIR "${SHARE_DIR}/${_EXE_TARGET}")
			set(ICON_DIR "${SHARE_DIR}/icons")
			set(BIN_DIR "games")
			configure_file(${ROOT_DIR}/contrib/installer/linux/editor.in ${PROJECT_BINARY_DIR}/${_EXE_TARGET}-editor)
			configure_file(${ROOT_DIR}/contrib/installer/linux/desktop.in ${PROJECT_BINARY_DIR}/${_EXE_TARGET}.desktop)
			configure_file(${ROOT_DIR}/contrib/installer/linux/snapcraft.yaml.in ${PROJECT_BINARY_DIR}/${_EXE_TARGET}.snapcraft.yaml)
			configure_file(${ROOT_DIR}/contrib/installer/linux/appdata.xml.in ${PROJECT_BINARY_DIR}/${_EXE_TARGET}.appdata.xml)
			install(FILES ${PROJECT_BINARY_DIR}/${_EXE_TARGET}-editor DESTINATION ${BIN_DIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
			install(FILES ${PROJECT_BINARY_DIR}/${_EXE_TARGET}.desktop DESTINATION ${SHARE_DIR}/applications)
			install(FILES ${PROJECT_BINARY_DIR}/${_EXE_TARGET}.appdata.xml DESTINATION ${SHARE_DIR}/metainfo)
			install(FILES ${PROJECT_BINARY_DIR}/${_EXE_TARGET}.snapcraft.yaml DESTINATION ${SHARE_DIR}/applications)
			set(MAN_PAGE ${ROOT_DIR}/contrib/installer/linux/${_EXE_TARGET}.6)
			if (EXISTS ${MAN_PAGE})
				install(FILES ${MAN_PAGE} DESTINATION ${SHARE_DIR}/man/man6)
			endif()
		endif()

		if (_EXE_WINDOWED)
			if (WINDOWS)
				set(GAMEEXPLORER_XML ${PROJECT_BINARY_DIR}/gameexplorer.xml)
				configure_file(${ROOT_DIR}/contrib/installer/windows/gameexplorer.xml.in ${GAMEEXPLORER_XML})
				configure_file(${ROOT_DIR}/src/project.rc.in ${PROJECT_BINARY_DIR}/project.rc)
				list(APPEND _EXE_SRCS ${PROJECT_BINARY_DIR}/project.rc)
				add_executable(${_EXE_TARGET} WIN32 ${_EXE_SRCS} ${TARGET_ASSETS})
				if (_EXE_TPS)
					texturepacker(PROJECTNAME ${_EXE_TARGET} FILELIST ${_EXE_TPS})
					textures(${_EXE_TARGET})
				endif()
				if (MSVC)
					set_target_properties(${_EXE_TARGET} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
				endif()
			elseif (DARWIN OR IOS)
				list(APPEND _EXE_SRCS ${PROJECT_BINARY_DIR}/Info.plist)
				add_executable(${_EXE_TARGET} ${_EXE_SRCS} ${TARGET_ASSETS})
				if (_EXE_TPS)
					texturepacker(PROJECTNAME ${_EXE_TARGET} FILELIST ${_EXE_TPS})
					textures(${_EXE_TARGET})
				endif()
				if (IOS)
					cp_ios_add_target_properties(${_EXE_TARGET} ${APPNAME} ${VERSION} ${VERSION_CODE})
					cp_ios_prepare_content(${_EXE_TARGET} ON)
				else()
					cp_osx_add_target_properties(${_EXE_TARGET} ${APPNAME} ${VERSION} ${VERSION_CODE})
					cp_osx_prepare_content(${_EXE_TARGET} ON)
				endif()
				#set_target_properties(${_EXE_TARGET} PROPERTIES RESOURCE ${ROOT_DIR}/${GAME_BASE_DIR})
			else()
				add_executable(${_EXE_TARGET} ${_EXE_SRCS} ${TARGET_ASSETS})
				if (_EXE_TPS)
					texturepacker(PROJECTNAME ${_EXE_TARGET} FILELIST ${_EXE_TPS})
					textures(${_EXE_TARGET})
				endif()
			endif()
		else()
			add_executable(${_EXE_TARGET} ${_EXE_SRCS} ${TARGET_ASSETS})
			if (_EXE_TPS)
				texturepacker(PROJECTNAME ${_EXE_TARGET} FILELIST ${_EXE_TPS})
				textures(${_EXE_TARGET})
			endif()
			if (IOS)
				cp_ios_add_target_properties(${_EXE_TARGET} ${APPNAME} ${VERSION} ${VERSION_CODE})
				cp_ios_prepare_content(${_EXE_TARGET} OFF)
			elseif (DARWIN)
				cp_osx_add_target_properties(${_EXE_TARGET} ${APPNAME} ${VERSION} ${VERSION_CODE})
				cp_osx_prepare_content(${_EXE_TARGET} OFF)
			elseif (WINDOWS)
				if (MSVC)
					set_target_properties(${_EXE_TARGET} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
				endif()
			endif()
		endif()
	endif()

	if (IOS AND RELEASE)
		set(RESULTS "")
		cp_ios_get_provisioning_profiles("Martin Gerhardy" "${IOS_PROVISIONG_PROFILES_DIR}" RESULTS)
		if (NOT RESULTS)
			if (RELEASE)
				cp_message("Could not find matching provisioning profile in ${IOS_PROVISIONG_PROFILES_DIR}")
			else()
				cp_message("Could not find matching provisioning profile in ${IOS_PROVISIONG_PROFILES_DIR}")
			endif()
		else()
			message(STATUS "Found provisioning profile hash ${RESULTS}")
		endif()
	elseif (NACL)
		set_target_properties(${_EXE_TARGET} PROPERTIES PROFILING_POSTFIX .pexe)
		set_target_properties(${_EXE_TARGET} PROPERTIES RELEASE_POSTFIX .pexe)
		set_target_properties(${_EXE_TARGET} PROPERTIES DEBUG_POSTFIX .pexe)
	elseif (EMSCRIPTEN)
		em_link_js_library(${_EXE_TARGET} ${ROOT_DIR}/contrib/installer/html5/library.js)
		set_target_properties(${_EXE_TARGET} PROPERTIES PROFILING_POSTFIX .html)
		set_target_properties(${_EXE_TARGET} PROPERTIES RELEASE_POSTFIX .html)
		set_target_properties(${_EXE_TARGET} PROPERTIES DEBUG_POSTFIX .html)
		configure_file(${ROOT_DIR}/contrib/installer/html5/shell.html.in ${CMAKE_CURRENT_BINARY_DIR}/shell.html @ONLY)
		set_target_properties(${_EXE_TARGET} PROPERTIES LINK_FLAGS "--preload-file ${ROOT_DIR}/${GAME_BASE_DIR}/${_EXE_TARGET}@/ --shell-file ${CMAKE_CURRENT_BINARY_DIR}/shell.html")
	endif()

	if (STEAMLINK)
		set(SHARE_DIR "${_EXE_TARGET}")
		set(GAMES_DIR "${_EXE_TARGET}")
		set(BIN_DIR "${_EXE_TARGET}")
		set(ICON_DIR "${_EXE_TARGET}")
		if (NOT TESTS)
			configure_file(${ROOT_DIR}/contrib/installer/steamlink/toc.txt.in ${PROJECT_BINARY_DIR}/toc.txt @ONLY)
			install(FILES ${PROJECT_BINARY_DIR}/toc.txt DESTINATION ${SHARE_DIR} COMPONENT ${_EXE_TARGET})
		endif()
	endif()

	if (NOT TESTS)
		if (IOS)
			set(SHARE_DIR "${_EXE_TARGET}.app")
			set(GAMES_DIR "${_EXE_TARGET}.app")
			set(ICON_DIR "${_EXE_TARGET}.app")
			set(BIN_DIR ".")
			install(FILES ${PROJECT_BINARY_DIR}/Info.plist DESTINATION ${SHARE_DIR} COMPONENT ${_EXE_TARGET})
			install(FILES ${ROOT_DIR}/contrib/installer/ios/PKgInfo DESTINATION ${SHARE_DIR} COMPONENT ${_EXE_TARGET})
		elseif (DARWIN)
			set(SHARE_DIR "${_EXE_TARGET}.app/Contents")
			set(GAMES_DIR "${_EXE_TARGET}.app/Contents/Resources")
			set(ICON_DIR "${_EXE_TARGET}.app/Contents/Resources")
			set(BIN_DIR "${_EXE_TARGET}.app/Contents/MacOS")
			install(FILES ${PROJECT_BINARY_DIR}/Info.plist DESTINATION ${SHARE_DIR} COMPONENT ${_EXE_TARGET})
			install(FILES ${ROOT_DIR}/contrib/installer/osx/PKgInfo DESTINATION ${SHARE_DIR} COMPONENT ${_EXE_TARGET})
		else()
			set(ICON "${_EXE_TARGET}-icon.png")
			if (EXISTS ${ROOT_DIR}/contrib/${ICON})
				install(FILES ${ROOT_DIR}/contrib/${ICON} DESTINATION ${ICON_DIR} COMPONENT ${_EXE_TARGET})
			endif()
		endif()
	endif()

	if (LINUX AND NOT TESTS)
		set(LANGUAGES en_GB de_DE)
		foreach(LANG ${LANGUAGES})
			add_custom_command(TARGET ${_EXE_TARGET} POST_BUILD
				COMMAND ${ROOT_DIR}/contrib/scripts/lang.sh
				ARGS ${LANG} ${_EXE_TARGET}
				COMMENT "Update language ${LANG} for ${_EXE_TARGET}"
			)
		endforeach()
	endif()

	set_target_properties(${_EXE_TARGET} PROPERTIES FOLDER ${_EXE_TARGET})
	# install relative to /usr/<APPNAME>
	if (NOT TESTS)
		if (PKGDATADIR)
			install(DIRECTORY ${RESOURCE_DIR} DESTINATION ${PKGDATADIR} COMPONENT ${_EXE_TARGET})
		else()
			install(DIRECTORY ${RESOURCE_DIR} DESTINATION ${GAMES_DIR}/${GAME_BASE_DIR} COMPONENT ${_EXE_TARGET})
		endif()
		install(TARGETS ${_EXE_TARGET} DESTINATION ${BIN_DIR} COMPONENT ${_EXE_TARGET})
	endif()
	configure_file(${ROOT_DIR}/src/game.h.in ${PROJECT_BINARY_DIR}/game.h)
	include_directories(${PROJECT_BINARY_DIR})

	if (MSVC)
		set_target_properties(${_EXE_TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
	endif()

	add_custom_target(${_EXE_TARGET}-run
		COMMAND $<TARGET_FILE:${_EXE_TARGET}>
		USES_TERMINAL
		DEPENDS ${_EXE_TARGET}
		WORKING_DIRECTORY "${ROOT_DIR}"
	)
	cp_add_debugger(${_EXE_TARGET})
endmacro()

#
# parameters:
# The code to check must be the first argument
# CXX|C: Defines which compiler should be used
# FLAGS: The compiler flags to use
# VAR: The cmake variable the result is exported into
# INCLUDE_DIRS: a list of include dirs that should be used
#
# Example: cp_check_source_compiles(CXX FLAGS "--some-compiler-flag --yet-another-one" VAR HAVE_COMPILE_FLAGS "int main(int argc, char *argv[]) {return 0;}")
#
macro(cp_check_source_compiles CODE)
	set(_OPTIONS_ARGS CXX C)
	set(_ONE_VALUE_ARGS FLAGS VAR)
	set(_MULTI_VALUE_ARGS INCLUDE_DIRS)

	cmake_parse_arguments(_COMPILE "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	if (NOT _COMPILE_VAR)
		message(FATAL_ERROR "cp_check_source_compiles requires the VAR argument")
	endif()
	if (NOT _COMPILE_C AND NOT _COMPILE_CXX)
		message(FATAL_ERROR "cp_check_source_compiles requires either CXX or C to be specified")
	endif()

	set(_TMP_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
	set(_TMP_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})

	if (_COMPILE_INCLUDE_DIRS)
		set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${_COMPILE_INCLUDE_DIRS}")
	endif()
	set(CMAKE_REQUIRED_FLAGS ${_COMPILE_FLAGS})
	cp_message("Check that this code compiles and links:\n${CODE}")
	if (_COMPILE_C)
		check_c_source_compiles("${CODE}" "${_COMPILE_VAR}")
	elseif (_COMPILE_CXX)
		check_cxx_source_compiles("${CODE}" "${_COMPILE_VAR}")
	endif()
	set(CMAKE_REQUIRED_FLAGS ${_TMP_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_INCLUDES ${_TMP_REQUIRED_INCLUDES})
endmacro()
