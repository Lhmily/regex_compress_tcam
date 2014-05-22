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
	_block_index = NULL;
	_block_size = 0;
	_block_bits = 0;
	_column_size = 0;
	_block_size = 0;
	_block_num = 0;
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
	this->release_blocks();
}

void transtable::handle_table(state_t **state_table, int size) {
	_state_size = size;
	_state_bits = ceil(log(_state_size) / log(2));

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
void transtable::reorder() {
	if (NULL == _state_rate)
		this->generate_state_rate();

	sort(_state_rate->begin(), _state_rate->end(), cmp_state_rate());

	vector_index_rate *state_rate_new = new vector_index_rate;
	state_rate_new->reserve(_state_size);
	vector_index_rate::iterator remove_it;

	size_t cur_state;
	while (!_state_rate->empty()) {
		cur_state = _state_rate->at(0)->second->at(0).first;

		remove_copy_if(_state_rate->begin(), _state_rate->end(),
				back_inserter(*state_rate_new),
				remove_copy_if_reorder(cur_state));

		remove_it = remove_if(_state_rate->begin(), _state_rate->end(),
				remove_if_reorder(cur_state));

		_state_rate->erase(remove_it, _state_rate->end());
	}

	delete _state_rate;

	_state_rate = state_rate_new;

}

void transtable::generate_state_rate() {

	assert(_state_rate==NULL);

	map<state, int> *state_rate_map = new map<state, int>;
	_state_rate = new vector_index_rate;
	pair_index_rate *pair_rate = NULL;
	map<state, int>::iterator map_it;

	for (size_t it_i = 0; it_i < _state_size; ++it_i) {

		state_rate_map->clear();
		//build map
		for_each(_table[it_i], _table[it_i] + _column_size,
				for_each_generate_state_rate(state_rate_map));

		pair_rate = new pair_index_rate;
		pair_rate->first = it_i;
		pair_rate->second = new vector_pair_rate;

		//get
		for_each(state_rate_map->begin(), state_rate_map->end(),
				for_each_state_rate_map(pair_rate->second));

		sort(pair_rate->second->begin(), pair_rate->second->end(),
				cmp_pair<pair<state, int> >());

		_state_rate->push_back(pair_rate);
	}

	delete state_rate_map;

}

void transtable::replace_table() {
	size_t *state_new = new size_t[_state_size];

	for (size_t it = 0; it < _state_size; ++it) {
		state_new[it] = _state_rate->at(it)->first;
	}

	state_t **temp = new state_t*[_state_size];
	for (size_t it = 0; it < _state_size; ++it) {
		temp[it] = _table[it];
	}

	for (size_t it = 0; it < _state_size; ++it) {
		_table[it] = temp[state_new[it]];
	}

	delete[] temp;

	for (size_t it_i = 0; it_i < _state_size; ++it_i) {
		for (size_t it_j = 0; it_j < _column_size; ++it_j) {

			_table[it_i][it_j] = find(state_new, state_new + _state_size,
					_table[it_i][it_j]) - state_new;
		}
	}
	delete[] state_new;
	this->release_state_rate();
	ofstream abc;
	abc.open("YES.txt");
	abc << "state_size" << _state_size;
	for (size_t it = 0; it < _state_size; ++it) {
		abc << endl;
		for (size_t jt = 0; jt < _column_size; ++jt) {
			abc << _table[it][jt] << "\t";
		}
	}
	this->generate_state_rate();
}
void transtable::release_state_rate() {
	if (NULL == _state_rate)
		return;

	for_each(_state_rate->begin(), _state_rate->end(), [](pair_index_rate* it) {
		delete it->second;
		delete it;
	});

	delete _state_rate;
	_state_rate = NULL;
}

void transtable::release_blocks() {
	if (NULL == _block_index)
		return;
	for (size_t it = 0; it < _block_num; ++it) {
		delete _block_index[it];
	}
	delete[] _block_index;
	_block_index = NULL;

	for_each(_vector_blocks, _vector_blocks + _block_num,
			[](vector<pair<size_t, size_t*> > * cur_block) {
				for_each(cur_block->begin(),cur_block->end(),[](pair<size_t, size_t*> cur) {
							delete []cur.second;
						});
				delete cur_block;
			});
	delete[] _vector_blocks;
	_vector_blocks = NULL;

}
//
void transtable::generate_blocks(int block_size) {
	_block_size = block_size;
	_block_bits = ceil(log(_block_size) / log(2));
	_block_index = new size_t*[_column_size];

	_block_num = ceil(static_cast<double>(_state_size) / block_size);
	for (size_t it = 0; it < _column_size; ++it) {
		_block_index[it] = new size_t[_block_num];
	}
	_vector_blocks = new vector<pair<size_t, size_t*> >*[_block_num];
	for (size_t it = 0; it < _block_num; ++it) {
		_vector_blocks[it] = new vector<pair<size_t, size_t*> >;
	}
	vector<pair<size_t, size_t*> > *block_group_insert;
	size_t block_row_start = 0, block_row_size = 0;

	vector<pair<size_t, size_t*> >::iterator block_it;
	int count = 0;
	for (size_t block_group_it = 0; block_group_it < _block_num;
			++block_group_it) {
		block_group_insert = _vector_blocks[block_group_it];

		block_row_start = block_group_it * block_size;
		block_row_size =
				(block_row_start + block_size) < _state_size ?
						_block_size : _state_size - block_row_start;

		for (size_t it = 0; it < _column_size; ++it) {

			count = 0;
			block_it = block_group_insert->begin();
			while (block_it != block_group_insert->end()) {
				if (find_if_generate_blocks(_table, block_row_size)(*block_it,
						block_row_start, it)) {
					_block_index[it][block_group_it] = count;
					break;
				}
				++count;
				++block_it;
			}
			if (block_it == block_group_insert->end()) {
				_block_index[it][block_group_it] = block_group_insert->size();
				size_t *block_temp = new size_t[block_row_size];

				for (size_t copy_it = 0; copy_it < block_row_size; ++copy_it) {
					block_temp[copy_it] = _table[block_row_start + copy_it][it];
				}

				block_group_insert->push_back(
						make_pair(block_row_size, block_temp));
			}

		}

	}
}

void transtable::print_characters(ofstream &fout,
		print_characters_fun fun) const {
	state_t begin_char = 0, end_char = 0;
	vector<state>::iterator it_i;

	for (size_t it = 0; it < _column_size; ++it) {

		fout << "the input character ASCII:";
		it_i = _header[it]->begin();

		while (it_i != _header[it]->end()) {
			end_char = begin_char = *it_i;
			do {
				end_char = *(it_i++);
			} while ((it_i != _header[it]->end()) && (end_char + 1 == *it_i));
			if (begin_char != end_char)
				fout << "\t" << begin_char << "-" << end_char;
			else
				fout << "\t" << begin_char;
		}
		(this->*fun)(fout, it);
	}

	fout << "\n\n\n";
}

void transtable::print_table_fun(ofstream &fout, size_t it) const {
	fout << "\t->\t[" << it << "]" << endl;
}
/*
 * print the transition table after compressed based on input character.
 * print some statistics data at the same time.
 */
void transtable::print_table(ofstream &fout) const {

	this->print_characters(fout, &transtable::print_table_fun);

	for (size_t it = 0; it < _column_size; ++it) {
		fout << "[" << it << "]\t";
	}

	fout << endl;
	for (size_t it_i = 0; it_i < _state_size; ++it_i) {
		copy(_table[it_i], &_table[it_i][_column_size],
				ostream_iterator<size_t>(fout, "	"));
		fout << endl;

	}

	fout << endl << endl << endl << "rates:" << endl;
	for (size_t it_i = 0; it_i < _state_size; ++it_i) {

		fout << "state:" << _state_rate->at(it_i)->first << "\t->\t";
		for (vector_pair_rate::iterator it_n =
				_state_rate->at(it_i)->second->begin();
				it_n != _state_rate->at(it_i)->second->end(); ++it_n) {
			fout << it_n->first << "(" << it_n->second << ")\t";

		}

		fout << "\n";
	}
}
void transtable::print_blocks_fun(ofstream &fout, size_t index) const {
	fout << "	->	";

	size_t sum_prefix = 0;
	for (size_t it = 0; it < _block_num; ++it) {
		if (0 != it)
			sum_prefix += _vector_blocks[it - 1]->size();

		fout << "#" << sum_prefix + _block_index[index][it] << "\t";
	}
	fout << "\n";
}
/*
 * print the blocks detail and 256 input characters block index after calling generate_blocks().
 */
void transtable::print_blocks(ofstream &fout) const {

	this->print_characters(fout, &transtable::print_blocks_fun);
	size_t count = 0;
	vector<pair<size_t, size_t*> >::iterator it;
	for (size_t i = 0; i < _block_num; ++i)
		for (it = _vector_blocks[i]->begin(); it != _vector_blocks[i]->end();
				++it) {
			fout << "#" << count++ << " block transition number is "
					<< it->first << " and block detail:\n";
			for (size_t it_i = 0; it_i < it->first; ++it_i) {
				fout << "\t\t" << (it->second)[it_i] << endl;
			}
			fout << endl;
		}
}

//compress
string transtable::state_convert_code(state s, const int bits) const {
	string ret;
	int i, j;
	for (i = bits - 1, j = 0; i >= 0; i--, j++) {
		if (0 == ((s >> i) & 0x01)) {
			ret.append("0");
		} else {
			ret.append("1");
		}
	}
	return ret;
}

void transtable::handle_block_code(const size_t *block, int index, int size,
		vector<string>* vector_code) {
	set<size_t> set_temp;
	for (int it = 0; it < size; ++it) {
		set_temp.insert(block[index + it]);
	}
	if (1 == set_temp.size()) {
//compress
		int suffix_num = ceil(log(size) / log(2));
		string code = state_convert_code(index, _block_bits);
		for (int suffix_it = _block_bits - suffix_num; suffix_it < _block_bits;
				++suffix_it) {
			code[suffix_it] = '*';
		}
//save
		vector_code->push_back(code);
		return;
	} else {
		int new_size = size / 2;
		handle_block_code(block, index, new_size, vector_code);
		handle_block_code(block, index + new_size, new_size, vector_code);
	}

}

void transtable::compress_blocks() {
//	size_t blocks_sum_size = _vector_blocks->size();
//	_vector_blocks_code = new CODE*[blocks_sum_size];
//	pair<size_t, size_t*> cur_block;
//
//	int deal_size = 0, pow_size = 0;
//
//	for (size_t block_it = 0; block_it < blocks_sum_size; ++block_it) {
//		cur_block = _vector_blocks->at(block_it);
//		deal_size = pow_size = 0;
//		_vector_blocks_code[block_it] = new CODE;
//		do {
//			pow_size = pow(2, floor(log(cur_block.first) / log(2)));
//
//			this->handle_block_code(cur_block.second, deal_size, pow_size,
//					_vector_blocks_code[block_it]);
//
//			deal_size += pow_size;
//		} while (deal_size < size);
//	}

}
