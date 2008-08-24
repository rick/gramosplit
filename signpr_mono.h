/* Convert to mono `Filter' - Header

 * Copyright (C) 1999 S.J. Tappin
 * (After signpr_copy.h: J.A. Bezemer 1998)
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_MONO_H
#define HAVE_SIGNPR_MONO_H


#include "signpr_general.h"

void monoize_param_defaults (parampointer_t parampointer);

void init_monoize_filter (int filterno, parampointer_t parampointer);

void delete_monoize_filter (parampointer_t parampointer);

sample_t monoize_filter (parampointer_t parampointer);


#endif /* HAVE_SIGNPR_MONO_H */
