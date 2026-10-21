cmake_minimum_required(VERSION 3.20)
project(LarpController CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Expect imgui + nlohmann headers under third_party/
set(IMGUI_DIR     ${CMAKE_SOURCE_DIR}/third_party/imgui)
set(NLOHMANN_DIR  ${CMAKE_SOURCE_DIR}/third_party/nlohmann)

add_executable(LarpController WIN32
    controller/main.cpp
    controller/roblox.cpp
    controller/presets.cpp
    controller/ui.cpp
    controller/watchdog.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

target_include_directories(LarpController PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${NLOHMANN_DIR}
)

target_link_libraries(LarpController PRIVATE d3d11 dwmapi)

# Compatibility check utility - console exe users run before install.
add_executable(LarpCompatCheck tools/compat_check.cpp)
target_link_libraries(LarpCompatCheck PRIVATE advapi32)
