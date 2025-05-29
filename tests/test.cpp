#include "../src/soa.hpp"
#include "AoSvsSoA_test.hpp"
#include "ranges_test.hpp"

#include <iostream>
#include <string>

struct FixedTestStruct {
	SoaVector<int, true> x;
	SoaVector<std::string, true> y;

private:
	void *data{};

public:
	void init(SoaVectorSizeType p_size) {
		uint64_t total_size = 0;
		int mem_offset_idx = 0;
		uint64_t memory_offsets[2];

		memory_offsets[mem_offset_idx] = total_size;
		total_size += sizeof(int) * p_size;
		mem_offset_idx++;

		memory_offsets[mem_offset_idx] = total_size;
		total_size += sizeof(std::string) * p_size;

		data = calloc(p_size, total_size / p_size);
		x.init(data, p_size, memory_offsets[0]);
		y.init(data, p_size, memory_offsets[1]);

		// Default construct objects that are not trivially constructible to avoid UB
		if constexpr (!std::is_trivially_constructible_v<std::string>) {
			for (SoaVectorSizeType i = 0; i < p_size; ++i) {
				new (&y[i]) std::string();
			}
		}
	}

	void set_x(int p_index, const int &p_item) { x[p_index] = p_item; }
	[[nodiscard]] int get_x(int p_index) const { return x[p_index]; }
	[[nodiscard]] const int &get_x(int p_index) { return x[p_index]; }

	void set_y(int p_index, const std::string &p_item) { y[p_index] = p_item; }
	[[nodiscard]] std::string get_y(int p_index) { return y[p_index]; }
	[[nodiscard]] const std::string &get_y(int p_index) const { return y[p_index]; }

	~FixedTestStruct() {
		x.reset();
		y.reset();
		free(data);
	}

	FixedTestStruct() = default;
	FixedTestStruct(const FixedTestStruct &) = default;
	FixedTestStruct &operator=(const FixedTestStruct &) = default;
	FixedTestStruct(FixedTestStruct &&) = default;
	FixedTestStruct &operator=(FixedTestStruct &&) = default;
};

struct FixedSOAMacroTestStruct {
	FixedSizeSOA( // This macro expands to the exact same code in FixedTestStruct.
		FixedSOAMacroTestStruct, 2,
		int, x,
		std::string, y
	)
};

struct TestVecStruct {
	DynamicSOA(
		TestVecStruct, 4,
		int, x,
		std::string, y,
		std::vector<std::string>, z,
		std::vector<int>, a
	)
};

struct DynamicTestStruct {
	SoaVector<int, false> x;
	SoaVector<std::string, false> y;

private:
	SoaVectorSizeType soa_capacity = 0;
	void *data{};
	void soa_realloc() {
		const SoaVectorSizeType starting_capacity = soa_capacity;
		soa_capacity = static_cast<SoaVectorSizeType>(soa_capacity * 1.5);
		const SoaVectorSizeType p_size = soa_capacity;

		uint64_t total_size = 0;
		int mem_offset_idx = 0;
		uint64_t memory_offsets[2];

		memory_offsets[mem_offset_idx] = total_size;
		total_size += sizeof(int) * p_size;
		mem_offset_idx++;

		memory_offsets[mem_offset_idx] = total_size;
		total_size += sizeof(std::string) * p_size;
		void *new_data = calloc(soa_capacity, total_size / p_size);

		int current_column = 0;
		x.soa_realloc(new_data, memory_offsets[current_column], starting_capacity);
		current_column++;
		y.soa_realloc(new_data, memory_offsets[current_column], starting_capacity);

		free(data);
		data = new_data;
	}

public:
	void init(int p_size) {
		uint64_t total_size = 0;
		int mem_offset_idx = 0;
		uint64_t memory_offsets[2];

		memory_offsets[mem_offset_idx] = total_size;
		total_size += sizeof(int) * p_size;
		mem_offset_idx++;

		memory_offsets[mem_offset_idx] = total_size;
		total_size += sizeof(std::string) * p_size;

		data = calloc(p_size, total_size / p_size);
		soa_capacity = p_size;
		x.init(data, p_size, memory_offsets[0]);
		y.init(data, p_size, memory_offsets[1]);
	}

	void set_x(int p_index, const int &p_item) { x[p_index] = p_item; }
	[[nodiscard]] int get_x(int p_index) { return x[p_index]; }
	[[nodiscard]] const int &get_x(int p_index) const { return x[p_index]; }

	void set_y(int p_index, const std::string &p_item) { y[p_index] = p_item; }

