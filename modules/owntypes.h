#if ! defined(__OWNTYPES_H)
#define __OWNTYPES_H

/*-------------------------------------------------------------------*/
/* OWNTYPES.H      Juha Kivij„rvi                                    */
/*                                                                   */
/* Own type definitions and useful macros                            */
/*                                                                   */
/* version       0.03                                                */
/* last updated  30.8.99                                             */
/*                                                                   */
/*-------------------------------------------------------------------*/


typedef  enum { NO=0, YES=1 }  YESNO;
#if defined(MSVC)
typedef  __int64               llong;
#define  MAXLLONG              0x7fffffffffffffff
#else
typedef  long long int         llong;
#define  MAXLLONG              0x7fffffffffffffffLL
#endif

typedef  unsigned char         BYTE;

#define  roundint(a)           ( (int) ((a) + 0.5) )

#endif /* __OWNTYPES_H */
