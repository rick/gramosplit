/* Signal Processing Main - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SIGNPROC_MAIN_H
#define HAVE_SIGNPROC_MAIN_H

#include "scrollmenu.h"

typedef struct
  {
    long begin;
    long end;
  }
beginendsample_t;

int load_track_times (char *filename, beginendsample_t * tracktimes,
		      int *number_of_tracks);

void signproc_main (char *startdir, char *infilename, char *outfilename);


#endif /* HAVE_SIGNPROC_MAIN_H */
