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
}

size_t transtable::getStateSize() const {
	return _state_size;
}

transtable::~transtable() {
	// TODO Auto-generated destructor stub
	if (NULL != _header) {
		for (int it = 0; it < _column_size; ++it)
			delete _header[it];
		delete[] _header;
	}
	_header = NULL;
	if (NULL != _table) {
		for (int it = 0; it < _state_size; ++it)
			delete[] _table[it];
		delete[] _table;
	}
	_table = NULL;
}

void transtable::handle_table(state_t **state_table, int size) {
	_state_size = size;

	int row_size = 1;

	vector<state> *compress_index[256];
	compress_index[0] = new vector<state>(1, 0u);

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

	for (int it = 0; it < _state_size; ++it) {
		_table[it] = new state_t[_column_size];
	}

	for (int it_j = 0; it_j < _column_size; ++it_j) {
		size_t index = *(_header[it_j]->begin());
		for (int it_i = 0; it_i < _state_size; ++it_i) {
			_table[it_i][it_j] = state_table[it_i][index];
		}
	}
}
void transtable::print_table(FILE *fptr) const {
	if (NULL == fptr)
		return;
	state_t begin_char = 0, end_char = 0;
	vector<state>::iterator it_i;

	for (int it = 0; it < _column_size; ++it) {

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
	for (int it = 0; it < _column_size; ++it) {
		fprintf(fptr, "[%d]	", it);
	}
	fprintf(fptr, "\n");
	for (int it_i = 0; it_i < _state_size; ++it_i) {
		for (int it_j = 0; it_j < _column_size; ++it_j) {
			fprintf(fptr, "%d	", _table[it_i][it_j]);
		}
		fprintf(fptr, "\n");
	}
}

