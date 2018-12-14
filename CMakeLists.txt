cmake_minimum_required(VERSION 3.0)
project(ntfs_volume_eraser)
set(TARGET ntfs_volume_eraser)

set(BOOST_MIN_VERSION "1.61.0")
set(Boost_ADDITIONAL_VERSIONS "1.61.0" "1.61")
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT MY_BOOST_DIR) 

find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS program_options REQUIRED)

add_executable(${TARGET} ntfs_journal.cpp command_line_parser.cpp command_line_parser.h)
target_link_libraries(${TARGET} ${Boost_LIBRARIES})



