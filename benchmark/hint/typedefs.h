/******************************************************************************/
/* "HINT" -- Hierarchical INTegration.                                        */
/* Copyright (C) 1994 by Iowa State University Research Foundation, Inc.      */
/*                                                                            */
/* Files needed for use:                                                      */
/*     * hint.c             ---- Driver source                                */
/*     * hkernel.c          ---- Kernel source                                */
/*     * hint.h             ---- General include file                         */
/*     * typedefs.h         ---- Include file for DSIZE and ISIZE             */
/*     * README             ---- These are the rules. Follow them!!!          */
/******************************************************************************/
/******************************************************************************/
/*      Defines                                                               */
/******************************************************************************/
#define NOERROR    0      /* Self explanatory                                 */
#define DISCRET    1      /* Error condition for out of discretization error  */
#define NOMEM      2      /* Error for out of memory                          */
/******************************************************************************/
/*      Type Defines                                                          */
/******************************************************************************/

#define QMSIZE long long

#if defined(LDOUBLE)
#define DSIZE long double
#define DSIZESTR "longdouble"
#define DSIZEFMT "%lf"

#elif defined(DOUBLE)
#define DSIZE double
#define DSIZEF REAL*8
#define MPI_DSIZE MPI_DOUBLE
#define DSIZESTR "double"
#define DSIZEFMT "%f"

#elif defined(FLOAT)
#define DSIZE float
#define DSIZEF REAL*4
#define MPI_DSIZE MPI_FLOAT
#define DSIZESTR "float"
#define DSIZEFMT "%f"

#elif defined(LONGLONG)
#define DSIZE long long
#define DSIZEF INTEGER*8
#define DSIZESTR "longlong"
#define DSIZEFMT "%lld"

#elif defined(LONG)
#define DSIZE long
#define DSIZEF INTEGER*4
#define DSIZESTR "long"
#define DSIZEFMT "%ld"

#elif defined(INT)
#define DSIZE int
#define DSIZEF INTEGER
#define MPI_DSIZE MPI_INT
#define DSIZESTR "int"
#define DSIZEFMT "%d"

#elif defined(SHORT)
#define DSIZE short
#define DSIZEF INTEGER*2
#define DSIZESTR "short"
#define DSIZEFMT "%hd"

#elif defined(CHAR)
#define DSIZE char
#define DSIZEF INTEGER*1
#define DSIZESTR "char"
#define DSIZEFMT "%hhd"

#else
#error "No Size defined"
#endif



#if defined(ILONGLONG)
#define ISIZE long long
#define ISIZEF INTEGER*8

#elif defined(ILONG)
#define ISIZE long
#define ISIZEF INTEGER*4

#elif defined(IINT)
#define ISIZE int
#define ISIZEF INTEGER

#else
#error "No ISize defined"
#endif
