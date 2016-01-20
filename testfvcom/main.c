#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "fvcom.h"
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
        printf("hashtable size(4 expected): %zu\n",gethashTabSize(ptable));

        printf("[%s, %s](1234 expected)\n","abcd", (char*)hashTabLookup(ptable,"abcd",4,&datasize,KEEP));
        hashTabLookup(ptable,"abcd",4,&datasize,REMOVE);
        printf("hashtable size(3 expected): %zu\n",gethashTabSize(ptable));
        hashTabLookup(ptable,"abcde",5,&datasize,REMOVE);
        printf("hashtable size(2 expected): %zu\n",gethashTabSize(ptable));
        hashTabLookup(ptable,"abcdef",5,&datasize,REMOVE);
        printf("hashtable size(1 expected): %zu\n",gethashTabSize(ptable));
        hashTabPut(ptable,"abcdef",6,"12345",5);
        hashTabPut(ptable,"abcde",5,"12345",5);
        printf("hashtable size(3 expected): %zu\n",gethashTabSize(ptable));
        hashTabLookup(ptable,"abcdef",5,&datasize,REMOVE);
        hashTabLookup(ptable,"abcde",5,&datasize,REMOVE);
        printf("hashtable size(1 expected): %zu\n",gethashTabSize(ptable));
        hashTabLookup(ptable,"abc",5,&datasize,REMOVE);
        printf("hashtable size(0 expected): %zu\n",gethashTabSize(ptable));
        hashTabDestroy(ptable);
    }
    ptable = getProperties("config");
    printError(0,"%s","new line added?");
    printf("check\n");

    printf("float:%zu\n",sizeof(float));

    NC_Handler hecwfs = NC_Open4Read("201602015.nc");
    if(hecwfs)
    {
        VarContext lon;
        assert(true == readAllVariable(hecwfs,"x",&lon));
        printf("%f\n",((float*)(lon.pdata))[0]);
        VarContext lat;
        assert(true == readAllVariable(hecwfs,"y",&lat));
        VarContext nv;
        assert(true == readAllVariable(hecwfs,"nv",&nv));
        assert(0 == getGrid("hecwfs-grid.shp","grid",26990, lon, lat, nv, getDimensionLength(hecwfs,nv,"nele")));

        destroyVariableContext(&lon);
        destroyVariableContext(&lat);
        destroyVariableContext(&nv);
        NC_Close(hecwfs);
    }

    NC_Handler slrfvm = NC_Open4Read("201532103.nc");
    if(slrfvm)
    {
        VarContext lon;
        assert(true == readAllVariable(slrfvm,"lon",&lon));
        printf("%f\n",((float*)(lon.pdata))[0]);

        VarContext lonfoobar;
        memset(&lonfoobar,0,sizeof(VarContext));
        assert(false == readAllVariable(slrfvm,"lonfoobar",&lonfoobar));
        assert(0 == lonfoobar.hvar);
        assert(0 == lonfoobar.pdata);
        assert(0 == lonfoobar.size);
        assert(0 == lonfoobar.type);

        VarContext lat;
        assert(true == readAllVariable(slrfvm,"lat",&lat));
        VarContext nv;
        assert(true == readAllVariable(slrfvm,"nv",&nv));
        assert(0 == getGrid("slrfvm-grid.shp","grid",4269, lon, lat, nv, getDimensionLength(slrfvm,nv,"nele")));

        destroyVariableContext(&lon);
        destroyVariableContext(&lat);
        destroyVariableContext(&nv);
        NC_Close(slrfvm);
    }
    assert(NULL == NC_Open4Read("nonexist.nc"));
    return 0;
}

