/* Simple Median Filter - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_CMF_H
#define HAVE_SIGNPR_CMF_H


#include "signpr_general.h"

#define SIGNPR_CMF_PARAMSCR_HEADERTEXT "Conditional Median Filter - Parameters"

void cond_median_param_defaults (parampointer_t parampointer);

void init_cond_median_filter (int filterno, parampointer_t parampointer);

void delete_cond_median_filter (parampointer_t parampointer);

sample_t cond_median_filter (parampointer_t parampointer);


#endif /* HAVE_SIGNPR_CMF_H */
