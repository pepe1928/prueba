
cmake_minimum_required (VERSION 3.0)

add_executable(SQC-bin main.cpp)

set(VERSION "\"GMV GRC SQC (version: ${VCS_TAG})\"")
target_compile_definitions(SQC-bin PUBLIC RELEASE_INFO=${VERSION})


add_library(SQC OBJECT  algorithm_manager.cpp
                        compute_manager.cpp
                        config.cpp
                        input_handler.cpp
                        writer.cpp)
                        
add_dependencies(SQC-bin GitVersion)

target_link_libraries(SQC-bin SQC)
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
