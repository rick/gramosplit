/* Copy Only `Filter' - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_COPY_H
#define HAVE_SIGNPR_COPY_H


#include "signpr_general.h"

void copyonly_param_defaults (parampointer_t parampointer);

void init_copyonly_filter (int filterno, parampointer_t parampointer);

void delete_copyonly_filter (parampointer_t parampointer);

sample_t copyonly_filter (parampointer_t parampointer);


#endif /* HAVE_SIGNPR_COPY_H */
