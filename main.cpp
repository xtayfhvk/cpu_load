
#include <iostream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <memory>

#include "engine.hpp"
#include "policies.hpp"

// 检查当前系统实际有多少核，防止用户输入超限
int get_system_core_count() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast<int>(sysInfo.dwNumberOfProcessors);
}

int main(int argc, char* argv[]) {
    // ---------- 第1步：解析命令行参数 ----------
    if (argc != 3) {
        std::cerr << "Usage: CPULoadGenerator.exe <core_count> <target_load_percent>" << std::endl;
        std::cerr << "Example: CPULoadGenerator.exe 4 70  (让4个核心跑70%负载)" << std::endl;
        return 1;
    }

    int core_count = std::atoi(argv[1]);
    int target_load = std::atoi(argv[2]);

    // 参数合法性校验
    int system_cores = get_system_core_count();
    if (core_count <= 0 || core_count > system_cores) {
        std::cerr << "Error: core_count must be between 1 and " << system_cores << std::endl;
        return 1;
    }
    if (target_load < 0 || target_load > 100) {
        std::cerr << "Error: target_load must be between 0 and 100" << std::endl;
        return 1;
    }

    std::cout << "System has " << system_cores << " cores." << std::endl;
    std::cout << "Will stress " << core_count << " cores to " << target_load << "% load." << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;

    // ---------- 第2步：根据负载值，选择模板策略（编译期多态） ----------
    // 注意：这里的 if 是在 *运行时* 判断，但每个分支内部实例化的模板，其代码在编译期就生成了。
    // 并没有虚表开销。
    if (target_load > 80) {
        // 高负载：使用浮点策略（FloatPolicy），周期 100ms
        using EngineType = CPULoadEngine<FloatPolicy, 100>;
        std::vector<std::unique_ptr<EngineType>> engines;

        for (int i = 0; i < core_count; ++i) {
            auto eng = std::make_unique<EngineType>();
            eng->start(target_load, i);  // 让第 i 个核心跑起来
            engines.push_back(std::move(eng));
        }

        // 阻塞主线程，等待 Ctrl+C
        std::cin.get();
        // 这里会触发 vector 析构，每个 Engine 的析构会调用 stop()，优雅退出。
        std::cout << "Stopping engines..." << std::endl;
    }
    else {
        // 中低负载：使用整数策略（IntPolicy）
        using EngineType = CPULoadEngine<IntPolicy, 100>;
        std::vector<std::unique_ptr<EngineType>> engines;

        for (int i = 0; i < core_count; ++i) {
            auto eng = std::make_unique<EngineType>();
            eng->start(target_load, i);
            engines.push_back(std::move(eng));
        }

        std::cin.get();
        std::cout << "Stopping engines..." << std::endl;
    }

    return 0;
}