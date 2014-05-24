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
	_vector_blocks_code = NULL;
	_vector_blocks = NULL;
	_block_size = 0;
	_state_bits = _block_bits = 0;
	_column_size = 0;
	_block_size = 0;
	_total_block_num = _block_num = 0;
	_state_size = 0;
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

	this->release_vector_blocks_code();
	this->release_blocks();
	this->release_state_rate();
}

void transtable::release_vector_blocks_code() {
	if (NULL == _vector_blocks_code)
		return;
	size_t group_block_size = 0;
	BLOCK_CODE::iterator iterator_code;
	for (size_t i = 0; i < _block_num; ++i) {
		group_block_size = _vector_blocks[i]->size();

		for (size_t it = 0; it < group_block_size; ++it) {
			for (iterator_code = _vector_blocks_code[i][it]->begin();
					iterator_code != _vector_blocks_code[i][it]->end();
					++iterator_code) {
				delete (*iterator_code);
			}
			delete _vector_blocks_code[i][it];
		}
		delete[] _vector_blocks_code[i];
	}
	delete[] _vector_blocks_code;
	_vector_blocks_code = NULL;
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

	stable_sort(_state_rate, _state_rate + _state_size, cmp_state_rate());

	size_t cur_state;
	pair<size_t, vector<pair<state, int> >*> **state_rate_new = new pair<size_t,
			vector<pair<state, int> >*> *[_state_size];
	size_t remove_size = 0, isnot_remove_size = _state_size;

	while (remove_size != _state_size) {

		cur_state = _state_rate[0]->second->begin()->first;

		size_t cur_index = 0, it = 0, before_remove_size = remove_size;

		while (it < isnot_remove_size) {
			if (cur_state == _state_rate[it]->second->begin()->first) {
				state_rate_new[remove_size++] = _state_rate[it];

			} else {
				_state_rate[cur_index++] = _state_rate[it];
			}
			++it;
		}
		isnot_remove_size -= (remove_size - before_remove_size);
	}

	for (size_t it = 0; it < _state_size; ++it) {
		_state_rate[it] = state_rate_new[it];
	}
	delete[] state_rate_new;
}

void transtable::generate_state_rate() {

	assert(_state_rate==NULL);

	map<state, int> state_rate_map;
	_state_rate = new pair<size_t, vector<pair<state, int> >*> *[_state_size];
	for (size_t it = 0; it < _state_size; ++it) {
		_state_rate[it] = new pair<size_t, vector<pair<state, int> >*>;
		_state_rate[it]->second = new vector<pair<state, int> >;
	}

	map<state, int>::iterator map_it;

	for (size_t it_i = 0; it_i < _state_size; ++it_i) {

		state_rate_map.clear();

		//build map
		for_each(_table[it_i], _table[it_i] + _column_size,
				for_each_generate_state_rate(&state_rate_map));

		//get
		for_each(state_rate_map.begin(), state_rate_map.end(),
				for_each_state_rate_map(_state_rate[it_i]->second));
		_state_rate[it_i]->first = it_i;

		stable_sort(_state_rate[it_i]->second->begin(),
				_state_rate[it_i]->second->end(), cmp_pair());
	}
}

