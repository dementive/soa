#pragma once

// Specialized vector for SOA structs. 
#include <cstddef>
#include <cstdint>
#include <type_traits>

template <typename T, bool FixedSize, typename U = uint32_t>
class SoaVector {
private:
	U count = 0;
	T *data = nullptr;

public:
	// Do not use this directly, it has to be public. Use push_X in the SOA struct instead.
	void push_soa_member(const T &p_elem) requires(!FixedSize) {
		if constexpr (!std::is_trivially_constructible_v<T>) {
			new (&data[count++]) T(p_elem);
		} else {
			data[count++] = p_elem;
		}
	}

	void soa_realloc(void *p_data) requires(!FixedSize) {
		data = reinterpret_cast<T*>(static_cast<std::byte*>(p_data));
	}

	void init(void *p_data, U p_size, int p_memory_offset) {
	    data = reinterpret_cast<T*>(static_cast<std::byte*>(p_data) + p_memory_offset);
	    if constexpr (FixedSize) {
	    	count = p_size;
	    }
	}

	void *get_data() { return data; }
	T *ptr() { return data; }
	const T *ptr() const { return data; }

	inline void clear() {
		U p_size = 0;
		if (data == nullptr) {
			count = p_size;
			return;
		}

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

	U find(const T &p_val, U p_from = 0) const {
		for (U i = p_from; i < count; i++) {
			if (data[i] == p_val) {
				return i;
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
		// if (data) {
		// 	reset();
		// }
	}
};