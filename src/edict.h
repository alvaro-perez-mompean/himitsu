#ifndef __MAIN_H__
	#define __MAIN_H__
	#include "main.h"
#endif

#ifndef __VOCAB_H__
	#define __VOCAB_H__
	#include "vocab.h"
#endif

FILE * load_edict();
bool add_line_to_node(vocab_t **, char [], bool, int, unsigned short);
void add_line(vocab_t **, FILE *archivo,int);
int longest_line(FILE *archivo);
