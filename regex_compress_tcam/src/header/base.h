/*
 * base.h
 *
 *  Created on: May 13, 2014
 *      Author: Lhmily
 */

#ifndef BASE_H_
#define BASE_H_

#include <stddef.h>
#include <vector>
#include <string>


using namespace std;

typedef size_t state;
typedef struct {
	size_t state;
	string code;
} DST_CODE;

typedef string SRC;
typedef struct {
	SRC src_code;
	DST_CODE dst_code;
} trans_CODE, *trans_CODE_ptr;
typedef vector<trans_CODE_ptr> BLOCK_CODE;

typedef BLOCK_CODE *BLOCK_CODE_PTR;

typedef struct {
	double search_power;
	double search_time;
} POWER_T;

#endif /* BASE_H_ */
