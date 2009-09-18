#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

#include "learning.h"

static void generate_revision(vocab_t *, int, pantalla_t *);
static void revision_state(unsigned short, pantalla_t *);
static void show_unknow(vocab_t *, int, pantalla_t *);

void revision_menu(vocab_t *listavocab, pantalla_t *pant) {
	
	int cat = 0, option;
	
	wclear(pant->menu);
	wclear(pant->buffer);
	upgrade_buffer(pant, FALSE);
	wrefresh(pant->ppal);
	option='a';
	while (option != '0') {
		draw_menu(pant, listavocab, cat, 1);
		option = 'a';
		option = wgetch(pant->menu);
		// If window change its size, we resize menu and ppal.
		if ((pant->cols != COLS) || (pant->lines != LINES)) {
			pant->cols = COLS;
			pant->lines = LINES;
			resize_pant(pant);
		}
		if ((option < '0') || (option > '4')) {
			// Invalid option.
			option='a';
		} else if (option == '1') {
			generate_revision(listavocab, cat,pant);

		} else if (option == '2') {
			wclear(pant->ppal);
			cat = 0;
			cat = select_cat(listavocab, cat, pant, "to work with");
				
		} else if (option == '3') {
			cat = select_cat(listavocab, cat, pant, "to work with");
			if (cat > 0) {
				show_unknow(listavocab,cat,pant);
				//wprintw(pant->,"\n");
			}
				
		} else if (option == '4') {
			wclear(pant->ppal);
			wprintw(pant->ppal,"Saving...\n\n");
			wrefresh(pant->ppal);
			save_vocab(listavocab);
			wclear(pant->ppal);
			wprintw(pant->ppal,"Saved!\n\n");
		}
	}
}


static void generate_revision(vocab_t *listavocab, int cat, pantalla_t *pant) {

	
	char respuesta[256];
	char selecc;
	int *preguntas = NULL, *noaprend = NULL;
	int resultados = 0, elementos = 0, numpreg, i,aciertos;
	bool repe = false;
	vocab_t *aux, *prepaso = listavocab;
	
	srand(time(NULL));
	
	// Clearing screen...
	wclear(pant->ppal);
	
	cat = select_cat(listavocab, cat, pant, "to review");
	draw_menu(pant, listavocab, cat, 1);	
		
	if (cat > 0) {
		prepaso = go_to_cat(prepaso, cat);
		
		// Here we are over the list that we're looking for.
		if (!prepaso->psiguiente) {
			cat = 0;
			wprintw(pant->buffer,"\nThe list is empthy.\n");
		} else if (prepaso->psiguiente->pcat) {
			cat = 0;
			wprintw(pant->buffer,"\nThe list is empthy.\n");
		} else {
			selecc = '0';
			while ((selecc != 'y') && (selecc != 'n')) {
				wprintw(pant->buffer,"\nInclude known words? (y/n): ");
				upgrade_buffer(pant, TRUE);
				selecc = wgetch(pant->ppal);
				wprintw(pant->buffer,"\n\n");
			}
					
			// Save list's number of elements.
			aux = prepaso->psiguiente;
			elementos = 0;
			while (aux && !aux->pcat) {
				if (selecc == 'y') {
					elementos++;
				} else {
					if (aux->learning != 3)
						elementos++;
				}
				aux = aux->psiguiente;
			}
			// If we don't include known words, use no aprend.
			if (selecc == 'n') {
				noaprend = (int *)calloc(elementos,sizeof(int));
				aux = prepaso->psiguiente;
				i = elementos = 0;
				while (aux && !aux->pcat) {
					i++;
					if (aux->learning != 3) {
						*(noaprend+elementos) = i;
						elementos++;
					}
					aux = aux->psiguiente;
				}
			}
					
					
		}
		
		/*if we're here and elementos' value is 0, it's because all the
		 * words were known.*/
		if (elementos == 0) {
			wprintw(pant->buffer,"\nAll list's elements was learned.\n\n");
		} else if (cat == 0) {
			wprintw(pant->buffer,"The list is empthy\n\n");
		} else {
			wprintw(pant->buffer,"How many words do you want to review? (1-%d): ",elementos);
			numpreg = 0;
			while (numpreg < 1 || numpreg > elementos) {
				upgrade_buffer(pant, TRUE);
				wscanw(pant->ppal,"%d",&numpreg);
				if (numpreg < 1 || numpreg > elementos)
					wprintw(pant->buffer,"Error, enter a numbre between 1 and %d: ",elementos);
			}
			wclear(pant->buffer);
			preguntas = (int *)calloc(numpreg,sizeof(int));
			resultados = 0;
			if (selecc == 'y') {
				while (resultados<numpreg) {
					*(preguntas+resultados)	= (rand() % elementos)+1;
					if (resultados>0) {
						i=0;
						repe = false;
						while (i<resultados && !repe) {
							if (*(preguntas+i) == *(preguntas+resultados))
								repe = true;
							i++;
						}
					}
					if (!repe)
						resultados++;
				}
				
			} else {
				while (resultados<numpreg) {
					*(preguntas+resultados) = *(noaprend+(rand() % elementos));
					if (resultados>0) {
						i=0;
						repe = false;
						while (i<resultados && !repe) {
							if (*(preguntas+i) == *(preguntas+resultados))
								repe = true;
							i++;
						}
						i=0;
					}
					if (!repe)
						resultados++;
				}
			}
		}
	}
	if ((cat != 0) && (elementos != 0)) {
		resultados = 0;
		aciertos = 0;
		while (resultados<numpreg) {
			aux = prepaso;
			// We're over the question.
			for (i=0;i<*(preguntas+resultados);i++)
				aux = aux->psiguiente;
				 
				resultados++;
				respuesta[0] = '\0';
				wprintw(pant->buffer,"[%d] %s.\n  Answer: ", resultados, aux->pmeaning);
				while (respuesta[0] == '\0'){
					upgrade_buffer(pant, TRUE);
					wscanw(pant->ppal,"%255[^\n]",respuesta);
					// If window changes, we resize menu and ppal.
					if ((pant->cols != COLS) || (pant->lines != LINES)) {
						pant->cols = COLS;
						pant->lines = LINES;
						resize_pant(pant);
						draw_menu(pant,listavocab,cat,1);
					}
				}
				wclear(pant->buffer);
				if (aux->pkanji) {
					if ( (strstr(aux->pkanji,respuesta) && (strlen(aux->pkanji) == strlen(respuesta))) ||  (strstr(aux->phiragana,respuesta) && (strlen(aux->phiragana) == strlen(respuesta)))) {
						aciertos++;
						wprintw(pant->buffer,"Correct answer. \"%s (%s)\". ", aux->pkanji, aux->phiragana);
						if (aux->learning == 0) {
							aux->learning = 2;
						} else if (aux->learning < 3) {
							aux->learning++;
						}
					} else {
						wprintw(pant->buffer,"Wrong answer (\"%s\").\n\"%s: \"%s (%s)\".",respuesta, aux->pmeaning, aux->pkanji, aux->phiragana);
						if (aux->learning > 0 && aux->learning <=3)
							aux->learning = 1;
					}
				} else {
					if (strstr(aux->phiragana,respuesta) && (strlen(aux->phiragana) == strlen(respuesta))) {
						aciertos++;
						wprintw(pant->buffer,"Correct answer. \"%s\". ", aux->phiragana);
						if (aux->learning == 0) {
							aux->learning =2;
					} else if (aux->learning < 3) {
						aux->learning++;
					}
				} else {
					wprintw(pant->buffer,"Wrong answer (\"%s\").\n\"%s: \"%s\".",respuesta, aux->pmeaning, aux->phiragana);
					if (aux->learning > 0 && aux->learning <=3)
						aux->learning = 1;
				}
			}
			wprintw(pant->buffer,"State of word: ");
			revision_state(aux->learning,pant);
		}
		wprintw(pant->buffer,"%d correct words of %d.\n\n",aciertos, numpreg);
	}
	upgrade_buffer(pant, FALSE);
	wrefresh(pant->ppal);
	if (preguntas)
		free(preguntas);
	if (noaprend)
		free(noaprend);
	preguntas = noaprend = NULL;
			
	
	
	
}

