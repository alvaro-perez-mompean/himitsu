#ifndef __MAIN_H__
	#define __MAIN_H__
	#include "main.h"
#endif

#ifndef __CURSES_H__
	#define __CURSES_H__
	#include "curses.h"
#endif

FILE * load_edict(bool);
void load_vocab(vocab_t **);
bool add_line_to_node(vocab_t **, char [], bool, int, unsigned short);
int add_line(vocab_t **, FILE *archivo,int);
int longest_line(FILE *archivo);
int show_vocab(vocab_t *, int, bool);
void show_vocab_item(vocab_t *, int);
int show_cat(vocab_t *);
void save_vocab(vocab_t *);
void edit_vocab(vocab_t *, char []);
bool does_not_exist(vocab_t **, vocab_t *, int);
void delete_vocab(vocab_t **);
void new_cat(vocab_t **, char []);
void delete_cat(vocab_t **, int);
int current_cat(vocab_t *, int, bool);
vocab_t * go_to_cat(vocab_t *, int);
vocab_t * go_to_item(vocab_t *, int, int);
int select_cat(vocab_t *, int, const char []);
int import_file(vocab_t **, const char []);
int export_file(vocab_t *);
