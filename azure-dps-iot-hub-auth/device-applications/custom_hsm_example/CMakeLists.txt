#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
    endif()
else()
#    set (CMAKE_C_STANDARD 99)
    set (CMAKE_C_FLAGS "--std=gnu99 ${CMAKE_C_FLAGS}")
    set (CMAKE_CXX_STANDARD 11)
endif()

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(HSM_INCLUDE_DIR "../../adapters")

set(HSM_CLIENT_H_FILES
    ../../adapters/hsm_client_data.h)
#set(HSM_CLIENT_C_FILES
#    ../../adapters/hsm_client_data.c)

include_directories(./inc ${HSM_INCLUDE_DIR})

set(source_files
    ./src/x509_impl.c
    ./src/custom_hsm_example.c
)

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ELSE()
    # Make warning as error
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic -Wno-variadic-macros -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -Wno-variadic-macros -fPIC")
ENDIF(WIN32)

option(compileOption_C "text" OFF)
option(compileOption_CXX "text" OFF)

if(NOT "${compileOption_C}" STREQUAL "OFF")
    set(CMAKE_C_FLAGS "${compileOption_C} ${CMAKE_C_FLAGS}")
endif()

if(NOT "${compileOption_CXX}" STREQUAL "OFF")
    set(CMAKE_CXX_FLAGS "${compileOption_CXX} ${CMAKE_CXX_FLAGS}")
endif()

MESSAGE(STATUS "CMAKE_C_FLAGS: " ${CMAKE_C_FLAGS})

#add_library(custom_hsm_example ${source_files} ${HSM_CLIENT_C_FILES} ${HSM_CLIENT_H_FILES})
add_library(custom_hsm_example ${source_files} ${HSM_CLIENT_H_FILES})
