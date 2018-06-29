project(ARDUINO_PROJECTS)
cmake_minimum_required(VERSION 2.6)

set(ARDUINO_IDE "${CMAKE_CURRENT_SOURCE_DIR}/../arduino-1.8.3")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../arduino-cmake-minimal/cmake/")

include(ArduinoMinimal)

set(SOURCE_FILES
  main.cpp endianess.cpp
)

set(LIBRARIES
  SPI
)

option(FK_CORE "Apply FieldKit Core configuration.")
if(FK_CORE)
  add_definitions(-DFK_CORE)
endif(FK_CORE)

option(FK_NATURALIST "Apply FieldKit Naturalist configuration.")
if(FK_NATURALIST)
  add_definitions(-DFK_NATURALIST)
endif(FK_NATURALIST)

option(ADAFRUIT_FEATHER "Apply Adafruit Feather configuration.")
if(ADAFRUIT_FEATHER)
  add_definitions(-DADAFRUIT_FEATHER)
endif(ADAFRUIT_FEATHER)

arduino(winc-firmware "${SOURCE_FILES}" "${LIBRARIES}")
