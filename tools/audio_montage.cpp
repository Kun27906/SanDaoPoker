// 试听拼盘工具:把多个 ogg 依次拼接(中间加静音)导出成一个 wav
#include <SFML/Audio.hpp>
#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 3) { std::printf("用法: audio_montage out.wav in1.ogg in2.ogg ...\n"); return 1; }
    std::vector<sf::Int16> all;
    unsigned rate = 0, ch = 0;
    for (int i = 2; i < argc; i++) {
        sf::SoundBuffer b;
        if (!b.loadFromFile(argv[i])) { std::printf("[跳过] %s\n", argv[i]); continue; }
        if (rate == 0) { rate = b.getSampleRate(); ch = b.getChannelCount(); }
        const sf::Int16* s = b.getSamples();
        all.insert(all.end(), s, s + b.getSampleCount());
        // 0.8 秒静音间隔
        all.insert(all.end(), static_cast<size_t>(rate * ch * 0.8), 0);
        std::printf("[%d] %s (%.2fs)\n", i - 1, argv[i],
                    b.getSampleCount() / double(rate) / ch);
    }
    if (all.empty()) { std::printf("没有可用音频\n"); return 1; }
    sf::SoundBuffer out;
    if (!out.loadFromSamples(all.data(), all.size(), ch, rate)) { std::printf("拼接失败\n"); return 1; }
    if (!out.saveToFile(argv[1])) { std::printf("保存失败\n"); return 1; }
    std::printf("已导出 %s  总时长 %.1f 秒\n", argv[1], all.size() / double(rate) / ch);
    return 0;
}