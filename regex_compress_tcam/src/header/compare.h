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

				return rate_a >= rate_b;
			}
		}

		return true;
	}
};
#endif /* COMPARE_H_ */
