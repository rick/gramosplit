/* Simple Median Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_median.h"
#include "signpr_general.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void
simple_median_param_defaults (parampointer_t parampointer)
{
  parampointer->postlength1 = 1;
  parampointer->prelength1 = 1;
}

void
init_simple_median_filter (int filterno, parampointer_t parampointer)
{
  parampointer->buffer = init_buffer (parampointer->postlength1,
				      parampointer->prelength1);

  parampointer->filterno = filterno;

  parampointer->sslist1 = (signed short *) malloc
    ((parampointer->postlength1 +
      parampointer->prelength1 + 1)
     * sizeof (signed short));

  parampointer->sslist2 = (signed short *) malloc
    ((parampointer->postlength1 +
      parampointer->prelength1 + 1)
     * sizeof (signed short));
}

void
delete_simple_median_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
  free (parampointer->sslist1);
  free (parampointer->sslist2);
}


sample_t
simple_median_filter (parampointer_t parampointer)
{
  sample_t sample;
  long i;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  for (i = 0; i <= parampointer->postlength1 + parampointer->prelength1;
       i++)
    {
      sample = get_from_buffer (&parampointer->buffer,
				i - parampointer->postlength1);
      parampointer->sslist1[i] = sample.left;
      parampointer->sslist2[i] = sample.right;
    }

  sample.left = median (parampointer->sslist1,
			parampointer->postlength1 +
			parampointer->prelength1 + 1);
  sample.right = median (parampointer->sslist2,
			 parampointer->postlength1 +
			 parampointer->prelength1 + 1);

  return sample;
}
