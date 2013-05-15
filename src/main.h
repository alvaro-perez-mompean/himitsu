typedef struct vocab {
	char *pkanji;
	char *phiragana;
	char *pmeaning;
	char *pcat;
	unsigned short learning;
	struct vocab *panterior;
	struct vocab *psiguiente;
}vocab_t;

typedef struct pantalla {
	unsigned short cols;
	unsigned short lines;
	
	WINDOW *menu;
	WINDOW *ppal;
	WINDOW *buffer;
	unsigned short ppal_cols;
	int ppal_pbuf;
	int ppal_finbuf;
}pantalla_t;


#ifndef __MAIN_H__
	#define __MAIN_H__
	#include "vocab.h"
	#define __VOCAB_H__
	#include "search.h"
	#include "kanji.h"
	#include "learning.h"
	#include "curses.h"
#endif

#define __HIMITSU_VERSION__ "0.0.3"


int main_menu(vocab_t *);
void draw_menu(vocab_t *, int, int);
void exit_mem(int, const char []);
bool clean_stdin();
