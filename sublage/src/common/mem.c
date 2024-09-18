#include <stdio.h>
#include <string.h>
#include <strings.h>
#define COMPILING_MEMOIRE_H 1 // pour éviter la redéfinition de malloc(), free(), ...
#include "sublage/thread.h"
#include "sublage/mem.h"
#include <stdlib.h>

// constante utilisée pour vérifier les dépassements mémoire
const int mem_debug_magic = 0xDEB0C;

// définition d'un bloc alloué.
// ces informations sont placées en début de chaque bloc mémoire.
typedef struct mem_bloc_t {
    // constante magique pour vérifier la validité du bloc
    int     magic1;
    // taille en octet du bloc alloué
    size_t  size;
    // localisation de l'appel de malloc()
    char*   file;
    int   line;
    // blocs suivants/précédents
    struct mem_bloc_t *next;
    struct mem_bloc_t *prev;
    // constante magique pour vérifier la validité du bloc
    int     magic2;
} mem_bloc_t;


// structure utilisée pour stocker les statistiques
typedef struct mem_stats_t {
    // nombre d'appels à malloc() et realloc()
    int   count;
    // nombre total d'octet alloués avec malloc() et realloc()
    int   total;
    // taille moyenne des blocs alloués
    int   avg;
} mem_stats_t;


// statistiques mémoire du programme en cours
static struct mem_stats_t *mem_stats = NULL;
// liste des blocs actuellement alloués
static struct mem_bloc_t *mem_blocs = NULL;
// drapeau de traçabilité
static bool mem_debug = false;
// drapeau pour savoir si la fonction d'affichage des
// statistiques a été enregistrée pour exécution en sortie
static bool mem_registered = false;
static ThreadMutex mem_mutex;


void memTrace(bool debug)
{
    mem_debug = debug;
}


void memPrintStats()
{
    // affichage des statistiques
    /*if (mem_stats != NULL) {
     printf("Memory statistics: \n  %d allocations\n  %d bytes allocated (total)\n  %d bytes per bloc (mean)\n",
     mem_stats->count, mem_stats->total, mem_stats->avg);
    }*/
    // affichage de la liste des blos non libérés si il y en a
    if (mem_blocs != NULL) {
        printf("Unfreed blocs: \n");
        mem_bloc_t *bloc = mem_blocs;
        while (bloc != NULL) {
            printf("  %ld bytes : %s, line %ld : 0x%x\n",
                   (long)bloc->size, (char*)bloc->file, (long)bloc->line,
                   (unsigned int)( (unsigned char*)bloc + sizeof(mem_bloc_t)));
            bloc = bloc->next;
        }
        ThreadMutexDestroy(&mem_mutex);
    }
}


// recherche d'un bloc alloué en fonction de l'adresse
// réelle d'allocation
mem_bloc_t* mem_findbloc (void*ptr)
{
    ThreadMutexLock(&mem_mutex);
    mem_bloc_t *bloc = mem_blocs;
    while (bloc != NULL) {
        unsigned char *bbloc = (unsigned char*)bloc;
        if ( (bbloc + sizeof(mem_bloc_t)) == ptr ) {
            ThreadMutexUnlock(&mem_mutex);
            return bloc;
        }
        bloc = bloc->next;
    }
    ThreadMutexUnlock(&mem_mutex);
    return NULL;
}

void memDebugInit() {
    if (!mem_registered) {
        mem_registered = true;
        // création et initialisation de la structure contenant les statistiques
        mem_stats = malloc(sizeof(mem_stats_t));
#ifdef WIN32
        FillMemory(mem_stats, 0, sizeof(mem_stats_t));
#else
        bzero(mem_stats, sizeof(mem_stats_t));
#endif
        // enregistrement de la fonction d'affichage des
        // statistiques pour affichage en fin de programme
        atexit(memPrintStats);
        ThreadMutexCreate(&mem_mutex);
    }    
}


