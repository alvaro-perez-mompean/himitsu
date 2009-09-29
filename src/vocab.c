/*
 *      vocab.c
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

#include <sys/stat.h>
#include "vocab.h"

#define WGET_EDICT "wget http://ftp.monash.edu.au/pub/nihongo/edict -O %s"
#define WGET_KANJIDIC "wget http://ftp.monash.edu.au/pub/nihongo/kanjidic -O %s"

FILE * load_edict(bool is_edict) {
	
	char *homediredict = NULL;
	FILE *edict;
	char *wgetstring = NULL;
	if (is_edict)
		edict = fopen("/usr/share/edict/edict", "r");
	else 
		edict = fopen("/usr/share/edict/kanjidic", "r");
	if (!edict) {
		// Check if file is in ~/.himitsu/edict directory.
		if (is_edict)
			homediredict = (char *)calloc((strlen(getenv("HOME"))+strlen("/.himitsu/edict"))+1,sizeof(char));
		else
			homediredict = (char *)calloc((strlen(getenv("HOME"))+strlen("/.himitsu/kanjidic"))+1,sizeof(char));
		strcpy(homediredict,getenv("HOME"));
		strcat(homediredict,"/.himitsu");
		edict = fopen(homediredict, "r");
	if (!edict)
		mkdir(homediredict,0755);
	else
		fclose(edict);
		
		if (is_edict)
			strcat(homediredict,"/edict");
		else
			strcat(homediredict,"/kanjidic");
		edict = fopen(homediredict, "r");
		if (!edict) {
			if (is_edict) {
				wgetstring = (char *)calloc(strlen(WGET_EDICT)+strlen(homediredict),sizeof(char));
				sprintf(wgetstring, WGET_EDICT, homediredict);
				printf("It seems that edict isn't in the system. Downloading...\n\n");
			} else {
				wgetstring = (char *)calloc(strlen(WGET_KANJIDIC)+strlen(homediredict),sizeof(char));
				sprintf(wgetstring, WGET_KANJIDIC, homediredict);
				printf("It seems that kanjidic isn't in the system. Downloading...\n\n");
			}
			if (system(wgetstring) != 0) {
				remove(homediredict);
				homediredict = (char *)realloc(homediredict,(strlen(homediredict)+strlen(" not found...")+1)*sizeof(char));
				strcat(homediredict, " not found...");
				exit_mem(EXIT_FAILURE, homediredict);
			} else
				edict = fopen(homediredict, "r");
		}
	}
	
	if (homediredict) {
		free(homediredict);
		homediredict = NULL;
	}
	
	return edict;
}


/* Load vocab files into listavocab structure. */

void load_vocab(vocab_t **listavocab) {
	FILE *vocab;
	char *homedir = NULL;
	
	homedir = (char *)calloc((strlen(getenv("HOME"))+strlen("/.himitsu/save"))+1,sizeof(char));
	strcpy(homedir,getenv("HOME"));
	strcpy((homedir+strlen(homedir)),"/.himitsu");
	vocab = fopen(homedir, "r");
	if (!vocab)
		mkdir(homedir,0755);
	else
		fclose(vocab);
	
	strcpy((homedir+strlen(homedir)),"/save");

	vocab = fopen(homedir, "a");
	if (!vocab)
		exit_mem(EXIT_FAILURE,"Error opening vocabulary file.");
	else if (fclose(vocab) != 0)
		exit_mem(EXIT_FAILURE,"Error closing vocabulary file.");

	vocab = fopen(homedir, "r");
	if (!vocab)
		exit_mem(EXIT_FAILURE,"Vocabulary file can't be opened.");
	else
		add_line(listavocab, vocab,0);

	if (fclose(vocab) != 0) 
		exit_mem(EXIT_FAILURE,"Error closing vocabulary file.\n");
	if (homedir) {
		free(homedir);
		homedir = NULL;
	}
	

}

