find_program(NDK_BUILD ndk-build)

if(NOT NDK_BUILD)
    message(FATAL_ERROR "ndk build not found, bye")
endif()

find_program(SDK_ANDROID android)

if(NOT SDK_ANDROID)
    message(FATAL_ERROR "sdk android tool not found, bye")
endif()

if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
	set(yabause_android_SHADOW
		AndroidManifest.xml
		project.properties
		assets/gpl-license
		assets/yabause_data.zip
		jni/yui.c
		jni/miniegl.h
		jni/sndaudiotrack.c
        jni/sndaudiotrack.h
		jni/sndopensl.c
		jni/sndopensl.h
		src/org/yabause/android/Config.java
		src/org/yabause/android/DataDownloader.java
		src/org/yabause/android/FileChooserActivity.java
		src/org/yabause/android/GamePad.java
		src/org/yabause/android/Globals.java
		src/org/yabause/android/IFileChooser.java
		src/org/yabause/android/IScancodeListener.java
		src/org/yabause/android/MainActivity.java
		src/org/yabause/android/MenuActivity.java
		src/org/yabause/android/MenuOption.java
		src/org/yabause/android/MenuSettingsInputConfigureButtonsActivity.java
		src/org/yabause/android/OptionArrayAdapter.java
		src/org/yabause/android/ScancodeDialog.java
		src/org/yabause/android/Yabause.java
		src/org/yabause/android/YabauseView.java
		res/drawable/hireslogo.png
		res/drawable/publisherlogo.png
		res/drawable-hdpi/icon.png
		res/drawable-hdpi/status.png
		res/drawable-ldpi/icon.png
		res/drawable-ldpi/status.png
		res/drawable-mdpi/icon.png
		res/drawable-mdpi/status.png
		res/layout/main.xml
		res/layout/menu_option.xml
		res/layout/scancode_dialog.xml
		res/menu/emulation.xml
		res/values/strings.xml
	)

	foreach(item IN LISTS yabause_android_SHADOW)
		message(STATUS ${item})
		add_custom_command(
			OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${item}"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/${item}"
			DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${item}"
		)
	endforeach()
endif()

set(YABAUSE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/jni/Android.mk.in
	${CMAKE_CURRENT_BINARY_DIR}/jni/Android.mk
	@ONLY
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/local.properties"
	COMMAND ${SDK_ANDROID} update project -p "${CMAKE_CURRENT_BINARY_DIR}"
	DEPENDS
		"${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml"
		"${CMAKE_CURRENT_BINARY_DIR}/project.properties"
		"${CMAKE_CURRENT_BINARY_DIR}/jni/Android.mk"
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/jni/libyabause.a"
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../libyabause.a ${CMAKE_CURRENT_BINARY_DIR}/jni/libyabause.a
	DEPENDS yabause "${CMAKE_CURRENT_BINARY_DIR}/../config.h"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/local.properties"
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libs/armeabi/libyabause.so"
	COMMAND "${NDK_BUILD}"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/jni/libyabause.a"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/jni/yui.c"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/jni/miniegl.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/jni/sndaudiotrack.c"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/jni/sndaudiotrack.h"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/jni/sndopensl.c"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/jni/sndopensl.h"
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set(yabause_android_RES
	"${CMAKE_CURRENT_BINARY_DIR}/assets/gpl-license"
	"${CMAKE_CURRENT_BINARY_DIR}/assets/yabause_data.zip"
	"${CMAKE_CURRENT_BINARY_DIR}/res/drawable/hireslogo.png"
	"${CMAKE_CURRENT_BINARY_DIR}/res/drawable/publisherlogo.png"
	"${CMAKE_CURRENT_BINARY_DIR}/res/drawable-hdpi/icon.png"
	"${CMAKE_CURRENT_BINARY_DIR}/res/drawable-hdpi/status.png"
	"${CMAKE_CURRENT_BINARY_DIR}/res/drawable-ldpi/icon.png"
	"${CMAKE_CURRENT_BINARY_DIR}/res/drawable-ldpi/status.png"
	"${CMAKE_CURRENT_BINARY_DIR}/res/drawable-mdpi/icon.png"
	"${CMAKE_CURRENT_BINARY_DIR}/res/drawable-mdpi/status.png"
	"${CMAKE_CURRENT_BINARY_DIR}/res/layout/main.xml"
	"${CMAKE_CURRENT_BINARY_DIR}/res/layout/menu_option.xml"
	"${CMAKE_CURRENT_BINARY_DIR}/res/layout/scancode_dialog.xml"
	"${CMAKE_CURRENT_BINARY_DIR}/res/menu/emulation.xml"
	"${CMAKE_CURRENT_BINARY_DIR}/res/values/strings.xml"
)
set(yabause_android_SRC
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/DataDownloader.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/Config.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/FileChooserActivity.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/GamePad.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/Globals.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/IFileChooser.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/IScancodeListener.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/MainActivity.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/MenuActivity.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/MenuOption.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/MenuSettingsInputConfigureButtonsActivity.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/OptionArrayAdapter.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/ScancodeDialog.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/Yabause.java"
    "${CMAKE_CURRENT_BINARY_DIR}/src/org/yabause/android/YabauseView.java"
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/bin/Yabause-debug.apk"
	COMMAND "ant" ARGS "debug"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libs/armeabi/libyabause.so"
	DEPENDS ${yabause_android_SRC}
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml"
	DEPENDS ${yabause_android_RES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/bin/Yabause-release-unsigned.apk"
	COMMAND "ant" ARGS "release"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libs/armeabi/libyabause.so"
	DEPENDS ${yabause_android_SRC}
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml"
	DEPENDS ${yabause_android_RES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_target(yabause-android ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/bin/Yabause-release-unsigned.apk")
else()
    add_custom_target(yabause-android ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/bin/Yabause-debug.apk")
endif()
