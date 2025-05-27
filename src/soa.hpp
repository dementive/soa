#pragma once

#include "SoaVector.hpp"
#include "ForEachMacro.hpp"

using Entity = uint32_t;

#define SOA_VECTOR_TYPE(m_type) SoaVector<m_type, Entity>

#define SOA_TYPES(m_type, m_name) \
	SOA_VECTOR_TYPE(m_type) m_name;

#define SOA_INIT(m_type, m_name) \
	m_name.init(data, p_size, memory_offsets[current_column]); \
	current_column++;

#define SOA_GET_MALLOC_SIZE(m_type, m_name) \
	memory_offsets[mem_offset_idx] = total_size; \
	total_size += sizeof(m_type) * p_size; \
	mem_offset_idx++;

#define SOA_SETGET(m_type, m_name) \
	void set_##m_name(Entity p_index, m_type p_item) { m_name[p_index] = p_item; } \
	m_type get_##m_name(Entity p_index) const { return m_name[p_index]; } \

#define SOA_DESTROY(m_type, m_name) \
	m_name.reset();

// SOA has a problem that you end up doing more total malloc's than with AOS since each individual element gets allocated on the heap instead of whole objects.
// FixedSizeSOA attempts to manage the memory better and can be used to make it so only 1 call to malloc will be made and only when initializing the struct.
// Have to pass in the total number of SOA member variables as first arg.
#define FixedSizeSOA(m_class_name, m_total_columns, ...) \
	FOR_EACH_TWO_ARGS(SOA_TYPES, __VA_OPT__(__VA_ARGS__, )) \
private: \
	int memory_offsets[m_total_columns]{}; \
	void *data{}; \
	constexpr int get_malloc_size(const Entity p_size) { \
		int total_size = 0; \
		int mem_offset_idx = 0; \
		FOR_EACH_TWO_ARGS(SOA_GET_MALLOC_SIZE, __VA_OPT__(__VA_ARGS__, )) \
		return total_size; \
	}	\
public: \
	constexpr void init(const Entity p_size) { \
		data = malloc(get_malloc_size(p_size)); \
		int current_column = 0; \
		FOR_EACH_TWO_ARGS(SOA_INIT, __VA_OPT__(__VA_ARGS__, )) \
	} \
	~m_class_name() { \
		FOR_EACH_TWO_ARGS(SOA_DESTROY, __VA_OPT__(__VA_ARGS__, ))\
		free(data); \
	} \
	FOR_EACH_TWO_ARGS(SOA_SETGET, __VA_OPT__(__VA_ARGS__, ))
