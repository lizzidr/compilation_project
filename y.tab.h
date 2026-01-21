#ifndef _yy_defines_h_
#define _yy_defines_h_

#define TOK_VOID 257
#define TOK_INT 258
#define TOK_BOOL 259
#define TOK_TRUE 260
#define TOK_FALSE 261
#define TOK_IF 262
#define TOK_DO 263
#define TOK_WHILE 264
#define TOK_FOR 265
#define TOK_PRINT 266
#define TOK_SEMICOL 267
#define TOK_COMMA 268
#define TOK_LPAR 269
#define TOK_RPAR 270
#define TOK_LACC 271
#define TOK_RACC 272
#define TOK_THEN 273
#define TOK_ELSE 274
#define TOK_AFFECT 275
#define TOK_OR 276
#define TOK_AND 277
#define TOK_BOR 278
#define TOK_BXOR 279
#define TOK_BAND 280
#define TOK_EQ 281
#define TOK_NE 282
#define TOK_GT 283
#define TOK_LT 284
#define TOK_GE 285
#define TOK_LE 286
#define TOK_SRL 287
#define TOK_SRA 288
#define TOK_SLL 289
#define TOK_PLUS 290
#define TOK_MINUS 291
#define TOK_MUL 292
#define TOK_DIV 293
#define TOK_MOD 294
#define TOK_UMINUS 295
#define TOK_NOT 296
#define TOK_BNOT 297
#define TOK_INTVAL 298
#define TOK_IDENT 299
#define TOK_STRING 300
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union YYSTYPE {
    int32_t intval;
    char * strval;
    node_t ptr;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE yylval;

#endif /* _yy_defines_h_ */
