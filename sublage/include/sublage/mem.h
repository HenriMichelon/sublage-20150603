#pragma once

#include <stdbool.h>

#define NDEBUG 1

#ifdef NDEBUG
#include <stdlib.h>
#define memAlloc(s) malloc(s)
#define memRealloc(p, s) realloc(p, s)
#define memFree(p) free(p)
#define memCalloc(c, s) calloc(c, s)
#else

#include <stddef.h>

// voir malloc()
void* _memAlloc(size_t, const char*, int);

// voir free()
void _memFree(void*, const char*, int);

// voir realloc()
void* _memRealloc(void*, size_t, const char*, int);

// voir calloc()
void* _memCalloc(size_t, size_t, const char*, int);

void memDebugInit();

/*
 * activation/désactivation du la traçabilité de malloc() et de free()
 * bool : true pour activer la tracabilité, false pour la désactiver
 */
void memTrace(bool);

/*
 * Affichage des statistiques de malloc() et de free() et
 * affichage de la liste des blocs non libérés.
 * Cette fonction est normalement appelée automatiquement à la fin
 * du programme, il est inutile de le faire explicitement, sauf si
 * vous avez besoin de voir les statistiques en cours d'exécution.
 */
void memPrintStats();

/*
 * les macros pour bénéficer des noms des fichiers sources et des numéros de lignes
 */
#define memAlloc(s) _memAlloc(s, __FILE__, __LINE__)
#define memRealloc(p, s) _memRealloc(p, s, __FILE__, __LINE__)
#define memFree(p) _memFree(p, __FILE__, __LINE__)
#define memCalloc(c, s) _memCalloc(c, s, __FILE__, __LINE__)

#endif

