#ifndef __MAIN_H__
	#define __MAIN_H__
	#include "main.h"
#endif

#ifndef __CURSES_H__
	#define __CURSES_H__
	#include "curses.h"
#endif

FILE * load_edict();
void load_vocab(vocab_t **);
bool add_line_to_node(vocab_t **, char [], bool, int, unsigned short);
void add_line(vocab_t **, FILE *archivo,int);
int longest_line(FILE *archivo);
int show_vocab(vocab_t *, int, pantalla_t *, bool);
void show_vocab_item(vocab_t *, pantalla_t *, int);
int show_cat(vocab_t *, pantalla_t *);
void save_vocab(vocab_t *);
void edit_vocab(vocab_t *, char [], pantalla_t *);
bool add_vocab(vocab_t **, vocab_t *, int);
void delete_vocab(vocab_t **, pantalla_t *);
void new_cat(vocab_t **, char []);
void delete_cat(vocab_t **, int, pantalla_t *);
int current_cat(vocab_t *, int, bool, WINDOW *);
vocab_t * go_to_cat(vocab_t *, int);
vocab_t * go_to_item(vocab_t *, int, int);
int select_cat(vocab_t *, int, pantalla_t *, const char []);