void transtable::replace_table() {
	size_t *state_new = new size_t[_state_size];

	for (size_t it = 0; it < _state_size; ++it) {
		state_new[it] = _state_rate[it]->first;
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

	this->generate_state_rate();
}

void transtable::release_state_rate() {
	if (NULL == _state_rate)
		return;

	for_each(_state_rate, _state_rate + _state_size,
			[](pair<size_t, vector<pair<state, int> >*>* it) {
				delete it->second;
				delete it;
			});

	delete[] _state_rate;
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
	for (size_t it = 0; it < _block_num; ++it) {
		_total_block_num += _vector_blocks[it]->size();
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
	if (NULL != _state_rate) {
		fout << endl << endl << endl << "rates:" << endl;
		for (size_t it_i = 0; it_i < _state_size; ++it_i) {

			fout << "state:" << _state_rate[it_i]->first << "\t->\t";
			for (vector<pair<state, int> >::iterator it_n =
					_state_rate[it_i]->second->begin();
					it_n != _state_rate[it_i]->second->end(); ++it_n) {
				fout << it_n->first << "(" << it_n->second << ")\t";

			}

			fout << "\n";
		}
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
		size_t block_index, BLOCK_CODE_PTR vector_code) {
	set<size_t> set_temp;
	for (int it = 0; it < size; ++it) {
		set_temp.insert(block[index + it]);
	}
	if (1 == set_temp.size()) {
		//compress
		int suffix_num = ceil(log(size) / log(2));
		string src_code = this->state_convert_code(
				index + block_index * _block_size, _state_bits);
		vector_code->push_back(
				get_CODE(src_code, _state_bits - suffix_num, block[index]));
		return;
	} else {
		int new_size = size / 2;
		handle_block_code(block, index, new_size, block_index, vector_code);
		handle_block_code(block, index + new_size, new_size, block_index,
				vector_code);
	}

}

void transtable::handle_each_block_code(BLOCK_CODE_PTR &block) {
	if (block->size() < 3)
		return;
	map<state, size_t> statistics_map;
	map<state, size_t>::iterator map_it;
	BLOCK_CODE::iterator block_it, cur_it;
	BLOCK_CODE_PTR block_new = new BLOCK_CODE;

	int count = 0;
	while (block->size() >= 3) {

		statistics_map.clear();

		for (block_it = block->begin(); block_it != block->end();
				++block_it) {

			size_t dst_state = (*block_it)->dst_code.state;
			map_it = statistics_map.find(dst_state);

			if (map_it != statistics_map.end()) {
				++map_it->second;
			} else {
				statistics_map.insert(make_pair(dst_state, 1));
			}
		}
		//find the max element of statistics_map
		map_it = max_element(statistics_map.begin(), statistics_map.end(),
				[](pair<state,size_t> max,pair<state,size_t> elem) {
					return elem.second>max.second;
				});
		if (map_it->second > 1) {
			//
			size_t dst_state = map_it->first;

			block_it = cur_it = block->begin();
			string src_code;

			while (block_it != block->end()) {

				if ((*block_it)->dst_code.state == dst_state) {
					if (src_code.empty()) {
						src_code = (*block_it)->src_code;
					}
					delete (*block_it);
				} else {
					*cur_it = *block_it;
					++cur_it;
				}
				++block_it;
			}

			block->erase(cur_it, block->end());

			block_new->push_back(
					get_CODE(src_code, _state_bits - _block_bits + count,
							dst_state));

		} else {
			break;
		}
		++count;
	}

	BLOCK_CODE::reverse_iterator r_it;
	for (r_it = block_new->rbegin(); r_it != block_new->rend(); ++r_it) {
		block->push_back(*r_it);
	}
	delete block_new;
}

void transtable::compress_each_block() {

	BLOCK_CODE_PTR cur_block = NULL;
	size_t group_block_size = 0;

	for (size_t it = 0; it < _block_num; ++it) {

		group_block_size = _vector_blocks[it]->size();

		for (size_t jt = 0; jt < group_block_size; ++jt) {
			cur_block = _vector_blocks_code[it][jt];
			this->handle_each_block_code(cur_block);
		}
	}
}

trans_CODE_ptr transtable::get_CODE(string src_code, int mask_index,
		size_t dst) {
	trans_CODE_ptr ret = new trans_CODE;

	for (int replace_index = mask_index; replace_index < _state_bits;
			++replace_index) {
		src_code[replace_index] = '*';
	}

	string dst_code = state_convert_code(dst, _state_bits);
	ret->src_code = src_code;
	ret->dst_code.state = dst;
	ret->dst_code.code = dst_code;

	return ret;
}

void transtable::compress_blocks() {

	_vector_blocks_code = new BLOCK_CODE_PTR *[_block_num];
	size_t group_block_size = 0;

	for (size_t it = 0; it < _block_num; ++it) {
		group_block_size = _vector_blocks[it]->size();

		_vector_blocks_code[it] = new BLOCK_CODE_PTR[group_block_size];

		for (size_t jt = 0; jt < group_block_size; ++jt) {
			BLOCK_CODE_PTR code_ptr = new BLOCK_CODE;
			handle_block_code(_vector_blocks[it]->at(jt).second, 0,
					_vector_blocks[it]->at(jt).first, it, code_ptr);
			_vector_blocks_code[it][jt] = code_ptr;

		}
	}
	this->compress_each_block();
}
void transtable::print_blocks_code(ofstream &fout) const {
	size_t count = 0;
	size_t group_block_size = 0;
	size_t trans_num = 0;
	BLOCK_CODE::iterator iterator_code;
	for (size_t i = 0; i < _block_num; ++i) {
		group_block_size = _vector_blocks[i]->size();

		for (size_t it = 0; it < group_block_size; ++it) {
			trans_num = _vector_blocks_code[i][it]->size();

			fout << "#" << count++ << " block transition number is "
					<< trans_num << " and block detail:\n";

			for (iterator_code = _vector_blocks_code[i][it]->begin();
					iterator_code != _vector_blocks_code[i][it]->end();
					++iterator_code) {
				fout << "\t\t" << (*iterator_code)->src_code << "->"
						<< (*iterator_code)->dst_code.code << "("
						<< (*iterator_code)->dst_code.state << ")" << endl;
			}

			fout << endl;
		}
	}
}

void transtable::generate_bolcks_code() {

}
