/* Simple Mean Filter - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_MEAN_H
#define HAVE_SIGNPR_MEAN_H


#include "signpr_general.h"

#define SIGNPR_MEAN_PARAMSCR_HEADERTEXT "Simple Mean Filter - Parameters"

void simple_mean_param_defaults (parampointer_t parampointer);

void simple_mean_param_screen (parampointer_t parampointer);

void init_simple_mean_filter (int filterno, parampointer_t parampointer);

void delete_simple_mean_filter (parampointer_t parampointer);

sample_t simple_mean_filter (parampointer_t parampointer);


#endif /* HAVE_SIGNPR_MEAN_H */
