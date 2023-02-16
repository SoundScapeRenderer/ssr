# run e.g.: mkdir -p _build && cd _build && cmake -DCMAKE_BUILD_TYPE=RELEASE .. && make -j3 && cd ..
# Specify the minimum version for CMake
message(STATUS "CMAKE_VERSION: " ${CMAKE_VERSION})
cmake_minimum_required(VERSION 3.8)

# Project's name
project(SSR)

# The version number
set(SSR_VERSION 0.6.0-cmake)

# Show more of make
set(CMAKE_VERBOSE_MAKEFILE ON)

# shared or static build
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
message(STATUS "Shared libs: " ${BUILD_SHARED_LIBS})

# Set the output folder where your program will be created
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/_build)
if(WIN32)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/ssr-win-bin)
else(WIN32)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src)
endif(WIN32)

# Show the compiler
message(STATUS "Comiler: " ${CMAKE_CXX_COMPILER})
message(STATUS "Compiler ID: " ${CMAKE_CXX_COMPILER_ID})

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The following folders will be included
include_directories(${PROJECT_SOURCE_DIR}/src)

# Include APF
set(APF_PATH ${PROJECT_SOURCE_DIR}/apf)
include_directories(${APF_PATH})

# Include GML
set(GML_PATH ${PROJECT_SOURCE_DIR}/gml/include)
include_directories(${GML_PATH})

# Files
set(SOURCE_FILES
    src/configuration.cpp
    src/ssr_global.cpp
    src/xmlparser.cpp
    src/legacy_directionalpoint.cpp
    src/legacy_orientation.cpp
    src/legacy_position.cpp
    src/legacy_scene.cpp
    )

# Libraries
# Add libraries to LINKLIBS
set(LINKLIBS)

#set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# TODO
find_package(PkgConfig REQUIRED)

if(WIN32)
  # Set Path to extern headers and libraries for Windows builds
  set(EXTERN_PATH ${PROJECT_SOURCE_DIR}/extern)
  
  #set(CMAKE_PREFIX_PATH ${EXTERN_PATH})
  #message(STATUS "WINDOWS Prefix: " ${CMAKE_PREFIX_PATH})

  # Include extern headers
  include_directories(${EXTERN_PATH}/include)
  message(STATUS "Included: " ${EXTERN_PATH}/include)

  # here is the target environment located
  set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} ${EXTERN_PATH})
endif(WIN32)

if(WIN32)
  # Use custom static libxml2 build for MSVC
  #if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  #  find_library(LibXml2 NAMES libxml2_msvc)
  #endif()
  if(NOT ${BUILD_SHARED_LIBS})
    add_definitions(-DLIBXML_STATIC)
  endif()
  find_package(LibXml2 REQUIRED)
  include_directories(${LIBXML2_INCLUDE_DIR})
  list(APPEND LINKLIBS ${LIBXML2_LIBRARY})
  #list(APPEND LINKLIBS "-lxml2 -lz -llzma -lws2_32")
  list(APPEND LINKLIBS "-llzma -lz")
  message(STATUS "Found XML2: " ${LIBXML2_LIBRARY})
  message(STATUS "Found XML2 headers: " ${LIBXML2_INCLUDE_DIR})
else(WIN32)
  find_package(LibXml2 REQUIRED)
  include_directories(${LIBXML2_INCLUDE_DIR})
  list(APPEND LINKLIBS ${LIBXML2_LIBRARY})
endif(WIN32)

find_library(LIBSNDFILE_LIBRARY NAMES sndfile libsndfile-1)
list(APPEND LINKLIBS ${LIBSNDFILE_LIBRARY})
list(APPEND LINKLIBS -lFLAC -lopus -lvorbisenc -lvorbis -logg)
message(STATUS "Found SNDFILE: " ${LIBSNDFILE_LIBRARY})

option(ENABLE_ECASOUND "Ecasound soundfile playback/recording" OFF)
if(ENABLE_ECASOUND)
  list(APPEND SOURCE_FILES
    src/audioplayer.cpp
    src/audiorecorder.cpp
    )
  find_path(ECASOUND_INCLUDE_DIR NAMES ecasoundc.h HINTS /usr/include/libecasoundc)
  include_directories(${ECASOUND_INCLUDE_DIR})
  find_library(ECASOUND_LIBRARY NAMES ecasoundc)
  list(APPEND LINKLIBS ${ECASOUND_LIBRARY})
  message(STATUS "Found ECASOUND: " ${ECASOUND_LIBRARY})
else(ENABLE_ECASOUND)
  message(WARNING "ECASOUND disabled.")
endif(ENABLE_ECASOUND)

find_library(JACK_LIBRARY NAMES jack libjack64)
list(APPEND LINKLIBS ${JACK_LIBRARY})
message(STATUS "Found JACK :" ${JACK_LIBRARY})

