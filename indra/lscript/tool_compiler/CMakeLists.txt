# -*- cmake -*-

project(tool_compiler)

include(00-Common)
include(LLCommon)
include(LScript)

include_directories(
    ${LLCOMMON_INCLUDE_DIRS}
    ${LSCRIPT_INCLUDE_DIRS}
    )
include_directories(SYSTEM
    ${LLCOMMON_SYSTEM_INCLUDE_DIRS}
    )

set(tool_compiler_SOURCE_FILES
    main.cpp
    )

set(tool_compiler_HEADER_FILES
    CMakeLists.txt
    main.h
    )

set_source_files_properties(${tool_compiler_HEADER_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND tool_compiler_SOURCE_FILES ${tool_compiler_HEADER_FILES})

add_executable(lsl_compiler ${tool_compiler_SOURCE_FILES})

target_link_libraries(lsl_compiler
        llcommon
        llinventory
        llfilesystem
        lscript
        lscript_compile
        )
