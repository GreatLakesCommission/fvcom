%error-verbose
%pure-parser
%name-prefix "fvm"

%lex-param {yyscan_t yyscanner}
%lex-param {fvcomDdsContext* fdc}
%parse-param {yyscan_t yyscanner}
%parse-param {fvcomDdsContext* fdc}

%{
#define YYERROR_VERBOSE
#include "fvmddscontext.h"
struct _datasetDef
{
    datasetDecl* declInternal;
    HASHTAB table;
    int numOfVars;
};
%}

%union{
    datasetDecl *dsDecl;
    dataVarDecl *varDecl;
    arrayDim *dimDecl;
    int intVal;
    char *strPtr;
}

%token DDS_ARRAY
%token DDS_ATTR
%token DDS_BYTE
%token DDS_DATASET
%token DDS_DATA
%token DDS_ERROR
%token DDS_FLOAT32
%token DDS_FLOAT64
%token DDS_GRID
%token DDS_INT16
%token DDS_INT32
%token DDS_SEQUENCE
%token DDS_STRING
%token DDS_STRUCTURE
%token DDS_UINT16
%token DDS_UINT32
%token <intVal> DDS_NUMBER
%token <strPtr> DDS_LITERAL

%type <dsDecl> datasetbody start
%type <varDecl> declarations declaration
%type <dimDecl> array_decls array_decl
%type <intVal> base_type
%type <strPtr> datasetname var_name name

%start start

%%
start:
    DDS_DATASET datasetbody
    {
        fdc->dataSet=$2;
    }
    ;

datasetbody:
    '{' declarations '}' datasetname ';'
    {
        $$ = fvmDDSDatasetCreate(fdc, $2, $4);
    }
    ;

declarations:
    declaration
    {
        $$ = fvmDDSVarAppend(fdc, NULL, $1);
    }
    | declarations declaration
    {
        $$ = fvmDDSVarAppend(fdc, $1, $2);
    }
    ;

declaration:
    base_type var_name array_decls ';'
    {
        $$ = fvmDDSVarDecl(fdc, $1, $2, $3);
    }
    | base_type var_name ';'
    {
        $$ = fvmDDSVarDecl(fdc, $1, $2, NULL);
    }
    ;

base_type:
    DDS_BYTE {$$=DDS_TYPE_BYTE;}
    | DDS_INT16 {$$=DDS_TYPE_INT16;}
    | DDS_UINT16 {$$=DDS_TYPE_UINT16;}
    | DDS_INT32 {$$=DDS_TYPE_INT32;}
    | DDS_UINT32 {$$=DDS_TYPE_UINT32;}
    | DDS_FLOAT32 {$$=DDS_TYPE_FLOAT32;}
    | DDS_FLOAT64 {$$=DDS_TYPE_FLOAT64;}
    | DDS_STRING {$$=DDS_TYPE_STRING;}
    ;

array_decls:
    array_decl
    {
        $$=fvmDDSArrayDecls(fdc,NULL,$1);
    }
    | array_decls array_decl
    {
        $$=fvmDDSArrayDecls(fdc,$1,$2);
    }
    ;

array_decl:
    '[' DDS_NUMBER ']'
    {
        $$=fvmDDSArrayDecl(fdc,NULL,$2);
    }
    | '[' name '=' DDS_NUMBER ']'
    {
        $$=fvmDDSArrayDecl(fdc,$2,$4);
    }
    ;

datasetname:
    var_name
    ;

var_name:
    name
    ;

name:
    DDS_LITERAL {$$=fvmDDSLiteralDecl(fdc,$1);}
    | DDS_ERROR {fvmerror(yyscanner, fdc, "Unknown characters in the dds file");}
    ;
%%

datasetDecl* fvmDDSDatasetCreate(fvcomDdsContext *fdc, dataVarDecl* pvars, char* dname)
{
    datasetDecl *pds= (datasetDecl*)fvm_malloc(sizeof(datasetDecl));
    pds->varDecl = pvars;
    pds->name = dname;
    if(NULL != dname)
        FVM_POP_LIST(fdc,strList)
    fdc->varList = NULL;
    return pds;
}

char* fvmDDSLiteralDecl(fvcomDdsContext *fdc, char *pstr)
{
    fvcomStrList *strlst = (fvcomStrList*)fvm_malloc(sizeof(fvcomStrList));
    strlst->next = NULL;
    strlst->pstr = pstr;
    FVM_PUSH_LIST(strlst, fdc, strList)
    return pstr;
}

dataVarDecl* fvmDDSVarDecl(fvcomDdsContext *fdc, uint8_t type, char* name, arrayDim* dims)
{
    dataVarDecl *pvar=(dataVarDecl*)fvm_malloc(sizeof(dataVarDecl));
    pvar->next = NULL;
    pvar->type = type;
    pvar->name = name;
    FVM_POP_LIST(fdc,strList)
    pvar->dims = dims;
    if(NULL != dims)
        fdc->dimList = NULL;
    return pvar;
}

