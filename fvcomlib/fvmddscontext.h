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
    char *name;
    uint32_t len;
    struct _arrayDim *next;
}arrayDim;

typedef struct _dataVarDecl
{
    uint8_t type;
    char *name;
    arrayDim *dims;
    struct _dataVarDecl *next;

}dataVarDecl;

typedef struct _datasetDecl
{
    char* name;
    dataVarDecl *varDecl;
}datasetDecl;

typedef struct _datasetDef datasetDef;
typedef struct _datasetDef* DatasetDef;

typedef struct _fvcomDdsContext
{
    fvcomStrList* strList;
    arrayDim* dimList;
    dataVarDecl* varList;
    datasetDecl* dataSet;
}fvcomDdsContext;

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

#define FVM_PUSH_LIST(obj,list,member) {if(NULL == ((list)->member)) \
                                        {((list)->member) = (obj);}\
                                        else \
                                        {(obj)->next = ((list)->member);((list)->member)=(obj);}}
#define FVM_POP_LIST(list,member) {if(NULL != ((list)->member)) \
                                        {((list)->member) = ((list)->member)->next;}}

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif
#define YY_DECL int fvmlex (YYSTYPE * yylval_param , yyscan_t yyscanner , fvcomDdsContext* fdc)

datasetDecl* fvmDDSDatasetCreate(fvcomDdsContext *fdc, dataVarDecl*, char* dname);
arrayDim* fvmDDSArrayDecl(fvcomDdsContext *fdc, char *dimName,uint32_t len);
arrayDim* fvmDDSArrayDecls(fvcomDdsContext *fdc, arrayDim* d1, arrayDim* d2);
char* fvmDDSLiteralDecl(fvcomDdsContext *fdc, char *pstr);
dataVarDecl* fvmDDSVarDecl(fvcomDdsContext *fdc, uint8_t type, char* name, arrayDim* dims);
dataVarDecl* fvmDDSVarAppend(fvcomDdsContext *fdc, dataVarDecl* d1, dataVarDecl* d2);
void fvmerror(yyscan_t yyscanner, fvcomDdsContext* fdc, char *s,...);
datasetDef* fvmDDSParse(const char *ddsFilePath);
void freeDatasetDef(datasetDef *def);
dataVarDecl* fvmDDSVarDef(datasetDef* def, const char *name, size_t size);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //FVCOM_FVMDDSCONTEXT_H
