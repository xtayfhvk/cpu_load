#pragma once

#include "policies.hpp"


//init
template<typename Policy>
struct Workload;

template<>
struct Workload<IntPolicy> {
	static void do_work() {
		volatile int slink = 0;
		for (int i = 0; i < 1024; i++)
		{
			slink += i * i;
		}
	}
};

template<>
struct Workload<FloatPolicy> {
	static void do_work() {
		volatile double sink = 0.0;
		for (int i = 0; i < 1024; ++i) {
			sink += static_cast<double>(i) * 3.14159f;
		}
	}
};






