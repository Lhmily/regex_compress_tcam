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

	//
	int _block_size;
	size_t _block_num;
	size_t **_block_index;
	vector<pair<size_t, size_t*> > *_vector_blocks;

private:
	typedef void (transtable::*print_characters_fun)(FILE *fptr,
			size_t index) const;
	void generate_state_rate();
	void release_state_rate();
	void print_table_fun(FILE *fptr, size_t it) const;
	void print_blocks_fun(FILE *fptr, size_t index) const;
	void print_characters(FILE *fptr, print_characters_fun fun) const;

public:
	transtable();
	virtual ~transtable();
	size_t getStateSize() const;
	//
	void handle_table(state_t **state_table, int size);

	void replace_table();
	void generate_blocks(int block_size);
	//print to file
	/*
	 * print the transition table after compressed based on input character.
	 * print some statistics data at the same time.
	 */
	void print_table(FILE *fptr) const;
	/*
	 * print the blocks detail and 256 input characters block index after calling generate_blocks().
	 */
	void print_blocks(FILE *fptr) const;

};

#endif /* TRANSTABLE_H_ */
