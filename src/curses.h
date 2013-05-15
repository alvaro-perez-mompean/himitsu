#ifndef __MAIN_H__
	#define __MAIN_H__
	#include "main.h"
#endif

void init_curses(pantalla_t *);
void resize_pant(pantalla_t *);
void upgrade_buffer(pantalla_t *, bool);
void scroll_keys(pantalla_t *, int, bool);
int select_item(pantalla_t *, int);
void scroll_scr(pantalla_t *, int);
void print_buffer(pantalla_t *, const char *, bool);
void print_new_line(pantalla_t *);