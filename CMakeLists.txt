cmake_minimum_required(VERSION 3.18)
project(glvk-interop)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources, exe
set(_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(GLOB _source_list "${_src_dir}/*.cpp" "${_src_dir}/*.hpp")
set(_target "glvk-interop")
add_executable(${_target} ${_source_list})

# Includes, libraries, compile options
find_package(Vulkan REQUIRED)
add_subdirectory(submodules/glfw)
add_subdirectory(external/glad)
target_include_directories(${_target} PRIVATE ${_src_dir} ${Vulkan_INCLUDE_DIRS} "submodules/glfw/include")
target_link_libraries(${_target} PRIVATE glfw ${Vulkan_LIBRARIES} glad)
target_compile_options(${_target} PRIVATE "/wd26812")

# Shaders
function(add_shader TARGET SHADER)
    find_program(GLSLC glslc)
    
    set(_shader_src_path ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER})
    set(_shader_output_path ${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv)

    get_filename_component(_shader_output_dir ${_shader_output_path} DIRECTORY)
    file(MAKE_DIRECTORY ${_shader_output_dir})

    add_custom_command(
           OUTPUT ${_shader_output_path}
           COMMAND ${GLSLC} -o ${_shader_output_path} ${_shader_src_path}
           DEPENDS ${_shader_src_path}
           IMPLICIT_DEPENDS CXX ${_shader_src_path}
           VERBATIM)
    
    set_source_files_properties(${_shader_output_path} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${_shader_output_path})
endfunction(add_shader)

file(GLOB _shader_list "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*")
foreach(_shader ${_shader_list})
    get_filename_component(_shader_filename ${_shader} NAME)
    add_shader(${_target} ${_shader_filename})
endforeach()

add_custom_target(shaders SOURCES  ${_shader_list}) # Just for grouping shaders in VS