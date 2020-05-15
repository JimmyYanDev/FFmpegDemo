cmake_minimum_required(VERSION 3.4.1)

include_directories(src/main/cpp/includes)

file(GLOB native-lib-src *.cpp src/main/cpp/ src/main/cpp/**/)
add_library( # Sets the name of the library.
        native-lib

        SHARED

        ${native-lib-src})

set(jnilibs-path "${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}")
add_library( avcodec-57 SHARED IMPORTED)
set_target_properties( avcodec-57
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs-path}/libavcodec-57.so)

add_library( avdevice-57 SHARED IMPORTED)
set_target_properties( avdevice-57
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs-path}/libavdevice-57.so)

add_library( avfilter-6 SHARED IMPORTED)
set_target_properties( avfilter-6
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs-path}/libavfilter-6.so)

add_library( avformat-57 SHARED IMPORTED)
set_target_properties( avformat-57
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs-path}/libavformat-57.so)

add_library( avutil-55 SHARED IMPORTED)
set_target_properties( avutil-55
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs-path}/libavutil-55.so)

add_library( postproc-54 SHARED IMPORTED)
set_target_properties( postproc-54
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs-path}/libpostproc-54.so)

add_library( swresample-2 SHARED IMPORTED)
set_target_properties( swresample-2
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs-path}/libswresample-2.so)

add_library( swscale-4 SHARED IMPORTED)
set_target_properties( swscale-4
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs-path}/libswscale-4.so)

target_link_libraries( # Specifies the target library.
        native-lib

        avcodec-57
        avdevice-57
        avfilter-6
        avformat-57
        avutil-55
        postproc-54
        swresample-2
        swscale-4

        # Links the target library to the log library
        # included in the NDK.
        log )