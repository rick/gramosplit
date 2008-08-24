/* Double Median Filter - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_DOUBMED_H
#define HAVE_SIGNPR_DOUBMED_H


#include "signpr_general.h"

#define SIGNPR_DOUBMED_PARAMSCR_HEADERTEXT "Double Median Filter - Parameters"

void double_median_param_defaults (parampointer_t parampointer);

void init_double_median_filter (int filterno, parampointer_t parampointer);

void delete_double_median_filter (parampointer_t parampointer);

sample_t double_median_filter (parampointer_t parampointer);


#endif /* HAVE_SIGNPR_DOUBMED_H */
