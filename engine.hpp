#pragma once

#include <thread>
#include <atomic>
#include <chrono>

#include <Windows.h>
#include "workload.hpp"




template<typename Policy, int PeriodMS>
class CPULoadEngine {
	std::jthread  m_thread;
	std::atomic<bool> m_stop{ false };

	void run_loop(int target_load, int core_id) {
		HANDLE hThread = GetCurrentThread();
		DWORD_PTR mask = 1ULL << core_id;
		SetThreadAffinityMask(hThread, mask);
		SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);



		constexpr int period_ms = PeriodMS;   // 显式定义为编译期常量
		auto busy_us = target_load * period_ms * 10;
		auto idle_us = (100 - target_load) * period_ms * 10;

			
		auto next_cycle_start = std::chrono::steady_clock::now();


		while (!m_stop.load(std::memory_order_relaxed)) {
			auto busy_end = next_cycle_start + std::chrono::microseconds(busy_us);
			while (std::chrono::steady_clock::now() < busy_end) {
				Workload<Policy>::do_work();
			}

			auto idle_end = busy_end + std::chrono::microseconds(idle_us);
			std::this_thread::sleep_until(idle_end);

			next_cycle_start = idle_end;
		}
	}


public:
	void start(int target_load, int core_id) {
		if (m_thread.joinable()) return;
		m_stop.store(false);

		m_thread = std::jthread([this, target_load, core_id](std::stop_token st) {

			run_loop(target_load, core_id);
			});
	}

	void stop() {
		m_stop.store(true);
		if (m_thread.joinable()) {
			m_thread.join();  
		}
	}

	~CPULoadEngine() {
		stop();
	}






};
