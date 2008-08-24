/* Conditional Median Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

/* Remove the `dont' to get b[t].left on left channel and g[t].left on
   right channel - useful for verifying properties.

   Note that
   b[t] is z[t] if RMSlength=RMFlength=1                                    
   and 
   b[t] is w[t] if RMFlength=1                                              
   (See also Signproc.txt)
 */
#define dontVIEW_INTERNALS


#include "signpr_cmf.h"
#include "signpr_general.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Macros I used first:

   COND_MEDIAN_MF_POSTLENGTH   ==  parampointer->postlength1
   COND_MEDIAN_MF_PRELENGTH    ==  parampointer->prelength1

   COND_MEDIAN_RMS_POSTLENGTH  ==  parampointer->postlength2
   COND_MEDIAN_RMS_PRELENGTH   ==  parampointer->prelength2

   COND_MEDIAN_RMF_POSTLENGTH  ==  parampointer->postlength3
   COND_MEDIAN_RMF_PRELENGTH   ==  parampointer->prelength3

   COND_MEDIAN_RMF_DECIMATE    ==  parampointer->int1 

   COND_MEDIAN_THRESHOLD       ==  parampointer->long1 
 */


void
cond_median_param_defaults (parampointer_t parampointer)
{
  /* Best tick-reduction: 21 - 9 - 11 - 5 - 2500 */

  parampointer->postlength1 = 10;
  parampointer->prelength1 = 10;
  parampointer->postlength2 = 4;
  parampointer->prelength2 = 4;
  parampointer->postlength3 = 5;
  parampointer->prelength3 = 5;
  parampointer->int1 = 5;	/* actually, this should really be 12 */
  parampointer->long1 = 2500;

  /* If you experience badly affected sound, try 15 - 11 - 9 - 4 - 2500 */
}

void
init_cond_median_filter (int filterno, parampointer_t parampointer)
{
  long total_post;
  long total_pre;

  total_post = parampointer->postlength1;
  if (parampointer->postlength2 > total_post)
    total_post = parampointer->postlength2;

  total_pre = parampointer->prelength1;
  if (parampointer->prelength2 +
      parampointer->prelength3 * parampointer->int1 + 1 > total_pre)
    total_pre = parampointer->prelength2 +
      parampointer->prelength3 * parampointer->int1 + 1;

  parampointer->buffer = init_buffer (total_post, total_pre);
  parampointer->buffer2 = init_buffer (parampointer->postlength2,
				       parampointer->prelength2);
  parampointer->buffer3 = init_buffer (parampointer->postlength3,
			     parampointer->prelength3 * parampointer->int1);

  parampointer->filterno = filterno;
}


void
delete_cond_median_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
  delete_buffer (&parampointer->buffer2);
  delete_buffer (&parampointer->buffer3);
}


sample_t
cond_median_highpass (long offset, long offset_zero,
		      parampointer_t parampointer)
{
  sample_t sample;
  longsample_t sum;

  offset += offset_zero;	/* middle for highpass filter in
				   'big buffer' */
  sum.left = 0;
  sum.right = 0;

#define notTEST_DAVE_PLATT
#ifndef TEST_DAVE_PLATT
  sample = get_from_buffer (&parampointer->buffer, offset - 1);
  sum.left += sample.left;
  sum.right += sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset);
  sum.left -= 2 * (long) sample.left;
  sum.right -= 2 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset + 1);
  sum.left += sample.left;
  sum.right += sample.right;

  sum.left /= 4;
  sum.right /= 4;
#else
  /* Testing, suggested by Dave Platt. Invert phase of one channel, then
     do tick detection using the sum signal. This is because most ticks
     are out-of-phase signals. I've not really tested this - it might
     require other settings for thresholds etc. */
  sample = get_from_buffer (&parampointer->buffer, offset - 1);
  sum.left += sample.left;
  sum.left -= sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset);
  sum.left -= 2 * (long) sample.left;
  sum.left += 2 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset + 1);
  sum.left += sample.left;
  sum.left -= sample.right;

  /* just in case L/R: 32000/-32000 -32000/32000 32000/-32000 : */
  sum.left /= 8;
  sum.right = sum.left;
