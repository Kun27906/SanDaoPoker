#pragma once

// ====== 版本信息与版本历史(成员C) ======
// GAME_VERSION    : 当前版本号(显示在 启动页/大厅/选房 左下角)
// VERSION_HISTORY : 版本历史(大厅/选房 点击左下角版本号 -> "版本历史"弹窗, 按时间顺序)
// 维护约定: 新增版本时改 GAME_VERSION, 并在 VERSION_HISTORY 末尾追加一条(时间从早到晚)。
constexpr const char* GAME_VERSION = "v1.3.2";

struct VersionEntry {
    const char* ver;    // 版本号
    const char* desc;   // 主要更新
};

constexpr VersionEntry VERSION_HISTORY[] = {
    {"v0.0.1b", "完整可执行文件生成，核心逻辑完成，图形化界面与素材落地"},
    {"v0.0.2b", "房间结构完善，构建AI接口"},
    {"v0.1.1b", "素材全面更新，AI接入测试完善"},
    {"v1.0.1",  "集成回归工具，全局冒烟测试完成，可正式运行"},
    {"v1.0.2",  "结算系统完善，正式版完整规则落地"},
    {"v1.1.1",  "加入正式大厅，选房界面完成"},
    {"v1.1.2",  "本地账户体系建立，游戏数据与可执行文件与编译区域分离"},
    {"v1.1.3",  "规则更新，加入更多关于底注的限制，修改注金模型"},
    {"v1.1.4",  "增加重置本地帐号功能，牌背与筹码素材颜色更替"},
    {"v1.2.1",  "加入菜单栏，音效修改系统，加入游戏规则说明"},
    {"v1.2.2",  "加入开发者模式"},
    {"v1.2.3",  "优化组排界面互动，增加拖动单张牌"},
    {"v1.2.4",  "重构结算显示，最终结算分条显示"},
    {"v1.3.1",  "随机昵称与头像系统上线，增加发牌动画"},
    {"v1.3.2",  "接入AI难度分级系统"},
};

constexpr int VERSION_HISTORY_COUNT =
    static_cast<int>(sizeof(VERSION_HISTORY) / sizeof(VERSION_HISTORY[0]));
