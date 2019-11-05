#if ! defined(__PARAMETR_H)
#define __PARAMETR_H

#if ! (defined(_WIN32) && ! defined(BATCH))

#include <stdio.h>

#endif

/* -------------------- Name of the fact file --------------------------- */

#if ! defined(FACTFILE)
#define FACTFILE  "parametr.fac"
#endif

/* ------------------------- Option symbol ------------------------------ */


#define     OPTION_SYMBOL      '-'


/* ------------------ Parameter info data structure --------------------- */

#if ! (defined(_WIN32) && ! defined(BATCH))

typedef enum { OUTFILE, INFILE, INOUTFILE, DONTCARE }  EXTENDEDFILEMODE;


typedef struct
        {
        char*             FileName;
        char*             Extension;
        int               Priority;
        EXTENDEDFILEMODE  FileType;
        } ParameterInfo;

#endif

/* --------------- Parameter values identifier definition --------------- */


#define Fact( id, doc, a1, a2, a3, a4, a5, a6, a7, a8, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, pr ) \
   enum __##id { e1, e2, e3, e4, e5, e6, e7, e8, e9, e10 };

#include FACTFILE

#undef Fact


/* ------------------ Parameter identifier definition ------------------- */


#define Fact( id, doc, a1, a2, a3, a4, a5, a6, a7, a8, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, pr )    \
        _##id,

enum ParameterIdentifiers
     {
       Pseudo_First=0,
       #include FACTFILE
       Pseudo_Last
     };

#undef Fact

#define First_Parameter     (Pseudo_First + 1)
#define Last_Parameter      (Pseudo_Last - 1)


/* -------------------------  PARAMETER FUNCTIONS  ----------------------- */
/*                                 via labels                              */
/*                                (USE THESE!)                             */

#define Document(x)        Document_(     _##x )
#define Key(x)             Key_(          _##x )
#define Number(x)          Number_(       _##x )
#define Type(x)            Type_(         _##x )
#define Value(x)           Value_(        _##x )
#define SetValue(x,v)      SetValue_(     _##x, v )
#define Max(x)             Max_(          _##x )
#define Min(x)             Min_(          _##x )
#define Default(x)         Default_(      _##x )
#define SaveToFile(x)      SaveToFile_(   _##x )
#define NameOfValue(x,v)   NameOfValue_(  _##x, v )

/* -------------------------  PARAMETER FUNCTIONS  ----------------------- */
/*                               via iterators                             */
/*                             (DON'T USE THESE!)                          */

extern char*  Document_(int x);
extern char   Key_(int x);
extern int    Number_(int x);
extern int    Type_(int x);
extern int    Value_(int x);
extern void   SetValue_(int x, int v);
extern int    Max_(int x);
extern int    Min_(int x);
extern int    Default_(int x);
extern int    SaveToFile_(int x);
extern char*  NameOfValue_(int x, int v);

/* ---------------------------  GLOBAL ROUTINES  ------------------------- */

#if ! (defined(_WIN32) && ! defined(BATCH))

void ReadOptions(FILE* f);
void WriteOptions(FILE* f);
void PrintOptions(void);
void PrintSelectedOptions(void);
int  ParseOptions(int ArgC, char* ArgV[]);
void ParseNames(int ArgC, char* ArgV[], int FileNameCount,
                int FileNamesGiven, ParameterInfo PInfo[]);
int  ParseParameters(int ArgC, char* ArgV[], int FileNameCount,
                     ParameterInfo PInfo[]);

#endif /*! (defined(_WIN32) && ! defined(BATCH)) */


#endif /* __PARAMETR_H */

