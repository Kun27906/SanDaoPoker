cmake_minimum_required(VERSION 3.16)
project(SanDaoPoker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ============================================================
# 模块目录(各成员往里填代码)
#   src/core   - 核心逻辑 (A)
#   src/ai     - AI 玩家   (B)
#   src/render - 渲染      (C)
#   src/ui     - 界面      (C)
# ============================================================

# ---- SFML 2.6 (third_party 本地包,zip 自动解压) ----
set(SFML_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SFML-2.6.1")
if(NOT EXISTS "${SFML_ROOT}/lib/cmake/SFML")
    message(STATUS "未找到 SFML 解压目录,正在自动解压 third_party/SFML-2.6.1.zip ...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SFML-2.6.1.zip"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
endif()
set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML")
find_package(SFML 2.6 COMPONENTS graphics window system audio network QUIET)

add_executable(SanDaoPoker src/main.cpp)

if(SFML_FOUND)
    target_link_libraries(SanDaoPoker PRIVATE sfml-graphics sfml-window sfml-system sfml-audio sfml-network)
    target_compile_definitions(SanDaoPoker PRIVATE WITH_SFML=1)
    message(STATUS "SFML 2.6 已找到,渲染模块可用")

    # ---- 拷贝 SFML DLL 到 exe 旁边(双击就能跑) ----
    if(WIN32)
        set(SFML_DLL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SFML-2.6.1/bin")
        add_custom_command(TARGET SanDaoPoker POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SFML_DLL_DIR}/sfml-graphics-2.dll"
                "${SFML_DLL_DIR}/sfml-window-2.dll"
                "${SFML_DLL_DIR}/sfml-system-2.dll"
                "${SFML_DLL_DIR}/sfml-audio-2.dll"
                "${SFML_DLL_DIR}/sfml-network-2.dll"
                $<TARGET_FILE_DIR:SanDaoPoker>)
        message(STATUS "已配置 DLL 自动拷贝到 exe 目录")
    endif()
else()
    message(WARNING "未找到 SFML,先编译纯 C++ 骨架(渲染模块后续接入)")
endif()
