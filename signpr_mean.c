/* Simple Mean Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_mean.h"
#include "signpr_general.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void
simple_mean_param_defaults (parampointer_t parampointer)
{
  parampointer->postlength1 = 1;
  parampointer->prelength1 = 1;
}


void
init_simple_mean_filter (int filterno, parampointer_t parampointer)
{
  parampointer->buffer = init_buffer (parampointer->postlength1,
				      parampointer->prelength1);

  parampointer->filterno = filterno;
}

void
delete_simple_mean_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
}


sample_t
simple_mean_filter (parampointer_t parampointer)
{
  longsample_t sum;
  sample_t sample;
  long i;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  sum.left = 0;
  sum.right = 0;

  for (i = 0; i <= parampointer->postlength1 + parampointer->prelength1;
       i++)
    {
      sample = get_from_buffer (&parampointer->buffer,
				i - parampointer->postlength1);
      sum.left += sample.left;
      sum.right += sample.right;
    }

  sample.left = sum.left / (parampointer->postlength1 +
			    parampointer->prelength1 + 1);
  sample.right = sum.right / (parampointer->postlength1 +
			      parampointer->prelength1 + 1);

  return sample;
}