	[[nodiscard]] std::string get_y(int p_index) { return y[p_index]; }
	[[nodiscard]] const std::string &get_y(int p_index) const { return y[p_index]; }

	~DynamicTestStruct() {
		x.reset();
		y.reset();
		free(data);
	}

	DynamicTestStruct() = default;
	DynamicTestStruct(const DynamicTestStruct &) = default;
	DynamicTestStruct &operator=(const DynamicTestStruct &) = default;
	DynamicTestStruct(DynamicTestStruct &&) = default;
	DynamicTestStruct &operator=(DynamicTestStruct &&) = default;

	void push_x(const int &p_elem) {
		if (x.size() == soa_capacity) {
			[[unlikely]] soa_realloc();
		}
		x.push_soa_member(p_elem);
	}

	void push_y(const std::string &p_elem) {
		if (y.size() == soa_capacity) {
			[[unlikely]] soa_realloc();
		}
		y.push_soa_member(p_elem);
	}
};

struct DynamicSOAMacroTestStruct {
	DynamicSOA( // This macro expands to the exact same code in DynamicTestStruct.
		DynamicSOAMacroTestStruct, 2,
		int, x,
		std::string, y
	)
};

void test_fixed_sized_macro() {
	FixedTestStruct test;
	test.init(2);

	for (int i = 0; i < 2; ++i) {
		test.set_x(i, i);
		test.set_y(i, "Hello");
	}
	FixedSOAMacroTestStruct test_macro;
	test_macro.init(2);

	for (int i = 0; i < 2; ++i) {
		test_macro.set_x(i, i);
		test_macro.set_y(i, "Hello");
	}

	std::cout << "FixedSizeSOA equality test: "
			  << ((test_macro.get_x(0) == test_macro.get_x(0) and test_macro.get_x(1) == test_macro.get_x(1) and test_macro.get_y(0) == test_macro.get_y(0) and
						  test_macro.get_y(1) == test_macro.get_y(1))
								 ? "Passed\n\n"
								 : "Failed.\n\n");
}

void test_dynamic_sized_macro() {
	DynamicTestStruct test;
	test.init(2);

	for (int i = 0; i < 2; ++i) {
		test.push_x(i);
		const std::string hello = std::string(std::to_string(i));
		test.push_y(hello);
	}

	// Test dynamic growth
	test.push_x(3);
	test.push_x(9);
	test.push_x(9);
	test.push_x(9);
	test.set_x(0, 999);

	std::cout << "DynamicSizeSOA size test: " << ((test.x.size() == 6 and test.y.size() == 2) ? "Passed\n" : "Failed.\n");

	DynamicSOAMacroTestStruct test_macro;
	test_macro.init(2);

	for (int i = 0; i < 2; ++i) {
		test_macro.push_x(i);
		const std::string hello = std::string(std::to_string(i));
		test_macro.push_y(hello);
	}

	test_macro.push_x(3);
	test_macro.push_x(9);
	test_macro.push_y("Hi there!");

	std::cout << "DynamicSizeSOA equality test: "
			  << ((test_macro.get_x(0) == test_macro.get_x(0) and test_macro.get_x(1) == test_macro.get_x(1) and test_macro.get_y(0) == test_macro.get_y(0) and
						  test_macro.get_y(1) == test_macro.get_y(1))
								 ? "Passed\n"
								 : "Failed.\n");
	std::cout << "DynamicSizeMacroSOA size test: " << ((test_macro.x.size() == 4 and test_macro.y.size() == 3) ? "Passed\n" : "Failed.\n");

	TestVecStruct vec_test;
	vec_test.init(2);
	for (int i = 0; i < 2; ++i) {
		vec_test.push_x(i);
		const std::string hello = std::string(std::to_string(i));
		vec_test.push_y(hello);

		std::vector<std::string> strings{ hello, hello };
		vec_test.push_z(strings);

		std::vector<int> ints{ i, 5 };
		vec_test.push_a(ints);
	}

	vec_test.push_x(3);
	vec_test.push_x(9);
	vec_test.push_x(9);
	vec_test.push_x(9);
	vec_test.set_x(0, 999);

	std::cout << "DynamicSizeSOA with vector: " << ((vec_test.get_a(0).at(1) == 5) ? "Passed\n" : "Failed.\n");
}

int main() {
	test_fixed_sized_macro();
	test_dynamic_sized_macro();
	soa_perf_test();
	soa_ranges_test();
	std::cout << "\nTests finished.";
	return 0;
}