void revision_state(unsigned short estado, pantalla_t *pant) {
	if (estado == 0) {
		wprintw(pant->buffer,"Unknown");
	} else if (estado == 1) {
		wprintw(pant->buffer,"Problematic");
	} else if (estado == 2) {
		wprintw(pant->buffer,"Familiar");
	} else if (estado == 3) {
		wprintw(pant->buffer,"Known");
	}
	wprintw(pant->buffer,"\n\n");
}

void show_unknow(vocab_t *listavocab, int cat, pantalla_t *pant) {
	
	int resultados = 0;
	
	wclear(pant->buffer);
	if (listavocab) {
		
		listavocab = go_to_cat(listavocab, cat);
		listavocab = listavocab->psiguiente;

		resultados = 0;
		//while ( listavocab && !listavocab->pcat && (getcury(pant->ppal) < LINES-2)) {
		while ( listavocab && !listavocab->pcat) {
			if (listavocab->learning != 3) {
				resultados++;
				if (listavocab->pkanji) {
					wprintw(pant->buffer,"[%d] %s (%s): %s\n",resultados,listavocab->pkanji,listavocab->phiragana,listavocab->pmeaning);
				// Temporaly if to study english irregular verbs.
				} else if (*listavocab->phiragana >= 97 && *listavocab->phiragana <= 122) {
					wprintw(pant->buffer,"[%d] %s: %s\n",resultados,listavocab->pmeaning,listavocab->phiragana);
				} else {
					wprintw(pant->buffer,"[%d] %s: %s\n",resultados,listavocab->phiragana,listavocab->pmeaning);
				}
			}
			
			listavocab = listavocab->psiguiente;
		}
	
		if (resultados == 0)
			wprintw(pant->buffer,"All list's elements was learned.\n\n");
	}
	upgrade_buffer(pant, FALSE);
	wrefresh(pant->ppal);
}
