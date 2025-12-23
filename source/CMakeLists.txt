# ------------------------------------------------------------------------------
#  MIT License
#  Copyright (c) 2025 Jason Wilden
# 
#  Permission to use, copy, modify, and/or distribute this code for any purpose
#  with or without fee is hereby granted, provided the above copyright notice an
#  this permission notice appear in all copies.
# ------------------------------------------------------------------------------
set(TARGET app.elf)

# STM32F411 development board type
set(BOARD discovery)
if(NOT DEFINED BOARD)
  message(FATAL_ERROR "BOARD type must be set to either blackpill or discovery")
endif()

# Source folder root directory
set(SRC_DIR ${PROJECT_SOURCE_DIR}/source)

# Component directories
set(UI_DIR ${SRC_DIR}/ui)
set(SYNTH_DIR ${SRC_DIR}/synth)
set(DAE_DIR ${SRC_DIR}/dae)
set(BSP_DIR ${SRC_DIR}/bsp)

# ------------------------------------------------------------------------------
# Application build 
# ------------------------------------------------------------------------------
set(SRCS_APP 
  ${SRC_DIR}/main.c    
  ${SRC_DIR}/ui/ui.c
)

set(INCL_APP ${SRC_DIR}/ui)
set(DEFS_APP $<$<CONFIG:DEBUG>: DEBUG>)

# ------------------------------------------------------------------------------
# These build items are specific to the MCU and physical board pins/layout
# ------------------------------------------------------------------------------
message(STATUS "Configuring board support for '${BOARD}'")

if(BOARD STREQUAL blackpill)     
  SET(SRCS_BSP ${BSP_DIR}/${BOARD}/board.c)  
  SET(INCL_BSP ${BSP_DIR}/${BOARD})
  SET(LINKER_SCRIPT ${BSP_DIR}/${BOARD}/STM32F411CEUX_FLASH.ld)    
  SET(DEFS_BSP HSE_VALUE=25000000 STM32F411xE)

elseif(BOARD STREQUAL discovery)  
  SET(SRCS_BSP ${BSP_DIR}/${BOARD}/board.c)
  SET(INCL_BSP ${BSP_DIR}/${BOARD})
  set(LINKER_SCRIPT ${BSP_DIR}/${BOARD}/STM32F411VEHX_FLASH.ld)    
  SET(DEFS_BSP HSE_VALUE=8000000 STM32F411xE)

else()
  message(FATAL_ERROR "No configuration found for board: '${BOARD}'.")  
endif()  


# ------------------------------------------------------------------------------
# Shared board support, these are common to the STM32F4 family and we append them
# to the board specific files we've already defined.
# ------------------------------------------------------------------------------
list(APPEND SRCS_BSP

  # Shared config and runtime support code
  ${BSP_DIR}/shared/system_stm32f4xx.c
  ${BSP_DIR}/shared/startup_stm32f411xe.s
  ${BSP_DIR}/shared/init.c  
  ${BSP_DIR}/shared/syscalls.c
  ${BSP_DIR}/shared/sysmem.c
   
  # STM LL (low-level) Drivers
  ${BSP_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_spi.c
  ${BSP_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c
  ${BSP_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c
  ${BSP_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma.c
  ${BSP_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c
  ${BSP_DIR}/drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_i2c.c
  
  # FreeRTOS 
  ${BSP_DIR}/middleware/FreeRTOS/Source/tasks.c
  ${BSP_DIR}/middleware/FreeRTOS/Source/list.c
  ${BSP_DIR}/middleware/FreeRTOS/Source/queue.c
  ${BSP_DIR}/middleware/FreeRTOS/Source/portable/MemMang/heap_4.c
  ${BSP_DIR}/middleware/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c     
)

list(APPEND INCL_BSP 
 ${BSP_DIR}/shared    
 ${BSP_DIR}/drivers/STM32F4xx_HAL_Driver/Inc
 ${BSP_DIR}/drivers/CMSIS/Include
 ${BSP_DIR}/drivers/CMSIS/Device/ST/STM32F4xx/Include
 ${BSP_DIR}/middleware/FreeRTOS/Source/include
 ${BSP_DIR}/middleware/FreeRTOS/Source/portable/GCC/ARM_CM4F
)

list(APPEND DEFS_BSP USE_FULL_LL_DRIVER)

# Compiler/linker options are common to the STM32F4xx family
set(OPTS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

# Compiler warnings
set(WARNINGS
  -Wall -Wextra -Werror
  -Wdouble-promotion
  -Wpointer-arith
  -Wno-unused-parameter 
  -Wno-unused-variable 
  -Wno-unused-function 
  -Wno-unused-but-set-variable 
  -Wno-unused-value 
  -Wno-unused-label 
  -Wno-unused-local-typedefs
)

# ------------------------------------------------------------------------------
# Segger RTT support for J-Link probes
# ------------------------------------------------------------------------------
if(DEFINED ENABLE_RTT) 
  list(APPEND SRCS_BSP 
      ${BSP_DIR}/middleware/Segger/RTT/SEGGER_RTT_printf.c
      ${BSP_DIR}/middleware/Segger/RTT/SEGGER_RTT.c
      )
  list(APPEND INCL_BSP   
      ${BSP_DIR}/middleware/Segger/RTT
      ${BSP_DIR}/middleware/Segger/Config
      )
  list(APPEND DEFS_BSP RTT_ENABLED)
endif()


# ------------------------------------------------------------------------------
# Build targets
# ------------------------------------------------------------------------------
add_executable(${TARGET} ${SRCS_APP} ${SRCS_DAE} ${SRCS_BSP})
target_include_directories(${TARGET} PRIVATE ${INCL_APP} ${INCL_DAE} ${INCL_BSP})
target_compile_definitions(${TARGET} PRIVATE ${DEFS_APP} ${DEFS_DAE} ${DEFS_BSP})
target_compile_options(${TARGET} PRIVATE ${OPTS} ${WARNINGS})
target_link_options(${TARGET} PRIVATE ${OPTS} -T ${LINKER_SCRIPT})

# ------------------------------------------------------------------------------
# Post-build: show memory usage
# ------------------------------------------------------------------------------
add_custom_command(TARGET ${TARGET} POST_BUILD
  COMMAND ${CMAKE_SIZE} ${TARGET}
)

