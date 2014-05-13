/*
 * transtable.h
 *
 *  Created on: May 13, 2014
 *      Author: Lhmily
 */

#ifndef TRANSTABLE_H_
#define TRANSTABLE_H_

#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "stdinc.h"
#include "compare.h"
#include "base.h"

using std::map;
using std::make_pair;
using std::cout;
using std::endl;




class transtable {
private:
	size_t _state_size;
	size_t _column_size;

	state_t **_table;
	vector<state> **_header;

	vector_index_rate *_state_rate;

public:
	transtable();
	virtual ~transtable();
	size_t getStateSize() const;
	//
	void handle_table(state_t **state_table, int size);
	void print_table(FILE *fptr) const;

};

#endif /* TRANSTABLE_H_ */
