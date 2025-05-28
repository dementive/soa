#pragma once

// Specialized vector for SOA structs.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <type_traits>

template <typename T, bool FixedSize, typename U = uint32_t> class SoaVector {
private:
	U count = 0;
	T *data = nullptr;

public:
	// Do not use this directly, it has to be public. Use push_X in the SOA struct instead.
	void push_soa_member(const T &p_elem) requires(!FixedSize) {
		if constexpr (!std::is_trivially_constructible_v<T>) {
			new (&data[count++]) T(p_elem);
		} else {
			data[count++] = std::move(p_elem);
		}
	}

	// This can't do a normal realloc, it has to either memcpy or move the bytes otherwise the offsets break.
	void soa_realloc(void *new_data, uint64_t p_memory_offset, int p_starting_capacity) {
		if constexpr (std::is_trivially_copyable_v<T>) {
			data = reinterpret_cast<T *>(memcpy(static_cast<std::byte *>(new_data) + p_memory_offset, data, p_starting_capacity * sizeof(int)));
		} else {
			T *new_column_data = reinterpret_cast<T *>(static_cast<std::byte *>(new_data) + p_memory_offset);
			for (int i = 0; i < size(); i++) {
				new (&new_column_data[i]) T(std::move(data[i]));
			}
			data = new_column_data;
		}
	}

	void init(void *p_data, U p_size, uint64_t p_memory_offset) {
		data = reinterpret_cast<T *>(static_cast<std::byte *>(p_data) + p_memory_offset);
		if constexpr (FixedSize) {
			count = p_size;
		}
	}

	void *get_data() { return data; }
	T *ptr() { return data; }
	[[nodiscard]] const T *ptr() const { return data; }

	void clear() {
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
		}
		count = p_size;
	}

	void reset() {
		clear();
		if (data) {
			data = nullptr;
		}
	}

	[[nodiscard]] bool is_empty() const { return count == 0; }

	[[nodiscard]] U size() const { return count; }

	const T &operator[](U p_index) const { return data[p_index]; }
	T &operator[](U p_index) { return data[p_index]; }

	[[nodiscard]] U find(const T &p_val, U p_from = 0) const {
		for (U i = p_from; i < count; i++) {
			if (data[i] == p_val) {
				return i;
			}
		}
		return -1;
	}

	[[nodiscard]] bool has(const T &p_val) const { return find(p_val) != -1; }

	// Iterator API (satisfies std::ranges::contiguous_range constraints https://stackoverflow.com/a/75061822)
	template <bool IsConst> class Iterator {
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = std::conditional_t<IsConst, const T *, T *>;
		using reference = std::conditional_t<IsConst, const T &, T &>;
		using iterator_category = std::contiguous_iterator_tag;

		Iterator(pointer p_ptr) : elem_ptr(p_ptr) {}
		Iterator() = default;

		reference operator*() const { return *elem_ptr; }
		pointer operator->() const { return elem_ptr; }

		Iterator &operator++() {
			++elem_ptr;
			return *this;
		}

		Iterator operator++(int) {
			Iterator temp = *this;
			++(*this);
			return temp;
		}

		Iterator &operator--() {
			--elem_ptr;
			return *this;
		}

		Iterator operator--(int) {
			Iterator temp = *this;
			--(*this);
			return temp;
		}

		// Random access operations
		Iterator operator+(const difference_type n) const { return Iterator(elem_ptr + n); }
		friend Iterator operator+(const difference_type value, const Iterator &other) { return other + value; }
		Iterator operator-(const difference_type n) const { return Iterator(elem_ptr - n); }
		difference_type operator-(const Iterator &other) const { return elem_ptr - other.elem_ptr; }

		// Compound assignment
		Iterator &operator+=(const difference_type n) {
			elem_ptr += n;
			return *this;
		}
		Iterator &operator-=(const difference_type n) {
			*this -= n;
			return *this;
		}

		// Subscript operator
		reference operator[](const difference_type n) const requires(IsConst) { return *(elem_ptr + n); }

		// Comparison operators
		bool operator==(const Iterator &other) const { return elem_ptr == other.elem_ptr; }
		bool operator!=(const Iterator &other) const { return !(*this == other); }
		bool operator<(const Iterator &other) const { return elem_ptr < other.elem_ptr; }
		bool operator>(const Iterator &other) const { return elem_ptr > other.elem_ptr; }
		bool operator<=(const Iterator &other) const { return !(*this > other); }
		bool operator>=(const Iterator &other) const { return !(*this < other); }
		constexpr auto operator<=>(const Iterator &rhs) const = default;

	private:
		pointer elem_ptr = nullptr;
	};

	Iterator<false> begin() { return Iterator<false>(data); }
	Iterator<false> end() { return Iterator<false>(data + size()); }
	[[nodiscard]] Iterator<true> begin() const { return Iterator<true>(ptr()); }
	[[nodiscard]] Iterator<true> end() const { return Iterator<true>(ptr() + size()); }
};