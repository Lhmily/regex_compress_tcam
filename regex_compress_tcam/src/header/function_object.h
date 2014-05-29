/*
 * compare.h
 *
 *  Created on: May 13, 2014
 *      Author: Lhmily
 */

#ifndef COMPARE_H_
#define COMPARE_H_
#include "base.h"
#include <map>
#include <algorithm>
//using namespace std;

class cmp_pair {
public:
	bool operator()(const pair<state, int> &o1, const pair<state, int> &o2) {
		return o1.second >= o2.second;
	}
};

class cmp_state_rate {
public:
	bool operator()(pair<size_t, vector<pair<state, int> >*>* o1,
			pair<size_t, vector<pair<state, int> >*>* o2) {

		vector<pair<state, int> >* a1 = o1->second;
		vector<pair<state, int> >* a2 = o2->second;
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

class for_each_generate_state_rate {
private:
	map<state, int> *state_rate_map;
public:
	for_each_generate_state_rate(map<state, int> *map) :
			state_rate_map(map) {
	}
	void operator()(size_t cur_state) {
		map<state, int>::iterator map_it = state_rate_map->find(cur_state);

		if (map_it != state_rate_map->end()) {
			//++map_it->second;
			map_it->second += 1;
		} else {
			state_rate_map->insert(make_pair(cur_state, 1));
		}
	}
};

class for_each_state_rate_map {
private:
	vector<pair<state, int> >* vector_ptr;
public:
	for_each_state_rate_map(vector<pair<state, int> > *ptr) :
			vector_ptr(ptr) {
	}
	void operator()(pair<state, int> data) {
		vector_ptr->push_back(make_pair(data.first, data.second));
	}
};

class find_if_vector_src_code {
private:
	string &_to_find_src_code;
	size_t _prefix;
public:
	find_if_vector_src_code(string &s, size_t prefix) :
			_to_find_src_code(s), _prefix(prefix) {
	}
	bool operator()(string &s_it) const {
		return 0 == s_it.compare(0, _prefix, _to_find_src_code, 0, _prefix);
	}
};

class find_if_bit {
private:
	size_t _bit;
public:
	find_if_bit(size_t bit) :
			_bit(bit) {
	}
	bool operator()(trans_CODE_ptr trans) {
		return trans->src_code[_bit] != '0';
	}
};

class for_each_map_count {
private:
	map<state, size_t> *_map_count;
public:
	for_each_map_count(map<state, size_t> *map_count) :
			_map_count(map_count) {
	}
	void operator()(trans_CODE_ptr trans) {
		map<state, size_t>::iterator map_it;
		map_it = _map_count->find(trans->dst_code.state);
		if (map_it == _map_count->end()) {
			//not found
			_map_count->insert(make_pair(trans->dst_code.state, 1));
		} else {
			//found
			map_it->second += 1;
		}
	}
};

#endif /* COMPARE_H_ */
