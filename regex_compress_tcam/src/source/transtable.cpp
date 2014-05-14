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
	this->release_state_rate();
	if (NULL != _block_index) {
		for (size_t it = 0; it < _block_num; ++it) {
			delete _block_index[it];
		}
		delete[] _block_index;
		_block_index = NULL;
		for (vector<pair<size_t, size_t*> >::iterator it =
				_vector_blocks->begin(); it != _vector_blocks->end(); ++it) {
			delete[] it->second;
		}
		delete _vector_blocks;
		_vector_blocks = NULL;
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
	this->generate_state_rate();
}

void transtable::generate_state_rate() {
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

void transtable::replace_table() {
	size_t *state_new = new size_t[_state_size];

	for (size_t it = 0; it < _state_size; ++it) {
		state_new[it] = _state_rate->operator [](it)->first;
	}

	state_t **temp = new state_t*[_state_size];
	for (size_t it = 0; it < _state_size; ++it) {
		temp[it] = _table[it];
	}

	for (size_t it = 0; it < _state_size; ++it) {
		_table[it] = temp[state_new[it]];
	}

	delete[] temp;
	size_t old_state = 0, new_state = 0;

	for (size_t it_i = 0; it_i < _state_size; ++it_i) {
		for (size_t it_j = 0; it_j < _column_size; ++it_j) {

			old_state = _table[it_i][it_j];
			for (size_t it = 0; it < _state_size; ++it) {
				if (state_new[it] == old_state) {
					new_state = it;
					break;
				}
			}

			_table[it_i][it_j] = new_state;
		}
	}

	this->release_state_rate();
	this->generate_state_rate();
}
void transtable::release_state_rate() {
	if (NULL == _state_rate)
		return;

	for (vector_index_rate::iterator it = _state_rate->begin();
			it != _state_rate->end(); ++it)
		delete (*it)->second;

	delete _state_rate;
	_state_rate = NULL;
}

//
void transtable::generate_blocks(int block_size) {
	_block_size = block_size;
	_block_index = new size_t*[_column_size];

	_block_num = ceil(static_cast<double>(_state_size) / block_size);
	for (size_t it = 0; it < _column_size; ++it) {
		_block_index[it] = new size_t[_block_num];
	}

	_vector_blocks = new vector<pair<size_t, size_t*> >;
	size_t block_row_start = 0, block_row_size = 0;
	vector<pair<size_t, size_t*> >::iterator block_it;
	bool is_exist = false;
	int count = 0;
	size_t *exist_block = NULL;

	for (size_t it = 0; it < _column_size; ++it) {
		for (size_t block = 0; block < _block_num; ++block) {

			block_row_start = block * block_size;
			block_row_size =
					(block_row_start + block_size) < _state_size ?
							block_size : _state_size - block_row_start;

			is_exist = false;
			exist_block = NULL;
			count = 0;

			for (block_it = _vector_blocks->begin();
					block_it != _vector_blocks->end(); ++block_it) {

				if (block_it->first == block_row_size) {
					exist_block = block_it->second;

					for (size_t exist_it = 0; exist_it < block_row_size;
							++exist_it) {
						if (exist_block[exist_it]
								== _table[block_row_start + exist_it][it]) {
							is_exist = true;
						} else {
							is_exist = false;
							break;
						}
					}
				}
				if (is_exist) {
					_block_index[it][block] = count;
					is_exist = true;
					break;
				}
				count++;
			}
			if (!is_exist) {
				//push_back
				exist_block = new size_t[block_row_size];

				for (size_t save_it = 0; save_it < block_row_size; ++save_it) {
					exist_block[save_it] =
							_table[block_row_start + save_it][it];
				}
				_block_index[it][block] = _vector_blocks->size();
				_vector_blocks->push_back(
						make_pair(block_row_size, exist_block));
			}
		}
	}

}

void transtable::print_characters(FILE *fptr, print_characters_fun fun) const {
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
		(this->*fun)(fptr, it);
	}

	fprintf(fptr, "\n\n\n");
}

void transtable::print_table_fun(FILE *fptr, size_t it) const {
	fprintf(fptr, "	->	[%d]\n", it);
}
/*
 * print the transition table after compressed based on input character.
 * print some statistics data at the same time.
 */
void transtable::print_table(FILE *fptr) const {
	if (NULL == fptr)
		return;

	this->print_characters(fptr, &transtable::print_table_fun);

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
void transtable::print_blocks_fun(FILE *fptr, size_t index) const {
	fprintf(fptr, "	->	");
	for (size_t it = 0; it < _block_num; ++it)
		fprintf(fptr, "#%d	", _block_index[index][it]);
	fprintf(fptr, "\n");
}
/*
 * print the blocks detail and 256 input characters block index after calling generate_blocks().
 */
void transtable::print_blocks(FILE *fptr) const {
	if (NULL == fptr)
		return;
	this->print_characters(fptr, &transtable::print_blocks_fun);
	size_t count = 0;
	for (vector<pair<size_t, size_t*> >::iterator it = _vector_blocks->begin();
			it != _vector_blocks->end(); ++it) {
		fprintf(fptr, "#%u block transition number is %u and block detail:\n",
				count++, it->first);
		for (size_t it_i = 0; it_i < it->first; ++it_i) {
			fprintf(fptr, "%u	", (it->second)[it_i]);
		}
		fprintf(fptr, "\n\n");
	}
}
