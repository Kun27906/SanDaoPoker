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

# 全局 UTF-8 源文件支持(中文注释必需)
# 注意:add_compile_options 必须放在所有 add_executable 之前才会对目标生效
if(MSVC)
    add_compile_options(/utf-8)
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
    src/render/Account.cpp
    src/render/AssetManager.cpp
    src/render/CardSprite.cpp
    src/render/GameApp.cpp
    src/render/SceneManager.cpp
    src/render/SceneTitle.cpp
    src/render/SceneLobby.cpp
    src/render/SceneRoomSelect.cpp
    src/render/SceneArrange.cpp
    src/render/SceneBattle.cpp
    src/render/SceneResult.cpp
    src/render/SoundManager.cpp
)

set(UI_SOURCES
    src/ui/Button.cpp
    src/ui/TextBox.cpp
    src/ui/CountdownBar.cpp
    src/ui/ChipBar.cpp
)

# ------------------------------------------------------------
# 可执行目标
# ------------------------------------------------------------
# exe + SFML DLL + assets 统一输出到根目录 bin/ 文件夹
# (保持根目录整洁; 用户双击 bin/SanDaoPoker.exe 即可运行,
#  程序相对 bin/ 目录访问 assets/ 与 game_data/)
set(SDP_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_executable(SanDaoPoker
    src/main.cpp
    ${CORE_SOURCES}
    ${AI_SOURCES}
    ${RENDER_SOURCES}
    ${UI_SOURCES}
)
set_target_properties(SanDaoPoker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${SDP_OUTPUT_DIR})

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
    # 用 copy_directory 全量拷贝 bin/:Debug(-d-2.dll)与 Release DLL 都在,
    # 避免构建类型与拷贝的 DLL 不匹配导致运行报"找不到 sfml-xxx-d-2.dll"
    if(WIN32)
        set(SFML_DLL_DIR "${SFML_ROOT}/bin")
        add_custom_command(TARGET SanDaoPoker POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${SFML_DLL_DIR}"
                $<TARGET_FILE_DIR:SanDaoPoker>)
        message(STATUS "已配置 DLL 自动拷贝到 exe 目录")
    endif()
else()
    message(WARNING "未找到 SFML,先编译纯 C++ 骨架(渲染模块后续接入)")
endif()

# ------------------------------------------------------------
# 资源拷贝:构建后自动将 assets/ 复制到可执行文件所在目录
# (exe 在项目根时 assets 已在根目录, 跳过自拷贝; 其他输出目录才拷贝)
# ------------------------------------------------------------
if(NOT "${SDP_OUTPUT_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    add_custom_command(TARGET SanDaoPoker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                "$<TARGET_FILE_DIR:SanDaoPoker>/assets"
        COMMENT "Copying assets to output directory"
    )
endif()

# ------------------------------------------------------------
# 编译器警告
# 编译器警告 + UTF-8 源文件支持(中文注释必需)
# 全局 /utf-8 已在文件顶部设置(须在 add_executable 之前);
# 这里对 SanDaoPoker 目标再显式追加一次,双保险
if(MSVC)
    target_compile_options(SanDaoPoker PRIVATE /W4 /utf-8)
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

# ------------------------------------------------------------
# 开发工具构建接口 (成员B 2026-09)
# 用法示例:
#   cmake --build build --target gen_winrate_table   # 生成胜率表
#   cmake --build build --target evaluate_ai         # AI 对战评估
#   cmake --build build --target human_test          # 人性化检验
#   cmake --build build --target gen_assets          # 素材生成(python)
# 说明: 工具只依赖 core+ai, 不链接 SFML
# ------------------------------------------------------------
option(BUILD_TOOLS "Build dev tools (winrate/evaluate/human)" ON)
if(BUILD_TOOLS)
    set(TOOL_COMMON_SOURCES
        ${CORE_SOURCES}
        ${AI_SOURCES}
    )
    set(TOOL_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # 1. 胜率表生成器(训练): 输出 assets/ai/winrate.bin
    add_executable(gen_winrate_table
        tools/gen_winrate_table.cpp
        ${TOOL_COMMON_SOURCES})
    target_include_directories(gen_winrate_table PRIVATE ${TOOL_INCLUDE_DIRS})
    if(MSVC)
        target_compile_options(gen_winrate_table PRIVATE /utf-8)
    endif()

    # 2. AI 对战评估器
    add_executable(evaluate_ai
        tools/evaluate_ai.cpp
        ${TOOL_COMMON_SOURCES})
    target_include_directories(evaluate_ai PRIVATE ${TOOL_INCLUDE_DIRS})
    if(MSVC)
        target_compile_options(evaluate_ai PRIVATE /utf-8)
    endif()

    # 3. 人性化快速检验
    add_executable(human_test
        tools/human_test.cpp
        ${TOOL_COMMON_SOURCES})
    target_include_directories(human_test PRIVATE ${TOOL_INCLUDE_DIRS})
    if(MSVC)
        target_compile_options(human_test PRIVATE /utf-8)
    endif()

    # 4. Room/Round 完整对局冒烟(集成回归): 16房间全流程
    add_executable(room_smoke_test
        tools/room_smoke_test.cpp
        ${TOOL_COMMON_SOURCES})
    target_include_directories(room_smoke_test PRIVATE ${TOOL_INCLUDE_DIRS})
    if(MSVC)
        target_compile_options(room_smoke_test PRIVATE /utf-8)
    endif()

    # 4. 素材生成(需 python3 + pillow): 牌面贴图 + UI 素材
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_FOUND)
        add_custom_target(gen_assets
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_card_sheet.py
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_ui_assets.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "生成牌面/UI 素材 (需 pillow)")
    else()
        message(WARNING "未找到 Python3,跳过 gen_assets 素材生成目标")
    endif()
endif()
