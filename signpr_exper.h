/* Experiment Filter - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_EXPER_H
#define HAVE_SIGNPR_EXPER_H


#include "signpr_general.h"

void experiment_param_defaults (parampointer_t parampointer);

void init_experiment_filter (int filterno, parampointer_t parampointer);

void delete_experiment_filter (parampointer_t parampointer);

sample_t experiment_filter (parampointer_t parampointer);


#endif /* HAVE_SIGNPR_EXPER_H */
