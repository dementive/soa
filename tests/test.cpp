#include <cstdlib>
#include <iostream>
#include <string>

#include "../src/soa.hpp"

struct TestStruct {
	SoaVector<int> x;
	SoaVector<std::string> y;

	void *data{};
	int memory_offsets[2];

	int get_malloc_size(int p_size) {
		int total_size = 0;
		int mem_offset_idx = 0;

		memory_offsets[mem_offset_idx] = total_size;
		total_size += sizeof(int) * p_size;
		mem_offset_idx++;

		memory_offsets[mem_offset_idx] = total_size;
		total_size += sizeof(std::string) * p_size;
		mem_offset_idx++;

		return total_size;
	}

	void init(int p_size) {
		data = malloc(get_malloc_size(p_size));
		x.init(data, p_size, memory_offsets[0]);
		y.init(data, p_size, memory_offsets[1]);
	}

	void set_x(int p_index, const int &p_item) { x[p_index] = p_item; }
	int get_x(int p_index) const { return x[p_index]; }
	void set_y(int p_index, const std::string &p_item) { y[p_index] = p_item; }
	std::string get_y(int p_index) const { return y[p_index]; }

	~TestStruct() {
		x.reset();
		y.reset();
		free(data);
	}
};

struct SOAMacroTestStruct {
	FixedSizeSOA( // This macro expands to the exact same code in TestStruct.
		SOAMacroTestStruct, 2,
		int, x,
		std::string, y
	)
};

void test_fixed_sized_macro() {
	TestStruct test;
	test.init(2);

	for (int i = 0; i < 2; ++i) {
		test.set_x(i, i);
		test.set_y(i, "Hello");
	}

	std::cout << "1st element: " << test.get_x(0) << ", " << test.get_y(0) << "\n";
	std::cout << "2nd element: " << test.get_x(1) << ", " << test.get_y(1) << "\n";

	TestStruct test_macro;
	test_macro.init(2);

	for (int i = 0; i < 2; ++i) {
		test_macro.set_x(i, i);
		test_macro.set_y(i, "Hello");
	}

	std::cout << "1st element: " << test_macro.get_x(0) << ", " << test_macro.get_y(0) << "\n";
	std::cout << "2nd element: " << test_macro.get_x(1) << ", " << test_macro.get_y(1) << "\n";
	std::cout << "FixedSizeSOA equality test: " << ((test_macro.get_x(0) == test.get_x(0) and test_macro.get_x(1) == test.get_x(1) and test_macro.get_y(0) == test.get_y(0) and test_macro.get_y(1) == test.get_y(1)) ? "Passed\n" : "Failed.\n");
}

int main(int argc, char const *argv[]) {
	test_fixed_sized_macro();
	return 0;
}