/*
 *      Himitsu
 *      
 *      Copyright 2009 Álvaro P. Mompeán <apmomp@altair>
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
#include <ncurses.h>


#include "main.h"



int main() {
	
	vocab_t *listavocab = NULL;
	pantalla_t *pant;
	
	FILE *edict, *kanjidic;
	
	pant = (pantalla_t *)malloc(sizeof(pantalla_t));
	
	edict = kanjidic = NULL;
	
	edict = load_edict();
	if (fclose(edict) != 0)
		exit_mem(EXIT_FAILURE, "Error closing edict file.");
	
	kanjidic = load_kanjidic();
	if (fclose(kanjidic) != 0)
		exit_mem(EXIT_FAILURE, "Error closing kannidic file.");
	
	
	if (!pant)
		exit_mem(EXIT_FAILURE, "Not enough memory.");


	init_curses(pant);
	wrefresh(pant->ppal);
	
	
	load_vocab(&listavocab);
	
	return(main_menu(listavocab, pant));
	
}

int main_menu(vocab_t *listavocab, pantalla_t *pant) {

	char palabra[256];
	int resultados = 0,registro=0,cat=0;
	char opcion = 'a';
	bool buscado = false;
	pant->cols = COLS;
	pant->lines = LINES;
	wprintw(pant->buffer,"\n Himitsu v%s (%s)\n",__HIMITSU_VERSION__, __TIMESTAMP__);
	wprintw(pant->buffer," Built on %s %s\n\n", __DATE__, __TIME__);
	wprintw(pant->buffer," Copyright (C) 2008, 2009 Álvaro P. Mompeán.\n\n");
	wprintw(pant->buffer," This program comes with ABSOLUTELY NO WARRANTY.\n");
	wprintw(pant->buffer," This is free software, and you are welcome to redistribute it\n");
	wprintw(pant->buffer," under certain conditions.\n\n");
	upgrade_buffer(pant, FALSE);
	wrefresh(pant->ppal);
	
	while(TRUE) {
		if ((pant->cols != COLS) || (pant->lines != LINES)) {
			resize_pant(pant);
			wrefresh(pant->ppal);
		}
		draw_menu(pant,listavocab, cat, 0);
		//wprintw(pant->ppal,"TECLA: %d", opcion);
		wrefresh(pant->ppal);
		opcion='a';
		//while (opcion < 0 || opcion > 10) {
		while (opcion == 'a') {
			draw_menu(pant, listavocab, cat, 0);
			opcion = 'a';
			//wscanw(pant->menu,"%d",&opcion);
			opcion = wgetch(pant->menu);
			//clean_stdin();
			if ((pant->cols != COLS) || (pant->lines != LINES)) {
				pant->cols = COLS;
				pant->lines = LINES;
				resize_pant(pant);
				wrefresh(pant->ppal);
			}

			if (((opcion < '0' || opcion > '9')) && opcion != 'r') {
				mvwprintw(pant->menu,16,1,"Option not valid: ",opcion);
				opcion=11;
			
			// save_vocab.
			} else if (opcion == '0') {
				save_vocab(listavocab);
				endwin();
				return 0;
			
			// Review mode.
			} else if (opcion == 'r') {
				buscado = false;
				revision_menu(listavocab, pant);
			
			// Delete list.
			} else if (opcion == '9') {
				wclear(pant->buffer);
				buscado = false;
				cat = 0;
				cat = select_cat(listavocab, cat, pant, "to delete");
				if (cat > 0)
					delete_cat(&listavocab, cat, pant);
				cat = 0;
				
				upgrade_buffer(pant,FALSE);
			
			// Create list.
			} else if (opcion == '8') {
				wclear(pant->buffer);
				buscado = false;
				wprintw(pant->buffer,"Enter the name for the new category: ");
				upgrade_buffer(pant, TRUE);
				palabra[0] = '\0';
				wscanw(pant->ppal,"%16[^\n]",palabra);
				if (palabra[0] == '\0') {
					wprintw(pant->buffer,"Name not valid.\n\n");
				} else {
					new_cat(&listavocab, palabra);
					wclear(pant->buffer);
					wprintw(pant->buffer, "\"%s\" created", palabra);
				}
				upgrade_buffer(pant,false);
			
			// Select list.
			} else if (opcion == '7') { 
				wclear(pant->buffer);
				//buscado = false;
				cat = 0;
				cat = select_cat(listavocab, cat, pant, "to work with");
			
			// Show kanjis.
			} else if (opcion == '6') {
				buscado = false;
				wclear(pant->buffer);
				cat = select_cat(listavocab, cat, pant, "to work with");
				if (cat > 0) {
					resultados=show_vocab(listavocab,cat,pant, true);
					if (resultados == 0) {
						wprintw(pant->buffer,"Vocabulary list is empthy.\n\n");
					} else {
						registro = 0;
						while ((registro != 13) && (registro != 27)) {
							wrefresh(pant->ppal);
							registro = wgetch(pant->menu);
							scroll_keys(pant,registro,TRUE);				
						}
						
							
						//registro = select_item(pant, chartemp, registro);
						registro = select_item(pant, registro);
							
							
						if ((registro <= resultados) && (registro > 0)) {
							wclear(pant->buffer);
							listavocab = go_to_item(listavocab, cat, registro);
							show_kanji(listavocab, pant);
						} else {
							wclear(pant->buffer);
							wprintw(pant->buffer,"Incorrect register...\n\n");
						}
						upgrade_buffer(pant,FALSE);
					}
				}
			
			// Edit word.
			} else if (opcion == '5') {
				wclear(pant->buffer);
				buscado = false;
				cat = select_cat(listavocab, cat, pant, "to work with");
				if (cat > 0) {
					resultados=show_vocab(listavocab,cat,pant, true);
					if (resultados == 0) {
						wprintw(pant->ppal,"The list is empthy.\n\n");
					} else {
						registro = 0;
						while ((registro != 13) && (registro != 27)) {
							wrefresh(pant->ppal);
							registro = wgetch(pant->menu);
							scroll_keys(pant,registro,TRUE);				
						}
						registro = select_item(pant, registro);
						
						if ((registro <= resultados) && (registro > 0)) {
							wclear(pant->buffer);
							listavocab = go_to_item(listavocab, cat, registro);
							wprintw(pant->buffer,"Enter the new text for \"");
							show_vocab_item(listavocab, pant, registro);
							wprintw(pant->buffer,"\": ");
							palabra[0] = '\0';
							upgrade_buffer(pant, TRUE);
							wscanw(pant->ppal,"%255[^\n]",palabra);
							if (palabra[0] == '\0') {
								wprintw(pant->buffer,"\n\nCancelled.\n");
							} else {
								edit_vocab(listavocab, palabra, pant);
								wprintw(pant->buffer,"\n");
							}
						} else {
							wprintw(pant->buffer,"Incorrect register...\n\n",resultados);
						}
					}
					upgrade_buffer(pant,FALSE);
				}
					
				
			// Delete word.
			} else if (opcion == '4') {
				wclear(pant->buffer);
				buscado = false;
				cat = select_cat(listavocab, cat, pant, "to work with");
				if (cat > 0) {
					resultados=show_vocab(listavocab,cat,pant, true);
					if (resultados == 0) {
						wprintw(pant->buffer,"The list is empthy.\n\n");
					} else {
						
						registro = 0;
						while ((registro != 13) && (registro != 27)) {
							wrefresh(pant->ppal);
							registro = wgetch(pant->menu);
							scroll_keys(pant,registro,TRUE);				
						}
						
						registro = select_item(pant, registro);
						
						if ((registro <= resultados) && (registro > 0)) {
							wclear(pant->buffer);
							listavocab = go_to_item(listavocab, cat, registro);
							//wprintw(pant->ppal,"\n");
							delete_vocab(&listavocab,pant);
						} else {
							wclear(pant->buffer);
							wprintw(pant->buffer,"Incorrect register...\n\n",resultados);
						}
					}
					upgrade_buffer(pant,FALSE);
				}
			
			// Show list.
			} else if (opcion == '3') {
				wclear(pant->buffer);
				buscado = false;
				cat = select_cat(listavocab, cat, pant, "to show");
				if (cat > 0) {
					resultados=show_vocab(listavocab, cat,pant, true);
					if (resultados == 0)
						wprintw(pant->buffer,"The list is empthy.\n\n");
				}
				wrefresh(pant->ppal);
			
			// Add word.
			} else if (opcion == '2') {
				if (!buscado) {
					wclear(pant->buffer);
					wprintw(pant->buffer,"First you must search something...\n\n");
				} else if (resultados == 0) {
					wprintw(pant->buffer,"There wasn't any search result, so there isn't any word to store.\n\n");
				} else {
					if (cat <= 0) {	
						wclear(pant->buffer);
						cat = select_cat(listavocab, cat, pant, "to store the word in");
					}
					if (cat > 0) {
						wclear(pant->buffer);
						search(&listavocab,palabra,0, cat, pant);
						
						while ((registro != 13) && (registro != 27)) {
							wrefresh(pant->ppal);
							registro = wgetch(pant->menu);
							scroll_keys(pant,registro,TRUE);				
						}
						
						registro = select_item(pant, registro);
						
						if ((resultados > 0) && (registro > 0) && (registro <= resultados)) {
							search(&listavocab,palabra,registro, cat, pant);
						} else {
							wprintw(pant->ppal,"\nIncorrect register...\n",resultados);
						}
					}
				}
				upgrade_buffer(pant, FALSE);
			
			// Search.
			} else if (opcion == '1') {
				//japo=1;
				buscado = true;
				registro = 0;
				wclear(pant->buffer);
				wprintw(pant->buffer,"Enter a word to search: ");
				palabra[0] = '\0';
				upgrade_buffer(pant, TRUE);
				wscanw(pant->ppal,"%39s[^\n]",palabra);
				printw("%s",palabra);
				resultados=search(&listavocab,palabra,registro,0,pant);
				if (resultados == 0) {
					wprintw(pant->buffer,"\"%s\" not found.\n\n",palabra);
					//japo=2;
					buscado = false;
				}
				upgrade_buffer(pant, FALSE);
				wrefresh(pant->ppal);
				
			// Scroll keys.
			} else
				scroll_keys(pant, opcion, FALSE);
			
		}
	}
}

// Clear input's buffer.
bool clean_stdin() {
	char c='0';
	// With datos we return if the buffer just contains '\n'.
	bool datos = false;
	while(c != '\n') {
		c = getchar();
		if (c != '\n')
			datos = true;
	}
	return datos;
}

void draw_menu(pantalla_t *pant, vocab_t *listavocab, int cat, int learning) {
	wrefresh(pant->ppal);
	if (learning == 0) {
		wclear(pant->menu);
		mvwprintw(pant->menu,1,1,"Chosen list:\n");
		if (cat > 0)
			current_cat(listavocab, cat, true, pant->menu);
		else
			mvwprintw(pant->menu,2,1,"None.");

		mvwprintw(pant->menu,4,1,"1) Search");
		mvwprintw(pant->menu,5,1,"2) Add word into the list");
		mvwprintw(pant->menu,6,1,"3) Show list");
		mvwprintw(pant->menu,7,1,"4) Delete word");
		mvwprintw(pant->menu,8,1,"5) Edit word");
		mvwprintw(pant->menu,9,1,"6) Show kanjis");
		mvwprintw(pant->menu,10,1,"7) Select list");
		mvwprintw(pant->menu,11,1,"8) Create new list");
		mvwprintw(pant->menu,12,1,"9) Delete list");
		
		mvwprintw(pant->menu,14,1,"0) Save and exit");
		mvwprintw(pant->menu,16,1,"r) Revision mode");
		
		mvwprintw(pant->menu,18,1,"Choose an option: ");
		
		box(pant->menu,0,0);
		wrefresh(pant->menu);
	} else {
		wclear(pant->menu);
		wrefresh(pant->ppal);
		mvwprintw(pant->menu,1,1,"Chosen list: ");
		if (cat>0)
			current_cat(listavocab, cat, true, pant->menu);
		else 
			mvwprintw(pant->menu,2,1,"Ninguna\n\n");
		mvwprintw(pant->menu,4,1,"1) Review list");
		mvwprintw(pant->menu,5,1,"2) Select List");
		mvwprintw(pant->menu,6,1,"3) Show unknown words");
		mvwprintw(pant->menu,7,1,"4) Save vocab lists");
		mvwprintw(pant->menu,9,1,"0) Exit revision mode");
		
		mvwprintw(pant->menu,11,1,"Choose an option: ");
		box(pant->menu,0,0);
		
		wrefresh(pant->menu);
	}
	
}

void exit_mem(int valor, const char mensaje[]) {
	
	endwin();
	printf("%s \n\n", mensaje);
	exit(valor);
	
}

// cleanstdin( clears stdin and returns if buffer is empthy.
bool cleanstdin() {
	char c='0';
	bool is_empthy = TRUE;
	while(c != '\n') {
		c = getchar();
		if (c != '\n')
			is_empthy = FALSE;
		
    }
    
    return is_empthy;
}

