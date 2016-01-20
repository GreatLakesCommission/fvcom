#include "nchelper.h"
#include <stdio.h>
#include <string.h>

int NC_EXT_VARIABLE_SIZE[]={0,1,1,2,4,4,8,1,2,4,8,8,1};

struct _nc_info
{
    int ncid;
    struct _dimInfo *dims;
    int dim_count;
    struct _var_Info *vars;
    int var_count;
    struct _attr_Info *attrs;
    int attr_count;
    int unlmt_dim_id;
};
struct _var_Info
{
    char name[NC_NAME];
    int id;
    nc_type type;
    size_t size;
    struct _dimInfo *pDims;
    int *piDims;
    int dim_count;
    struct _attr_Info *attrs;
    int attr_count;
    size_t *start;
    size_t *count;
};
struct _attr_Info
{
    char name[NC_NAME];
    int id;
    size_t len;
    nc_type type;
};

/*
*Adapter functions between nc error and UNIX errno
*
*@params:
*          errno:  netcdf error number, NOT POSIX errno
*          msg:    user defined error message
*/
inline static void print_nc_error(int nc_errno,const char *msg)
{
    return printError(nc_errno != 0 ? 1 : 0,"NETCDF Error %d: %s\n",nc_errno,msg);
}
inline static void print_nc_error1(int nc_errno,const char *msg,const char *msg1)
{
    return printError(nc_errno != 0 ? 1 : 0,"NETCDF Error %d: %s %s\n",nc_errno,msg,msg1);
}
inline size_t getDimensionSize(NC_Handler pNcInfo,int did)
{
    if(pNcInfo != NULL && did >= 0 && pNcInfo->dims != NULL && did < pNcInfo->dim_count)
        return pNcInfo->dims[did].size;
    return -1;
}
/*
*Retrieve the unlimited dimension id if it exists.
*Must call int NC_MetaData_Inq(NC_Handler handle) first
*
*
*@params:
*	    pNcInfo:  handler for the nc file
*@return:
*           the dimension id or -1 for nothing
*/
inline int getUnlimitedDimension(NC_Handler pNcInfo)
{
    if(pNcInfo)
        return pNcInfo->unlmt_dim_id;
    return -1;
}
/*
*Retrieve the dimension count
*Must call int NC_MetaData_Inq(NC_Handler handle) first
*
*
*@params:
*           pNcInfo:  handler for the nc file
*@return:
*           the dimension count or 0 for nothing
*/
inline int getDimensionCount(NC_Handler pNcInfo)
{
    return pNcInfo == NULL ? 0 : pNcInfo->dim_count;
}
inline int getVariableCount(NC_Handler pNcInfo)
{
    return pNcInfo == NULL ? 0: pNcInfo->var_count;
}
inline int getGlobalAttrCount(NC_Handler pNcInfo)
{
    return pNcInfo == NULL ? 0: pNcInfo->attr_count;
}
inline const char* getDimensionName(NC_Handler pNcInfo, int id)
{
    if(pNcInfo && id >= 0 && pNcInfo->dims != NULL && id < pNcInfo->dim_count)
        return pNcInfo->dims[id].name;
    return NULL;
}
inline const char* getGlobalAttrName(NC_Handler pNcInfo, int id)
{
    if(pNcInfo && id>=0 && pNcInfo->attrs !=NULL && id < pNcInfo->attr_count)
        return pNcInfo->attrs[id].name;
    return NULL;
}
inline const char* getVariableName(NC_Handler pNcInfo, int id)
{
    if(pNcInfo && id >= 0 && pNcInfo->vars != NULL && id < pNcInfo->var_count)
        return pNcInfo->vars[id].name;
    return NULL;
}
inline nc_type getVariableType(NC_Variable var)
{
    return (var != NULL) ? var->type : -1;
}
inline size_t getVariableOverallSize(NC_Variable var)
{
    if(var != NULL && var->pDims != NULL)
    {
        size_t s=1;
        for(int i=0;i < var->dim_count;++i)
           s*=var->pDims[i].size;
        return s;
    }
    return 0;
}
inline static void initNcInfo(struct _nc_info *nc,int nid)
{
    if(!nc)return;
    nc->ncid=nid;
    nc->dim_count=0;
    nc->var_count=0;
    nc->attr_count=0;
    nc->unlmt_dim_id=-1;
    nc->dims=NULL;
    nc->vars=NULL;
    nc->attrs=NULL;
}
inline static void disposeNcInfo(struct _nc_info *pNcInfo)
{
    if(!pNcInfo)return;
    if(pNcInfo->dims != NULL)
        free(pNcInfo->dims);
    if(pNcInfo->attrs != NULL)
        free(pNcInfo->attrs);
    if(pNcInfo->vars != NULL)
    {
        for(int i=0;i < pNcInfo->var_count;++i)
        {
            if(pNcInfo->vars[i].piDims != NULL)
                free(pNcInfo->vars[i].piDims);
            if(pNcInfo->vars[i].attrs != NULL)
                free(pNcInfo->vars[i].attrs);
        }
        free(pNcInfo->vars);
    }
    free(pNcInfo);
}
NC_Handler _nc_open(const char *file,int mode)
{
    struct _nc_info *nc=NULL;
    if(file)
    {
        int tmp=-1;
        int err1=NC_NOERR;
        err1=nc_open(file,mode,&tmp);
        if(NC_NOERR != err1)
            print_nc_error(err1,"Can not open the file!");
        else
        {
            nc=(struct _nc_info*)malloc(sizeof(struct _nc_info));
            if(!nc)
                printError(1,"NETCDF Error: %s","Can not allocate memory in _nc_open(), NULL returned from malloc!");
            else
                initNcInfo(nc,tmp);
        }
    }
    return nc;
}
/*
*Open a netcdf file for reading data
*
*@params:
*           file:  file path and name of the netcdf file
*@return:
*           a _nc_info struct pointer that is redefined as NC_Handler to public
*/
NC_Handler NC_Open4Read(const char *file)
{
    return _nc_open(file,0);
}
/*
*Open a netcdf file for reading and writing data
*
*@params:
*           file:  file path and name of the netcdf file
*@return:
*           a _nc_info struct pointer that is redefined as NC_Handler to public
*/
NC_Handler NC_Open4Write(const char* file)
{
    return _nc_open(file,NC_WRITE);
}
static int lookupDimByName(NC_Handler nc,const char* name)
{
    if(nc != NULL && name != NULL)
    {
        if(nc->dims != NULL)
        {
            for(int i = 0;i < nc->dim_count;++i)
                if(0 == strcmp(nc->dims[i].name,name))
                    return i;
        }
    }
    return -1;
}
NC_Handler NC_Open4Copy(NC_Handler pNcInfo,const char *file)
{
    if(pNcInfo != NULL && file != NULL)
    {
        int err=0;
        int nid=0;
        int did=-1;
        struct _nc_info *nc=NULL;
        err=nc_create(file,NC_NOCLOBBER,&nid);
        if(NC_NOERR != err)
        {
            print_nc_error1(err,nc_strerror(err),file);
            return NULL;
        }
        nc=(struct _nc_info*)malloc(sizeof(struct _nc_info));
        nc->ncid=nid;
        nc->dim_count=pNcInfo->dim_count;
        nc->var_count=pNcInfo->var_count;
        nc->attr_count=pNcInfo->attr_count;
        nc->unlmt_dim_id=pNcInfo->unlmt_dim_id;
        nc->vars=NULL;
        nc->attrs=NULL;
        nc->dims=(struct _dimInfo*)malloc(sizeof(struct _dimInfo)*nc->dim_count);
        size_t dsize=0;
        for(int i = 0;i < pNcInfo->dim_count;++i)
        {
            if(i != pNcInfo->unlmt_dim_id)
                dsize=pNcInfo->dims[i].size;
            else
                dsize=NC_UNLIMITED;
            err=nc_def_dim(nc->ncid,pNcInfo->dims[i].name,dsize,&did);
            if(NC_NOERR != err)
            {
                print_nc_error1(err,nc_strerror(err),pNcInfo->dims[i].name);
                disposeNcInfo(nc);
                return NULL;
            }
            nc->dims[i].id=did;
            nc->dims[i].size=pNcInfo->dims[i].size;
            memset(nc->dims[i].name,0,NC_NAME);
            strcpy(nc->dims[i].name,pNcInfo->dims[i].name);
        }
        for(int i = 0;i < pNcInfo->attr_count;++i)
        {
            err=nc_copy_att(pNcInfo->ncid,NC_GLOBAL,pNcInfo->attrs[i].name,nc->ncid,NC_GLOBAL);
            if(NC_NOERR != err)
            {
                print_nc_error1(err,"Failed to copy global attribute ",pNcInfo->attrs[i].name);
                disposeNcInfo(nc);
                return NULL;
            }
        }
        nc->vars=(struct _var_Info*)malloc(sizeof(struct _var_Info)*pNcInfo->var_count);
        memset(nc->vars,0,sizeof(struct _var_Info)*pNcInfo->var_count);
        for(int i = 0;i<pNcInfo->var_count;++i)
        {
            err=nc_def_var(nc->ncid,pNcInfo->vars[i].name,pNcInfo->vars[i].type,pNcInfo->vars[i].dim_count,pNcInfo->vars[i].piDims,&nc->vars[i].id);
            if(NC_NOERR != err)
            {
                print_nc_error1(err,nc_strerror(err),pNcInfo->vars[i].name);
                disposeNcInfo(nc);
                return NULL;
            }
            memset(nc->vars[i].name,0,NC_NAME);
            strcpy(nc->vars[i].name,pNcInfo->vars[i].name);
            nc->vars[i].type=pNcInfo->vars[i].type;
            nc->vars[i].dim_count=pNcInfo->vars[i].dim_count;
            nc->vars[i].piDims=(int*)malloc(sizeof(int)*nc->vars[i].dim_count);
            for(int j = 0;j < nc->vars[i].dim_count;++j)
            {
                int did=lookupDimByName(nc,pNcInfo->dims[pNcInfo->vars[i].piDims[j]].name);
                if(did<0)
                    print_nc_error1(-1,"dimension id is not valid for",pNcInfo->dims[pNcInfo->vars[i].piDims[j]].name);
                nc->vars[i].piDims[j]=did;
            }
            for(int j = 0;j < pNcInfo->vars[i].attr_count;++j)
            {
                err=nc_copy_att(pNcInfo->ncid,pNcInfo->vars[i].id,pNcInfo->vars[i].attrs[j].name,nc->ncid,nc->vars[i].id);
                if(NC_NOERR != err)
                {
                    print_nc_error1(err,nc_strerror(err),pNcInfo->vars[i].name);
                    disposeNcInfo(nc);
                    return NULL;
                }
            }
        }
        err=nc_enddef(nc->ncid);
        if(NC_NOERR != err)
        {
            print_nc_error(err,nc_strerror(err));
            disposeNcInfo(nc);
            return NULL;
        }
        return nc;
    }
    return NULL;
}
static int lookupVariableByName(NC_Handler onc,const char* name)
{
    if(onc != NULL && name != NULL)
    {
        if(onc->vars != NULL)
        {
            for(int i = 0;i < onc->var_count;++i)
                if(0==strcmp(onc->vars[i].name,name))
                    return i;
        }
    }
    return -1;
}
static size_t lookupVariableTypeSize(nc_type vartype)
{
    switch(vartype)
    {
        case NC_CHAR:
            return 1;
            break;
        case NC_INT:
            return 4;
            break;
        case NC_FLOAT:
            return 4;
            break;
        case NC_DOUBLE:
            return 8;
            break;
        default:
            break;
    }
    return 0;
}
char writeFloatAll(NC_Handler handler,NC_Variable pvar,double* pdata)
{
    int err=nc_put_vara(handler->ncid,pvar->id,pvar->start,pvar->count,pdata);
    if(NC_NOERR != err)
    {
        print_nc_error(err,"nc_put_vara failed to write the variable");
        return -1;
    }
    return 0;
}
int copyVariablesByUlimitDim(NC_Handler onc,NC_Handler nc,int start,int offset)
{
    if(onc != NULL && nc != NULL && start >= 0 && offset > 0)
    {
        int err=0;
        void *data=NULL;
        for(int i = 0;i < nc->var_count;++i)
        {
            data=NULL;
            int ovarid=lookupVariableByName(onc,nc->vars[i].name);
            size_t typesize=lookupVariableTypeSize(nc->vars[i].type);
            if(0 == typesize)
                return -1;
            nc->vars[i].start=(size_t*)malloc(sizeof(size_t)*nc->vars[i].dim_count);
            nc->vars[i].count=(size_t*)malloc(sizeof(size_t)*nc->vars[i].dim_count);
            int udidx=-1;
            for(int j = 0;j < nc->vars[i].dim_count;++j)
            {
                if(nc->vars[i].piDims[j] == nc->unlmt_dim_id)
                {
                    nc->vars[i].start[j]=start;
                    nc->vars[i].count[j]=offset;
                    udidx=j;
                    typesize*=offset;
                }
                else
                {
                    nc->vars[i].start[j]=0;
                    nc->vars[i].count[j]=nc->dims[nc->vars[i].piDims[j]].size;
                    typesize*=nc->vars[i].count[j];
                }
            }
            data = malloc(typesize);//printf("%lu\n",typesize);
            err = nc_get_vara(onc->ncid,ovarid,nc->vars[i].start,nc->vars[i].count,data);
            if(NC_NOERR != err)
            {
                print_nc_error1(err,nc_strerror(err),onc->vars[i].name);
                free(nc->vars[i].start);
                free(nc->vars[i].count);
                free(data);
                return -1;
            }
            if(udidx >= 0)
                nc->vars[i].start[udidx]=0;
            err = nc_put_vara(nc->ncid,nc->vars[i].id,nc->vars[i].start,nc->vars[i].count,data);
            if(NC_NOERR != err)
            {
                print_nc_error1(err,nc_strerror(err),nc->vars[i].name);
                free(nc->vars[i].start);
                free(nc->vars[i].count);
                free(data);
                return -1;
            }
            free(nc->vars[i].start);
            free(nc->vars[i].count);
            free(data);

        }
        return 0;
    }
    return -1;
}
/*
*Close a netcdf file
*
*@params:
*           pNcInfo:  the handler representing the pointer of _nc_info struct
*@return:
*           none
*/
void NC_Close(NC_Handler pNcInfo)
{
    if(pNcInfo)
    {
        int err=NC_NOERR;
        err=nc_close(pNcInfo->ncid);
        if(NC_NOERR != err)
            print_nc_error(err,nc_strerror(err));
        disposeNcInfo(pNcInfo);
    }
}
/*
*Setter for a dimension's name. Initialize the memebers of NC_DimInfo struct
*including name,start, and count
*
*@params:
*          pDimInfo:  pointer to a NC_DimInfo struct
*
*/
void NC_SetDimName(NC_DimInfo *pDimInfo,const char *pszName)
{
    if(pDimInfo && pszName)
    {
        memset(pDimInfo->name,0,NC_NAME);
        strncpy(pDimInfo->name,pszName,NC_NAME-1);
        pDimInfo->start=0;
        pDimInfo->count=0;
    }
}
NC_DimInfo* NC_GetDimByName(struct _var_Info* var,const char *dimName)
{
    NC_DimInfo *pdim=NULL;
    if(var&&dimName&&var->pDims)
    {
        for(int i = 0;i < var->dim_count;++i)
        {
            if(0 == strncmp((var->pDims+i)->name,dimName,strlen((var->pDims+i)->name)))
                return var->pDims+i;
        }
    }
    return pdim;
}
static NC_DimInfo* NC_Inq_VarDims(NC_Handler handle,int vid,int *len)
{
    int err = NC_NOERR;
    if(handle && vid>=0&&len)
    {
        err = nc_inq_varndims(handle->ncid,vid,len);
        if(NC_NOERR != err)
        {
            print_nc_error(err,"NC_Inq_VarDims can not inquire variable's dimension count for variable");
            return NULL;
        }
        NC_DimInfo *dimp=(NC_DimInfo*)malloc(sizeof(NC_DimInfo)*(*len));
        int *idimp=(int*)malloc(sizeof(int)*(*len));
        err=nc_inq_vardimid(handle->ncid,vid,idimp);
        if(NC_NOERR != err)
        {
            print_nc_error(err,"NC_Inq_VarDims can not inquire variable's dimension ids.");
            free(idimp);
            free(dimp);
            return NULL;
        }
        for(int i = 0;i<*len;++i)
        {
            dimp[i].id=idimp[i];
            dimp[i].size=0;
            dimp[i].start=0;
            dimp[i].count=0;
            dimp[i].name[0]=0;
            if(0 != NC_ReadDimInfo(&dimp[i],handle))
            {
                free(dimp);
                dimp=NULL;
                break;
            }
        }
        free(idimp);
        return dimp;
    }
    return NULL;
}
NC_DimInfo* NC_Inq_VarDimensions(NC_Handler handle,NC_Variable pVar,int *len)
{
    if(handle && pVar&&len)
        return NC_Inq_VarDims(handle,pVar->id,len);
    return NULL;
}
NC_Variable NC_Inq_Var(NC_Handler handle, const char *var)
{
    int err=NC_NOERR;
    int vid=0;
    nc_type xtype;

    if(var)
    {
        if(handle->vars != NULL && handle->var_count > 0)//if data have been populated
        {
            for(int i = 0;i<handle->var_count;++i)
            {
                if(0==strncmp(handle->vars[i].name,var,strlen(handle->vars[i].name)))
                    return &handle->vars[i];
            }

        }
        else//more efficient for inquiring only a single variable
        {
            err=nc_inq_varid(handle->ncid, var, &vid);
            if(NC_NOERR != err)
            {
                print_nc_error1(err,"NC_Inq_Var can not inquire variable",var);
                return NULL;
            }
            err=nc_inq_vartype(handle->ncid,vid,&xtype);
            if(NC_NOERR != err)
            {
                print_nc_error1(err,"NC_Inq_Var can not inquire variable type for",var);
                return NULL;
            }
            int dimlen=0;
            NC_DimInfo* pdiminfo=NC_Inq_VarDims(handle,vid,&dimlen);
            if(pdiminfo)
            {
                 NC_Variable pvar= NC_DefineVariable(var,handle,pdiminfo,dimlen);
                 if(pvar)
                 {
                     pvar->type=xtype;
                     pvar->start=(size_t*)malloc(sizeof(size_t)*dimlen);
                     pvar->count=(size_t*)malloc(sizeof(size_t)*dimlen);
                     return pvar;
                 }
            }
        }
    }
    return NULL;
}
int NC_MetaData_Inq(NC_Handler handle)
{
    int err=NC_NOERR;
    int ndims,nvars,ngatts,unlimdimid;
    struct _dimInfo *pdim=NULL;
    struct _attr_Info *pattr=NULL;
    struct _var_Info *pvar=NULL;
    int ndimsp=0;
    err = nc_inq(handle->ncid,&ndims,&nvars,&ngatts,&unlimdimid);
    if(NC_NOERR != err)
    {
        print_nc_error(err,"Can not inquire metadata.");
        return err;
    }
    handle->dim_count=ndims;
    handle->var_count=nvars;
    handle->attr_count=ngatts;
    handle->unlmt_dim_id=unlimdimid;
    if(ndims > 0)
        handle->dims=(struct _dimInfo*)malloc(sizeof(struct _dimInfo)*ndims);
    if(ngatts > 0)
        handle->attrs=(struct _attr_Info*)malloc(sizeof(struct _attr_Info)*ngatts);
    if(nvars > 0)
        handle->vars=(struct _var_Info*)malloc(sizeof(struct _var_Info)*nvars);
    for(int i = 0;i < ndims;++i)
    {
        pdim=&(handle->dims[i]);
        pdim->id=i;
        memset(pdim->name,0,NC_NAME);
        pdim->size=0;
        err=nc_inq_dim(handle->ncid,pdim->id,pdim->name,&(pdim->size));
        if(NC_NOERR!=err)
        {
            print_nc_error(err,"Can not inquire dimension.");
            return err;
        }
        //printf("%s:%lu:%d\n",name,lengthp);
        //memset(name,0,NC_MAX_NAME+1);
    }
    for(int i = 0 ;i < ngatts;++i)
    {
        pattr = &(handle->attrs[i]);
        pattr->id=i;
        memset(pattr->name,0,NC_NAME);
        pattr->type=0;
        pattr->len=0;
        err=nc_inq_attname(handle->ncid,NC_GLOBAL,pattr->id,pattr->name);
        if(NC_NOERR != err)
        {
            print_nc_error(err,"Can not inquire global attribute' name.");
            return err;
        }
        err=nc_inq_att(handle->ncid,NC_GLOBAL,pattr->name,&(pattr->type), &(pattr->len));
        if(NC_NOERR != err)
        {
            print_nc_error(err,"Can not inquire metatdata of the global attribute.");
            return err;
        }
        //printf("Global attr:%s:%lu\n",name,lenp);
        //memset(name,0,NC_MAX_NAME+1);
    }
    for(int i = 0;i < nvars;++i)
    {
        pvar = &(handle->vars[i]);
        pvar->id=i;
        pvar->type=0;
        pvar->attr_count=0;
        memset(pvar->name,0,NC_NAME);
        pvar->dim_count=0;
        pvar->piDims=NULL;
        pvar->attrs=NULL;
        err=nc_inq_varndims(handle->ncid, pvar->id, &(pvar->dim_count));
        if(NC_NOERR != err)
        {
            print_nc_error(err,"Can not inquire dimensions of the variable.");
            return err;
        }
        if(pvar->dim_count>0)
        {
            pvar->piDims=(int*)malloc(sizeof(int)*pvar->dim_count);
            memset(pvar->piDims,0,sizeof(int)*pvar->dim_count);
        }
        err=nc_inq_var(handle->ncid,pvar->id,pvar->name,&(pvar->type),&ndimsp,pvar->piDims,&(pvar->attr_count));
        if(NC_NOERR != err)
        {
            print_nc_error(err,"Can not inquire metadata of the variable.");
            return err;
        }
        //printf("%s:%d;%d|",name,ndimsp,nattsp);
        //int j=0;
        //for(;j<ndimsp;++j)
        //    printf("%d,",dimids[j]);
        //printf("\n");
        //free(dimids);
        //memset(name,0,NC_MAX_NAME+1);
        if(pvar->attr_count > 0)
            pvar->attrs=(struct _attr_Info*)malloc(sizeof(struct _attr_Info)*pvar->attr_count);
        for(int j = 0;j < pvar->attr_count;++j)
        {
            pattr = &(pvar->attrs[j]);
            pattr->id=j;
            pattr->len=0;
            pattr->type=0;
            memset(pattr->name,0,NC_NAME);
            err=nc_inq_attname(handle->ncid,pvar->id,pattr->id,pattr->name);
            if(NC_NOERR != err)
            {
                print_nc_error(err,"Can not inquire name of the attribute for the variable.");
                return err;
            }
            err=nc_inq_att(handle->ncid,pvar->id,pattr->name,&(pattr->type), &(pattr->len));
            if(NC_NOERR != err)
            {
                print_nc_error(err,"Can not inquire metadata of the attribute for the variable.");
                return err;
            }
            //printf("    attr:%s:%lu\n",name,lenp);
            //memset(name,0,NC_MAX_NAME+1);
        }
    }
    return 0;
}
/*
*Read a dimension info by its name or id from a nectcdf file. Even an error occurs, it
*won't stop execute but returning -1 stands for error
*
*@params:
*          pDimInfo:  a pointer to an initialized NC_DimInfo struct
*          pNcInfo:   a NC_Handler (the pointer of _nc_info)
*                     representing a netcdf file
*
*@return:
*          0:  no error
*          -1: error occurs
*/
char NC_ReadDimInfo(NC_DimInfo *pDimInfo,NC_Handler pNcInfo)
{
    char result=-1;
    if(pDimInfo && pNcInfo)
    {
        int dlen = strlen(pDimInfo->name);
        if(dlen > 0 || pDimInfo->id >= 0)
        {
            int err=NC_NOERR;
            if(dlen > 0)
                err=nc_inq_dimid(pNcInfo->ncid,pDimInfo->name,&pDimInfo->id);
            else
                err=nc_inq_dimname(pNcInfo->ncid,pDimInfo->id,pDimInfo->name);
            if(NC_NOERR != err)
                print_nc_error1(err,"Can not query dimension id for",pDimInfo->name);
            else
            {
                err=nc_inq_dimlen(pNcInfo->ncid,pDimInfo->id,&pDimInfo->size);
                if(NC_NOERR != err)
                    print_nc_error1(err,"Can not query dimension length for",pDimInfo->name);
                else
                    result=0;
            }
        }
    }
    return result;
}
/*
*Populate a variable's nid by its name. Initilize the members of _var_Info struct
*link all dimension infos to _var_Info
*
*@params:
*          pszName:  variable name
*          pNcInfo:  NC_Handler (the pointer of _nc_info)
*          pDimInfo: a NC_DimInfo struct pointer to an array of dimensions metatdata
*          dim_len:  the length of dimensions
*
*@return:
*          a pointer to _var_Info struct that is redefined as NC_Variable to public
*/
NC_Variable NC_DefineVariable(const char *pszName,NC_Handler pNcInfo,NC_DimInfo *pDimInfo,size_t dim_len)
{
    struct _var_Info *var=NULL;
    int err=NC_NOERR;
    int temp;
    if(pNcInfo && pszName && pDimInfo && dim_len > 0)
    {
        err=nc_inq_varid(pNcInfo->ncid,pszName,&temp);
        if(NC_NOERR!=err)
            print_nc_error1(err,"Can not query variable id for",pszName);
        else
        {
            var=(struct _var_Info*)malloc(sizeof(struct _var_Info));
            if(!var)
                printError(1,"NETCDF Error: %s","Can not allocate memory in NC_DefineVariable, NULL returned from malloc!\n");
            else
            {
                memset(var->name,0,NC_NAME);
                strncpy(var->name,pszName,NC_NAME-1);
                var->id=temp;
                var->pDims=pDimInfo;
                var->dim_count=dim_len;
                var->attrs=NULL;
                var->piDims=NULL;
                var->start=NULL;
                var->count=NULL;
           }
        }
    }
    return var;
}
NC_Variable getEmptyVariable()
{
    NC_Variable ptr=(NC_Variable)malloc(sizeof(struct _var_Info));
    ptr->id=-1;
    ptr->type=-1;
    memset(ptr->name,0,NC_NAME);
    ptr->pDims=NULL;
    ptr->piDims=NULL;
    ptr->attrs=NULL;
    ptr->dim_count=0;
    ptr->attr_count=0;
    ptr->start=NULL;
    ptr->count=NULL;
    return ptr;
}
void releaseVariable(NC_Variable pVar)
{
    if(pVar!=NULL)
    {
        if(pVar->pDims!=NULL)
            free(pVar->pDims);
        if(pVar->piDims!=NULL)
            free(pVar->piDims);
        if(pVar->attrs!=NULL)
            free(pVar->attrs);
        if(pVar->start!=NULL)
            free(pVar->start);
        if(pVar->count!=NULL)
            free(pVar->count);
        free(pVar);
    }
}
/*int NC_DefineVariable(NC_Handler pNcInfo,NC_Variable *pVar)
{
    if(pNcInfo!=NULL&&pVar!=NULL&&pNcInfo->vars!=NULL)
    {
        if(pVar->id>=0&&pVar->id<pNcInfo->var_count)
        {
            strcpy(pVar->name,pNcInfo->vars[pVar->id].name);
            pVar->type=pNcInfo->vars[pVar->id].type;
            pVar->dim_count=pNcInfo->vars[pVar->id].dim_count;
            pVar->piDims=(int*)malloc(sizeof(int)*pVar->dim_count);
            memcpy(pVar->piDims,pNcInfo->vars[pVar->id].piDims,pNcInfo->vars[pVar->id].dim_count);
        }
    }
    return -1;
}*/
/*
*Destroy variable info struct
*
*@params:
*          pVar: a handler to NC_Variable that is a pointer to _var_Info struct
*/
void NC_DestroyVariable(NC_Variable pVar)
{
    if(pVar)
    {
        if(pVar->start)
            free(pVar->start);
        if(pVar->count)
            free(pVar->count);
        free(pVar);
    }
}
/*
*Populate real data values to an array
*
*@params:
*          pNcInfo:  the handler representing the pointer of _nc_info struct
*          pVarInfo:  a handler to NC_Variable that is a pointer to _var_Info struct
*          pfVars:  a double array as a container for data values
*
*@return:
*          0: no error
*         -1: error occurs
*/
char NC_ReadFloatArray(NC_Handler pNcInfo,NC_Variable pVarInfo,double *pfVars)
{
    char result=-1;
    if(pNcInfo && pVarInfo && pfVars)
    {
        if(!pVarInfo->start)
        {
            pVarInfo->start=(size_t*)malloc(sizeof(size_t)*(pVarInfo->dim_count));
            if(!pVarInfo->start)
            {
                printError(1,"NETCDF Error: %s","Can not allocate memory in NC_ReadFloatArray, NULL returned from malloc!\n");
                return result;
            }
        }
        if(!pVarInfo->count)
        {
            pVarInfo->count=(size_t*)malloc(sizeof(size_t)*(pVarInfo->dim_count));
            if(!pVarInfo->count)
            {
                printError(1,"NETCDF Error: %s","Can not allocate memory in NC_ReadFloatArray, NULL returned from malloc!\n");
                return result;
            }
        }
        for(int i = 0;i < pVarInfo->dim_count;++i)
        {
            pVarInfo->start[i]=pVarInfo->pDims[i].start;
            pVarInfo->count[i]=pVarInfo->pDims[i].count;
        }
        int err = NC_NOERR;
        err = nc_get_vara_double(pNcInfo->ncid,pVarInfo->id,pVarInfo->start,pVarInfo->count,pfVars);
        if(NC_NOERR != err)
            print_nc_error1(err,"Can not read values for variable(double)",pVarInfo->name);
        else
            result=0;
    }
    return result;
}
/*
*Populate all double data values to an array
*
*@params:
*          pNcInfo:  the handler representing the pointer of _nc_info struct
*          pVarInfo:  a handler to NC_Variable that is a pointer to _var_Info struct
*          pfVars:  a double array as a container for data values
*
*@return:
*          0: no error
*         -1: error occurs
*/
char NC_ReadAllFloat(NC_Handler pNcInfo,NC_Variable pVarInfo,double *pfVars)
{
    if(pVarInfo && pVarInfo->pDims)
    {
        for(int i = 0;i<pVarInfo->dim_count;++i)
            pVarInfo->pDims[i].count=pVarInfo->pDims[i].size;
        return NC_ReadFloatArray(pNcInfo,pVarInfo,pfVars);
    }
    return -1;
}
static char NC_ReadArray(NC_Handler pNcInfo,NC_Variable pVarInfo,void *pfVars)
{
    char result=-1;
    if(pNcInfo && pVarInfo && pfVars)
    {
        if(!pVarInfo->start)
        {
            pVarInfo->start=(size_t*)malloc(sizeof(size_t)*(pVarInfo->dim_count));
            if(!pVarInfo->start)
            {
                printError(1,"NETCDF Error: %s","Can not allocate memory in NC_ReadArray, NULL returned from malloc!\n");
                return result;
            }
        }
        if(!pVarInfo->count)
        {
            pVarInfo->count=(size_t*)malloc(sizeof(size_t)*(pVarInfo->dim_count));
            if(!pVarInfo->count)
            {
                printError(1,"NETCDF Error: %s","Can not allocate memory in NC_ReadArray, NULL returned from malloc!\n");
                return result;
            }
        }
        for(int i=0;i<pVarInfo->dim_count;++i)
        {
            pVarInfo->start[i]=pVarInfo->pDims[i].start;
            pVarInfo->count[i]=pVarInfo->pDims[i].count;
        }
        int err = NC_NOERR;
        err = nc_get_vara(pNcInfo->ncid,pVarInfo->id,pVarInfo->start,pVarInfo->count,pfVars);
        if(NC_NOERR != err)
            print_nc_error1(err,"Can not read values for variable",pVarInfo->name);
        else
            result=0;
    }
    return result;
}
char NC_ReadValues(NC_Handler pNcInfo,NC_Variable pVarInfo,void *pfVars)
{
    if(pVarInfo && pVarInfo->pDims)
        return NC_ReadArray(pNcInfo,pVarInfo,pfVars);
    return -1;
}
char NC_ReadAllValues(NC_Handler pNcInfo,NC_Variable pVarInfo,void *pVars)
{
    if(pVarInfo && pVarInfo->pDims)
    {
        for(int i = 0;i<pVarInfo->dim_count;++i)
        {
            pVarInfo->pDims[i].start=0;
            pVarInfo->pDims[i].count=pVarInfo->pDims[i].size;
        }
        return NC_ReadArray(pNcInfo,pVarInfo,pVars);
    }
    return -1;
}
char NC_AddVariableTextAttribute(NC_Handler pNcInfo,NC_Variable pVar,const char *name,const char *value)
{
    if(pNcInfo && pVar&&name&&value)
    {
        int err = NC_NOERR;
        err=nc_redef(pNcInfo->ncid);
        if(err != NC_NOERR)
            print_nc_error(err,"Failed to enter redef mode");
        else
        {
            err = nc_put_att_text(pNcInfo->ncid,pVar->id,name,strlen(value),value);
            if(NC_NOERR != err)
                print_nc_error1(err,"Failed to add attribute",name);
            nc_enddef(pNcInfo->ncid);
            if(NC_NOERR == err)
                return 0;
        }
    }
    return -1;
}
/*
*Remove attributes for a variable
*@return:
*          0: no error
*         -1: error occurs
*/
char NC_RemoveVariableAttributes(NC_Handler pNcInfo,NC_Variable pVar,char **attrs,int len)
{
    if(pNcInfo && pVar && attrs && len > 0)
    {
        int err=NC_NOERR;
        err=nc_redef(pNcInfo->ncid);
        if(NC_NOERR != err)
            print_nc_error(err,"Failed to enter redef mode");
        else
        {
            for(int i = 0;i < len;++i)
            {
                err = nc_del_att(pNcInfo->ncid,pVar->id,attrs[i]);
                if(NC_NOERR != err)
                    print_nc_error1(err,"Failed to delete attribute",attrs[i]);
            }
            nc_enddef(pNcInfo->ncid);
            if(NC_NOERR != err)
                return 0;
        }
    }
    return -1;
}

