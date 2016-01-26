#ifndef FVCOM_UTIL_H
#define FVCOM_UTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ogr_api.h>
#include <ogr_srs_api.h>

#ifdef PATH_MAX
static int pathmax=PATH_MAX;
#else
static int pathmax=0;
#endif

#define SUSV3 200112L
static long posix_version=0;

#define PATH_MAX_GUESS 1024

#define NBUCKETS 31

#define READBUFFSIZ 1024

#define MJD2EPOCH_MAGICNUM (2440587.5-2400000.5)
#define MJD2EPOCH(MJDC) (time_t)(((MJDC)-(MJD2EPOCH_MAGICNUM))*24*3600)

#ifdef __cplusplus
extern "C"{
#endif

struct _hash_table;
typedef struct _hash_table* HASHTAB;
typedef enum{
    KEEP,
    REMOVE
}HASH_LOOKUP_MOD;

typedef enum{
    NON_RESURSIVE,
    RESURSIVE

}DIR_SEARCH_MOD;

typedef struct _coordinate
{
    double x;
    double y;
}Coordinate;
typedef struct _gridcell
{
    Coordinate coord1;
    Coordinate coord2;
    Coordinate coord3;
    Coordinate coord4;
    Coordinate* pcoords;
    size_t length;
}GridCell;

char* trimString(char *str);
void toLowerString(char *str);
int iterateRegFileInDir(const char* dirName,DIR_SEARCH_MOD mode,void (*fun)(const char*));
char* unix_path_alloc(int *sizep);
void printError(int errNoFlag,const char *fmt,...);
struct _hash_table* createHashTab();
int hashTabPut(struct _hash_table *table,char * const key,size_t keysize,void *data,size_t datasize);
void* hashTabLookup(struct _hash_table *table,const char * const key,size_t keysize,size_t *pdatasize,HASH_LOOKUP_MOD mod);
void hashTabDestroy(struct _hash_table *table);
size_t gethashTabSize(struct _hash_table *table);
struct _hash_table* getProperties(const char *path);
int createGrid(const char *pname,const char *playername,int epsg,GridCell *cells,int count);
inline void* fvm_malloc(size_t size)
{
    void *ptr=NULL;
    if(size > 0)
    {
        ptr = malloc(size);
        if(NULL == ptr)
        {
            printError(0, "Failed to allocate memory with size of: u% bytes", size);
            abort();
        }
    }
    return ptr;
}

#ifdef __cplusplus
}
#endif

#endif // FVCOM_UTIL_H
