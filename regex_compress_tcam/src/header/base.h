/*
 * base.h
 *
 *  Created on: May 13, 2014
 *      Author: Lhmily
 */

#ifndef BASE_H_
#define BASE_H_

#include <stddef.h>
#include <utility>
#include <vector>

using std::pair;
using std::vector;

typedef size_t state;
typedef pair<state, int> pair_rate;
typedef vector<pair_rate> vector_pair_rate;
typedef pair<size_t,vector_pair_rate*> pair_index_rate;
typedef vector<pair_index_rate*> vector_index_rate;


#endif /* BASE_H_ */
