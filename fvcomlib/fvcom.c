#include "fvcom.h"

bool readAllVariable(NC_Handler nc,const char* const vName,VarContext *pvc)
{
    if(pvc && vName)
    {
        pvc->hvar = NC_Inq_Var(nc,vName);
        if(!pvc->hvar)
            return false;
        pvc->type = getVariableType(pvc->hvar);
        if(pvc->type < NC_TYPE_ARR_LEN)
        {
            pvc->size = getVariableOverallSize(pvc->hvar);
            pvc->pdata = malloc(NC_EXT_VARIABLE_SIZE[pvc->type] * (pvc->size));
            if(true == NC_ReadAllValues(nc,pvc->hvar,pvc->pdata))
                return true;
        }
        else
            printError(0, "Unrecognized data type in NetCDF file.");
    }
    return false;
}
int getVariableContext(NC_Handler nc,VarContext *pvc,const char * const name,size_t size)
{
    if(pvc && nc && name)
    {
        pvc->hvar=NC_Inq_Var(nc,name);
        if(!pvc->hvar)
            return 10;
        pvc->type = getVariableType(pvc->hvar);
        if(pvc->type < NC_TYPE_ARR_LEN)
        {
            pvc->size = size;
            pvc->pdata = malloc(NC_EXT_VARIABLE_SIZE[pvc->type] * (pvc->size));
            return 0;
        }
        else
            printError(0, "Unrecognized data type in NetCDF file.");
    }
    return 11;
}
void destroyVariableContext(VarContext *pvc)
{
    if(pvc)
    {
        if(pvc->pdata)
            free(pvc->pdata);
        if(pvc->hvar)
            releaseVariable(pvc->hvar);
    }
}
size_t getDimensionLength(NC_Handler handler,VarContext vctx,const char *dname)
{
    size_t result=0;
    int dLen=0;
    NC_DimInfo* pdims=NC_Inq_VarDimensions(handler,vctx.hvar,&dLen);
    if(!pdims || !dLen)
        return 0;
    for(int i = 0;i < dLen;++i)
    {
        if(0 == strcmp((pdims+i)->name,dname))
            result = (pdims+i)->size;
    }
    free(pdims);
    return result;
}
int getGrid(const char *fname,const char*lname,int epsg,VarContext x,VarContext y,VarContext nv,size_t nele)
{
    GridCell *pgc=(GridCell*)malloc(sizeof(GridCell)*nele);
    GridCell *ptor=pgc;
    if(!pgc)
        return 1;
    for(size_t i = 0;i < nele;++i,++ptor)
    {
        ptor->length=3;
        ptor->pcoords=NULL;
        /*GLERL chose float (4-bytes on 64bit gnu c) for coordinates x y...*/
        ptor->coord1.x=(double)(*(((float*)(x.pdata))+*(((int*)(nv.pdata))+i)-1));
        ptor->coord1.y=(double)(*(((float*)(y.pdata))+*(((int*)(nv.pdata))+i)-1));
        ptor->coord2.x=(double)(*(((float*)(x.pdata))+*(((int*)(nv.pdata))+nele+i)-1));
        ptor->coord2.y=(double)(*(((float*)(y.pdata))+*(((int*)(nv.pdata))+nele+i)-1));
        ptor->coord3.x=(double)(*(((float*)(x.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1));
        ptor->coord3.y=(double)(*(((float*)(y.pdata))+*(((int*)(nv.pdata))+nele*2+i)-1));
    }
    int result=createGrid(fname,lname,epsg,pgc,nele);
    free(pgc);
    return result;
}
