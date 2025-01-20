# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/myry/esp/esp-idf/components/bootloader/subproject")
  file(MAKE_DIRECTORY "/Users/myry/esp/esp-idf/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "/Users/myry/Documents/Github/esp32s3-develop-board/02.Software/02_ESP-IDF/00_HelloWorld/build/bootloader"
  "/Users/myry/Documents/Github/esp32s3-develop-board/02.Software/02_ESP-IDF/00_HelloWorld/build/bootloader-prefix"
  "/Users/myry/Documents/Github/esp32s3-develop-board/02.Software/02_ESP-IDF/00_HelloWorld/build/bootloader-prefix/tmp"
  "/Users/myry/Documents/Github/esp32s3-develop-board/02.Software/02_ESP-IDF/00_HelloWorld/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/myry/Documents/Github/esp32s3-develop-board/02.Software/02_ESP-IDF/00_HelloWorld/build/bootloader-prefix/src"
  "/Users/myry/Documents/Github/esp32s3-develop-board/02.Software/02_ESP-IDF/00_HelloWorld/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/myry/Documents/Github/esp32s3-develop-board/02.Software/02_ESP-IDF/00_HelloWorld/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/myry/Documents/Github/esp32s3-develop-board/02.Software/02_ESP-IDF/00_HelloWorld/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
