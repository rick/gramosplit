/* Signal Processing - Wave File Functions - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPR_WAV_H
#define HAVE_SIGNPR_WAV_H


#include "signpr_general.h"

/* ----- SOURCE & READING -------------------------------------------------- */

/*extern int sourcefd; */

int openwavsource (char *filename);
/* returns 0: failure (error_window has been displayed), 1: success. */

void closewavsource ();

int seeksamplesource (long samplenumber);
/* returns 0: failure, 1: success */

sample_t readsamplesource ();

/* ----- DESTINATION & WRITING --------------------------------------------- */

/*extern int destfd; */

int openwavdest (char *filename, long bcount);
/* returns 0: failure (error_window has been displayed), 1: success. */

void closewavdest ();

void writesampledest (sample_t sample);


#endif /* HAVE_SIGNPR_WAV_H */
