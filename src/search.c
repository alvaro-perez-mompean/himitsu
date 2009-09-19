/*
 *      search.c
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
#include <string.h>
#include <ncurses.h>

#include "search.h"
#include <iconv.h>

/*This function searches for a word and adds it to a list.*/
int search(vocab_t **listavocab, char search[], int registro, int cat, pantalla_t *pant) {
	
	
	int resultados=0, tam_buffer=0;
	char *busq;
	bool exac=true; // Partial search.
	bool encont, japo;
	int x, conc; // Conc tells if jap's word is at the beginning.
	
	// Vars for conv.
	char *buffer, *buffer_utf8;
	
	char *pent, *psal;
	FILE *edict;
	size_t codent, codsal;
	iconv_t desc;
	
	buffer = buffer_utf8 = NULL;
	pent = psal = NULL;
	
	busq = NULL;
	
	// Load edict.
	edict = load_edict();
	
	tam_buffer = (longest_line(edict)+1);
	rewind(edict);
	
	buffer = (char *)malloc(tam_buffer*sizeof(char));
	buffer_utf8 = (char *)malloc(tam_buffer*sizeof(char));
	
	
	
	if (edict) {
		if ((search[0] >= 65) && (search[0] <= 122))
			japo = false;
		else
			japo = true;
	
		if (!japo) {
			busq = (char *)calloc((strlen(search)+1),sizeof(char));
			strcpy((busq), search);
		} else if (japo) {
			busq = (char *)calloc((strlen(search)+1),sizeof(char));
			strcpy((busq), search);
		}

		if (japo) {	
			if (*(busq+strlen(busq)-1) == '*') {
				*(busq+strlen(busq)-1) = '\0';
				exac=false;
			}
		} else if (!japo) {
			if (*(busq+strlen(busq)-1) == '*') {
				*(busq+strlen(busq)-1) = '\0';
				exac=false;
			}
		}
		// Jump to the second line.
		fscanf(edict,"%[^\n]%*[\n]",buffer);
		desc = iconv_open("UTF-8", "EUC-JP");
		wclear(pant->buffer);
		while (fscanf(edict,"%[^\n]%*[\n]",buffer) != EOF) {
			encont = false;
			pent = &buffer[0];
			psal = &buffer_utf8[0];
			codent = strlen(buffer)*sizeof(char)+1;
			codsal = tam_buffer*sizeof(char);
			iconv(desc,&pent,&codent,&psal,&codsal);
			
			// It's a japanese word.
			if (japo) {
				// It doesn't contain kanjis.
				if (!strstr(buffer_utf8,"[")) {
					if (strstr(buffer_utf8, busq)) {
						// Check if the word is at the string's beginning.
						for (x=0,conc=0;x<(int)strlen(busq);x++) {
							if (busq[x] == *(buffer_utf8+x))
								conc++;
						}
						if ( (conc == (int)strlen(busq)) && 
						( !exac || *(buffer_utf8+strlen(busq)) == ' ') )   {
							resultados++;
							encont = true;
						}		
					}
					// It contains kanjis.
				} else {
					if (strstr(buffer_utf8, busq)) {
						// Check if the word is at the string's beginning.
						for (x=0,conc=0;x<(int)strlen(busq);x++) {
							if (busq[x] == *(buffer_utf8+x))
								conc++;
						}
						// Kanji
						if ( (conc == (int)strlen(busq)) &&
						(!exac || *(buffer_utf8+strlen(busq)) == ' ') ) {
							resultados++;
							encont = true;
						// Hiragana
						} else if ( (*(strstr(buffer_utf8,busq)-1) == '[') &&
						( !exac || *(strstr(buffer_utf8,busq)+strlen(busq)) == ']') ) {
							resultados++;
							encont = true;
						}
					}
				}
				
			// It's an english word.
			} else {
				if (strstr(buffer_utf8, busq)) {
					if ( ((*((strstr(buffer_utf8, busq))+strlen(busq)) == '/' && *((strstr(buffer_utf8, busq))-1) == ' ' && *((strstr(buffer_utf8, busq))-2) == ')') 
					|| (*((strstr(buffer_utf8, busq))+strlen(busq)) == '/' && *((strstr(buffer_utf8, busq))-1) == '/')) ||
					
					(!exac && ( ( *((strstr(buffer_utf8, busq))-1) == ' ' && *((strstr(buffer_utf8, busq))-2) == ')' )
					|| (*(strstr(buffer_utf8, busq))-1) == '/')) ) {
						resultados++;
						encont = true;
					}
				}
	
			}
			
			if (encont) {
				if (registro == 0)
					wprintw(pant->buffer,"[%d] %s\n", resultados, buffer_utf8);					
				else if (registro == resultados) {
					if (add_line_to_node(listavocab,buffer_utf8,true,cat,0))
							wprintw(pant->buffer,"Word stored correctly.\n\n");
						else
							wprintw(pant->buffer,"That word already exists in this list.\n\n");
							
						wrefresh(pant->buffer);
				}
	
			}
		}
		iconv_close(desc);
			
	}
		
	if (fclose(edict) != 0)
		exit_mem(EXIT_FAILURE, "Error closing edict file.");
	
	if (resultados > 0)
	    wprintw(pant->buffer,"\n");
	if (busq)
		free(busq);
	if (buffer)
		free(buffer);
	if (buffer_utf8)
		free(buffer_utf8);
	busq = buffer = buffer_utf8 = NULL;
	
	wprintw(pant->buffer,"\n");
	upgrade_buffer(pant, FALSE);
	
	
	return resultados;
}


