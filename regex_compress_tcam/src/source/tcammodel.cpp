/*
 * tcammodel.cpp
 *
 *  Created on: May 24, 2014
 *      Author: Lhmily
 */

#include "../header/tcammodel.h"

tcam_model::tcam_model(transtable *compress_table) :
		_compress_table(compress_table) {
	// TODO Auto-generated constructor stub

}
void tcam_model::init() {
	if (NULL == _compress_table)
		return;

	_block_size = _compress_table->getBlockSize();
	_state_size = _compress_table->getStateSize();
	_column_size = _compress_table->getColumnSize();
	_state_bits = _compress_table->getStateBits();

	_input_block_num = _compress_table->getInputBlockSize();

	_trans_num = _compress_table->getTotalFinalTransitionsNum();
	_header = _compress_table->getHeader();
	_block_num_each_input = _compress_table->getFinalBlocksNumEachInput();

	tcam_parameters.CMOS_tech = 0.18;
	tcam_parameters.Nrd = 1;
	init_leakage_params(tcam_parameters.CMOS_tech);

	_performance = new ofstream[4];
	for (size_t it = 0; it < 4; ++it) {
		switch (it) {
		case 0:
			_performance[it].open("Performance_DFA.txt");
			break;
		case 1:
			_performance[it].open("Performance_Character_Index.txt");
			break;
		case 2:
			_performance[it].open("Performance_State_Replace.txt");
			break;
		default:
			_performance[it].open("Performance_Unique_Compress.txt");
		}
	}
	int cur_input_size = 0;
	int ascii_index = 0;
	for (size_t it = 0; it < _column_size; ++it) {
		cur_input_size = _header[it]->size();
		for (size_t jt = 0; jt < cur_input_size; ++jt) {
			_ascii_block_num[ascii_index++] = _block_num_each_input[it];
		}
	}
	sort(_ascii_block_num, _ascii_block_num + 256);
}

void tcam_model::setBlockNum() {
	_block_num = _compress_table->getTotalFinalBlocksNum();

}
tcam_model::~tcam_model() {
// TODO Auto-generated destructor stub
	for (size_t it = 0; it < 4; ++it) {
		_performance[it].close();
	}
	delete[] _performance;
}
void tcam_model::print() {

}
void tcam_model::print_DFA_performance(ofstream &fout, size_t bits,
		size_t block_num) {
	tcam_parameters.number_of_column_bits = bits;

	tcam_parameters.number_of_subbanks = block_num;
	tcam_parameters.number_of_rows = block_num * _block_size;

	calculate_tcam_power(tcam_parameters, &tcam_search_power,
			&tcam_leakage_power, &tcam_write_power, &tcam_read_power,
			&tcam_search_delay);
	fout << endl << "\t" << _block_size << "\t"
			<< tcam_search_delay.max_delay * 1e9 << "\t"
			<< tcam_search_power.total_power * 1e9 << block_num;
}
void tcam_model::print_character_index_performance() {
	this->print_DFA_performance(_performance[1], _state_bits, _input_block_num);
}
void tcam_model::print_state_replace_performance() {
	this->print_DFA_performance(_performance[2], _state_bits, _input_block_num);
}
void tcam_model::print_unique_compress_performance() {

}
