/*
 *      curses.c
 *      
 *      Copyright 2009 Álvaro P. Mompeán <apmomp@gmail.com>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>
#include <locale.h>

#include "curses.h"

#define TAM_BUF 1024

void init_curses(pantalla_t *pantalla) {
	
	setlocale(LC_CTYPE, "es_ES.UTF-8");
	// Init screen.
    (void) initscr();
	// Enabling keyboard mapping.
    keypad(stdscr, TRUE);
	// Tell to Ncursesw that musn't do NL->CR/NL.
    (void) nonl();
	// Take character one by one, without waiting to "\n."
    (void) cbreak();
	scrollok(stdscr, TRUE); /* Enable scroll */
	if (COLS <= 40 || LINES < 20) {
		exit_mem(EXIT_FAILURE, "ERROR, terminal too small. Interface can't be built.");
	}
	pantalla->menu = newwin(LINES,27,0,COLS-27);
	pantalla->ppal = newwin(LINES,COLS-27,0,0);
	pantalla->lines = LINES;
	pantalla->ppal_cols = COLS-27;
	pantalla->buffer = newpad(TAM_BUF,COLS-27);
	keypad(pantalla->menu, TRUE);
	keypad(pantalla->ppal, TRUE);
	
	scrollok(pantalla->ppal, TRUE);
	wborder(pantalla->menu,0,0,0,0,0,0,0,0);

    if (has_colors()) {
        start_color();
				init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
        init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    }
}

// resize_pant() resizes "buffer", "menu" and "ppal".
void resize_pant(pantalla_t *pant) {
	
	if (COLS > 40) {
		pant->cols = COLS;
		pant->lines = LINES;
		pant->lines = LINES;
		pant->ppal_cols = COLS-27;
		wresize(pant->ppal, LINES, COLS-27);
		wresize(pant->buffer, TAM_BUF, COLS-27);
		
		wresize(pant->menu, LINES,27);
		mvwin(pant->menu, 0,COLS-27);
		

		wrefresh(pant->menu);
		upgrade_buffer(pant, FALSE);
		wrefresh(pant->ppal);
	}
}

// upgrade_buffer() moves pant->buffer into pant->ppal.
void upgrade_buffer(pantalla_t *pant, bool move_cursor) {
	
	wclear(pant->ppal);
	pant->ppal_finbuf = getcury(pant->buffer);

	scroll_scr(pant, 0);
	
	if (move_cursor)
		wmove(pant->ppal,getcury(pant->buffer),getcurx(pant->buffer));

}

void scroll_keys(pantalla_t *pant, int key_pressed, bool submenu) {
	// Up
	if (key_pressed == KEY_UP) {
		if (((!submenu) && (pant->ppal_pbuf > 0))
		|| ((pant->ppal_pbuf > 0) && !(getcury(pant->ppal) > 0)) ) {
			scroll_scr(pant, -1);
		} else {
				wmove(pant->ppal,getcury(pant->ppal)-1,0);
		}
	// Down.
	} else if (key_pressed == KEY_DOWN) {
		if (((!submenu) && (pant->ppal_finbuf > (pant->ppal_pbuf+pant->lines)))
		|| (!(getcury(pant->ppal) < pant->lines-1) && (pant->ppal_finbuf > (pant->ppal_pbuf+pant->lines)))) {
			scroll_scr(pant, 1);
		} else {
			wmove(pant->ppal,getcury(pant->ppal)+1,0);
		}
	// Re pag.
	} else if ((key_pressed == KEY_PPAGE) && (pant->ppal_pbuf >= pant->lines)) {
		scroll_scr(pant, -(pant->lines));
	} else if ((key_pressed == KEY_PPAGE) && (pant->ppal_pbuf > 0)) {
		scroll_scr(pant, 0);
	// Av pag.
	} else if ((key_pressed == KEY_NPAGE) && ((pant->ppal_finbuf-pant->lines) > (pant->ppal_pbuf+pant->lines))) {
		scroll_scr(pant, pant->lines);
	} else if ((key_pressed == KEY_NPAGE) && ((pant->ppal_finbuf > 0) && (pant->ppal_finbuf > pant->lines))) {
		scroll_scr(pant, pant->ppal_finbuf);
	}
	if ((pant->cols != COLS) || (pant->lines != LINES)) {
			resize_pant(pant);
			wrefresh(pant->ppal);
	}
}


int select_item(pantalla_t *pant, int registro) {
	
	int i=0;
	
	char *char_temp;
	char_temp = (char *)calloc(1,sizeof(char));
	
	if ((mvwinch(pant->ppal,getcury(pant->ppal),getcurx(pant->ppal)) == '[') && (registro != 27)) {
		for (i=0; mvwinch(pant->ppal,getcury(pant->ppal),getcurx(pant->ppal)+1) != ']'; i++) {
			char_temp = (char *)realloc(char_temp,(i+2)*sizeof(char));
			*(char_temp+i) = mvwinch(pant->ppal,getcury(pant->ppal),getcurx(pant->ppal));
		}

		*(char_temp+i) = '\0';
		registro = atoi(char_temp);
		
		
	} else
		registro = 0;
		
	if (char_temp) {
		free(char_temp);
		char_temp = NULL;
	}
		
	return registro;
}

void scroll_scr(pantalla_t *pant, int elem) {
	if (elem == 0) {
		copywin(pant->buffer, pant->ppal,0,0,0,0,pant->lines-1,pant->ppal_cols-1,FALSE);
		pant->ppal_pbuf = 0;
	} else if (elem == pant->ppal_finbuf) {
		copywin(pant->buffer, pant->ppal,pant->ppal_finbuf-pant->lines,0,0,0,pant->lines-1,pant->ppal_cols-1,FALSE);
		pant->ppal_pbuf = pant->ppal_finbuf-pant->lines;
	} else {
		copywin(pant->buffer, pant->ppal,pant->ppal_pbuf+elem,0,0,0,pant->lines-1,pant->ppal_cols-1,FALSE);
		pant->ppal_pbuf = pant->ppal_pbuf+elem;
	}
}

void print_buffer(pantalla_t *pant, const char *str, bool new_line) {
	wprintw(pant->buffer, str);
	if (new_line)
		print_new_line(pant);
}

void print_new_line(pantalla_t *pant) {
	wprintw(pant->buffer, "\n");
}