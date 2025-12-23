# ------------------------------------------------------------------------------
#  MIT License
#  Copyright (c) 2025 Jason Wilden
# 
#  Permission to use, copy, modify, and/or distribute this code for any purpose
#  with or without fee is hereby granted, provided the above copyright notice an
#  this permission notice appear in all copies.
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)

PROJECT("Axis" C ASM)

# We want to be using a modern C
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
add_subdirectory(source)

