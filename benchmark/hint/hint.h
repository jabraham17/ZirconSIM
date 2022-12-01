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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

/******************************************************************************/
/*   Vendor Specific defines and includes                                     */
/******************************************************************************/

#define NTIME 1.0

/******************************************************************************/
/*      Macro Declarations                                                    */
/******************************************************************************/
#define MIN(x, y) (((x) > (y)) ? (y) : (x))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/******************************************************************************/
/*      Adjustable Defines                                                    */
/*      These may be adjusted according to the HINT rules.                    */
/******************************************************************************/
#define ADVANCE                                                                \
    1.1 /* 1.2589 */ /* Multiplier. We use roughly 1 decibel step size. */
                     /* Closer to 1.0 takes longer to run, but might    */
                     /* produce slightly higher net QUIPS.              */
#define NCHUNK 4     /* Number of chunks for scatter decomposition      */
                     /* Larger numbers increase time to first result    */
                     /* (latency) but sample domain more evenly.        */
#define NSAMP 200    /* Maximum number of QUIPS measurements            */
                     /* Increase if needed, e.g. if ADVANCE is smaller  */
#define NTRIAL 15    /* Normal number of times to run a trial           */
                     /* Increase if computer is prone to interruption   */
#define PATIENCE 13  /* Number of times to rerun a bogus trial          */
#define RUNTM 1.0    /* Target time, seconds. Reduce for high-res timer.*/
                     /* Should be much larger than timer resolution.    */
#define STOPRT 0.1   /* Ratio of current to peak QUIPS to stop at       */
                     /* Smaller numbers will beat on virtual memory.    */
#define STOPTM 100   /* Longest time acceptable, seconds.  Most systems */
                     /* run out of decent-speed memory well before this */
#define MXPROC                                                                 \
    32 /* Maximum number of processors to use in shared   */ /* memory         \
                                                                configuration. \
                                                                Adjust as      \
                                                                necessary. */

/******************************************************************************/
/*      Non - Adjustable Defines                                              */
/******************************************************************************/
#define DSREFS 90
#define ISREFS 10
#define HI 0           /* Index of high values                            */
#define LO 1           /* Index of low values                             */
#define TRUE (1 == 1)  /* Self explanatory                                */
#define FALSE (1 == 0) /* Self explanatory                                */
#define MSIZE int      /* Integral type for allocating memory             */
#define NMIN 2         /* Minimum number of subintervals                  */
#define ANSWER 0.38629436111989061883446424291653136151000 /*  roughly!!! */
#define MAXMESSAGE 512

/******************************************************************************/
/*      Type Defines                                                          */
/******************************************************************************/
#include "typedefs.h" /* Sets data type DSIZE and index type ISIZE       */

/******************************************************************************/
/*      Type Declarations                                                     */
/******************************************************************************/
typedef struct {
    DSIZE ahi, /* Upper bound on rectangle areas                  */
        alo,   /* Lower bound on rectangle areas                  */
        dx,    /* Interval widths                                 */
        flh,   /* Function values of left coordinates, high       */
        fll,   /* Function values of left coordinates, low        */
        frh,   /* Function values of right coordinates, high      */
        frl,   /* Function values of right coordinates, low       */
        xl,    /* Left x-coordinates of subintervals              */
        xr;    /* Right x-coordinates of subintervals             */
} RECT;

typedef struct {
    double t, /* Time for a given run                            */
        qp,   /* QUIPS for a given run                           */
        delq; /* Change in Quality                               */
    ISIZE n;  /* Subintervals for a given run                    */
    int laps;
} Speed;

typedef ISIZE ERROR;

/******************************************************************************/
/*      Prototypes                                                            */
/******************************************************************************/
double When();

double
Run(int laps,
    DSIZE* gamut,
    DSIZE scx,
    DSIZE scy,
    DSIZE dmax,
    ISIZE memry,
    ERROR* eflag);
DSIZE
Hint(
    DSIZE* scx,
    DSIZE* scy,
    DSIZE* dmax,
    ISIZE* mcnt,
    RECT* rect,
    DSIZE* errs,
    ISIZE* ixes,
    ERROR* eflag);
