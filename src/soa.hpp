#pragma once

#include "SoaVector.hpp"
#include "ForEachMacro.hpp"

using SoaVectorSizeType = uint32_t;

#define SOA_FIXED_VECTOR_TYPE(m_type) SoaVector<m_type, true, SoaVectorSizeType>
#define SOA_DYNAMIC_VECTOR_TYPE(m_type) SoaVector<m_type, false, SoaVectorSizeType>

#define SOA_FIXED_TYPES(m_type, m_name) \
	SOA_FIXED_VECTOR_TYPE(m_type) m_name;

#define SOA_DYNAMIC_TYPES(m_type, m_name) \
	SOA_DYNAMIC_VECTOR_TYPE(m_type) m_name;

#define SOA_INIT(m_type, m_name) \
	m_name.init(data, p_size, memory_offsets[current_column]); \
	current_column++;

#define SOA_GET_MALLOC_SIZE(m_type, m_name) \
	memory_offsets[mem_offset_idx] = total_size; \
	total_size += sizeof(m_type) * p_size; \
	mem_offset_idx++;

#define SOA_SETGET(m_type, m_name) \
	void set_##m_name(SoaVectorSizeType p_index, const m_type &p_item) { m_name[p_index] = p_item; } \
	m_type get_##m_name(SoaVectorSizeType p_index) { return m_name[p_index]; } \
	const m_type &get_##m_name(SoaVectorSizeType p_index) const { return m_name[p_index]; }

#define SOA_PUSH(m_type, m_name) \
	void push_##m_name(const m_type &p_elem) { \
		if (m_name.size() == soa_capacity) [[unlikely]] { \
			soa_realloc(soa_capacity); \
		} \
		m_name.push_soa_member(p_elem); \
	}

#define SOA_REALLOC(m_type, m_name) \
	m_name.soa_realloc(new_data, memory_offsets[current_column], starting_capacity); \
	current_column++;

#define SOA_DESTROY(m_type, m_name) m_name.reset();

#define DynamicSOA(m_class_name, m_total_columns, ...) \
	FOR_EACH_TWO_ARGS(SOA_DYNAMIC_TYPES, __VA_OPT__(__VA_ARGS__, )) \
private: \
	int memory_offsets[m_total_columns]{}; \
	int total_objects_size{}; \
	void *data{}; \
	int soa_capacity = 0; \
	int get_soa_malloc_size(const SoaVectorSizeType p_size) { \
		int total_size = 0; \
		int mem_offset_idx = 0; \
		FOR_EACH_TWO_ARGS(SOA_GET_MALLOC_SIZE, __VA_OPT__(__VA_ARGS__, )) \
		total_objects_size = total_size / p_size; \
		return total_size; \
	}	\
	void soa_realloc(int p_size) { \
		int starting_capacity = soa_capacity; \
	    if (soa_capacity == 0) { \
	        soa_capacity = 1; \
	    } else { \
	        soa_capacity = static_cast<int>(soa_capacity * 1.5); \
	    } \
 		\
	    \
	    void *new_data = malloc(soa_capacity * total_objects_size); \
	    get_soa_malloc_size(soa_capacity); \
	    int current_column = 0; \
	    FOR_EACH_TWO_ARGS(SOA_REALLOC, __VA_OPT__(__VA_ARGS__, )) \
	    free(data); \
	    data = new_data; \
	} \
public: \
	void init(const SoaVectorSizeType p_size) { \
		data = malloc(get_soa_malloc_size(p_size)); \
		soa_capacity = p_size; \
		int current_column = 0; \
		FOR_EACH_TWO_ARGS(SOA_INIT, __VA_OPT__(__VA_ARGS__, )) \
	} \
	~m_class_name() { \
		FOR_EACH_TWO_ARGS(SOA_DESTROY, __VA_OPT__(__VA_ARGS__, )) \
		free(data); \
	} \
	FOR_EACH_TWO_ARGS(SOA_SETGET, __VA_OPT__(__VA_ARGS__, )) \
	FOR_EACH_TWO_ARGS(SOA_PUSH, __VA_OPT__(__VA_ARGS__, ))

#define FixedSizeSOA(m_class_name, m_total_columns, ...) \
	FOR_EACH_TWO_ARGS(SOA_FIXED_TYPES, __VA_OPT__(__VA_ARGS__, )) \
private: \
	int memory_offsets[m_total_columns]{}; \
	void *data{}; \
	int get_soa_malloc_size(const SoaVectorSizeType p_size) { \
		int total_size = 0; \
		int mem_offset_idx = 0; \
		FOR_EACH_TWO_ARGS(SOA_GET_MALLOC_SIZE, __VA_OPT__(__VA_ARGS__, )) \
		return total_size; \
	}	\
public: \
	void init(const SoaVectorSizeType p_size) { \
		data = malloc(get_soa_malloc_size(p_size)); \
		int current_column = 0; \
		FOR_EACH_TWO_ARGS(SOA_INIT, __VA_OPT__(__VA_ARGS__, )) \
	} \
	~m_class_name() { \
		FOR_EACH_TWO_ARGS(SOA_DESTROY, __VA_OPT__(__VA_ARGS__, )) \
		free(data); \
	} \
	FOR_EACH_TWO_ARGS(SOA_SETGET, __VA_OPT__(__VA_ARGS__, ))