find_library(FFTW_LIBRARY NAMES fftw3 libfftw3-3)
list(APPEND LINKLIBS ${FFTW_LIBRARY})
message(STATUS "Found FFTW: " ${FFTW_LIBRARY})

find_library(FFTWF_LIBRARY NAMES fftw3f libfftw3f-3)
list(APPEND LINKLIBS ${FFTWF_LIBRARY})
message(STATUS "Found FFTWF: " ${FFTWF_LIBRARY})

# GUI
option(GUI "Build GUI" OFF)
if(GUI)
  # Find includes in corresponding build directories
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
  # Turn on automatic invocation of the MOC
  set(CMAKE_AUTOMOC ON)

  include_directories(${PROJECT_SOURCE_DIR}/src/gui)  # necessary?
  aux_source_directory(${PROJECT_SOURCE_DIR}/src/gui GUI_SOURCE_FILES) 
  list(APPEND SOURCE_FILES ${GUI_SOURCE_FILES})

  option(ENABLE_GUI "graphical user interface (using Qt)" ON)
  option(ENABLE_FLOATING_CONTROL_PANEL "separate control window" OFF)


  find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)
  message(STATUS "Found QT5: " ${Qt5_DIR})

  if(WIN32)
    if(NOT ${BUILD_SHARED_LIBS})
      option(STATIC_QT "Use static Qt5 libraries (Windows)" ON)
    endif()
    if(STATIC_QT)
      add_definitions(-DQT_STATICPLUGIN)
    endif(STATIC_QT)

    # Platform support libs for Windows
    set(QT5PLATFORMSUPPORT_LIBS
        Qt5::QWindowsIntegrationPlugin
        )

    # Qt5PlatformSupport modules
    find_library(Qt5PlatformCompositorSupportLIBRARY Qt5PlatformCompositorSupport)
    list(APPEND QT5PLATFORMSUPPORT_LIBS ${Qt5PlatformCompositorSupportLIBRARY})
    find_library(Qt5ThemeSupport_LIBRARY Qt5ThemeSupport)
    list(APPEND QT5PLATFORMSUPPORT_LIBS ${Qt5ThemeSupport_LIBRARY})
    find_library(Qt5FontDatabaseSupport_LIBRARY Qt5FontDatabaseSupport)
    list(APPEND QT5PLATFORMSUPPORT_LIBS ${Qt5FontDatabaseSupport_LIBRARY})
    find_library(Qt5EventDispatcherSupport_LIBRARY Qt5EventDispatcherSupport)
    list(APPEND QT5PLATFORMSUPPORT_LIBS ${Qt5EventDispatcherSupport_LIBRARY})
    find_library(Qt5AccessibilitySupport_LIBRARY Qt5AccessibilitySupport)
    list(APPEND QT5PLATFORMSUPPORT_LIBS ${Qt5AccessibilitySupport_LIBRARY})
    find_library(Qt5WindowsUIAutomationSupport_LIBRARY Qt5WindowsUIAutomationSupport)
    list(APPEND QT5PLATFORMSUPPORT_LIBS ${Qt5WindowsUIAutomationSupport_LIBRARY})

    find_package(Freetype REQUIRED)
    list(APPEND QT5PLATFORMSUPPORT_LIBS ${FREETYPE_LIBRARIES})

    #list(APPEND QT5PLATFORMSUPPORT_LIBS "-lwtsapi32")

    message(STATUS "Found Qt5PlatformSupport: " ${QT5PLATFORMSUPPORT_LIBS})
  endif(WIN32)

  # OpenGL
  find_package(OpenGL REQUIRED)
  include_directories(${OPENGL_INCLUDE_DIR})
  list(APPEND LINKLIBS ${OPENGL_gl_LIBRARY})
  list(APPEND LINKLIBS ${OPENGL_glu_LIBRARY})

  message(STATUS "Build GUI")

else(GUI)
  message(WARNING "GUI disabled. Switch on with 'cmake .. -DGUI=ON'")
endif(GUI)

# Network
option(ENABLE_WEBSOCKET_INTERFACE "network websocket interface" OFF)
if(ENABLE_WEBSOCKET_INTERFACE)
  add_definitions(-DASIO_STANDALONE)
  set(ASIO_PATH ${PROJECT_SOURCE_DIR}/extern/include/asio/asio/include/)
  include_directories(${ASIO_PATH})

  add_definitions(-D_WEBSOCKETPP_CPP11_THREAD_)  # https://github.com/zaphoyd/websocketpp/issues/980
  include_directories(${EXTERN_PATH}/include/websocketpp)
  set(RAPIDJSON_PATH ${PROJECT_SOURCE_DIR}/rapidjson)
  include_directories(${RAPIDJSON_PATH}/include/)

  set(WEBSOCKET_PATH ${PROJECT_SOURCE_DIR}/src/websocket/)
  include_directories(${WEBSOCKET_PATH})
  message(STATUS "Enabled WEBSOCKET_INTERFACE interface")