/* This function calls "add_line_to_node" for each line of the vocab file. */
int add_line(vocab_t **listavocab, FILE *archivo, int cat) {

	char *buffer;
	int num_nodes = 0;
	unsigned short learning = 0;
	char tipo;
	
	buffer = (char *)calloc(longest_line(archivo)+1,sizeof(char));
	rewind(archivo);
	
	while (fscanf(archivo,"%c", &tipo) != EOF) {
		// It's a word.
		if (tipo == '&') {
			fscanf(archivo,"%hu %*c %[^\n] %*[\n]",&learning, buffer);
			if (add_line_to_node(listavocab,buffer, true, cat, learning))
				num_nodes++;
		// It's a category.
		} else if (tipo == ';') {
			fscanf(archivo," %*c %[^\n] %*[\n]", buffer);
			if (add_line_to_node(listavocab,buffer,false, cat, learning))
				num_nodes++;
		}
	}
	
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}

	return num_nodes;
}

/* This function adds a node to listavocab */
bool add_line_to_node(vocab_t **listavocab, char linea[], bool is_word, int cat, unsigned short learning) {
	char *pkan=NULL, *phir=NULL, *pmea=NULL, *plinea=NULL;
	bool almacenado=false;
	int i,buno,bdos;
	vocab_t *pnuevonodov = NULL;
	buno = bdos = 0;
	if (is_word) {
		
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
	// Reserve memory for new node.
	pnuevonodov = (vocab_t *)malloc(sizeof(vocab_t));
		if (!pnuevonodov) {
			exit_mem(EXIT_FAILURE, "Not enough memory.");
		} else {
			if (is_word) {
				// Reserve memory for word.
				if (buno == bdos) {
					if (is_word) {
						pnuevonodov->pkanji = NULL;
						pnuevonodov->pcat = NULL;
					}
				} else {
					if (is_word) 
						pnuevonodov->pcat = NULL;
			
					pnuevonodov->pkanji = (char *)malloc ((strlen(pkan)+1)*sizeof(char));
					if (!pnuevonodov->pkanji)
						exit_mem(EXIT_FAILURE, "Not enough memory.");
			
				}
				pnuevonodov->phiragana = (char *)malloc ((strlen(phir)+1)*sizeof(char));
				pnuevonodov->pmeaning = (char *)malloc ((strlen(pmea)+1)*sizeof(char));
				if (!pnuevonodov->phiragana || !pnuevonodov->pmeaning)
					exit_mem(EXIT_FAILURE, "Not enough memory.");
			
				if (buno != bdos)
					strcpy(pnuevonodov->pkanji, pkan);
				strcpy(pnuevonodov->phiragana, phir);
				strcpy(pnuevonodov->pmeaning, pmea);
				pnuevonodov->learning = learning;
		
			// It's a category.
			} else {
				pnuevonodov->pcat = (char *)calloc((strlen(plinea)+1),sizeof(char));
				if (!pnuevonodov->pcat)
					exit_mem(EXIT_FAILURE, "Not enough memory.");
		
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
				almacenado=true;
			} else {
				if (does_not_exist(listavocab,pnuevonodov,cat)) {
		
		
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
			
		}

	if (pkan) {
		free(pkan);
		pkan = NULL;
	}
	if (phir) {
		free(phir);
		phir = NULL;
	}
	if (pmea) {
		free(pmea);
		pmea = NULL;
	}
	if (plinea) {
		free(plinea);
		plinea = NULL;
	}
	
	
	return almacenado;

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


/* Show list's elements. */

int show_vocab(vocab_t *listavocab, int cat, pantalla_t *pant, bool imprime) {

int resultados = 0;
	
	
wprintw(pant->buffer,"\n");
if (listavocab) {
	
	listavocab = go_to_cat(listavocab,cat);
	listavocab = listavocab->psiguiente;

	resultados = 0;
	if (listavocab) {
		wclear(pant->buffer);
		pant->ppal_finbuf = getcury(pant->buffer);
			while (listavocab && !listavocab->pcat) {
				resultados++;
				if (imprime) {
					show_vocab_item(listavocab,pant,resultados);
					wprintw(pant->buffer,"\n");

				}
				listavocab = listavocab->psiguiente;					
        	}
        	
	}

}

wprintw(pant->buffer,"\n");
upgrade_buffer(pant, FALSE);

return resultados;
}

void show_vocab_item(vocab_t *listavocab, pantalla_t *pant, int resultado) {
	
	if (listavocab) {
		if (listavocab->pkanji)
			wprintw(pant->buffer,"[%d] %s (%s): %s",resultado,listavocab->pkanji,listavocab->phiragana,listavocab->pmeaning);
		// Temporaly if to study english irregular verbs.
		else if (*listavocab->phiragana >= 97 && *listavocab->phiragana <= 122)
			wprintw(pant->buffer,"[%d] %s: %s",resultado,listavocab->pmeaning,listavocab->phiragana);
		else
			wprintw(pant->buffer,"[%d] %s: %s",resultado,listavocab->phiragana,listavocab->pmeaning);     	
        	
 	}

}
	
	

/* Show vocab lists and return an int with the number of them. */

int show_cat(vocab_t *listavocab, pantalla_t *pant) {

	int i=0,resultados = 0;
	
	vocab_t *nnodos = NULL;
	//wprintw(pant->ppal,"\n");
	if (!listavocab) {
		wprintw(pant->buffer,"There aren't vocabulary lists.\n\n");
	} else {
		while (listavocab->panterior)
			listavocab = listavocab->panterior;

		while (listavocab) {
			if (listavocab->pcat) {
				i++;
				nnodos = listavocab->psiguiente;
				resultados=0;
				if (nnodos) {
					while (nnodos && !nnodos->pcat) {
						resultados++;
						nnodos = nnodos->psiguiente;
					}
				}
				wprintw(pant->buffer,"[%d] %s (%d)\n",i,listavocab->pcat,resultados);
			}
			listavocab = listavocab->psiguiente;
		}
	}
	wprintw(pant->buffer,"\n");
	return i;
}


/* This function saves listavocab into a file */
void save_vocab(vocab_t *listavocab) {
	FILE *fd;
	char *homedir = NULL;
	
	homedir = (char *)calloc((strlen(getenv("HOME"))+strlen("/.himitsu/save"))+1,sizeof(char));
	strcpy(homedir,getenv("HOME"));
	strcat(homedir,"/.himitsu/save");

	fd=fopen(homedir,"wt");
	if (!fd)
		exit_mem(EXIT_FAILURE,"Error saving vocabulary file.");

	if (listavocab) {
		listavocab = go_to_cat(listavocab, 1);
		while (listavocab) {
			if (listavocab->pcat) {
				fprintf(fd,";;%s\n",listavocab->pcat);
			} else if (listavocab->pkanji) {
				fprintf(fd,"&%hu&%s [%s] /%s/\n",listavocab->learning,listavocab->pkanji,listavocab->phiragana,listavocab->pmeaning);
			} else {
				fprintf(fd,"&%hu&%s /%s/\n",listavocab->learning,listavocab->phiragana,listavocab->pmeaning);
			}
			listavocab = listavocab->psiguiente;
		}
	}

	fclose(fd);
	if (homedir) {
		free(homedir);
		homedir = NULL;
	}
}


/* Edit a word */

void edit_vocab(vocab_t *pnodoedit, char secmea[], pantalla_t *pant) {

	char opcion = '0';
	
	wprintw(pant->buffer,"\n");
	while ((opcion != 'y') && (opcion != 'n')) {
		if (pnodoedit->pkanji)
			wprintw(pant->buffer,"%s (%s) -> \"%s\" ¿ok? (y/n): ",pnodoedit->pkanji,pnodoedit->phiragana,secmea);
		else
			wprintw(pant->buffer,"%s -> %s \"¿ok?\" (y/n): ",pnodoedit->phiragana,secmea);
		
		upgrade_buffer(pant, TRUE);
		opcion = wgetch(pant->ppal);
		wprintw(pant->buffer,"\n");
	}
	
	if (opcion == 'y') {
	
		if (!pnodoedit->pmeaning) {
			pnodoedit->pmeaning = (char *)malloc ((strlen(secmea)+1)*sizeof(char));
			if (!pnodoedit->pmeaning)
				exit_mem(EXIT_FAILURE,"Not enough memory.");
		} else {
			pnodoedit->pmeaning = (char *)realloc (pnodoedit->pmeaning,(strlen(secmea)+1)*sizeof(char));
			if (!pnodoedit->pmeaning)
				exit_mem(EXIT_FAILURE,"Not enough memory.");
		}
		strcpy(pnodoedit->pmeaning, secmea);
		wprintw(pant->buffer,"\nRenaming...\n");
	} else {
		wprintw(pant->buffer,"\nCancelling...\n");
	}
}


/* This function checks that the word doesn't exist in the list */

bool does_not_exist(vocab_t **listavocab, vocab_t *pnuevonodov, int cat) {
	bool norepe = true;
		
	vocab_t *aux = *listavocab;

	aux = go_to_cat(aux,cat);


	if (aux->psiguiente) {
		aux = aux->psiguiente;
			while (aux && !aux->pcat && norepe) {
				if (!aux->pcat) {
					if ((!aux->pkanji) || (!pnuevonodov->pkanji)) {
						if ((strstr(aux->phiragana,pnuevonodov->phiragana)) && (strlen(aux->phiragana)) == (strlen(pnuevonodov->phiragana)))
							norepe = false;
					} else {
						if ((strstr(aux->pkanji,pnuevonodov->pkanji)) && (strlen(aux->pkanji)) == (strlen(pnuevonodov->pkanji))) {
							if ((strstr(aux->phiragana,pnuevonodov->phiragana)) && (strlen(aux->phiragana)) == (strlen(pnuevonodov->phiragana)))
								norepe = false;
						}
					}
				}
				aux = aux->psiguiente;
			}
	}
	
	return norepe;

}

/* Delete a word */

void delete_vocab(vocab_t **listavocab, pantalla_t *pant) {

	char ok='0';

	vocab_t *pnodoelim=*listavocab;
	vocab_t *aux, *aux2;
	aux=aux2=NULL;
	
	while (ok != 'y' && ok != 'n') {
		if (pnodoelim->pkanji)
			wprintw(pant->buffer,"Do you want to delete \"%s (%s): %s\"? (y/n): ",pnodoelim->pkanji,pnodoelim->phiragana, pnodoelim->pmeaning);
		else
			wprintw(pant->buffer,"Do you want to delete \"%s: %s\"? (y/n): ",pnodoelim->phiragana, pnodoelim->pmeaning);
		upgrade_buffer(pant, TRUE);
		ok = wgetch(pant->ppal);
		wprintw(pant->buffer,"\n");
	}
	
	if (ok == 'y') {
		if (pnodoelim->psiguiente && pnodoelim->panterior) {
			aux = pnodoelim->psiguiente;
			aux2 = pnodoelim->panterior;

			aux->panterior = aux2;
			aux2->psiguiente = aux;

		} else if (!pnodoelim->psiguiente && pnodoelim->panterior) {
			aux = pnodoelim->panterior;
			aux->psiguiente = NULL;
			*listavocab = aux;
		} else if (!pnodoelim->panterior && pnodoelim->psiguiente) {
			aux = pnodoelim->psiguiente;
			aux->panterior = NULL;
		}

		if (!pnodoelim->psiguiente && !pnodoelim->panterior) {
			*listavocab = NULL;
		}
		// Vamos a liberar memoria.

		if (pnodoelim->phiragana) {
			free(pnodoelim->phiragana);
			pnodoelim->phiragana = NULL;
		}
		if (pnodoelim->pkanji) {
			free(pnodoelim->pkanji);
			pnodoelim->pkanji = NULL;
		}
		if (pnodoelim->pmeaning) {
			free(pnodoelim->pmeaning);
			pnodoelim->pmeaning = NULL;
		}
		pnodoelim->psiguiente = NULL;
		pnodoelim->panterior = NULL;
		free (pnodoelim);


		wprintw(pant->buffer,"Word deleted correctly.\n\n");
		*listavocab = aux;
	} else
		wprintw(pant->buffer,"\nCancelling...\n");
		
	

}


/* This function creates a new category */

void new_cat(vocab_t **listavocab, char categoria[]) {

	vocab_t *pnuevonodov = NULL;
	pnuevonodov = (vocab_t *)malloc(sizeof(vocab_t));
	if (!pnuevonodov) {
		exit_mem(EXIT_FAILURE,"Not enough memory.");
	} else {
		pnuevonodov->pcat = (char *)calloc((strlen(categoria)+1),sizeof(char));
		if (!pnuevonodov->pcat) 
			exit_mem(EXIT_FAILURE,"Not enough memory.");
		strcpy(pnuevonodov->pcat,categoria);
		pnuevonodov->pmeaning = NULL;
		pnuevonodov->pkanji = NULL;
		pnuevonodov->phiragana = NULL;
		pnuevonodov->learning = 0;

		pnuevonodov->panterior = *listavocab;
		pnuevonodov->psiguiente = NULL;

		if (pnuevonodov->panterior)
			pnuevonodov->panterior->psiguiente = pnuevonodov;
		*listavocab = pnuevonodov;
	}
}


/* Delete a category list and its content */
void delete_cat(vocab_t **listavocab, int categoria, pantalla_t *pant) {

	char conf='0';
	bool vacia = true;

        vocab_t *pcatelim = *listavocab, *aux, *aux2;
		pcatelim = go_to_cat(pcatelim,categoria);
	
        // Check if list is empty.
        if (!pcatelim->psiguiente->pcat) {
			while ((conf != 'y') && (conf != 'n')) {
				wprintw(pant->buffer,"WARNING! \"%s\" list isn't empthy. Do you want to delete it? (y/n): ",pcatelim->pcat);
				upgrade_buffer(pant,TRUE);
				conf = wgetch(pant->ppal);
				wprintw(pant->buffer,"\n");
			}
			vacia = false;
        }


        if (vacia) {
			wprintw(pant->buffer,"\nDeleting...\n");
                if (pcatelim->psiguiente && pcatelim->panterior) {
                        aux = pcatelim->psiguiente;
                        aux2 = pcatelim->panterior;

                        aux->panterior = aux2;
                        aux2->psiguiente = aux;

                } else if (!pcatelim->psiguiente && pcatelim->panterior) {
                        aux = pcatelim->panterior;
                        aux->psiguiente = NULL;
                        *listavocab = aux;
                } else if (!pcatelim->panterior && pcatelim->psiguiente) {
                        aux = pcatelim->psiguiente;
                        aux->panterior = NULL;
                }

                if (!pcatelim->psiguiente && !pcatelim->panterior) {
                        *listavocab = NULL;
                }
			
			if (pcatelim->pcat) {
				free(pcatelim->pcat);
				pcatelim->phiragana = NULL;
			}
			pcatelim->psiguiente = NULL;
			pcatelim->panterior = NULL;
			free(pcatelim);
			
			
        } else if ((!vacia) && (conf == 'y')) {
			// Delete a list an its content.
			wprintw(pant->buffer,"\nDeleting...\n");
			if (pcatelim->psiguiente && pcatelim->panterior) {
				aux = pcatelim->panterior;
				aux2 = pcatelim->psiguiente;
				while (aux2 && !aux2->pcat) {
					if (aux2->phiragana) {
						free(aux2->phiragana);
						aux2->phiragana = NULL;
					}
					if (aux2->pkanji) {
						free(aux2->pkanji);
						aux2->pkanji = NULL;
					}
					if (aux2->pmeaning) {
						free(aux2->pmeaning);
						aux2->pmeaning = NULL;
					}
					if (aux2->psiguiente) {
						aux2 = aux2->psiguiente;
						free(aux2->panterior);
					} else {
						free(aux2);
						aux2 = NULL;
					}
				
				}
				// We are over next pcat or over NULL.
				if (aux2) {
					aux->psiguiente = aux2;
					aux2->panterior = aux;
				} else {
					aux->psiguiente = NULL;
					*listavocab = aux;
				}
				
		
			} else if (!pcatelim->panterior && pcatelim->psiguiente) {
				aux = pcatelim->psiguiente;
				while (aux && !aux->pcat) {
					if (aux->phiragana) {
						free(aux->phiragana);
						aux->phiragana = NULL;
					}
					if (aux->pkanji) {
						free(aux->pkanji);
						aux->pkanji = NULL;
					}
					if (aux->pmeaning) {
						free(aux->pmeaning);
						aux->pmeaning = NULL;
					}
					if (aux->psiguiente) {
						aux = aux->psiguiente;
						free(aux->panterior);
					} else {
						free(aux);
						aux = NULL;
					}
				}
				if (aux) {
					aux->panterior = NULL;
				} else {
					*listavocab = NULL;
				}
			}
				
			
		} else
			wprintw(pant->buffer,"\nCancelling...\n"); 
}

// Return current category.
int current_cat(vocab_t *listavocab, int cat, bool imprime, WINDOW *menu) {
	int elementos = 0;
	
	listavocab = go_to_cat(listavocab,cat);
	
	if (imprime)
		mvwprintw(menu,2,1,"%s ",listavocab->pcat);
	if (listavocab->psiguiente) {
		listavocab = listavocab->psiguiente;
		elementos = 0;
		while (listavocab && !listavocab->pcat) {
			elementos++;
			listavocab = listavocab->psiguiente;
		}
	}
	if (imprime)
		wprintw(menu,"(%d)\n",elementos);
	
	return elementos;
}

// Return a pointer to a category.
vocab_t * go_to_cat(vocab_t *listavocab, int cat) {

	int i = 0;
	
	// We are at the beginning.
	while (listavocab->panterior)
		listavocab = listavocab->panterior;

	// Go to the chosen category.
	while (i<cat) {
		if (listavocab->pcat)
			i++;
		if (i<cat)
			listavocab = listavocab->psiguiente;
	}
	
	return listavocab;
}

vocab_t * go_to_item(vocab_t *listavocab, int cat, int item) {
	
	int i=0;
	listavocab = go_to_cat(listavocab, cat);
	
	for (i=0;i<item;i++)
		listavocab = listavocab->psiguiente;
		
	return listavocab;
	
}

// select_cat() eturns 0 if lista is empty. Returns -1 if input's value is invalid.
int select_cat(vocab_t *listavocab, int cat, pantalla_t *pant, const char texto[]) {
	
	int numcat = 0;
	
	// If previously we don't select any category...
	if (cat <= 0) {
		numcat = show_cat(listavocab,pant);

		if (numcat>0) {
			wprintw(pant->buffer,"Choose a list %s.\n\n", texto);
			cat = 0;
			upgrade_buffer(pant, FALSE);
			while ((cat != 13) && (cat != 27)) {
				wrefresh(pant->ppal);
				 cat = wgetch(pant->menu);
				scroll_keys(pant,cat,TRUE);				
			}
			
			cat = select_item(pant, cat);
			
			if (cat <=0 || cat > numcat) {
				wprintw(pant->buffer,"Category not valid.\n\n");
				cat = -1;
			}
		} else
			cat = 0;
	}
	upgrade_buffer(pant, FALSE);
	wrefresh(pant->ppal);
					
	return cat;
	
}

int import_file(vocab_t **listavocab, const char param[]) {
	
	FILE *vocab;
	char *title_file, tipo;
	int num_nodes = 0;
	
	title_file = (char *)calloc(32,sizeof(char));
	
	vocab = fopen(param, "r");
	if (!vocab) {
		printf("\"%s\" can't be loaded.\n\n", param);
	} else { 
		fscanf(vocab,"%c", &tipo);
		if (tipo == ')') {
			fscanf(vocab,"%c", &tipo);
				if (tipo == ')') {
					fscanf(vocab,"%[^\n]", title_file);
					num_nodes = add_line(listavocab, vocab,0);
						if (num_nodes == 0)
							printf("Error: \"%s\" can't be imported", title_file);
						else
							printf("%d lines of \"%s (%s)\" were successfully imported!\n\n", num_nodes, param, title_file);
				} else 
					printf("\"%s\" isn't a valid vocab file.\n\n", param);
		} else
			printf("\"%s\" isn't a valid vocab file.\n\n", param);
	}
	
	if (title_file) {
		free(title_file);
		title_file = NULL;
	}
	return num_nodes;
	
}

int export_file(vocab_t *listavocab, pantalla_t *pant) {
	
	int cat;
	
	cat = select_cat(listavocab, 0,  pant, "to export");
	
	if ((cat > 0) && (show_vocab(listavocab, cat, pant, false) > 0 ))
		return EXIT_SUCCESS;
		
		// This function isn't implemented yet.
		
	
	
	
}
