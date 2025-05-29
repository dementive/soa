#pragma once

#include "../src/soa.hpp"

#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

struct SoaStruct {
	FixedSizeSOA(
		SoaStruct, 3,
		int, a,
		int, b,
		int, c
	)
};

namespace views = std::views;
namespace ranges = std::ranges;

inline bool greater_than_2(const int elem) { return elem > 2; }

// https://en.cppreference.com/w/cpp/algorithm/ranges.html
// https://en.cppreference.com/w/cpp/ranges.html

#define TEST(m_name, m_condition)                                                                                                                                                            \
	std::cout << m_name;                                                                                                                                                                     \
	std::cout << ((m_condition) ? "Passed\n" : "Failed\n");

inline void soa_ranges_test() {
	SoaStruct soa_struct;
	soa_struct.init(5);
	static_assert(std::ranges::contiguous_range<decltype(soa_struct.a)>);

	for (int i = 0; i < 5; ++i) {
		soa_struct.set_a(i, i);
		soa_struct.set_b(i, i + i);
	}

	// Ranges stuff
	auto reverse_and_drop_test = soa_struct.a | views::reverse | views::drop(1);
	TEST("\nReverse and drop 1 element: ", reverse_and_drop_test.front() == 3)

	auto filter_test = soa_struct.a | views::filter(greater_than_2);
	TEST("Filter greater than 2: ", filter_test.front() == 3)

	auto zip_test = views::zip(soa_struct.a, soa_struct.b);
	std::tuple<int, int> fifth_row = zip_test[4];
	TEST("Zip: ", std::get<0>(fifth_row) == 4 and std::get<1>(fifth_row) == 8)

	// have to use ranges::to to sort (or modify in any way) a subrange otherwise it will modify the original data.
	auto subrange_test = ranges::subrange(zip_test.begin(), zip_test.end()) | ranges::to<std::vector<std::tuple<int, int>>>();
	ranges::sort(subrange_test, std::greater());
	auto first_sorted_pair = subrange_test[0];
	TEST("Sorting subrange: ", std::get<0>(first_sorted_pair) == 4 and std::get<1>(first_sorted_pair) == 8)

	std::vector<int> range_to_test = soa_struct.a | ranges::to<std::vector<int>>();
	TEST("Ranges to: ", range_to_test[4] == 4)

	// Constrained algorithms
	auto find_result = ranges::find_if(soa_struct.a, greater_than_2);
	TEST("Find: ", find_result != soa_struct.a.end())

	TEST("Any of: ", ranges::any_of(soa_struct.a, greater_than_2))
	TEST("Count test: ", ranges::count(soa_struct.a, 0) == 1)

	ranges::sort(soa_struct.b, std::greater());
	TEST("Sort test: ", soa_struct.get_b(0) == 8)
}
