#ifndef FVCOM_FVMDDSCONTEXT_H
#define FVCOM_FVMDDSCONTEXT_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "util.h"
#ifdef __cplusplus
extern "C"{
#endif

typedef struct _fvcomStrList
{
    char *pstr;
    struct _fvcomStrList *next;
}fvcomStrList;

typedef struct _arrayDim
{
    const char *name;
    uint32_t len;
    struct _arrayDim *next;
}arrayDim;

typedef struct _dataVarDecl
{
    uint8_t type;
    const char *name;
    arrayDim *dims;
    struct _dataVarDecl *next;

}dataVarDecl;

typedef struct _fvcomDdsContext
{
    fvcomStrList* strList;
    arrayDim* dimList;
    dataVarDecl* varList;
}fvcomDdsContext;

typedef struct _datasetDecl
{
    int numOfVars;
    dataVarDecl *varDecl;
}datasetDecl;

enum
{
    DDS_TYPE_BYTE = 1,
    DDS_TYPE_INT16,
    DDS_TYPE_UINT16,
    DDS_TYPE_INT32,
    DDS_TYPE_UINT32,
    DDS_TYPE_FLOAT32,
    DDS_TYPE_FLOAT64,
    DDS_TYPE_STRING
};
#define FVM_MEMBER_ACCESS(obj,member) obj->member
#define FVM_PUSH_LIST(obj,list) {if(NULL == (list)) \
                                        {    (list) = (obj);}\
                                        else \
                                        {(obj)->next = (list);(list)=(obj);}}

#define YY_DECL int fvmlex \
               (YYSTYPE * yylval_param , yyscan_t yyscanner , fvcomDdsContext* fdc)
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

datasetDecl* fvmDDSDatasetCreate(fvcomDdsContext *fdc, dataVarDecl*, const char* dname);
arrayDim* fvmDDSArrayDecl(fvcomDdsContext *fdc, char *dimName,uint32_t len);
arrayDim* fvmDDSArrayDecls(fvcomDdsContext *fdc, arrayDim* d1, arrayDim* d2);
char* fvmDDSLiteraldecl(fvcomDdsContext *fdc, char *pstr);
dataVarDecl* fvmDDSVarDecls(fvcomDdsContext *fdc, uint8_t type, const char* name, arrayDim* dims);
dataVarDecl* fvmDDSVarAppend(fvcomDdsContext *fdc, dataVarDecl* d1, dataVarDecl* d2);
void fvmerror(yyscan_t yyscanner, fvcomDdsContext* fdc, char *s,...);
void fvmDDSParse(const char *ddsFilePath);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //FVCOM_FVMDDSCONTEXT_H
