/*
 * transtable.cpp
 *
 *  Created on: May 13, 2014
 *      Author: Lhmily
 */

#include "../header/transtable.h"

transtable::transtable() {
	// TODO Auto-generated constructor stub
	_table = NULL;
	_header = NULL;
	_state_rate = NULL;
}

size_t transtable::getStateSize() const {
	return _state_size;
}

transtable::~transtable() {
	// TODO Auto-generated destructor stub
	if (NULL != _header) {
		for (size_t it = 0; it < _column_size; ++it)
			delete _header[it];
		delete[] _header;
		_header = NULL;
	}

	if (NULL != _table) {
		for (size_t it = 0; it < _state_size; ++it)
			delete[] _table[it];
		delete[] _table;
		_table = NULL;
	}

	if (NULL != _state_rate) {
		for (vector_index_rate::iterator it = _state_rate->begin();
				it != _state_rate->end(); ++it)
			delete (*it)->second;
		delete _state_rate;
		_state_rate = NULL;
	}
}

void transtable::handle_table(state_t **state_table, int size) {
	_state_size = size;

	int row_size = 1;

	vector<state> *compress_index[256];
	compress_index[0] = new vector<state>(1, 0);

	bool is_exist = false;
	int find_index = 0;

	for (int ascii = 1; ascii < 256; ++ascii) {
		is_exist = false;
		find_index = ascii;

		for (int it_j = 0; it_j < row_size; ++it_j) {

			int it_j_index = static_cast<int>(compress_index[it_j]->at(0));
			for (int it_i = 0; it_i < size; ++it_i) {

				if (state_table[it_i][it_j_index] == state_table[it_i][ascii]) {
					is_exist = true;
				} else {
					is_exist = false;
					break;
				}

			}

			if (is_exist) {
				find_index = it_j;
				break;
			}

		}

		if (is_exist) {
			compress_index[find_index]->push_back(ascii);
		} else {
			compress_index[row_size++] = new vector<state>(1, ascii);
		}

	}

	_column_size = row_size;
	_header = new vector<state>*[_column_size];

	for (int it = 0; it < row_size; ++it) {
		_header[it] = compress_index[it];
	}
	_table = new state_t*[_state_size];

	for (size_t it = 0; it < _state_size; ++it) {
		_table[it] = new state_t[_column_size];
	}

	for (size_t it_j = 0; it_j < _column_size; ++it_j) {
		size_t index = *(_header[it_j]->begin());
		for (size_t it_i = 0; it_i < _state_size; ++it_i) {
			_table[it_i][it_j] = state_table[it_i][index];
		}
	}

	//generate _state_rate
	map<state, int> *state_rate_map = new map<state, int> [_state_size];
	map<state, int>::iterator map_it;
	for (size_t it_i = 0; it_i < _state_size; ++it_i) {

		for (size_t it_j = 0; it_j < _column_size; ++it_j) {

			map_it = state_rate_map[it_i].find(_table[it_i][it_j]);
			if (map_it != state_rate_map[it_i].end()) {
				++map_it->second;
			} else {
				state_rate_map[it_i].insert(
						pair<state, int>(_table[it_i][it_j], 1));
			}

		}

	}
	_state_rate = new vector_index_rate;
	pair_index_rate *pair_rate = NULL;
	for (size_t it_i = 0; it_i < _state_size; ++it_i) {
		pair_rate = new pair_index_rate;
		pair_rate->first = it_i;
		pair_rate->second = new vector_pair_rate;
		_state_rate->push_back(pair_rate);

		for (map_it = state_rate_map[it_i].begin();
				map_it != state_rate_map[it_i].end(); ++map_it) {
			pair_rate->second->push_back(
					make_pair(map_it->first, map_it->second));
		}
		sort(pair_rate->second->begin(), pair_rate->second->end(),
				cmp_pair<pair<state, int> >());
	}
	delete[] state_rate_map;
	sort(_state_rate->begin(), _state_rate->end(), cmp_state_rate());

}
void transtable::print_table(FILE *fptr) const {
	if (NULL == fptr)
		return;
	state_t begin_char = 0, end_char = 0;
	vector<state>::iterator it_i;

	for (size_t it = 0; it < _column_size; ++it) {

		fprintf(fptr, "the input character ASCII:");
		it_i = _header[it]->begin();

		while (it_i != _header[it]->end()) {
			end_char = begin_char = *it_i;
			do {
				end_char = *(it_i++);
			} while ((it_i != _header[it]->end()) && (end_char + 1 == *it_i));
			if (begin_char != end_char)
				fprintf(fptr, " %d-%d", begin_char, end_char);
			else
				fprintf(fptr, " %d", begin_char);
		}

		fprintf(fptr, "	->	[%d]\n", it);
	}
	fprintf(fptr, "\n\n\n");
	for (size_t it = 0; it < _column_size; ++it) {
		fprintf(fptr, "[%d]	", it);
	}

	fprintf(fptr, "\n");
	for (size_t it_i = 0; it_i < _state_size; ++it_i) {
		for (size_t it_j = 0; it_j < _column_size; ++it_j) {
			fprintf(fptr, "%d	", _table[it_i][it_j]);
		}
		fprintf(fptr, "\n");

	}
	fprintf(fptr, "\n\n\nrates:\n");
	for (size_t it_i = 0; it_i < _state_size; ++it_i) {
		fprintf(fptr, "state:%d	->	", _state_rate->at(it_i)->first);
		for (vector_pair_rate::iterator it_n =
				_state_rate->at(it_i)->second->begin();
				it_n != _state_rate->at(it_i)->second->end(); ++it_n) {
			fprintf(fptr, "%d(%d)	", it_n->first, it_n->second);
		}
		fprintf(fptr, "\n");
	}
}