#endif /* TEST_DAVE_PLATT */

  sample.left = sum.left;
  sample.right = sum.right;

  return sample;
}

fillfuncpointer_t cond_median_highpass_pointer = cond_median_highpass;

sample_t
cond_median_rms (long offset, long offset_zero,
		 parampointer_t parampointer)
{
  sample_t sample;
  doublesample_t doublesample;
  doublesample_t sum;
  long i;

  advance_current_pos_custom (&parampointer->buffer2,
			      cond_median_highpass_pointer,
			      offset + offset_zero,
			      parampointer);

  sum.left = 0;
  sum.right = 0;

  for (i = -parampointer->postlength2; i <= parampointer->prelength2;
       i++)
    {
      sample = get_from_buffer (&parampointer->buffer2, i);
      doublesample.left = sample.left;
      doublesample.right = sample.right;
      sum.left += doublesample.left * doublesample.left;
      sum.right += doublesample.right * doublesample.right;
    }

  sum.left /= (parampointer->postlength2 +
	       parampointer->prelength2 + 1);
  sum.right /= (parampointer->postlength2 +
		parampointer->prelength2 + 1);

  sample.left = sqrt (sum.left + 1);
  sample.right = sqrt (sum.right + 1);

  return sample;
}

fillfuncpointer_t cond_median_rms_pointer = cond_median_rms;

sample_t
cond_median_filter (parampointer_t parampointer)
{
  sample_t sample;
  sample_t w_t;
  sample_t b_t;
  sample_t returnval;
  signed short list1[parampointer->postlength3 +
		     parampointer->prelength3 * parampointer->int1 + 1];
  signed short list2[parampointer->postlength3 +
		     parampointer->prelength3 * parampointer->int1 + 1];
  signed short list3[parampointer->postlength1 + parampointer->prelength1 + 1];
  long i, j;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  advance_current_pos_custom (&parampointer->buffer3,
			      cond_median_rms_pointer,
			      0,
			      parampointer);

  w_t = get_from_buffer (&parampointer->buffer3, 0);

  /* The RMF Filter */

  for (i = 0; i < parampointer->postlength3; i++)
    {
      sample = get_from_buffer (&parampointer->buffer3,
				i - parampointer->postlength3);
      list1[i] = sample.left;
      list2[i] = sample.right;
    }

  j = i;

  for (; i <= parampointer->postlength3 +
       parampointer->prelength3 * parampointer->int1;
       i += parampointer->int1)
    {
      sample = get_from_buffer (&parampointer->buffer3,
				i - parampointer->postlength3);
      list1[j] = sample.left;
      list2[j] = sample.right;
      j++;
    }

  b_t.left = median (list1, j);
  b_t.right = median (list2, j);

  put_in_buffer (&parampointer->buffer3, 0, b_t);

#ifdef VIEW_INTERNALS

  returnval.left = b_t.left * 10;

  if (
       (labs (w_t.left - b_t.left) * 1000)
       /
       b_t.left > parampointer->long1)
    returnval.right = 2000;
  else
    returnval.right = 0;

#else /* not VIEW_INTERNALS */

  returnval = get_from_buffer (&parampointer->buffer, 0);

  /* Median Filters - if necessary */

  if (
       (labs (w_t.left - b_t.left) * 1000)
       /
       b_t.left > parampointer->long1)
    {
      for (i = 0; i <= parampointer->postlength1 +
	   parampointer->prelength1; i++)
	list3[i] = get_from_buffer (&parampointer->buffer,
				    i - parampointer->postlength1).left;

      returnval.left = median (list3, parampointer->postlength1 +
			       parampointer->prelength1 + 1);
    }

  if (
       (labs (w_t.right - b_t.right) * 1000)
       /
       b_t.right > parampointer->long1)
    {
      for (i = 0; i <= parampointer->postlength1 +
	   parampointer->prelength1; i++)
	list3[i] = get_from_buffer (&parampointer->buffer,
				    i - parampointer->postlength1).right;

      returnval.right = median (list3, parampointer->postlength1 +
				parampointer->prelength1 + 1);
    }

#endif /* VIEW_INTERNALS */

  return returnval;
}
