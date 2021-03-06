/*
 * transtable.h
 *
 *  Created on: May 13, 2014
 *      Author: Lhmily
 */

#ifndef TRANSTABLE_H_
#define TRANSTABLE_H_

#include <iostream>
#include <set>
#include <iterator>
#include <fstream>
#include <cassert>

#include "stdinc.h"
#include "function_object.h"

class transtable {
private:
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
	size_t _total_block_entry_size;
	size_t **_block_index;
	size_t _input_ascii_dst[256];
	vector<pair<string, size_t> > _input_ascii_compress;

	vector<pair<size_t, size_t*> > **_vector_blocks;

	/*
	 *blocks encode
	 */
	BLOCK_CODE_PTR **_vector_blocks_code;
	BLOCK_CODE_PTR *_transitions;
	/*
	 * statistics information
	 */
	size_t _total_final_transitions_num;
	size_t _total_final_blocks_num;
	size_t *_final_blocks_num_each_input;
private:
	typedef void (transtable::*print_characters_fun)(ofstream &fout,
			size_t index) const;
	//reorder

	void generate_state_rate();

	//compress
	string state_convert_code(state s, const int bits) const;
	void compress_each_block();
	trans_CODE_ptr get_CODE(string src_code, int mask_index, int mask_size,
			size_t dst);

	//print
	void print_table_fun(ofstream &fout, size_t it) const;
	void print_blocks_fun(ofstream &fout, size_t index) const;
	void print_characters(ofstream &fout, print_characters_fun fun) const;
	void print_transitions_fun(ofstream &fout, size_t it) const;

	//encode
	void handle_block_code(size_t *block, int index, int size,
			size_t block_index, BLOCK_CODE_PTR vector_code);
	void handle_each_block_code(BLOCK_CODE_PTR cur_block, int mask_index,
			int mask_size);
	void handle_each_input(size_t *block, int index, int size,
			BLOCK_CODE_PTR vector_code);
	size_t generate_default_transition(BLOCK_CODE_PTR vector_code, size_t index,
			size_t length, int mask_bit);
	size_t default_transition_compress(BLOCK_CODE_PTR vector_code, size_t index,
			size_t length, int mask_bit);

	void default_transition_compression(BLOCK_CODE_PTR vector_code);
	size_t handle_defcompr(BLOCK_CODE_PTR vector_code, size_t index,
			size_t prefix_conut, size_t &len);
	void generate_input_ascii_dst();
	void handle_input_ascii_compress(int index, int end);

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

	void compress_index_table();
	void print_index_table();
	//encode
	void generate_bolcks_code(size_t block_size);

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

	void print_transitions(ofstream &fout) const;
	/*
	 * getters and setters
	 */
	size_t getTotalFinalBlocksNum() const;

	size_t getTotalFinalTransitionsNum() const;
	size_t getStateBits() const;
	vector<state> ** getHeader() const;
	size_t getBlockSize() const;
	size_t getInputBlockSize() const;
	size_t* getFinalBlocksNumEachInput() const;
	size_t getColumnSize() const;
	size_t getTotalBlockNum() const;
	size_t getTotalBlockEntrySize() const;

	size_t getIndexTableSize() const;

	//release
	void release_state_rate();
	void release_blocks();
	void release_vector_blocks_code();
	void release_transitions();
};

#endif /* TRANSTABLE_H_ */
