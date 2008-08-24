/* Seconds to Hour:Minute:Seconds and back

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include <string.h>
#include <stdio.h>
#include <math.h>


void
secs2hms (long seconds, char *outstring)	/*  3610  ->  1:00:10  */
{
  sprintf (outstring, "%ld:%02ld:%02ld", seconds / 3600,
	   (seconds / 60) % 60,
	   seconds % 60);
}

void
fsec2hmsf (double seconds, char *outstring)
{
  double intpart = 0;
  double floatpart;
  long i;
  char helpstring[250];

  floatpart = modf (seconds, &intpart);
  i = intpart;
  secs2hms (i, outstring);

  sprintf (helpstring, "%.3f", floatpart);
  strcat (outstring, helpstring + 1);
}

int
hmsf2fsec (char *instring, double *seconds)
/* returns 0: failure, 1: success; instring will be modified. */
{
  char *charptr;
  int i = 0;

  if (!strlen (instring))
    return 0;			/* empty string */

  *seconds = 0;

  /* seconds */

  charptr = strrchr (instring, ':');
  if (charptr == NULL)
    charptr = instring;
  else
    {
      *charptr = '\0';		/* put a '\0' in place of the ':' */
      charptr++;
    }
  /* charptr is now start of seconds */
  if (!sscanf (charptr, "%lf", seconds))
    return 0;

  if (charptr == instring)
    return 1;

  /* minutes */

  charptr = strrchr (instring, ':');
  if (charptr == NULL)
    charptr = instring;
  else
    {
      *charptr = '\0';		/* put a '\0' in place of the ':' */
      charptr++;
    }
  /* charptr is now start of minutes */
  if (!sscanf (charptr, "%d", &i))
    return 0;
  *seconds += i * 60;

  if (charptr == instring)
    return 1;

  /* hours */

  charptr = strrchr (instring, ':');
  if (charptr == NULL)
    charptr = instring;
  else
    {
      *charptr = '\0';		/* put a '\0' in place of the ':' */
      charptr++;
    }
  /* charptr is now start of hours */
  if (!sscanf (charptr, "%d", &i))
    return 0;
  *seconds += i * 3600;

  if (charptr == instring)
    return 1;			/* nothing before hours: OK */

  return 0;			/* something before hours. days?? */
}
