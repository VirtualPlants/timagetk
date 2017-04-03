/*************************************************************************
 * chamfercoefficients.c - computation of chamfer distance coefficients
 *
 * $Id$
 *
 * Copyright (c) INRIA 2016
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 *
 * CREATION DATE:
 * Mer 16 mar 2016 16:21:18 CET
 *
 * ADDITIONS, CHANGES
 *
 * -
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <time.h>
#include <sys/types.h>

#ifndef WIN32
#include <sys/times.h>
#endif

#include <chamfercoefficients.h>




#include "chamfercoefficients-common.c"

#include "chamfercoefficients-build.c"

#include "chamfercoefficients-weights.c"

#include "chamfercoefficients-results.c"
