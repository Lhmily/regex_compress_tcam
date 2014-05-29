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
	_tcam_parameters.CMOS_tech = 0.18;
	_tcam_parameters.Nrd = 1;
	init_leakage_params(_tcam_parameters.CMOS_tech);

	_state_size = _compress_table->getStateSize();
	_column_size = _compress_table->getColumnSize();
	_header = _compress_table->getHeader();
	_state_bits = _compress_table->getStateBits();

	_power_map = new map<size_t, POWER_T>;

	_performance = new ofstream[3];
	for (size_t it = 0; it < 3; ++it) {
		switch (it) {
		case 0:
			_performance[it].open("Performance_DFA.txt");
			this->print_meta_informance(_performance[it], _state_bits + 8,
					_state_size * 256);
			break;
		case 1:
			_performance[it].open("Performance_Character_Index.txt");
			this->print_meta_informance(_performance[it], _state_bits,
					_state_size * _column_size);
			break;
//		case 2:
//			_performance[it].open("Performance_State_Replace.txt");
//
//			_performance[it] << "State size: " << _state_size << endl
//					<< "Entry bits:" << _state_bits << endl
//					<< "Blocks size\tns\tnj\t#start blocks number\ttotal blocks number\ttotal entry size";
//
//			break;
		default:

			_performance[it].open("Performance_Compress.txt");
			_performance[it] << "State size: " << _state_size << endl
					<< "Entry bits:" << _state_bits << endl << "Blocks size"
					<< "\t" << "avg(ns)\tavg(nj)\tavg(#start)" << "\t"
					<< "max(ns)\tmax(nj)\tmax(#start)" << "\t"
					<< "min(ns)\tmin(nj)\tmin(#start)" << "\t"
					<< "mid(ns)\tmid(nj)\tmid(#start)" << "\t"
					<< "total blocks number" << "\t" << "total entry size";
		}
	}

}
void tcam_model::print_meta_informance(ofstream &fout, size_t entry_bits,
		size_t entry_size) const {
	fout << "State size: " << _state_size << endl << "Entry bits:" << entry_bits
			<< endl << "Total entries size: " << entry_size << endl
			<< "Blocks size\tns\tnj\t#start blocks number\ttotal blocks number";
}
void tcam_model::setProperties() {

	_block_num = _compress_table->getTotalFinalBlocksNum();

	_block_size = _compress_table->getBlockSize();
	//_input_block_num = _compress_table->getInputBlockSize();
	_input_block_num = ceil(_state_size * 1.0 / _block_size);
	//_state_replace_block_num = _compress_table->getTotalBlockNum();
	_trans_num = _compress_table->getTotalFinalTransitionsNum();
	_block_num_each_input = _compress_table->getFinalBlocksNumEachInput();
	//_state_replace_block_entry_size = _compress_table->getTotalBlockEntrySize();

	size_t cur_input_size = 0;
	int ascii_index = 0;
	for (size_t it = 0; it < _column_size; ++it) {
		cur_input_size = _header[it]->size();
		for (size_t jt = 0; jt < cur_input_size; ++jt) {
			_ascii_block_num[ascii_index++] = _block_num_each_input[it];
		}
	}
	stable_sort(_ascii_block_num, _ascii_block_num + 256);

}
tcam_model::~tcam_model() {
// TODO Auto-generated destructor stub
	for (size_t it = 0; it < 3; ++it) {
		_performance[it].close();
	}
	delete[] _performance;
	delete _power_map;
}
void tcam_model::print() {

	this->setProperties();
	size_t dfa_total_blocks_num = ceil(_state_size * 256.0 / _block_size);
	this->print_DFA_performance(_performance[0], _state_bits + 8,
			dfa_total_blocks_num, dfa_total_blocks_num, 0);
	this->print_character_index_performance();
	//this->print_state_replace_performance();

	this->print_compress_performance();
}
void tcam_model::print_DFA_performance(ofstream &fout, size_t bits,
		size_t block_num, size_t total_block_number, size_t total_entry_size) {
	_tcam_parameters.number_of_column_bits = bits;

	_tcam_parameters.number_of_subbanks = block_num;

	_tcam_parameters.number_of_rows = block_num * _block_size;

	calculate_tcam_power(_tcam_parameters, &_tcam_search_power,
			&_tcam_leakage_power, &_tcam_write_power, &_tcam_read_power,
			&_tcam_search_delay);
	fout << endl << "\t" << _block_size << "\t"
			<< _tcam_search_delay.max_delay * 1e9 << "\t"
			<< _tcam_search_power.total_power * 1e9 << "\t" << block_num << "\t"
			<< total_block_number;
	if (0 != total_entry_size) {
		fout << "\t" << total_entry_size;
	}
}
void tcam_model::print_character_index_performance() {
	this->print_DFA_performance(_performance[1], _state_bits, _input_block_num,
			_input_block_num * _column_size, 0);
}
//void tcam_model::print_state_replace_performance() {
//	this->print_DFA_performance(_performance[2], _state_bits, _input_block_num,
//			_state_replace_block_num, _state_replace_block_entry_size);
//}
void tcam_model::print_compress_performance() {
	_tcam_parameters.number_of_column_bits = _state_bits;
	_power_map->clear();

	map<size_t, POWER_T>::iterator map_it;
	POWER_T power;
	size_t final_block_num = 0;

	for (size_t it = 0; it < _column_size; ++it) {
		final_block_num += _block_num_each_input[it];
		map_it = _power_map->find(_block_num_each_input[it]);
		if (map_it == _power_map->end()) {

			_tcam_parameters.number_of_subbanks = _block_num_each_input[it];
			_tcam_parameters.number_of_rows = _block_num_each_input[it]
					* _block_size;

			calculate_tcam_power(_tcam_parameters, &_tcam_search_power,
					&_tcam_leakage_power, &_tcam_write_power, &_tcam_read_power,
					&_tcam_search_delay);

			power.search_power = _tcam_search_power.total_power * 1e9;
			power.search_time = _tcam_search_delay.max_delay * 1e9;

			_power_map->insert(make_pair(_block_num_each_input[it], power));

		}
	}
	size_t cur_input_size = 0, ascii_index = 0;

	for (size_t it = 0; it < _column_size; ++it) {
		cur_input_size = _header[it]->size();

		map_it = _power_map->find(_block_num_each_input[it]);

		for (size_t jt = 0; jt < cur_input_size; ++jt) {
			_ascii_power[ascii_index].search_power =
					map_it->second.search_power;
			_ascii_power[ascii_index++].search_time =
					map_it->second.search_time;
		}
	}
	stable_sort(_ascii_power, _ascii_power + 256,
			[](const POWER_T &x,const POWER_T &y) {
				double epsilon = 1e-7;
				double difference = x.search_power - y.search_power;

				return !(difference > epsilon);
			});

	_performance[2] << endl << _block_size << "\t";

	double sum_power = 0.0, sum_time = 0.0;
	size_t sum_block = 0;
	for (int i = 0; i < 256; ++i) {
		sum_power += _ascii_power[i].search_power;
		sum_time += _ascii_power[i].search_time;
		sum_block += _ascii_block_num[i];
	}
	size_t avg_block_num = ceil(sum_block / 256.0);
	double mid_power = (_ascii_power[127].search_power
			+ _ascii_power[128].search_power) / 2;

	double mid_time = (_ascii_power[127].search_time
			+ _ascii_power[128].search_time) / 2;
	size_t mid_block = ceil(
			(_ascii_block_num[127] + _ascii_block_num[128]) / 2.0);

	for (int j = 0; j < 4; j++) {
		switch (j) {
		case 0:
			_performance[2] << sum_time / 256.0 << "\t" << sum_power / 256.0
					<< "\t" << avg_block_num << "\t";
			break;
		case 1:
			_performance[2] << _ascii_power[255].search_time << "\t"
					<< _ascii_power[255].search_power << "\t"
					<< _ascii_block_num[255] << "\t";
			break;
		case 2:
			_performance[2] << _ascii_power[0].search_time << "\t"
					<< _ascii_power[0].search_power << "\t"
					<< _ascii_block_num[0] << "\t";
			break;
		case 3:
			_performance[2] << mid_time << "\t" << mid_power << "\t"
					<< mid_block << "\t";
			break;
		}
	}
	_performance[2] << final_block_num << "\t" << _trans_num;
}
