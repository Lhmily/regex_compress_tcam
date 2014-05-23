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
#include <string>
#include <set>
#include <iterator>
#include <fstream>
#include <cassert>
#include "stdinc.h"
#include "function_object.h"
#include "base.h"

using namespace std;

class transtable {
private:
	/*
	 * pair<src code, dst code>
	 */

	typedef struct {
		size_t state;
		string code;
	} DST_CODE;

	typedef string SRC;
	typedef struct {
		SRC src_code;
		DST_CODE dst_code;
	} trans_CODE, *trans_CODE_ptr;
	typedef vector<trans_CODE_ptr> BLOCK_CODE;

	typedef BLOCK_CODE *BLOCK_CODE_PTR;
	/*
	 * base information
	 */
	size_t _state_size;
	size_t _column_size;

	state_t **_table;
	vector<state> **_header;

	pair<size_t, vector<pair<state, int> >*> **_state_rate;

	/*
	 * blocks information
	 */
	int _block_size;
	int _block_bits;
	int _state_bits;
	size_t _block_num;
	size_t _total_block_num;
	size_t **_block_index;

	vector<pair<size_t, size_t*> > **_vector_blocks;

	/*
	 *blocks encode
	 */
	BLOCK_CODE_PTR **_vector_blocks_code;
private:
	typedef void (transtable::*print_characters_fun)(ofstream &fout,
			size_t index) const;
	//reorder

	void generate_state_rate();

	//compress
	string state_convert_code(state s, const int bits) const;

	//print
	void print_table_fun(ofstream &fout, size_t it) const;
	void print_blocks_fun(ofstream &fout, size_t index) const;
	void print_characters(ofstream &fout, print_characters_fun fun) const;

	//release
	void release_state_rate();
	void release_blocks();

public:
	transtable();
	virtual ~transtable();
	size_t getStateSize() const;
	//
	void handle_table(state_t **state_table, int size);
	void reorder();

	void replace_table();
	void generate_blocks(int block_size);
	void compress_blocks();
	void handle_block_code(const size_t *block, int index, int size,
			size_t block_index, BLOCK_CODE_PTR vector_code);

	//print to file
	/*
	 * print the transition table after compressed based on input character.
	 * print some statistics data at the same time.
	 */
	void print_table(ofstream &fout) const;
	/*
	 * print the blocks detail and 256 input characters block index after calling generate_blocks().
	 */
	void print_blocks(ofstream &fout) const;
	void print_blocks_code(ofstream &fout) const;

};

#endif /* TRANSTABLE_H_ */
