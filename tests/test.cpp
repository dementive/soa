#include "../src/soa.hpp"
#include "AoSvsSoA_test.hpp"
#include "ranges_test.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

struct FixedTestStruct {
	soa::SoaVector<int> x;
	soa::SoaVector<std::string> y;

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
		x.init_fixed(data, p_size, memory_offsets[0]);
		y.init_fixed(data, p_size, memory_offsets[1]);

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
	soa::SoaVector<int> x;
	soa::SoaVector<std::string> y;

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

struct MutableTestStruct {
	soa::SoaVector<int> x;
	soa::SoaVector<std::string> y;

private:
	SOA_MAP_TYPE index_map; // entity id -> index map
	SoaVectorSizeType soa_capacity = 0;
	SoaVectorSizeType soa_size = 0;
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

	// NOTE: This does not work like the std::vector::erase. It moved the last element in the vector to the removed spot instead of shifting everything down.
	// This makes erase of SoaVector O(1) instead of O(N) like std::vector.
	void erase(SoaVectorSizeType p_entity_id) {
		if (!index_map.contains(p_entity_id)) {
			return;
		}

		SoaVectorSizeType index_to_erase = index_map.at(p_entity_id);
		const SoaVectorSizeType end_index = soa_size - 1;
		SoaVectorSizeType entity_id_to_move = index_map.find(end_index)->second;

		index_map.erase(p_entity_id);
		index_map[entity_id_to_move] = index_to_erase;

		if constexpr (!std::is_trivially_destructible_v<int>) {
			x.destroy_at(index_to_erase);
		}
		if constexpr (!std::is_trivially_destructible_v<std::string>) {
			y.destroy_at(index_to_erase);
		}

		if (index_to_erase < --soa_size) {
			x.post_erase(index_to_erase, end_index);
			y.post_erase(index_to_erase, end_index);
		}
	}

	void set_x(SoaVectorSizeType p_entity_id, const int &p_item) {
		const SoaVectorSizeType &index = index_map.at(p_entity_id);
		x[index] = p_item;
	}
	[[nodiscard]] int get_x(SoaVectorSizeType p_entity_id) {
		const SoaVectorSizeType &index = index_map.at(p_entity_id);
		return x[index];
	}
	[[nodiscard]] const int &get_x(SoaVectorSizeType p_entity_id) const {
		const SoaVectorSizeType &index = index_map.at(p_entity_id);
		return x[index];
	}

	void set_y(SoaVectorSizeType p_entity_id, const std::string &p_item) {
		const SoaVectorSizeType &index = index_map.at(p_entity_id);
		y[index] = p_item;
	}

	[[nodiscard]] std::string get_y(SoaVectorSizeType p_entity_id) {
		const SoaVectorSizeType &index = index_map.at(p_entity_id);
		return y[index];
	}
	[[nodiscard]] const std::string &get_y(SoaVectorSizeType p_entity_id) const {
		const SoaVectorSizeType &index = index_map.at(p_entity_id);
		return y[index];
	}

	~MutableTestStruct() {
		x.reset();
		y.reset();
		free(data);
	}

	MutableTestStruct() = default;
	MutableTestStruct(const MutableTestStruct &) = default;
	MutableTestStruct &operator=(const MutableTestStruct &) = default;
	MutableTestStruct(MutableTestStruct &&) = default;
	MutableTestStruct &operator=(MutableTestStruct &&) = default;

	void push_x(const int &p_elem) {
		if (x.size() == soa_capacity) {
			[[unlikely]] soa_realloc();
		}
		SoaVectorSizeType new_index = x.push_soa_member(p_elem) - 1;
		soa_size = std::max(soa_size, new_index + 1);
		index_map[new_index] = new_index;
	}

	void push_y(const std::string &p_elem) {
		if (y.size() == soa_capacity) {
			[[unlikely]] soa_realloc();
		}

		SoaVectorSizeType new_index = y.push_soa_member(p_elem) - 1;
		soa_size = std::max(soa_size, new_index + 1);
		index_map[new_index] = new_index;
	}
};

struct MutableTestStructMacro {
	MutableSOA( // This macro expands to the exact same code in MutableTestStruct. It's too big to expand on hover though so can use the clangd code action to compare.
		MutableTestStructMacro, 2,
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

	std::cout << "DynamicSizeSOA with vector: " << ((vec_test.get_a(0).at(1) == 5) ? "Passed\n\n" : "Failed.\n\n");
}

void test_mutable_macro() {
	MutableTestStruct mutable_test;
	mutable_test.init(5);

	for (int i = 0; i < 2; ++i) {
		mutable_test.push_x(i);
		const std::string hello = std::string(std::to_string(i));
		mutable_test.push_y(hello);
	}

	// Test dynamic growth
	mutable_test.push_x(3);
	mutable_test.push_x(9);
	mutable_test.push_x(9);
	mutable_test.push_x(888);
	mutable_test.set_x(0, 999);

	mutable_test.erase(0);
	mutable_test.erase(2);

	std::cout << "MutableSOA size test: " << ((mutable_test.x.size() == 4 and mutable_test.y.size() == 2) ? "Passed\n" : "Failed.\n");
	std::cout << "MutableSOA get after erase: " << ((mutable_test.get_x(5) == 888) ? "Passed\n" : "Failed.\n");

	MutableTestStructMacro mutable_macro_test;
	mutable_macro_test.init(5);

	for (int i = 0; i < 2; ++i) {
		mutable_macro_test.push_x(i);
		const std::string hello = std::string(std::to_string(i));
		mutable_macro_test.push_y(hello);
	}

	// Test dynamic growth
	mutable_macro_test.push_x(3);
	mutable_macro_test.push_x(9);
	mutable_macro_test.push_x(9);
	mutable_macro_test.push_x(888);
	mutable_macro_test.set_x(0, 999);

	// Test erasing
	mutable_macro_test.erase(0);
	mutable_macro_test.erase(2);

	std::cout << "MutableSOA equality test: " << ((mutable_test.get_x(5) == 888 and mutable_macro_test.get_x(5) == 888) ? "Passed\n" : "Failed.\n");
}

int main() {
	test_fixed_sized_macro();
	test_dynamic_sized_macro();
	test_mutable_macro();
	soa_perf_test();
	soa_ranges_test();
	std::cout << "\nTests finished.";
	return 0;
}