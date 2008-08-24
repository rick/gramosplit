/* Double Median Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_doubmed.h"
#include "signpr_general.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void
double_median_param_defaults (parampointer_t parampointer)
{
  parampointer->postlength1 = 2;
  parampointer->prelength1 = 2;
  parampointer->postlength2 = 2;
  parampointer->prelength2 = 2;
}


void
init_double_median_filter (int filterno, parampointer_t parampointer)
{
  long total_post;

  total_post = parampointer->postlength2;
  if (parampointer->postlength1 > total_post)
    total_post = parampointer->postlength1;

  parampointer->buffer = init_buffer (total_post,
				      parampointer->prelength1 +
				      parampointer->prelength2);
  parampointer->buffer2 = init_buffer (parampointer->postlength2,
				       parampointer->prelength2);

  parampointer->filterno = filterno;
}

void
delete_double_median_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
  delete_buffer (&parampointer->buffer2);
}

sample_t
double_median_1 (long offset, long offset_zero,
		 parampointer_t parampointer)
{
  sample_t sample;
  signed short list1[parampointer->postlength1 + parampointer->prelength1 + 1];
  signed short list2[parampointer->postlength1 + parampointer->prelength1 + 1];
  long i;

  for (i = 0; i <= parampointer->postlength1 +
       parampointer->prelength1; i++)
    {
      sample = get_from_buffer (&parampointer->buffer,
				i - parampointer->postlength1 +
				offset + offset_zero);
      list1[i] = sample.left;
      list2[i] = sample.right;
    }

  sample.left = median (list1, parampointer->postlength1 +
			parampointer->prelength1 + 1);
  sample.right = median (list2, parampointer->postlength1 +
			 parampointer->prelength1 + 1);
  return sample;
}

fillfuncpointer_t double_median_1_pointer = double_median_1;

sample_t
double_median_filter (parampointer_t parampointer)
{
  sample_t sample;
  sample_t sample2;
  sample_t returnval;
  signed short list1[parampointer->postlength2 + parampointer->prelength2 + 1];
  signed short list2[parampointer->postlength2 + parampointer->prelength2 + 1];
  long i, j;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  advance_current_pos_custom (&parampointer->buffer2,
			      double_median_1_pointer,
			      0,
			      parampointer);


  for (i = 0; i <= parampointer->postlength2 +
       parampointer->prelength2; i++)
    {
      sample = get_from_buffer (&parampointer->buffer,
				i - parampointer->postlength2);
      sample2 = get_from_buffer (&parampointer->buffer2,
				 i - parampointer->postlength2);

      j = sample.left - sample2.left;
      j /= 2;
      list1[i] = j;

      j = sample.right - sample2.right;
      j /= 2;
      list2[i] = j;
    }

  sample2 = get_from_buffer (&parampointer->buffer2, 0);

  returnval.left = median (list1, parampointer->postlength2 +
			   parampointer->prelength2 + 1) * 2
    + sample2.left;

  returnval.right = median (list2, parampointer->postlength2 +
			    parampointer->prelength2 + 1) * 2
    + sample2.right;

  return returnval;
}
