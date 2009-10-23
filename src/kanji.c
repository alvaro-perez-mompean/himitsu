/*
 *      kanji.c
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
#include <iconv.h>
#include <ncurses.h>

#include "kanji.h"

static int longest_line_kanji(FILE *);


int show_kanji(vocab_t *listavocab, pantalla_t *pant) {
	unsigned short resultados=0; // Aux is used to order kanjis.
	int tam_buffer = 0;
	int tam_buffer_atrib = 0;
	int i;
	char *imitemp = NULL; // Shows meaning without keys and brackets.
	char *kanji, *atributos, *reading, *meaning;
	char *buffer, *buffer_utf8;
	char *pos;
	unsigned short heisig, strokes;
	char *pent, *psal;
	size_t codent, codsal;
	iconv_t desc;
	FILE *kanjidic;
	
	// Load kanjidic.
	kanjidic = load_edict(false);
	
	tam_buffer = (longest_line(kanjidic)+1);
	rewind(kanjidic);
	tam_buffer_atrib = (longest_line_kanji(kanjidic)+1);
	rewind(kanjidic);
	
	
	kanji = (char *)calloc(4,sizeof(char));
	atributos = (char *)calloc(tam_buffer_atrib,sizeof(char));
	reading = (char *)calloc(tam_buffer_atrib,sizeof(char));
	meaning = (char *)calloc(tam_buffer_atrib,sizeof(char));
	
	
	// Necesary for iconv.
	buffer = (char *)calloc(tam_buffer,sizeof(char));
	buffer_utf8 = (char *)calloc(tam_buffer,sizeof(char));
	
	
	pent = psal = NULL;
	
	if (listavocab) {

		// Make the search and show the result on the screen.
		resultados=0;
		// If the word has kanji...
		if (listavocab->pkanji) {
			if (kanjidic) {
				wprintw(pant->buffer,"(%s):\n\n",listavocab->pkanji);
				if (fscanf(kanjidic,"%[^\n]%*[\n]",buffer) != EOF) {
					desc = iconv_open("UTF-8", "EUC-JP");
					while (fscanf(kanjidic,"%[^\n]%*[\n]",buffer) != EOF ) {
						pent = &buffer[0];
						psal = &buffer_utf8[0];
						codent = strlen(buffer)*sizeof(char)+1;
						codsal = tam_buffer*sizeof(char);
						iconv(desc,&pent,&codent,&psal,&codsal);
						for (i=0;i<3;i++)
							*(kanji+i) = *(buffer_utf8+i);
						*(kanji+i+1) = '\0';
						sscanf(buffer_utf8,"%*s %[A-Za-z0-9 .-] %[^{] %[^\n]", atributos, reading, meaning);
				
						// Make query.
						if (strstr(listavocab->pkanji, kanji)) {
							resultados++;
						
							// Delete names for reading.
							if (strstr(reading,"T1"))
								sscanf(reading, "%[^T]",reading);
						
							// Storing number of strokes.
							pos = NULL;
							strokes = 0;
							pos = strstr(atributos, " S");
							if (pos) {
								pos = pos+2;
							sscanf(pos,"%hu", &strokes);
							}
						
							// Storing heisig number.
							pos = NULL;
							heisig = 0;
							pos = strstr(atributos, " L");
							if (pos) {
								pos = pos+2;
							sscanf(pos,"%hu", &heisig);
							}
						
							wprintw(pant->buffer,"%s:\n",kanji);
							wprintw(pant->buffer,"[%d]Kanji: %s   Heisig: %hu  Strokes: %hu\n",
							resultados,kanji,heisig,strokes);
							wprintw(pant->buffer,"Reading: %s\n", reading);
						
							// Delete keys and brackets from "meaning".
							pos = NULL;
							if (!imitemp)
								imitemp = (char *)calloc(strlen(meaning)+1,sizeof(char));
							else 
								imitemp = (char *)realloc(imitemp,strlen((meaning)+1)*sizeof(char));
							pos = strstr(meaning,"{")+1;
							sscanf(pos,"%[^}]",imitemp);
							if ((*(imitemp) >= 97) && (*(imitemp) <= 122))
								*(imitemp) = *(imitemp)-32;
							while (strstr(pos,"{")) {
								strcpy(imitemp+(strlen(imitemp)),", ");
								pos = strstr(pos,"{")+1;
								sscanf(pos,"%[^}]",imitemp+(strlen(imitemp)));
							}
							strcpy(imitemp+(strlen(imitemp)),".");
							wprintw(pant->buffer,"Meaning: %s\n\n", imitemp);
						
						}
					}
			
					iconv_close(desc);
				}
				
				if (fclose(kanjidic) != 0)
					exit_mem(EXIT_FAILURE, "Error closing kanjidic file.");
					
			}
		} else
		wprintw(pant->buffer,"%s doesn't contain any kanji.\n\n",listavocab->phiragana);	
			
	}
	
	if (imitemp) {
		free(imitemp);
		imitemp = NULL;
	}
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}
	if (buffer_utf8) {
		free(buffer_utf8);
		buffer_utf8 = NULL;
	}
	if (kanji) {
		free(kanji);
		kanji = NULL;
	}
	if (atributos) {
		free(atributos);
		atributos = NULL;
	}
	if (reading) {
		free(reading);
		reading = NULL;
	}
	if (meaning) {
		free(meaning);
		meaning = NULL;
	}
	
	return resultados;
	
}

int longest_line_kanji(FILE *archivo) {

	int n_line = 0;
	int max_line = 0;
	char c;
	
	while ((c=fgetc(archivo)) != EOF) {
		if ((c == '{') || (c == '\n')) {
			if (n_line > max_line)
				max_line = n_line;
			n_line=0;
		} else {
			n_line++;
		}
	}
	
	return max_line;
}
