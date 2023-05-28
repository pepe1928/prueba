cmake_minimum_required (VERSION 3.0)

get_target_property(LIBCOMMON_INCLUDES Common_Lib INCLUDE_DIRECTORIES)
include_directories(${LIBCOMMON_INCLUDES})
get_target_property(CONFIG_INCLUDES Config INCLUDE_DIRECTORIES)
include_directories(${CONFIG_INCLUDES})

include_directories("include")
add_subdirectory("src")

add_executable(SQC-bin main.cpp)

set(VERSION "\"GMV GRC SQC (version: ${VCS_TAG})\"")
target_compile_definitions(SQC-bin PUBLIC RELEASE_INFO=${VERSION})

add_dependencies(SQC-bin GitVersion)

target_link_libraries(SQC-bin SQC)
target_link_libraries(SQC-bin Common_Lib)
target_link_libraries(SQC-bin Config)
target_link_libraries(SQC-bin ${Boost_LIBRARIES})
target_link_libraries(SQC-bin pthread)
target_link_libraries(SQC-bin yaml-cpp)

# now we rename using target properties
set_target_properties(SQC-bin
                      PROPERTIES OUTPUT_NAME SQC)

add_custom_command(TARGET SQC-bin POST_BUILD
                   COMMAND $<$<CONFIG:release>:${CMAKE_STRIP}> $<TARGET_FILE:SQC-bin>)

install(TARGETS SQC-bin
        RUNTIME DESTINATION bin)
