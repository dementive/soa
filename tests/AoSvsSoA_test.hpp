#include "../src/soa.hpp"

#include <chrono>
#include <iostream>

struct Vector2 {
	float x = 1;
	float y = 1;
};

struct SoaPerfTestStruct {
	FixedSizeSOA(
		SoaPerfTestStruct, 8,
		int, a,
		Vector2, b,
		Vector2, c,
		Vector2, d,
		Vector2, e,
		Vector2, f,
		int, g,
		int, h
	)
};

struct AosPerfTestStruct {
	int a{};
	Vector2 b;
	Vector2 c;
	Vector2 d;
	Vector2 e;
	Vector2 f;
	int g{};
	int h{};
};

template <typename Func> auto measure_time(Func func) {
	auto start = std::chrono::high_resolution_clock::now();
	func();
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double, std::milli>(end - start).count();
}

inline int soa_perf_test() {
	const int size = 10000;
	SoaPerfTestStruct soa_struct{};
	soa_struct.init(size);

	int add = 0;
	const double soa_fill_time = measure_time([&]() {
		for (int i = 0; i < size; ++i) {
			soa_struct.set_a(i, i);
			soa_struct.set_b(i, Vector2());
			soa_struct.set_c(i, Vector2());
			soa_struct.set_d(i, Vector2());
			soa_struct.set_e(i, Vector2());
			soa_struct.set_f(i, Vector2());
			soa_struct.set_g(i, add++ + i);
			soa_struct.set_h(i, add++ + i);
		}
	});
	std::cout << "\nSOA set all members time: " << soa_fill_time << " ms\n";

	std::vector<AosPerfTestStruct> aos_arr(size);
	int add2 = 0;
	const double aos_fill_time = measure_time([&]() {
		for (int i = 0; i < size; ++i) {
			AosPerfTestStruct aos_struct{};
			aos_struct.a = i;
			aos_struct.b = Vector2();
			aos_struct.c = Vector2();
			aos_struct.d = Vector2();
			aos_struct.e = Vector2();
			aos_struct.f = Vector2();
			aos_struct.g = add2++ + i;
			aos_struct.h = add2++ + i;
			aos_arr[i] = aos_struct;
		}
	});
	std::cout << "AOS set all members time: " << aos_fill_time << " ms\n\n";

	soa_struct.clear();
	aos_arr.clear();
	soa_struct.init(size);
	const double soa_fill_2_members_time = measure_time([&]() {
		for (int i = 0; i < size; ++i) {
			soa_struct.set_a(i, i);
		}
	});
	std::cout << "SOA set 1 member time: " << soa_fill_2_members_time << " ms\n";

	const double aos_fill_2_members_time = measure_time([&]() {
		for (int i = 0; i < size; ++i) {
			aos_arr[i].a = i;
		}
	});
	std::cout << "AOS set 1 member time: " << aos_fill_2_members_time << " ms\n\n";

	uint64_t soa_sum_a = 0;

	const double soa_access_time = measure_time([&]() {
		for (int i = 0; i < size; ++i) {
			soa_sum_a += soa_struct.a[i];
		}
	});
	std::cout << "SOA sum time: " << soa_access_time << " ms\n";

	uint64_t aos_sum_a = 0;
	const double aos_access_time = measure_time([&]() {
		for (int i = 0; i < size; ++i) {
			aos_sum_a += aos_arr[i].a;
		}
	});
	std::cout << "AOS sum time: " << aos_access_time << " ms\n\n";
	std::cout << "SOA sum_a: " << soa_sum_a << "\n";
	std::cout << "AOS sum_a: " << aos_sum_a << "\n";
	return 0;
}
