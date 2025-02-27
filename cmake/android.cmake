set(ANDROID_ROOT ${ROOT_DIR}/android-project/)

set(ANDROID_SDK_ROOT $ENV{ANDROID_SDK_ROOT})
set(ANDROID_SDK_BUILD_TOOLS_VERSION "29.0.3")
set(ANDROID_PLATFORM_TOOLS_ROOT ${ANDROID_SDK_ROOT}/platform-tools)
set(ANDROID_EXTRAS_ROOT ${ANDROID_SDK_ROOT}/extras)
set(ANDROID_GOOGLE_PLAY_SERVICES_ROOT ${ANDROID_EXTRAS_ROOT}/google/google_play_services)
set(ANDROID_BUILD_TOOLS_ROOT ${ANDROID_SDK_ROOT}/build-tools/${ANDROID_SDK_BUILD_TOOLS_VERSION})
set(ANDROID_API "android-28" CACHE STRING "Android platform version")

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
	set(ANDROID_NDK_EXE_EXT ".exe")
	set(ANDROID_SDK_TOOL_EXT ".bat")
endif()

if (VERBOSE)
	set(ANT_FLAGS -v)
	set(ZIPALIGN_FLAGS -v)
	set(KEYTOOL_FLAGS -v)
	set(ANDROID_TOOL_FLAGS "${ANDROID_TOOL_FLAGS} -v")
endif()
if (GRADLE)
	set(ANDROID_TOOL_FLAGS "${ANDROID_TOOL_FLAGS} -g")
endif()

set(ANDROID_SDK_TOOL "${ANDROID_SDK_ROOT}/tools/bin/sdkmanager${ANDROID_SDK_TOOL_EXT}")

if (NOT CMAKE_ANDROID_NDK)
	message(FATAL_ERROR "ANDROID_NDK environment variable is not set")
endif()
if (NOT ANDROID_SDK_ROOT)
	message(FATAL_ERROR "ANDROID_SDK environment variable is not set")
endif()
message(STATUS "Android NDK root: ${CMAKE_ANDROID_NDK}")
message(STATUS "Android SDK root: ${ANDROID_SDK_ROOT}")

if (RELEASE)
	set(ANT_TARGET release)
	set(ANT_INSTALL_TARGET installr)
	set(MANIFEST_DEBUGGABLE false)
else()
	set(ANT_TARGET debug)
	set(ANT_INSTALL_TARGET installd)
	set(MANIFEST_DEBUGGABLE true)
endif()

if (GRADLE)
	find_host_program(ANDROID_GRADLE gradle)
	if (ANDROID_GRADLE)
		message(STATUS "gradle found")
	else()
		message(FATAL_ERROR "gradle not found in path!")
	endif()
endif()

find_host_program(JARSIGNER "jarsigner")
find_host_program(ANDROID_NDK_STACK "ndk-stack" HINTS ${CMAKE_ANDROID_NDK})
find_host_program(ANDROID_ZIPALIGN "zipalign" HINTS ${ANDROID_SDK_ROOT}/tools)
if (ANDROID_ZIPALIGN)
	message(STATUS "zipalign tool found")
else()
	set(ANDROID_ZIPALIGN ${ANDROID_BUILD_TOOLS_ROOT}/zipalign)
	message(STATUS "could not find zipalign in path - use ${ANDROID_ZIPALIGN}")
endif()

option(HD_VERSION "Build the HD versions of the games" OFF)
set(TOOLS OFF)
set(USE_BUILTIN ON)

find_host_program(ANDROID_ADB adb PATHS ${ANDROID_SDK_ROOT}/platform-tools/)
if (ANDROID_ADB)
	message(STATUS "adb tool found")
else()
	message(FATAL_ERROR "adb tool not found in path!")
endif()

set(DEBUG_KEYSTORES "$ENV{HOME}/.android/debug.keystore" "$ENV{HOMEPATH}/.android/debug.keystore")
foreach(DEBUG_KEYSTORE ${DEBUG_KEYSTORES})
	if (EXISTS ${DEBUG_KEYSTORE})
		set(ANDROID_DEBUG_KEYSTORE ${DEBUG_KEYSTORE})
	endif()
