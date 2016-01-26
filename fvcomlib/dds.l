%option noyywrap nodefault yylineno case-insensitive reentrant bison-bridge
%{
#include "fvmddscontext.h"
#include "dds.h"
%}

%%
"Byte" {return DDS_BYTE;}
"Int16" {return DDS_INT16;}
"Uint16" {return DDS_UINT16;}
"Int32" {return DDS_INT32;}
"Uint32" {return DDS_UINT32;}
"Float32" {return DDS_FLOAT32;}
"Float64" {return DDS_FLOAT64;}
"String" {return DDS_STRING;}
"Dataset" {return DDS_DATASET;}
[1-9][0-9]* {
                 yylval->intVal=atoi(yyget_text(yyscanner));
                 return DDS_NUMBER;
            }
[a-zA-Z][a-zA-Z0-9]* {
                     yylval->strPtr = strdup(yyget_text(yyscanner));
                     return DDS_LITERAL;
                 }
[ \t] {}
. {
      return DDS_ERROR;
  }
%%