dataVarDecl* fvmDDSVarAppend(fvcomDdsContext *fdc, dataVarDecl* d1, dataVarDecl* d2)
{
    if(NULL != d1)
    {
        dataVarDecl *itor = d1;
        while(NULL != itor->next)
            itor = itor->next;
        itor->next = d2;
        return d1;
    }
    else
    {
        fdc->varList = d2;
        return d2;
    }
}

arrayDim* fvmDDSArrayDecls(fvcomDdsContext *fdc, arrayDim* d1, arrayDim* d2)
{
    if(NULL != d1)
    {
        arrayDim *itor = d1;
        while(NULL != itor->next)
            itor = itor->next;
        itor->next = d2;
        return d1;
    }
    else
    {
        fdc->dimList = d2;
        return d2;
    }
}

arrayDim* fvmDDSArrayDecl(fvcomDdsContext *fdc, char *dimName,uint32_t len)
{
    arrayDim *pdim=(arrayDim*)fvm_malloc(sizeof(arrayDim));
    pdim->next = NULL;
    pdim->len = len;
    pdim->name = dimName;
    if(NULL != dimName)
        FVM_POP_LIST(fdc,strList)
    pdim->next = NULL;
    return pdim;
}

static void freeDimensions(arrayDim *pdim)
{
    if(pdim)
    {
        arrayDim* dtmp = NULL;
        while(NULL != pdim)
        {
            dtmp = pdim->next;
            free(pdim->name);
            free(pdim);
            pdim = dtmp;
        }
    }
}

void freeVariables(dataVarDecl *pvar)
{
    if(pvar)
    {
        dataVarDecl* vtmp = NULL;
        while(NULL != pvar)
        {
            vtmp = pvar->next;
            free(pvar->name);
            freeDimensions(pvar->dims);
            free(pvar);
            pvar = vtmp;
        }
    }
}

void freeDatasetDef(datasetDef *def)
{
    if(NULL != def)
    {
        hashTabDestroy(def->table);
        if(NULL != def->declInternal)
        {
            if(NULL != def->declInternal->name)
                free(def->declInternal->name);
            freeVariables(def->declInternal->varDecl);
        }
    }
}

dataVarDecl* fvmDDSVarDef(datasetDef* def, const char *name, size_t size)
{
    dataVarDecl* decl = NULL;
    if(NULL != def)
    {
        size_t datasize = 0;
        decl = (dataVarDecl*)hashTabLookup(def->table, name, size, &datasize, KEEP);
    }
    return decl;
}

datasetDef* fvmDDSParse(const char *ddsFilePath)
{
    yyscan_t fvmscanner;
    fvcomDdsContext ctx;
    memset(&ctx, 0, sizeof(fvcomDdsContext));
    datasetDef *def = NULL;
    FILE *f=fopen(ddsFilePath,"r");
    if(NULL != f)
    {
        fvmlex_init(&fvmscanner);
        fvmset_in(f,fvmscanner);
        fvmparse(fvmscanner, &ctx);
        fvmlex_destroy(fvmscanner);
        fclose(f);
        if(NULL != ctx.dataSet)
        {
            def = (datasetDef*)fvm_malloc(sizeof(datasetDef));
            def->declInternal = ctx.dataSet;
            def->table = createHashTab();
            dataVarDecl* pvar = ctx.dataSet->varDecl;
            size_t dlen = 0;
            while(NULL != pvar)
            {
                void* data = hashTabLookup(def->table, pvar->name, strlen(pvar->name), &dlen, KEEP);
                if(NULL != data)
                {
                    free(data);
                    freeDatasetDef(def);
                    printError(0,"DDS Parsing found variables with identical names: %s", pvar->name);
                    return NULL;
                }
                hashTabPut(def->table, pvar->name, strlen(pvar->name), pvar, sizeof(dataVarDecl));
                def->numOfVars++;
                pvar = pvar->next;
            }
        }
    }
    return def;
}

static void freeContextList(fvcomDdsContext *pctx)
{
    if(NULL != pctx)
    {
        fvcomStrList *sitor = pctx->strList;
        fvcomStrList *stmp = NULL;
        while(NULL != sitor)
        {
            free(sitor->pstr);
            stmp = sitor->next;
            free(sitor);
            sitor = stmp;
        }
        if(NULL != pctx->dimList)
            freeDimensions(pctx->dimList);
        if(NULL != pctx->varList)
            freeVariables(pctx->varList);
    }
}

void fvmerror(yyscan_t yyscanner, fvcomDdsContext* fdc, char *s,...)
{
    freeContextList(fdc);
    printError(0,"DDS Parsing Error:%s with text: %s at line: %d",s, fvmget_text(yyscanner), fvmget_lineno(yyscanner));
}
