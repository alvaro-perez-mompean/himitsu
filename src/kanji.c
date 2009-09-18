#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <ncurses.h>

#include "kanji.h"

static int longest_line_kanji(FILE *archivo);

FILE * load_kanjidic() {

	FILE *kanjidic;
	char *homedirkanjidic = NULL;
	
	
	
	kanjidic = fopen("/usr/share/edict/kanjidic", "r");
	if (!kanjidic) {
		// Check if file is in ~/.himitsu/kanjidic directory.
		homedirkanjidic = (char *)calloc((strlen(getenv("HOME"))+strlen("/.himitsu/kanjidic"))+1,sizeof(char));
		strcpy(homedirkanjidic,getenv("HOME"));
		strcat(homedirkanjidic,"/.himitsu/kanjidic");
		kanjidic = fopen(homedirkanjidic, "r");
		if (!kanjidic) {
			strcat(homedirkanjidic, " not found...");
			exit_mem(EXIT_FAILURE, homedirkanjidic);
		}
	}
	
	if (homedirkanjidic)
		free(homedirkanjidic);
	homedirkanjidic = NULL;
	
	return kanjidic;

}


int show_kanji(vocab_t *listavocab, pantalla_t *pant) {
	unsigned short resultados=0; // Aux is used to order kanjis.
	int tam_buffer = 0;
	int tam_buffer_atrib = 0;
	char *imitemp = NULL; // Shows meaning without keys and brackets.
	char *kanji, *atributos, *reading, *meaning;
	char *buffer, *buffer_utf8;
	char *pos;
	unsigned short heisig, strokes;
	char *pent, *psal;
	size_t codent, codsal;
	iconv_t desc;
	FILE *kanjidic;
	
	buffer = buffer_utf8 = NULL;
	
	// Load kanjidic.
	kanjidic = load_kanjidic();
	
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
				fscanf(kanjidic,"%[^\n]%*[\n]",buffer);
				desc = iconv_open("UTF-8", "EUC-JP");
				while (fscanf(kanjidic,"%[^\n]%*[\n]",buffer) != EOF ) {
					pent = &buffer[0];
					psal = &buffer_utf8[0];
					codent = strlen(buffer)*sizeof(char)+1;
					codsal = tam_buffer*sizeof(char);
					iconv(desc,&pent,&codent,&psal,&codsal);
					sscanf(buffer_utf8,"%s %[A-Za-z0-9 .-] %[^{] %[^\n]",kanji, atributos, reading, meaning);
				
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
				
				if (fclose(kanjidic) != 0)
					exit_mem(EXIT_FAILURE, "Error closing kanjidic file.");
					
			}
		} else
		wprintw(pant->buffer,"%s doesn't contain any kanji.\n\n",listavocab->phiragana);	
			
	}
	
	if (imitemp)
		free(imitemp);
	if (buffer)
		free(buffer);
	if (buffer_utf8)
		free(buffer_utf8);
	if (kanji)
		free(kanji);
	if (atributos)
		free(atributos);
	if (reading)
		free(reading);
	if (meaning)
		free(meaning);
	
	
	imitemp = buffer = buffer_utf8 = kanji = atributos = reading = meaning = NULL;
	
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
