/*
 *
 *  Created on: May 13, 2014
 *      Author: Lhmily
 */

#include <strstream>

#include "../header/dfa.h"
#include "../header/nfa.h"
#include "../header/parser.h"

#include "../header/tcammodel.h"

//using namespace std;

void init_conf(void);
static int parse_arguments(int argc, char **argv);
static void usage(void);
void print_conf(void);
void check_file(char *filename, char *mode);
void handle_regex_file(NFA *&nfa, DFA *&dfa);
void handle_compress(DFA *dfa);

int VERBOSE;
int DEBUG;

/* configuration */
static struct conf {
	char *regex_file;
	bool i_mod;
	bool m_mod;
	bool tcam;
} config;

int main(int argc, char **argv) {
	init_conf();
	while (!parse_arguments(argc, argv))
		usage();
	print_conf();
	if (config.regex_file != NULL)
		check_file(config.regex_file, "r");

	/* FA declaration */
	NFA *nfa = NULL;  	// NFA
	DFA *dfa = NULL;		// DFA

	handle_regex_file(nfa, dfa);
	handle_compress(dfa);
	return 0;
}
/* initialize the configuration */
void init_conf(void) {
	config.regex_file = NULL;
	config.i_mod = false;
	config.m_mod = false;
	config.tcam = false;
}
/* parse the main call parameters */
static int parse_arguments(int argc, char **argv) {
	int i = 1;
	if (argc < 2) {
		usage();
		return 0;
	}
	while (i < argc) {
		if (strcmp(argv[i], "-tcam") == 0) {
			config.tcam = true;
		} else if (strcmp(argv[i], "-h") == 0
				|| strcmp(argv[i], "--help") == 0) {
			usage();
			return 0;
		} else if (strcmp(argv[i], "-p") == 0
				|| strcmp(argv[i], "--parse") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "Regular expression file name missing.\n");
				return 0;
			}
			config.regex_file = argv[i];
		} else if (strcmp(argv[i], "--m") == 0) {
			config.m_mod = true;
		} else if (strcmp(argv[i], "--i") == 0) {
			config.i_mod = true;
		} else {
			fprintf(stderr, "Ignoring invalid option %s\n", argv[i]);
		}
		i++;
	}
	return 1;
}
/* usage */
static void usage(void) {
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: regex_compress_tcam [options]\n");
	fprintf(stderr,
			"             [--parse|-p <regex_file> [--m|--i]]   [-tcam] \n\n");
	fprintf(stderr, "\nOptions:\n");
	fprintf(stderr, "    --help,-h       print this message\n");
	fprintf(stderr, "\nOther:\n");
	fprintf(stderr, "    --parse,-p <regex_file>  process regex file\n");
	fprintf(stderr, "    -tcam                    \n");
	fprintf(stderr, "\n");
	exit(0);
}
/* print the configuration */
void print_conf(void) {
	fprintf(stderr, "\nCONFIGURATION: \n");
	if (config.regex_file)
		fprintf(stderr, "- RegEx file: %s\n", config.regex_file);
	if (config.tcam)
		fprintf(stderr, "- Compress the transition table and tcam modeling\n");
}

/* check that the given file can be read/written */
void check_file(char *filename, char *mode) {
	FILE *file = fopen(filename, mode);
	if (file == NULL) {
		fprintf(stderr, "Unable to open file %s in %c mode", filename, mode);
		fatal("\n");
	} else
		fclose(file);
}
void handle_regex_file(NFA *&nfa, DFA *&dfa) {

	if (config.regex_file == NULL)
		return;

	FILE *regex_file = fopen(config.regex_file, "r");
	fprintf(stderr, "\nParsing the regular expression file %s ...\n",
			config.regex_file);
	regex_parser *parse = new regex_parser(config.i_mod, config.m_mod);
	nfa = parse->parse(regex_file);
	nfa->remove_epsilon();
	nfa->reduce();
	dfa = nfa->nfa2dfa();
	if (dfa == NULL)
		printf(
				"Max DFA size %ld exceeded during creation: the DFA was not generated\n",
				MAX_DFA_SIZE);
	else
		dfa->minimize();
	fclose(regex_file);
	delete parse;

}
void handle_compress(DFA *dfa) {
	if ((dfa == NULL) || (!config.tcam))
		return;
	ofstream *table_fout = new ofstream();
	(*table_fout).open("transition_table.txt");

	transtable *table = new transtable();
	table->handle_table(dfa->get_state_table(), dfa->size());
	table->reorder();

	table->replace_table();
	table->print_table(*table_fout);

	ofstream *blocks_fout = new ofstream[8];
	ofstream *blocks_code_fout = new ofstream[8];
	ofstream *inputs_code_fout = new ofstream[8];

	string pre_blocks_fout = "blocks_";
	string pre_blocks_code_fout = "blocks_code_";
	string pre_inputs_code_fout = "inputs_code_";
	string suff_str = ".txt";
	string block_str;
	strstream ss;

	string file_str;
	tcam_model *tcam = new tcam_model(table);
	tcam->init();
	size_t block_size = 16;
	//block_size[]
	for (int i = 0; i < 8; i++) {
		block_size <<= 1;		// = 32 * pow(2, i);
		ss.clear();
		ss << block_size;
		block_str.clear();
		ss >> block_str;

//		file_str = pre_blocks_fout + block_str + suff_str;
//		blocks_fout[i].open(file_str.data());
//
//		file_str = pre_blocks_code_fout + block_str + suff_str;
//		blocks_code_fout[i].open(file_str.data());

		file_str = pre_inputs_code_fout + block_str + suff_str;
		inputs_code_fout[i].open(file_str.data());

//		table->release_vector_blocks_code();
//		table->release_blocks();
		//table->release_state_rate();
		table->release_transitions();

//		table->generate_blocks(block_size);
//
//		table->print_blocks(blocks_fout[i]);
//
//		table->compress_blocks();
//		table->print_blocks_code(blocks_code_fout[i]);

		table->generate_bolcks_code(block_size);
		table->print_transitions(inputs_code_fout[i]);

		tcam->print();

//		blocks_fout[i].close();
//		blocks_code_fout[i].close();
		inputs_code_fout[i].close();
	}

	table_fout->close();

	delete table_fout;
	delete[] blocks_fout;
	delete[] inputs_code_fout;
	delete[] blocks_code_fout;
	delete tcam;
	delete table;
}
