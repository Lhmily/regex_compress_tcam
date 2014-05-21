/*
 * compare.h
 *
 *  Created on: May 13, 2014
 *      Author: Lhmily
 */

#ifndef COMPARE_H_
#define COMPARE_H_
#include "base.h"
#include <stdio.h>

template<typename T>
class cmp_pair {
public:
	bool operator()(const T &o1, const T &o2) const {
		return o1.second >= o2.second;
	}
};

class cmp_state_rate {
public:
	bool operator()(pair_index_rate* o1, pair_index_rate* o2) const {

		vector_pair_rate* a1 = o1->second;
		vector_pair_rate* a2 = o2->second;
		int size_a = a1->size();
		int size_b = a2->size();
		int size_min = size_a < size_b ? size_a : size_b;

		int rate_a = 0, rate_b = 0;
		for (int it = 0; it < size_min; ++it) {
			rate_a = a1->at(it).second;
			rate_b = a2->at(it).second;
			if (rate_a != rate_b) {

				return rate_a > rate_b;
			}
		}
		if (size_a == size_b) {
			return o1->first < o2->first;
		}
		return size_a < size_b;
	}
};

class remove_copy_if_reorder {
private:
	size_t state;
public:
	remove_copy_if_reorder(size_t s) :
			state(s) {
	}
	bool operator ()(pair_index_rate* obj) {

		return obj->second->at(0).first == state ? false : true;
	}
};
class remove_if_reorder: remove_copy_if_reorder {
public:
	remove_if_reorder(size_t s) :
			remove_copy_if_reorder(s) {
	}
	bool operator ()(pair_index_rate* obj) {

		return !(remove_copy_if_reorder::operator ()(obj));
	}
};

class find_if_generate_blocks {
private:
	size_t size;
	state_t **table;
public:
	find_if_generate_blocks(state_t **table_t, size_t n) :
			table(table_t), size(n) {
	}
	bool operator ()(pair<size_t, size_t*> obj, size_t index_start,
			size_t ascii) {
		size_t obj_size = obj.first;
		size_t *obj_data = obj.second;
		if (obj_size == size) {
			for (size_t it = 0; it < size; ++it) {
				if (table[index_start + it][ascii] != obj_data[it])
					return false;
			}
			return true;
		}
		return false;
	}
};
#endif /* COMPARE_H_ */
