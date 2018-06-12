cmake_minimum_required(VERSION 3.8)
project(mailer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

# dependencies
find_package(CURL)
include_directories(deps)
include(deps/error/build.cmake)
include(deps/panic/build.cmake)
include(deps/option/build.cmake)
include(deps/alligator/build.cmake)

# archive
include_directories(sources)
include(sources/build.cmake)

# examples
include(examples/build.cmake)
