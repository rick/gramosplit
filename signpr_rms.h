/* RMS Filter - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_RMS_H
#define HAVE_SIGNPR_RMS_H


#include "signpr_general.h"

#define SIGNPR_RMS_PARAMSCR_HEADERTEXT "RMS Filter - Parameters"

void rms_param_defaults (parampointer_t parampointer);

void init_rms_filter (int filterno, parampointer_t parampointer);

void delete_rms_filter (parampointer_t parampointer);

sample_t rms_filter (parampointer_t parampointer);


#endif /* HAVE_SIGNPR_RMS_H */