endforeach()

set(ANDROID_DEBUG_ALIAS androiddebugkey)
set(ANDROID_DEBUG_STOREPASS android)
set(ANDROID_DEBUG_KEYPASS android)
if ("${ANDROID_DEBUG_KEYSTORE}" STREQUAL "")
	find_host_program(KEYTOOL keytool)
	if (KEYTOOL)
		message(STATUS "keytool found (${KEYTOOL})- generate debug keystore at $ENV{HOME}/.android with storepass ${ANDROID_DEBUG_STOREPASS}, alias ${ANDROID_DEBUG_ALIAS}, keypass ${ANDROID_DEBUG_KEYPASS}")
		file(MAKE_DIRECTORY $ENV{HOME}/.android)
		execute_process(COMMAND ${KEYTOOL} -genkeypair ${KEYTOOL_FLAGS} -keyalg RSA -keystore $ENV{HOME}/.android/debug.keystore -storepass ${ANDROID_DEBUG_STOREPASS} -alias ${ANDROID_DEBUG_ALIAS} -keypass ${ANDROID_DEBUG_KEYPASS} -dname "CN=Android Debug,O=Android,C=US" RESULT_VARIABLE exitcode)
		execute_process(COMMAND ${KEYTOOL} --help)
		message(STATUS "keytool exited with ${exitcode}")
		set(DEBUG_KEYSTORE "$ENV{HOME}/.android/debug.keystore")
		if (NOT EXISTS ${DEBUG_KEYSTORE})
			message(FATAL_ERROR "Could not create debug keystore")
		else()
			set(ANDROID_DEBUG_KEYSTORE ${DEBUG_KEYSTORE})
		endif()
	else()
		message(FATAL_ERROR "keytool not found in path! no debug keystore found")
	endif()
else()
	message(STATUS "Found android debug keystore at ${ANDROID_DEBUG_KEYSTORE}")
endif()

set(ANDROID_RELEASE_KEYSTORE $ENV{CAVEPRODUCTIONS_KEYSTORE_PATH})
set(ANDROID_RELEASE_ALIAS CaveProductions)
set(ANDROID_RELEASE_STOREPASS $ENV{CAVEPRODUCTIONS_KEYSTORE_PASSWD})
set(ANDROID_RELEASE_KEYPASS $ENV{CAVEPRODUCTIONS_ALIAS_PASSWD})

if (RELEASE)
	if (NOT DEFINED ENV{CAVEPRODUCTIONS_KEYSTORE_PASSWD})
		message(FATAL_ERROR "No keystore password set - export CAVEPRODUCTIONS_KEYSTORE_PASSWD env var")
	endif()
	if (NOT DEFINED ENV{CAVEPRODUCTIONS_KEYSTORE_PATH})
		message(FATAL_ERROR "No keystore path set - export CAVEPRODUCTIONS_KEYSTORE_PATH env var")
	endif()
	set(ANDROID_KEYSTORE ${ANDROID_RELEASE_KEYSTORE})
	set(ANDROID_ALIAS ${ANDROID_RELEASE_ALIAS})
	set(ANDROID_STOREPASS ${ANDROID_RELEASE_STOREPASS})
	set(ANDROID_KEYPASS ${ANDROID_RELEASE_KEYPASS})
else()
	set(ANDROID_KEYSTORE ${ANDROID_DEBUG_KEYSTORE})
	set(ANDROID_ALIAS ${ANDROID_DEBUG_ALIAS})
	set(ANDROID_STOREPASS ${ANDROID_DEBUG_STOREPASS})
	set(ANDROID_KEYPASS ${ANDROID_DEBUG_KEYPASS})
endif()

add_definitions(-DGL_GLEXT_PROTOTYPES)

set(CMAKE_C_STANDARD_LIBRARIES "-ldl -landroid -llog -lm -lz -lc -lgcc")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES}")
