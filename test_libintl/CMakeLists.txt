cmake_minimum_required(VERSION 2.8.12)
Project(test_libintl)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "You must set your binary directory different from your source")
endif()

# default to Release build
if (NOT CMAKE_BUILD_TYPE)
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Sets the configuration to build (Release, Debug, etc...)")
endif()

### Add C++11 standard flag

set (CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

### Boilerplate
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(AssetUtilities)

### Find libs
find_package ( Boost REQUIRED )
find_package ( Gettext REQUIRED )
find_package ( Intl REQUIRED )

MESSAGE(STATUS "Boost INCLUDE DIR = " ${Boost_INCLUDE_DIRS})

SET (PO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/po)
SET (SPIRIT_PO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

### Options

OPTION(NOEXCEPT "Enable the SPIRIT_PO_NOEXCEPT flag" OFF)
if (NOEXCEPT)
  ADD_DEFINITIONS(-DSPIRIT_PO_NOEXCEPT)
endif()

OPTION(DEBUG "Enable the SPIRIT_PO_DEBUG flag" OFF)
if (DEBUG)
  ADD_DEFINITIONS(-DSPIRIT_PO_DEBUG)
endif()

### Make target

add_executable(${PROJECT_NAME} main.cpp)
include_directories (${PROJECT_NAME} SYSTEM ${Boost_INCLUDE_DIRS})
include_directories (${PROJECT_NAME} SYSTEM ${SPIRIT_PO_INCLUDE_DIR})
target_link_libraries (${PROJECT_NAME} ${INTL_LIBRARIES})

### Make po_path marker
CreateFolderMarkerFile(${PROJECT_NAME} po_path.txt ${PO_DIR})

### Set list of po files
file(GLOB ALL_PO ${PO_DIR}/*.po)
#MESSAGE(STATUS "ALL_PO= " ${ALL_PO})

### Make a directory for mo files

set(LOCALE "en")
# So that the test harness can find all the mo files, we put them all under
# the system locale folder, and arrange them as different text domains with
# names matching the po-file that generates them.
# My system locale is "en", if yours is different you must change it above.

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mo/${LOCALE}/LC_MESSAGES)

### Run msgfmt over the po directory
set(MO_FILES)
foreach(PO ${ALL_PO})
  get_filename_component(PO_STEM ${PO} NAME_WE)
  set(MO ${CMAKE_CURRENT_BINARY_DIR}/mo/${LOCALE}/LC_MESSAGES/${PO_STEM}.mo)
  set(MO_FILES ${MO_FILES} ${MO})
  add_custom_command(
    OUTPUT ${MO}
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} ${PO} -o ${MO} --use-fuzzy #if we don't use fuzzy then spirit-po will get confused for sure
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${PO})
endforeach()
add_custom_target(create-mo ALL DEPENDS ${MO_FILES})

