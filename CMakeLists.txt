cmake_minimum_required(VERSION 3.16)
project(SanDaoPoker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# 模块目录(各成员往里填代码)
#   src/core   - 核心逻辑 (A)
#   src/ai     - AI 玩家   (B)
#   src/render - 渲染      (C)
#   src/ui     - 界面      (C)
# ============================================================

# SFML 2.6:找到就链接,找不到先编译纯 C++ 骨架(渲染模块后续接入)
find_package(SFML 2.6 COMPONENTS graphics window system QUIET)

add_executable(SanDaoPoker src/main.cpp)

if(SFML_FOUND)
    target_link_libraries(SanDaoPoker PRIVATE sfml-graphics sfml-window sfml-system)
    target_compile_definitions(SanDaoPoker PRIVATE WITH_SFML=1)
    message(STATUS "SFML 2.6 已找到,渲染模块可用")
else()
    message(WARNING "未找到 SFML,先编译纯 C++ 骨架(第2天接入渲染)")
endif()
