cmake_minimum_required(VERSION 3.28)
project(binpacking)

# Debug or Release
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE MODULES "${CMAKE_CURRENT_SOURCE_DIR}/3DBinPackingGindeaTocu/src/*.ixx")
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/3DBinPackingGindeaTocu/src/*.cpp")

find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	find_package(tinyfiledialogs CONFIG REQUIRED)
	find_package(nlohmann_json CONFIG REQUIRED)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

target_sources(${PROJECT_NAME} PRIVATE
    FILE_SET cxx_modules TYPE CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3DBinPackingGindeaTocu/src
    FILES ${MODULES}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".html"
    )

    target_link_options(${PROJECT_NAME} PRIVATE
        -sMAX_WEBGL_VERSION=2
        -sMIN_WEBGL_VERSION=2
        -sFULL_ES3=1
        -sALLOW_MEMORY_GROWTH=1
		-sUSE_GLFW=3
		
		-sEXPORTED_FUNCTIONS=["_malloc","_free","_main"]
        -sEXPORTED_RUNTIME_METHODS=["HEAPU8","UTF8ToString","ccall"]
    )
	
	target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_WEB GRAPHICS_API_OPENGL_ES3)
else()
    target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/3DBinPackingGindeaTocu/src/pch/pch.h>")
    target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json tinyfiledialogs::tinyfiledialogs)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--no-undefined -Wl,--gc-sections)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE NO_FONT_AWESOME)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    imgui::imgui
	implot::implot
	raylib
	glfw
)
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/3DBinPackingGindeaTocu/src/pch"
)