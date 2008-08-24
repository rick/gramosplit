/* RMS Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_rms.h"
#include "signpr_general.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


void
rms_param_defaults (parampointer_t parampointer)
{
  parampointer->postlength1 = 1;
  parampointer->prelength1 = 1;
}


void
init_rms_filter (int filterno, parampointer_t parampointer)
{
  parampointer->buffer = init_buffer (parampointer->postlength1,
				      parampointer->prelength1);

  parampointer->filterno = filterno;
}

void
delete_rms_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
}


sample_t
rms_filter (parampointer_t parampointer)
{
  doublesample_t sum;
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
      sum.left += sample.left * sample.left;
      sum.right += sample.right * sample.right;
    }

  sample.left = sqrt (sum.left / (parampointer->postlength1 +
				  parampointer->prelength1 + 1));
  sample.right = sqrt (sum.right / (parampointer->postlength1 +
				    parampointer->prelength1 + 1));

  return sample;
}
