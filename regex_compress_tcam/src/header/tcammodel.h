/*
 * tcammodel.h
 *
 *  Created on: May 24, 2014
 *      Author: Lhmily
 */

#ifndef TCAMMODEL_H_
#define TCAMMODEL_H_

#include "transtable.h"
#include <iomanip>
extern "C" {
#include "tcam-power.h"
#include "leakage.h"
}

using namespace std;
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

	size_t *_block_num_each_input;

	size_t _ascii_block_num[256];
	vector<state> **_header;

	ofstream *_performance;

	/*
	 * tcam struct
	 */
	struct parameters tcam_parameters;
	struct search_power tcam_search_power;
	struct static_power_worst_case tcam_leakage_power;
	struct read_power tcam_read_power;
	struct write_power tcam_write_power;
	struct search_delay tcam_search_delay;
private:
	void print_DFA_performance(ofstream &fout, size_t bits, size_t block_num);
	void print_character_index_performance();
	void print_state_replace_performance();
	void print_unique_compress_performance();
public:
	tcam_model(transtable *compress_table);

	virtual ~tcam_model();
	void setBlockNum();
	void init();
	void print();
};

#endif /* TCAMMODEL_H_ */
