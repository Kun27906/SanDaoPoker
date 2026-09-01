# ============================================================
# SanDaoPoker (炸金花三道) — 顶层构建脚本
# 技术栈：C++17 + SFML 2.6 + CMake，单机版（1 真人 + AI）
# 目录约定：头文件统一放 include/<模块>/，实现放 src/<模块>/
#           源码中按 #include "core/Card.h" 方式引用头文件
# 分支约定：main 正式版；A/B/C/D 各成员工作分支，由 D 合并回 main
# ============================================================
cmake_minimum_required(VERSION 3.16)
project(SanDaoPoker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ------------------------------------------------------------
# 模块目录(各成员往里填代码)
#   include/core + src/core       - 核心逻辑 (A)
#   include/ai   + src/ai         - AI 玩家   (B)
#   include/render + src/render   - 渲染      (C)
#   include/ui   + src/ui         - 界面控件  (C)
#   assets/ CMakeLists.txt third_party/      (D)
# ------------------------------------------------------------

# ---- SFML 2.6 (third_party 本地包, zip 自动解压, 克隆即编译) ----
set(SFML_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SFML-2.6.1")
if(NOT EXISTS "${SFML_ROOT}/lib/cmake/SFML")
    message(STATUS "未找到 SFML 解压目录,正在自动解压 third_party/SFML-2.6.1.zip ...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SFML-2.6.1.zip"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
endif()
set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML")
find_package(SFML 2.6 COMPONENTS graphics window system audio network QUIET)

# ------------------------------------------------------------
# 源文件清单(按模块分组,与 src/<模块>/ 目录一一对应)
# 注:RuleConfig.h 为纯头文件(位于 include/core/),无需编译
# ------------------------------------------------------------
set(CORE_SOURCES
    src/core/Card.cpp
    src/core/Deck.cpp
    src/core/HandEvaluator.cpp
    src/core/Player.cpp
    src/core/Round.cpp
    src/core/Room.cpp
)

set(AI_SOURCES
    src/ai/AIPlayer.cpp
)

set(RENDER_SOURCES
    src/render/AssetManager.cpp
    src/render/CardSprite.cpp
    src/render/GameApp.cpp
    src/render/SceneManager.cpp
    src/render/SceneMenu.cpp
    src/render/SceneArrange.cpp
    src/render/SceneBattle.cpp
    src/render/SceneResult.cpp
)

set(UI_SOURCES
    src/ui/Button.cpp
    src/ui/TextBox.cpp
    src/ui/CountdownBar.cpp
)

# ------------------------------------------------------------
# 可执行目标
# ------------------------------------------------------------
add_executable(SanDaoPoker
    src/main.cpp
    ${CORE_SOURCES}
    ${AI_SOURCES}
    ${RENDER_SOURCES}
    ${UI_SOURCES}
)

target_include_directories(SanDaoPoker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include      # 公共头文件(include/<模块>/)
    ${CMAKE_CURRENT_SOURCE_DIR}/src          # 实现文件(兼容相对包含)
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party  # 第三方库头文件
)

if(SFML_FOUND)
    target_link_libraries(SanDaoPoker PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        sfml-network
    )
    target_compile_definitions(SanDaoPoker PRIVATE WITH_SFML=1)
    message(STATUS "SFML 2.6 已找到,渲染模块可用")

    # 拷贝 SFML DLL 到 exe 旁边(双击就能跑)
    if(WIN32)
        set(SFML_DLL_DIR "${SFML_ROOT}/bin")
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

# ------------------------------------------------------------
# 资源拷贝:构建后自动将 assets/ 复制到可执行文件所在目录
# (保证 exe 在任意目录运行都能找到素材)
# ------------------------------------------------------------
add_custom_command(TARGET SanDaoPoker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:SanDaoPoker>/assets"
    COMMENT "Copying assets to output directory"
)

# ------------------------------------------------------------
# 编译器警告
# 编译器警告 + UTF-8 源文件支持(中文注释必需)
if(MSVC)
    add_compile_options(/utf-8)
    target_compile_options(SanDaoPoker PRIVATE /W4)
    if(TARGET test_hand_evaluator)
        target_compile_options(test_hand_evaluator PRIVATE /W4)
    endif()
else()
    target_compile_options(SanDaoPoker PRIVATE -Wall -Wextra)
    if(TARGET test_hand_evaluator)
        target_compile_options(test_hand_evaluator PRIVATE -Wall -Wextra)
    endif()
endif()

# ------------------------------------------------------------
# 单元测试(可选,默认关闭):cmake -DBUILD_TESTS=ON ..
# 测试仅依赖核心逻辑(不链接 SFML)
# ------------------------------------------------------------
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_executable(test_hand_evaluator
        tests/test_hand_evaluator.cpp
        ${CORE_SOURCES})
    target_include_directories(test_hand_evaluator PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
    if(MSVC)
        target_compile_options(test_hand_evaluator PRIVATE /utf-8)
    endif()
    add_test(NAME hand_evaluator COMMAND test_hand_evaluator)
endif()
