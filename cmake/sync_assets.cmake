# ============================================================
# sync_assets.cmake — assets 增量同步脚本(成员C)
# 用法: cmake -DSRC=<源 assets 目录> -DDST=<输出 assets 目录> -P sync_assets.cmake
#
# 行为:
#   1) 遍历源目录, 仅拷贝"新增或内容有变化"的文件(时间戳/大小相同则跳过)
#   2) 删除输出目录中"源已不存在"的文件(避免旧素材残留)
# 说明: file(COPY) 会保留源文件时间戳, 因此第二次同步可直接跳过, 开销极小。
# ============================================================

if(NOT DEFINED SRC OR NOT DEFINED DST)
    message(FATAL_ERROR "sync_assets.cmake: 需要 -DSRC=<dir> -DDST=<dir>")
endif()

if(NOT EXISTS "${SRC}")
    return()
endif()

# ---- 1. 拷贝新增/修改的文件 ----
file(GLOB_RECURSE src_files RELATIVE "${SRC}" "${SRC}/*")
set(copied 0)
foreach(f IN LISTS src_files)
    set(s "${SRC}/${f}")
    set(d "${DST}/${f}")
    set(need_copy TRUE)
    if(EXISTS "${d}")
        file(TIMESTAMP "${s}" st "%Y%m%d%H%M%S" UTC)
        file(TIMESTAMP "${d}" dt "%Y%m%d%H%M%S" UTC)
        file(SIZE "${s}" ss)
        file(SIZE "${d}" ds)
        if((st STREQUAL dt) AND (ss EQUAL ds))
            set(need_copy FALSE)
        endif()
    endif()
    if(need_copy)
        get_filename_component(ddir "${d}" DIRECTORY)
        file(MAKE_DIRECTORY "${ddir}")
        file(COPY "${s}" DESTINATION "${ddir}")
        message(STATUS "  [assets] sync ${f}")
        math(EXPR copied "${copied} + 1")
    endif()
endforeach()

# ---- 2. 清理输出目录中源已删除的文件 ----
set(removed 0)
if(EXISTS "${DST}")
    file(GLOB_RECURSE dst_files RELATIVE "${DST}" "${DST}/*")
    foreach(f IN LISTS dst_files)
        if(NOT EXISTS "${SRC}/${f}")
            file(REMOVE "${DST}/${f}")
            message(STATUS "  [assets] remove stale ${f}")
            math(EXPR removed "${removed} + 1")
        endif()
    endforeach()
endif()

if(copied EQUAL 0 AND removed EQUAL 0)
    message(STATUS "  [assets] 已是最新, 无需同步")
else()
    message(STATUS "  [assets] 同步完成: 更新 ${copied} 个, 清理 ${removed} 个")
endif()
