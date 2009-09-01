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
	buffer = buffer_utf8 = NULL;
	char *pent, *psal;
	size_t codent, codsal;
	iconv_t desc;
	
	pent = psal = NULL;
	
	busq = NULL;
	
	// Load edict.
	FILE *edict;
	
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
		
	if (fclose(edict) != 0) {
		endwin();
		printf("Error closing edict file.\n");
		exit(1);
	}
	
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