else(ENABLE_WEBSOCKET_INTERFACE)
  message(WARNING "WEBSOCKET_INTERFACE disabled.")
endif(ENABLE_WEBSOCKET_INTERFACE)

option(ENABLE_LEGACY_NETWORK_INTERFACE "network xml interface (legacy)" OFF)
if(ENABLE_LEGACY_NETWORK_INTERFACE)
  add_definitions(-DASIO_STANDALONE)
  set(ASIO_PATH ${PROJECT_SOURCE_DIR}/extern/include/asio/asio/include/)
  include_directories(${ASIO_PATH})

  aux_source_directory(${PROJECT_SOURCE_DIR}/src/legacy_network NETWORK_SOURCE_FILES) 
  list(APPEND SOURCE_FILES ${NETWORK_SOURCE_FILES})

  include_directories(${PROJECT_SOURCE_DIR}/src/legacy_network)
  if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)  # min Windows 7
    list(APPEND LINKLIBS "-lws2_32 -lmswsock")
  endif(WIN32)
  message(STATUS "Enabled LEGACY_NETWORK interface")
else(ENABLE_LEGACY_NETWORK_INTERFACE)
  message(WARNING "LEGACY_NETWORK_INTERFACE disabled.")
endif(ENABLE_LEGACY_NETWORK_INTERFACE)

# Head tracker
option(HEAD_TRACKER "Basic head tracker support" OFF)
if(HEAD_TRACKER)
  list(APPEND SOURCE_FILES
    src/trackerrazor.cpp
    src/razor-ahrs/RazorAHRS.cpp
    src/trackerpolhemus.cpp
    )
  set(ENABLE_RAZOR ON)
  set(ENABLE_POLHEMUS ON)
  if(WIN32)
    add_definitions(-DENABLE_O_NDELAY_WORKAROUND)  # bits/fcntl2.h
  endif(WIN32)
  message(STATUS "Basic head tracker support")
else(HEAD_TRACKER)
  message(WARNING "HEAD_TRACKER disabled.")
endif(HEAD_TRACKER)


# Preprocessor symbols
add_definitions(-DSSR_DATA_DIR="./data")
add_definitions(-DAPF_JACK_POLICY_DEBUG)
add_definitions(-DAPF_JACKCLIENT_DEBUG)

# Config definitions
# configure a header file to pass some of the CMake settings
# to the source code
add_definitions(-DHAVE_CONFIG_H)
configure_file(
  "${PROJECT_SOURCE_DIR}/generate_config.h.in"
  "${PROJECT_SOURCE_DIR}/src/config.h"
  )


# Compiler flags
# Add a few for release builds (MAKE_BUILD_TYPE=RELEASE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -fomit-frame-pointer -ffast-math -funroll-loops -march=x86-64 -mmmx -msse -msse2 -mfpmath=sse")
message(STATUS "Release Flags: " ${CMAKE_CXX_FLAGS_RELEASE})
# And for debug builds (MAKE_BUILD_TYPE=DEBUG)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wa,-mbig-obj")  # -Weffc++ 
message(STATUS "Debug Flags: " ${CMAKE_CXX_FLAGS_DEBUG})

# Compile executables
set(EXECUTABLES
    ssr-binaural
#    ssr-brs
#    ssr-vbap
#    ssr-wfs
#    ssr-aap
#    ssr-dca
#    ssr-generic
    )
add_executable(ssr-binaural src/ssr_binaural.cpp ${SOURCE_FILES})
#add_executable(ssr-brs src/ssr_brs.cpp ${SOURCE_FILES})
#add_executable(ssr-vbap src/ssr_vbap.cpp ${SOURCE_FILES})
#add_executable(ssr-wfs src/ssr_wfs.cpp ${SOURCE_FILES})
#add_executable(ssr-aap src/ssr_aap.cpp ${SOURCE_FILES})
#add_executable(ssr-dca src/ssr_dca.cpp ${SOURCE_FILES})
#add_executable(ssr-generic src/ssr_generic.cpp ${SOURCE_FILES})


# Link
if(NOT ${BUILD_SHARED_LIBS})
  set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
endif()

message(STATUS "Linking: "  ${LINKLIBS} ${QT5PLATFORMSUPPORT_LIBS})

foreach(executable ${EXECUTABLES})
  target_link_libraries(${executable} Threads::Threads)
  target_link_libraries(${executable} ${LINKLIBS})
  if(GUI)
    target_link_libraries(${executable} Qt5::Gui Qt5::Core Qt5::Widgets Qt5::OpenGL ${QT5PLATFORMSUPPORT_LIBS})
  endif(GUI)
endforeach(executable ${EXECUTABLES})


# ../../mxe/tools/copydlldeps.sh --infile ./ssr-binaural.exe --destdir ./deps --recursivesrcdir ../../mxe/usr/x86_64-w64-mingw32.shared/ --srcdir ./ --copy --enforcedir ../../mxe/usr/x86_64-w64-mingw32.shared/qt5/plugins/platforms/
