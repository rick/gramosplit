/* Seconds to Hour:Minute:Seconds and back - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SECSHMS_H
#define HAVE_SECSHMS_H


void secs2hms (long seconds, char *outstring);	/*  3610  ->  1:00:10  */

void fsec2hmsf (double seconds, char *outstring);	/* 10.4 -> 0:00:10.400 */

int hmsf2fsec (char *instring, double *seconds);
/* returns 0: failure, 1: success; instring will be modified. */


#endif /* HAVE_SECSHMS_H */