void* _memAlloc(size_t size, const char*file, int line)
{
    if (!mem_registered) { memDebugInit(); }
    ThreadMutexLock(&mem_mutex);
    
    // Allocation d'un nouveau bloc mémoire
    // nouvelle taille = taille du bloc + taille de la structure d'infos +
    // taille de la constante magique
    void *newptr = malloc(size + sizeof(mem_bloc_t) + sizeof(mem_debug_magic));
    mem_bloc_t *newnode = (mem_bloc_t*)newptr;
    newnode->size = size;
    newnode->file = (char*)file;
    newnode->line = line;
    newnode->magic1 = mem_debug_magic;
    newnode->magic2 = mem_debug_magic;
    
    // Ajout du nouveau bloc en tête de liste
    newnode->next = mem_blocs;
    newnode->prev = NULL;
    if (mem_blocs != NULL) {
        mem_blocs->prev = newnode;
    }
    mem_blocs = newnode;
    
    // Insertion de la constante magique
    unsigned char *bptr = ((unsigned char*)newptr) + sizeof (mem_bloc_t) + size;
    *((int*)bptr) = mem_debug_magic;
    
    // Mise à jour des statistiques
    mem_stats->count++;
    mem_stats->total += size;
    mem_stats->avg = (int)(mem_stats->avg + size) / 2;
    ThreadMutexUnlock(&mem_mutex);
    
    // Calcul de la vrai adresse d'allocation
    bptr = ((unsigned char*)newptr) + sizeof (mem_bloc_t);
    
    // Tracabilité
    if (mem_debug) {
        printf("memAlloc: 0x%x, %ld bytes %s, line %ld\n",
               (unsigned int)bptr, size, file, (long)line);
    }
    
    return (void*)bptr;
}


void _memFree(void*ptr, const char*file, int line)
{
    unsigned char *bptr;
    mem_bloc_t *bloc;
    
    if (mem_blocs == NULL) {
        printf("memFree: memAlloc was never called: %s, line %ld\n",
               file, (long)line);
        return;
    }
    
    if (ptr == NULL) {
        printf("memFree: trying to free a NULL pointer : %s, line %ld\n",
               file, (long)line);
        return;
    }
    
    if ( (bloc = mem_findbloc(ptr)) == NULL) {
        printf("memFree: trying to free unknown bloc 0x%x: %s, line %ld\n",
               (unsigned int)ptr, file, (long)line);
        return;
    }
    
    ThreadMutexLock(&mem_mutex);
    if (bloc->next != NULL) {
        bloc->next->prev = bloc->prev;
    }
    if (bloc->prev != NULL) {
        bloc->prev->next = bloc->next;
    }
    if (bloc == mem_blocs) {
        mem_blocs = bloc ->next;
    }
    
    bptr = (unsigned char*)ptr;
    
    if (mem_debug) {
        printf ("memFree: 0x%x: %s, line %ld\n", (unsigned int)ptr, file, (long)line);
    }
    
    ptr = (void*)(bptr - sizeof(mem_bloc_t));
    bloc = (mem_bloc_t*)ptr;
    bptr += bloc->size;
    ThreadMutexUnlock(&mem_mutex);
    
    if (*((int*)bptr) !=  mem_debug_magic) {
        printf("memFree: la mémoire aprés le bloc 0x%x a été écrasée : %s, ligne %ld\n",
               (unsigned int)ptr, file, (long)line);
    }
    if ( (bloc->magic1 != mem_debug_magic) ||
        (bloc->magic2 != mem_debug_magic) ) {
        printf("memFree: la mémoire avant le bloc 0x%x a été écrasée : %s, ligne %ld\n",
               (unsigned int)ptr, file, (long)line);
    }
    free (ptr);
}


void* _memCalloc(size_t number, size_t size, const char*file, int line)
{
    void* ptr = _memAlloc(number * size, file, line);
    if (ptr != NULL) {
#ifdef WIN32
        FillMemory(ptr, 0, number * size);
#else
        bzero(ptr, number * size);
#endif
    }
    return ptr;
}


void* _memRealloc(void*oldptr, size_t size, const char*file, int line)
{
    if (oldptr == NULL) {
        return _memAlloc(size, file, line);
    }
    mem_bloc_t *bloc;
    if ( (bloc = mem_findbloc(oldptr)) == NULL) {
        printf("memRealloc: trying to realloc the unknown memory bloc 0x%x: %s, line %ld\n",
               oldptr, file, (long)line);
        return NULL;
    }
    size_t oldsize = bloc->size;
    
    void* newptr = _memAlloc(size, file, line);
    size_t min;
    if (size > oldsize) {
        min = oldsize;
    }
    else {
        min = size;
    }
    memcpy(newptr, oldptr, min);
    _memFree(oldptr, file, line);
    return newptr;
}
