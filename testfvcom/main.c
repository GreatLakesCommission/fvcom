#include <stdio.h>
#include <assert.h>
#include "util.h"
int main(void)
{
    struct _hash_table* ptable = createHashTab();
    if(ptable)
    {
        hashTabPut(ptable,"abcd",4,"12345",5);
        hashTabPut(ptable,"abcd",4,"1234",5);
        hashTabPut(ptable,"abc",3,"12345",5);
        hashTabPut(ptable,"abcdef",6,"12345",5);
        hashTabPut(ptable,"abcde",5,"12345",5);

        size_t datasize=0;
        assert(NULL==hashTabLookup(ptable,"ert",5,&datasize,REMOVE));
        assert(NULL==hashTabLookup(ptable,"ab",5,&datasize,REMOVE));
        assert(NULL==hashTabLookup(ptable,"abcdefg",5,&datasize,REMOVE));
        printf("hashtable size(4 expected): %u\n",gethashTabSize(ptable));

        printf("[%s, %s](1234 expected)\n","abcd", (char*)hashTabLookup(ptable,"abcd",4,&datasize,KEEP));
        hashTabLookup(ptable,"abcd",4,&datasize,REMOVE);
        printf("hashtable size(3 expected): %u\n",gethashTabSize(ptable));
        hashTabLookup(ptable,"abcde",5,&datasize,REMOVE);
        printf("hashtable size(2 expected): %u\n",gethashTabSize(ptable));
        hashTabLookup(ptable,"abcdef",5,&datasize,REMOVE);
        printf("hashtable size(1 expected): %u\n",gethashTabSize(ptable));
        hashTabPut(ptable,"abcdef",6,"12345",5);
        hashTabPut(ptable,"abcde",5,"12345",5);
        printf("hashtable size(3 expected): %u\n",gethashTabSize(ptable));
        hashTabLookup(ptable,"abcdef",5,&datasize,REMOVE);
        hashTabLookup(ptable,"abcde",5,&datasize,REMOVE);
        printf("hashtable size(1 expected): %u\n",gethashTabSize(ptable));
        hashTabLookup(ptable,"abc",5,&datasize,REMOVE);
        printf("hashtable size(0 expected): %u\n",gethashTabSize(ptable));
        hashTabDestroy(ptable);
    }
    ptable = getProperties("config");
    printError(0,"%s","new line added?");
    printf("check\n");
    return 0;
}

