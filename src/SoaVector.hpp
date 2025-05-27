#pragma once

// Specialized vector for SOA structs. 
#include <cstddef>
#include <cstdint>
#include <type_traits>

template <typename T, typename U = uint32_t>
class SoaVector {
private:
	U count = 0;
	T *data = nullptr;

public:
	void init(void *p_data, U p_size, int memory_offset) {
	    data = reinterpret_cast<T*>(static_cast<std::byte*>(p_data) + memory_offset);
	    count = p_size;
	}

	T *ptr() { return data; }
	const T *ptr() const { return data; }

	inline void clear() {
		U p_size = 0;
		if (p_size < count) {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				for (U i = p_size; i < count; i++) {
					data[i].~T();
				}
			}
			count = p_size;
		}
	}

	inline void reset() {
		clear();
		if (data) {
			data = nullptr;
		}
	}

	inline bool is_empty() const { return count == 0; }

	inline U size() const { return count; }
	inline const T &operator[](U p_index) const {
		return data[p_index];
	}
	inline T &operator[](U p_index) {
		return data[p_index];
	}

	int64_t find(const T &p_val, U p_from = 0) const {
		for (U i = p_from; i < count; i++) {
			if (data[i] == p_val) {
				return int64_t(i);
			}
		}
		return -1;
	}

	bool has(const T &p_val) const {
		return find(p_val) != -1;
	}

	struct Iterator {
		inline T &operator*() const {
			return *elem_ptr;
		}
		inline T *operator->() const { return elem_ptr; }
		inline Iterator &operator++() {
			elem_ptr++;
			return *this;
		}
		inline Iterator &operator--() {
			elem_ptr--;
			return *this;
		}

		inline bool operator==(const Iterator &b) const { return elem_ptr == b.elem_ptr; }
		inline bool operator!=(const Iterator &b) const { return elem_ptr != b.elem_ptr; }

		Iterator(T *p_ptr) { elem_ptr = p_ptr; }
		Iterator() = default;
		Iterator(const Iterator &p_it) { elem_ptr = p_it.elem_ptr; }

	private:
		T *elem_ptr = nullptr;
	};

	struct ConstIterator {
		inline const T &operator*() const {
			return *elem_ptr;
		}
		inline const T *operator->() const { return elem_ptr; }
		inline ConstIterator &operator++() {
			elem_ptr++;
			return *this;
		}
		inline ConstIterator &operator--() {
			elem_ptr--;
			return *this;
		}

		inline bool operator==(const ConstIterator &b) const { return elem_ptr == b.elem_ptr; }
		inline bool operator!=(const ConstIterator &b) const { return elem_ptr != b.elem_ptr; }

		ConstIterator(const T *p_ptr) { elem_ptr = p_ptr; }
		ConstIterator() = default;
		ConstIterator(const ConstIterator &p_it) { elem_ptr = p_it.elem_ptr; }

	private:
		const T *elem_ptr = nullptr;
	};

	inline Iterator begin() { return {data}; }
	inline Iterator end() { return {data + size()}; }

	inline ConstIterator begin() const { return {ptr()}; }
	inline ConstIterator end() const { return {ptr() + size()}; }

	inline SoaVector() = default;
	inline ~SoaVector() {
		if (data) {
			reset();
		}
	}
};