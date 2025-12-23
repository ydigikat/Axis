# ------------------------------------------------------------------------------
#  Author: Jason Wilden
# ------------------------------------------------------------------------------
#  MIT License
#  Copyright (c) 2025 Jason Wilden
# 
#  Permission to use, copy, modify, and/or distribute this code for any purpose
#  with or without fee is hereby granted, provided the above copyright notice an
#  this permission notice appear in all copies.
# ------------------------------------------------------------------------------

# Env variables
set(HOME_DIR "$ENV{HOME}")


# Cross compiler setttings
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(TOOLS_PREFIX "arm-none-eabi-")
set(TOOLS_DIR "GNU-tools-for-STM32/bin/")

# .EXE suffix on windows
if(WIN32)
    set(EXE_SUFFIX ".exe")    
endif()

# Function to validate and setup tool environment variables
function(setup_tool_environment VAR_NAME EXPECTED_EXECUTABLE)
    set(options OPTIONAL)
    set(oneValueArgs MESSAGE_PREFIX WINDOWS_EXECUTABLE)
    set(multiValueArgs "")
    cmake_parse_arguments(SETUP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Set defaults
    if(NOT SETUP_MESSAGE_PREFIX)
        set(SETUP_MESSAGE_PREFIX "${VAR_NAME}")
    endif()
    if(NOT SETUP_WINDOWS_EXECUTABLE)
        set(SETUP_WINDOWS_EXECUTABLE "${EXPECTED_EXECUTABLE}.exe")
    endif()
    
    if(NOT DEFINED ENV{${VAR_NAME}})
        if(SETUP_OPTIONAL)
            message(STATUS "${SETUP_MESSAGE_PREFIX} not set - feature disabled")
            set(${VAR_NAME}_AVAILABLE FALSE PARENT_SCOPE)
            return()
        else()
            message(FATAL_ERROR
                "${VAR_NAME} is not set, this is used to locate tools for CMake and VS Code.\n"
                "Please set the environment variable ${VAR_NAME}"
            )
        endif()
    endif()
    
    # Normalize and setup path
    file(TO_CMAKE_PATH "$ENV{${VAR_NAME}}" TOOL_ROOT)
    if(NOT TOOL_ROOT MATCHES "/$")
        set(TOOL_ROOT "${TOOL_ROOT}/")
    endif()
    
    # Check executable exists
    if(WIN32)
        set(CHECK_EXECUTABLE "${TOOL_ROOT}${SETUP_WINDOWS_EXECUTABLE}")
    else()
        set(CHECK_EXECUTABLE "${TOOL_ROOT}${EXPECTED_EXECUTABLE}")
    endif()
    
    if(NOT EXISTS "${CHECK_EXECUTABLE}")
        message(FATAL_ERROR
            "Invalid ${SETUP_MESSAGE_PREFIX} installation at:\n"
            "  ${TOOL_ROOT}\n"
            "Expected ${CHECK_EXECUTABLE}"
        )
    endif()
    
    # Export results
    set(${VAR_NAME}_ROOT "${TOOL_ROOT}" PARENT_SCOPE)
    set(${VAR_NAME}_AVAILABLE TRUE PARENT_SCOPE)
    message(STATUS "${SETUP_MESSAGE_PREFIX} found at ${TOOL_ROOT}")
endfunction()


setup_tool_environment(STM32CUBECLT_PATH 
    "GNU-tools-for-STM32/bin/arm-none-eabi-gcc"
    MESSAGE_PREFIX "STM32CubeCLT tools")

# Setup optional JLink tools
setup_tool_environment(JLINK_PATH 
    "JLinkGDBServer" 
    WINDOWS_EXECUTABLE "JLinkGDBServerCL.exe"
    MESSAGE_PREFIX "JLink tools"
    OPTIONAL)

# Configure RTT based on JLink availability
if(JLINK_PATH_AVAILABLE)
    set(ENABLE_RTT ON)
    message(STATUS "RTT support enabled")
else()
    set(ENABLE_RTT OFF)
endif()

# Set individual tool name and paths
set(CMAKE_C_COMPILER ${STM32CUBECLT_PATH_ROOT}${TOOLS_DIR}${TOOLS_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${STM32CUBECLT_PATH_ROOT}${TOOLS_DIR}${TOOLS_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${STM32CUBECLT_PATH_ROOT}${TOOLS_DIR}${TOOLS_PREFIX}g++)
set(CMAKE_OBJCOPY ${STM32CUBECLT_PATH_ROOT}${TOOLS_DIR}${TOOLS_PREFIX}objcopy)
set(CMAKE_SIZE ${STM32CUBECLT_PATH_ROOT}${TOOLS_DIR}${TOOLS_PREFIX}size)



# General compiler/linker flags, these are not specific to the hardware being used.  We are however using the nano runtime
# library.
set(CMAKE_C_FLAGS "-fdata-sections -fstack-usage -ffunction-sections --specs=nano.specs -Wl,--gc-sections -u _printf_float")
set(CMAKE_ASM_FLAGS "-x assembler-with-cpp")
set(CMAKE_C_FLAGS_DEBUG "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE "-Ofast")

# We're cross-compiling, so disable CMake test executables
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

