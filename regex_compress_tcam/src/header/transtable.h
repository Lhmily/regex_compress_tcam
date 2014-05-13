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
#include "stdinc.h"
#include <vector>

using std::vector;
using std::cout;
using std::endl;
typedef size_t state;

class transtable {
private:
	size_t _state_size;
	size_t _column_size;

	state_t **_table;
	vector<state> **_header;



public:
	transtable();
	virtual ~transtable();
	size_t getStateSize() const;
	//
	void handle_table(state_t **state_table, int size);
	void print_table(FILE *fptr) const;

};

#endif /* TRANSTABLE_H_ */
