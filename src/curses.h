#ifndef __MAIN_H__
	#define __MAIN_H__
	#include "main.h"
#endif

void init_curses(pantalla_t *);
void resize_pant();
void upgrade_buffer(bool);
void scroll_keys(int, bool);
int select_item(int);
void scroll_scr(int);
void print_buffer(const char *, bool);
void print_buffer_new_line();
void clear_buffer();
pantalla_t * get_curses();
