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
        $$=$2;
    }
    ;

datasetbody:
    '{' declarations '}' datasetname ';'
    {
        $$ = fvmDDSDatasetCreate(fdc, $2, $4);
    }
    ;

declarations:
    {$$=NULL;}
    | declarations declaration
    {
        $$ = fvmDDSVarAppend(fdc, $1, $2);
    }
    ;

declaration:
    base_type var_name array_decls ';'
    {
        $$ = fvmDDSVarDecls(fdc, $1, $2, $3);
    }
    | base_type var_name ';'
    {
        $$ = fvmDDSVarDecls(fdc, $1, $2, NULL);
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
    array_decls array_decl
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
    | error
      {
          fvmerror(fdc, "Illegal dimension declaration");
          YYABORT;
      }
    ;

datasetname:
    var_name {$$=$1;}
    ;

var_name:
    name {$$=$1;}
    ;

name:
    DDS_LITERAL {$$=fvmDDSLiteraldecl(fdc,$1);}
    | DDS_ERROR {$$="Error";}
    ;
%%
datasetDecl* fvmDDSDatasetCreate(fvcomDdsContext *fdc, dataVarDecl* pvars, const char* dname)
{
    datasetDecl *pds= (datasetDecl*)fvm_malloc(sizeof(datasetDecl));
    pds->numOfVars = 0;
    pds->varDecl = pvars;
    while(NULL != pvars)
    {
        pds->numOfVars++;
        pvars = pvars->next;
    }
    return pds;
}
char* fvmDDSLiteraldecl(fvcomDdsContext *fdc, char *pstr)
{
    fvcomStrList *strlst = (fvcomStrList*)fvm_malloc(sizeof(fvcomStrList));
    strlst->next = NULL;
    strlst->pstr = pstr;
    FVM_PUSH_LIST(strlst, FVM_MEMBER_ACCESS(fdc, strList))
    return pstr;
}

dataVarDecl* fvmDDSVarDecls(fvcomDdsContext *fdc, uint8_t type, const char* name, arrayDim* dims)
{
    dataVarDecl *pvar=(dataVarDecl*)fvm_malloc(sizeof(dataVarDecl));
    pvar->next = NULL;
    FVM_PUSH_LIST(pvar, FVM_MEMBER_ACCESS(fdc, varList))
    pvar->type = type;
    pvar->name = name;
    pvar->dims = dims;
    return pvar;
}

dataVarDecl* fvmDDSVarAppend(fvcomDdsContext *fdc, dataVarDecl* d1, dataVarDecl* d2)
{
    if(NULL != d1)
    {
        d1->next = d2;
        return d1;
    }
    else
        return d2;
}

arrayDim* fvmDDSArrayDecls(fvcomDdsContext *fdc, arrayDim* d1, arrayDim* d2)
{
    if(NULL != d1)
    {
        d1->next = d2;
        return d1;
    }
    else
        return d2;
}

arrayDim* fvmDDSArrayDecl(fvcomDdsContext *fdc, char *dimName,uint32_t len)
{
    arrayDim *pdim=(arrayDim*)fvm_malloc(sizeof(arrayDim));
    pdim->next = NULL;
    FVM_PUSH_LIST(pdim, FVM_MEMBER_ACCESS(fdc, dimList))
    pdim->len = len;
    pdim->name = dimName;
    pdim->next = NULL;
    return pdim;
}

void fvmDDSParse(const char *ddsFilePath)
{
    yyscan_t fvmscanner;
    fvcomDdsContext ctx;
    FILE *f=fopen(ddsFilePath,"r");
    fvmlex_init(&fvmscanner);
    fvmset_in(f,fvmscanner);
    fvmparse(fvmscanner, &ctx);
    mklex_destroy(fvmscanner);
    fclose(f);
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
        arrayDim* ditor = pctx->dimList;
        arrayDim* dtmp = NULL;
        while(NULL != ditor)
        {
            dtmp = ditor->next;
            free(ditor);
            ditor = dtmp;
        }
        dataVarDecl* vitor = pctx->varList;
        dataVarDecl* vtmp = NULL;
        while(NULL != vitor)
        {
            vtmp = vitor->next;
            free(vitor);
            vitor = vtmp;
        }
    }
}

void fvmerror(yyscan_t yyscanner, fvcomDdsContext* fdc, char *s,...)
{
    freeContextList(fdc);
    printError(0,"DDS Parsing Error:%s\n",s);
}
