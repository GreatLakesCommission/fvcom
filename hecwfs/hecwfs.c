#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "util.h"
#include "nchelper.h"

typedef struct _varContext
{
    size_t size;
    nc_type type;
    NC_Variable hvar;
    void *pdata;
}VarContext;
int main(void)
{
    return 0;
}

