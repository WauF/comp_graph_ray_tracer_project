cmake_minimum_required(VERSION 3.16)
project(raytracer LANGUAGES CXX)

# --- Dil standardı ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # 17 yoksa sessizce düşme, hata ver
set(CMAKE_CXX_EXTENSIONS OFF)         # -std=gnu++17 değil, -std=c++17

# --- Build tipi verilmezse Release olsun (performans puanı!) ---
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# --- pugixml'i ayrı bir küçük kütüphane olarak derle ---
add_library(pugixml STATIC third_party/pugixml/pugixml.cpp)
target_include_directories(pugixml PUBLIC third_party/pugixml)

# --- Asıl program ---
add_executable(raytracer
  src/main.cpp
  # yeni .cpp ekledikçe buraya yaz
)

target_include_directories(raytracer PRIVATE src)
target_include_directories(raytracer SYSTEM PRIVATE third_party/stb)

find_package(Threads REQUIRED)
target_link_libraries(raytracer PRIVATE pugixml Threads::Threads)

target_compile_options(raytracer PRIVATE -Wall -Wextra -Wpedantic)