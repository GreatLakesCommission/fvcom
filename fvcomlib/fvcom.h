#ifndef FVCOM_H
#define FVCOM_H
#include "stdlib.h"
#include "nchelper.h"
#include "util.h"
#ifdef __cplusplus
extern "C"{
#endif
typedef struct _varContext
{
    size_t size;
    nc_type type;
    NC_Variable hvar;
    void *pdata;
}VarContext;
bool readAllVariable(NC_Handler nc,const char* const vName,VarContext *pvc);
int getVariableContext(NC_Handler nc,VarContext *pvc,const char * const name,size_t size);
void destroyVariableContext(VarContext *pvc);
size_t getDimensionLength(NC_Handler handler,VarContext vctx,const char *dname);
void destroyVariableContext(VarContext *pvc);
size_t getDimensionLength(NC_Handler handler,VarContext vctx,const char *dname);
int getGrid(const char *fname,const char*lname,int epsg,VarContext x,VarContext y,VarContext nv,size_t nele);
#ifdef __cplusplus
}
#endif //__cplusplus
#endif // FVCOM_H
