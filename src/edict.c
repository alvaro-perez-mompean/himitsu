#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <ncurses.h>

#include "edict.h"


FILE * load_edict() {
	
	char *homediredict = NULL;
	FILE *edict;
	edict = fopen("/usr/share/edict/edict", "r");
	if (!edict) {
		// Check if file is in ~/.himitsu/edict directory.
		homediredict = (char *)calloc((strlen(getenv("HOME"))+strlen("/.himitsu/edict"))+1,sizeof(char));
		strcpy(homediredict,getenv("HOME"));
		strcat(homediredict,"/.himitsu/edict");
		edict = fopen(homediredict, "r");
		if (!edict) {
			endwin();
			printf("\nERROR: Edict not found.\nMust be at \"%s\" directory.\n\n", homediredict);
			exit(1);
		}
	}
	
	if (homediredict)
		free(homediredict);
	
	return edict;
}


/* This function adds a node to listavocab */
bool add_line_to_node(vocab_t **listavocab, char linea[], bool espalabra, int cat, unsigned short learning) {
	char *pkan=NULL, *phir=NULL, *pmea=NULL, *plinea=NULL;
	bool almacenado=false;
	int i,buno,bdos;
	if (espalabra) {
		buno=0;
		bdos=0;
		for (i=0;i<(int)strlen(linea);i++) {
			if ((linea[i] == '[') && (buno == 0))
				buno=i;
			else if ((linea[i] == '/') && ((buno != 0) && (bdos == 0)))
				bdos=i;
			else if ((linea[i] == '/') && (buno == 0)) {
				buno=i;
				bdos=i;
			}
		}
	
		// Kanji.
		pkan = (char *)calloc(buno,sizeof(char));
		for (i=0;i<(buno-1);i++) {
			*(pkan+i)=linea[i];
		}

		// Hiragana.
		if (buno != bdos) {
			phir = (char *)calloc((bdos-2)-buno,sizeof(char));
			for (i=buno+1;i<(bdos-2);i++)
				*(phir+(i-(buno+1)))=linea[i];
		} else {
			phir = (char *)calloc(buno,sizeof(char));
			for (i=0;i<(buno-1);i++)
				*(phir+i)=linea[i];
		}

		// Meaning.
		pmea = (char *)calloc(strlen(linea)-(bdos+1),sizeof(char));
		for (i=bdos+1;i<(int)(strlen(linea)-1);i++)
			*(pmea+(i-(bdos+1)))=linea[i];
		
	} else {
		// It's a category.
		plinea = (char *)calloc(strlen(linea)+1,sizeof(char));
		strcpy(plinea, linea);
	}
	
	vocab_t *pnuevonodov = NULL;
	if (espalabra) {
		// Reserve memory for new node.
		pnuevonodov = (vocab_t *)malloc(sizeof(vocab_t));
		if (!pnuevonodov)
			exit_mem(1, "Not enough memory");

        // Reserve memory for word.
		if (buno == bdos) {
			if (espalabra) {
				pnuevonodov->pkanji = NULL;
				pnuevonodov->pcat = NULL;
			}
		} else {
			if (espalabra) 
				pnuevonodov->pcat = NULL;
			
			pnuevonodov->pkanji = (char *)malloc ((strlen(pkan)+1)*sizeof(char));
			if (!pnuevonodov->pkanji)
				exit_mem(1, "Not enough memory");
			
		}
		pnuevonodov->phiragana = (char *)malloc ((strlen(phir)+1)*sizeof(char));
		pnuevonodov->pmeaning = (char *)malloc ((strlen(pmea)+1)*sizeof(char));
		if (!pnuevonodov->phiragana || !pnuevonodov->pmeaning)
			exit_mem(1, "Not enough memory");
			
		if (buno != bdos)
			strcpy(pnuevonodov->pkanji, pkan);
		strcpy(pnuevonodov->phiragana, phir);
		strcpy(pnuevonodov->pmeaning, pmea);
		pnuevonodov->learning = learning;
		
		// It's a category.
	} else if (!espalabra) {
		pnuevonodov = (vocab_t *)malloc(sizeof(vocab_t));
		if (!pnuevonodov)
			exit_mem(1, "Not enough memory");

		pnuevonodov->pcat = (char *)calloc((strlen(plinea)+1),sizeof(char));
		if (!pnuevonodov->pcat)
			exit_mem(1, "Not enough memory");
		
		strcpy(pnuevonodov->pcat,plinea);
		pnuevonodov->pmeaning = pnuevonodov->pkanji = pnuevonodov->phiragana = NULL;
		pnuevonodov->learning = 0;
	}
	
	// Insert element in the list.        
	if (cat == 0) {
		pnuevonodov->panterior = *listavocab;
		pnuevonodov->psiguiente = NULL;
		
		
		if (pnuevonodov->panterior) 
			pnuevonodov->panterior->psiguiente = pnuevonodov;
		*listavocab = pnuevonodov;
	} else {
		if (add_vocab(listavocab,pnuevonodov,cat)) {
		
		
			vocab_t *pnodoultimo=*listavocab, *aux;
			pnodoultimo = go_to_cat(pnodoultimo,cat);
		
			// Here we are on first of the list that are looking for.
			if (pnodoultimo->psiguiente) {
				while (pnodoultimo->psiguiente && !pnodoultimo->psiguiente->pcat)
					pnodoultimo = pnodoultimo->psiguiente;
			}
			// We add the word..
			if (pnodoultimo->psiguiente) {
				aux = pnodoultimo->psiguiente;
				aux->panterior = pnuevonodov;
				pnodoultimo->psiguiente = pnuevonodov;
				pnuevonodov->panterior = pnodoultimo;
				pnuevonodov->psiguiente = aux;
			} else {
				pnuevonodov->panterior = pnodoultimo;
				pnuevonodov->psiguiente = NULL;
				pnodoultimo->psiguiente = pnuevonodov;
			}
		
			while (pnuevonodov->psiguiente)
				pnuevonodov = pnuevonodov->psiguiente;
			*listavocab = pnuevonodov;
			almacenado=true;
		} else 
			almacenado=false;
			
	}

	if (pkan)
		free(pkan);
	if (phir)
		free(phir);
	if (pmea)
		free(pmea);
	if (plinea)
		free(plinea);
	pkan = phir = pmea = plinea = NULL;
	
	return almacenado;

}

/* This function calls "add_line_to_node" for each line of the vocab file. */
void add_line(vocab_t **listavocab, FILE *archivo, int cat) {

	char *buffer;
	buffer = (char *)calloc(1152,sizeof(char));
	
	unsigned short learning = 0;
	char tipo;
	
	while (fscanf(archivo,"%c", &tipo) != EOF) {
		// It's a word.
		if (tipo == '&') {
			fscanf(archivo,"%hu %*c %[^\n] %*[\n]",&learning, buffer);
			add_line_to_node(listavocab,buffer, true, cat, learning);
		// It's a category.
		} else if (tipo == ';') {
			fscanf(archivo," %*c %[^\n] %*[\n]", buffer);
			add_line_to_node(listavocab,buffer,false, cat, learning);
		}
	}
	
	if (buffer)
		free(buffer);
	buffer = NULL;

}

int longest_line(FILE *archivo) {

	int n_line = 0;
	int max_line = 0;
	char c;
	
	while ((c=fgetc(archivo)) != EOF) {
		if (c == '\n') {
			if (n_line > max_line)
				max_line = n_line;
			n_line=0;
		} else {
			n_line++;
		}
	}
	
	return max_line;
}
