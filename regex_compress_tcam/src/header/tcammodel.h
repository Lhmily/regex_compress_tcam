/*
 * tcammodel.h
 *
 *  Created on: May 24, 2014
 *      Author: Lhmily
 */

#ifndef TCAMMODEL_H_
#define TCAMMODEL_H_

#include "transtable.h"

extern "C" {
#include "tcam-power.h"
#include "leakage_function.h"
}

class tcam_model {
private:
	transtable *_compress_table;
	size_t _state_size;
	size_t _column_size;
	size_t _block_size;
	size_t _state_bits;
	size_t _block_num;
	size_t _trans_num;

	size_t _input_block_num;
//	size_t _state_replace_block_num;
//	size_t _state_replace_block_entry_size;

	size_t *_block_num_each_input;

	vector<state> **_header;

	ofstream *_performance;
	map<size_t, POWER_T> *_power_map;

	size_t _ascii_block_num[256];
	POWER_T _ascii_power[256];

	/*
	 * tcam struct
	 */
	struct parameters _tcam_parameters;
	struct search_power _tcam_search_power;
	struct static_power_worst_case _tcam_leakage_power;
	struct read_power _tcam_read_power;
	struct write_power _tcam_write_power;
	struct search_delay _tcam_search_delay;

private:
	void print_meta_informance(ofstream &fout, size_t entry_bits,
			size_t entry_size) const;

	void print_DFA_performance(ofstream &fout, size_t bits, size_t block_num,
			size_t total_block_number, size_t total_entry_size);
	void print_character_index_performance();
//	void print_state_replace_performance();
	void print_compress_performance();

public:
	tcam_model(transtable *compress_table);

	virtual ~tcam_model();
	void setProperties();
	void init();
	void print();
};

#endif /* TCAMMODEL_H_ */
