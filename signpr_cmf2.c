/* Conditional Median Filter - Better Version

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

/* Remove the `dont' to get the gate instead of the normal output - useful
   for verifying properties. */
#define dontVIEW_INTERNALS

/* Choose the highpass filter: */
#define noSECOND_ORDER
#define FOURTH_ORDER
#define noSIXTH_ORDER

/* To use the (slow) fancy interpolation routine, define FANCY_FILL here */
#define noFANCY_FILL


#include "signpr_cmf2.h"
#include "signpr_general.h"
#include "signpr_l1fit.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void
cond_median2_param_defaults (parampointer_t parampointer)
{
  parampointer->postlength2 = 4;
  parampointer->prelength2 = 4;
  parampointer->postlength3 = 5;
  parampointer->prelength3 = 5;
  parampointer->postlength4 = 100;
  parampointer->prelength4 = 100;
  parampointer->int1 = 12;
#if defined (SECOND_ORDER)
  parampointer->long1 = 1000;	/* Threshold to detect precise tick length. */
  parampointer->long2 = 2500;	/* Must be above this to be a tick. */
#elif defined (FOURTH_ORDER)
  parampointer->long1 = 2000;
  parampointer->long2 = 8500;
#elif defined (SIXTH_ORDER)
  parampointer->long1 = 1500;
  parampointer->long2 = 7500;
#else
#error A Highpass version must be defined (signpr_cmf2.c)
#endif

}


#ifdef FOURTH_ORDER
#undef SIGNPR_CMF2_PARAMSCR_HEADERTEXT
#define SIGNPR_CMF2_PARAMSCR_HEADERTEXT "CMF II [FOURTH ORDER] - Parameters"
#endif

#ifdef SIXTH_ORDER
#undef SIGNPR_CMF2_PARAMSCR_HEADERTEXT
#define SIGNPR_CMF2_PARAMSCR_HEADERTEXT "CMF II [SIXTH ORDER] - Parameters"
#endif

void
init_cond_median2_filter (int filterno, parampointer_t parampointer)
{
  long total_post;
  long total_pre;
  long l;

  total_post = parampointer->postlength4 + parampointer->prelength4 + 1 + 4;
  /* +1+4=+5 : for highpass, max. 11th order */

  total_pre = parampointer->postlength4 + parampointer->prelength4 + 1;
  l = parampointer->prelength4 + parampointer->prelength3 *
    parampointer->int1 + parampointer->prelength2 + 5;
  /* + 5 : for highpass, max. 11th order */
  if (l > total_pre)
    total_pre = l;

  parampointer->buffer = init_buffer (total_post, total_pre);
  parampointer->buffer2 = init_buffer (parampointer->postlength2,
				       parampointer->prelength2);
  parampointer->buffer3 = init_buffer (parampointer->postlength3,
			     parampointer->prelength3 * parampointer->int1);
  parampointer->buffer4 = init_buffer (parampointer->postlength4,
				       parampointer->prelength4);

  parampointer->filterno = filterno;
}


void
delete_cond_median2_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
  delete_buffer (&parampointer->buffer2);
  delete_buffer (&parampointer->buffer3);
  delete_buffer (&parampointer->buffer4);
}


sample_t
cond_median2_highpass (long offset, long offset_zero,
		       parampointer_t parampointer)
{
  sample_t sample;
  longsample_t sum;

  offset += offset_zero;	/* middle for highpass filter in
				   'big buffer' */
  sum.left = 0;
  sum.right = 0;

#if defined (SECOND_ORDER)
#define notTEST_DAVE_PLATT
#ifndef TEST_DAVE_PLATT
  /* Original: */
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
#else /* TEST_DAVE_PLATT */
  /* Testing, suggested by Dave Platt. Invert phase of one channel, then
     do tick detection using the sum signal. This is because most ticks
     are out-of-phase signals. I've not really tested this - it might
     require other settings for thresholds etc.
     Note: implemented for second_order only! */
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

#elif defined (FOURTH_ORDER)
  sample = get_from_buffer (&parampointer->buffer, offset - 2);
  sum.left += sample.left;
  sum.right += sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset - 1);
  sum.left -= 4 * (long) sample.left;
  sum.right -= 4 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset);
  sum.left += 6 * (long) sample.left;
  sum.right += 6 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset + 1);
  sum.left -= 4 * (long) sample.left;
  sum.right -= 4 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset + 2);
  sum.left += sample.left;
  sum.right += sample.right;

  sum.left /= 4;
  sum.right /= 4;

#elif defined (SIXTH_ORDER)
  sample = get_from_buffer (&parampointer->buffer, offset - 3);
  sum.left -= sample.left;
  sum.right -= sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset - 2);
  sum.left += 6 * (long) sample.left;
  sum.right += 6 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset - 1);
  sum.left -= 15 * (long) sample.left;
  sum.right -= 15 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset);
  sum.left += 20 * (long) sample.left;
  sum.right += 20 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset + 1);
  sum.left -= 15 * (long) sample.left;
  sum.right -= 15 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset + 2);
  sum.left += 6 * (long) sample.left;
  sum.right += 6 * (long) sample.right;
  sample = get_from_buffer (&parampointer->buffer, offset + 3);
  sum.left -= sample.left;
  sum.right -= sample.right;

  /* Should be /64, but the signal is extremely soft, so divide by less to
     get more quantization levels (more accurate) */
  sum.left /= 4;
  sum.right /= 4;
#endif

  if (sum.left > 32767)
    sample.left = 32767;
  else if (sum.left < -32768)
    sample.left = -32768;
  else
    sample.left = sum.left;


  if (sum.right > 32767)
    sample.right = 32767;
  else if (sum.right < -32768)
    sample.right = -32768;
  else 
    sample.right = sum.right;


  return sample;
}

fillfuncpointer_t cond_median2_highpass_pointer = cond_median2_highpass;

sample_t
cond_median2_rms (long offset, long offset_zero,
		  parampointer_t parampointer)
{
  sample_t sample;
  doublesample_t doublesample;
  doublesample_t sum;
  long i;

  advance_current_pos_custom (&parampointer->buffer2,
			      cond_median2_highpass_pointer,
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

fillfuncpointer_t cond_median2_rms_pointer = cond_median2_rms;

sample_t
cond_median2_gate (long offset, long offset_zero,
		   parampointer_t parampointer)
/* Well, not a `gate' any more - just (w[t]-b[t])/b[t], comparision to
   make the real gate is done later. */
{
  sample_t sample;
  sample_t w_t;
  sample_t b_t;
  sample_t returnval;
  signed short list1[parampointer->postlength3 +
		     parampointer->prelength3 * parampointer->int1 + 1];
  signed short list2[parampointer->postlength3 +
		     parampointer->prelength3 * parampointer->int1 + 1];
  long i, j;

  advance_current_pos_custom (&parampointer->buffer3,
			      cond_median2_rms_pointer,
			      offset + offset_zero,
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


  i = (labs (w_t.left - b_t.left) * 1000)
    /
    b_t.left;
  if (i > 32767)
    i = 32767;
  else if (i < -32768)
    i = -32768;

  returnval.left = i;

  i = (labs (w_t.right - b_t.right) * 1000)
    /
    b_t.right;
  if (i > 32767)
    i = 32767;
  else if (i < -32768)
    i = -32768;
  returnval.right = i;

  return returnval;
}

fillfuncpointer_t cond_median2_gate_pointer = cond_median2_gate;

sample_t
cond_median2_filter (parampointer_t parampointer)
{
  sample_t sample, gate, returnval;
  /* max. length if toleft=postlength4 and toright=prelength4 */
  signed short list3[2 * (parampointer->postlength4 +
			  parampointer->prelength4 + 1) + 1];
#ifdef FANCY_FILL
  signed short list4[2 * (parampointer->postlength4 +
			  parampointer->prelength4 + 1) + 1];
  double a1, b1;
#endif

  long i;
  int toleft, toright;
  signed short maxval;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  advance_current_pos_custom (&parampointer->buffer4,
			      cond_median2_gate_pointer,
			      0,
			      parampointer);

  gate = get_from_buffer (&parampointer->buffer4, 0);

#ifdef VIEW_INTERNALS
  returnval.left = 0;
  returnval.right = 0;
#else
  /* 'Default' value - unchanged if there is no tick */
  returnval = get_from_buffer (&parampointer->buffer, 0);
#endif

  if (gate.left > parampointer->long1)
    {
      maxval = gate.left;

      toleft = -1;
      sample.left = 0;
      do
	{
	  toleft++;
	  if (toleft < parampointer->postlength4)
	    {
	      sample = get_from_buffer (&parampointer->buffer4, -toleft - 1);
	      if (sample.left > maxval)
		/* if so, the `while' will continue anyway, so maxval may
		   be adjusted here already (if necessary) */
		maxval = sample.left;
	    }
	}
      while (toleft < parampointer->postlength4 &&
	     sample.left > parampointer->long1);

      toright = -1;
      sample.left = 0;
      do
	{
	  toright++;
	  if (toright < parampointer->prelength4)
	    {
	      sample = get_from_buffer (&parampointer->buffer4, toright + 1);
	      if (sample.left > maxval)
		/* if so, the `while' will continue anyway, so maxval may
		   be adjusted here already (if necessary) */
		maxval = sample.left;
	    }
	}
      while (toright < parampointer->prelength4 &&
	     sample.left > parampointer->long1);

      /* total length of gate is toleft+toright+1, so a median with
         size 2*(toleft+toright+1)+1 is needed. */

      /* only interpolate if there really is a tick */
      if (maxval > parampointer->long2)
	{

#ifdef VIEW_INTERNALS
	  returnval.left = (toright + toleft + 1) * 500;
#else
#ifdef FANCY_FILL
	  for (i = 0; i <= 2 * (toleft + toright + 1); i++)
	    {
	      list3[i] = get_from_buffer (&parampointer->buffer,
					  i - (toleft + toright + 1)).left;
	      list4[i] = i - (toleft + toright + 1);
	    }
	  l1fit (list4, list3, 2 * (toleft + toright + 1), &a1, &b1);
	  returnval.left = (signed short) rint (a1);
#else
	  for (i = 0; i <= 2 * (toleft + toright + 1); i++)
	    list3[i] = get_from_buffer (&parampointer->buffer,
					i - (toleft + toright + 1)).left;

	  returnval.left = median (list3, 2 * (toleft + toright + 1) + 1);
#endif
#endif
	}
    }

  if (gate.right > parampointer->long1)
    {
      maxval = gate.right;

      toleft = -1;
      sample.right = 0;
      do
	{
	  toleft++;
	  if (toleft < parampointer->postlength4)
	    {
	      sample = get_from_buffer (&parampointer->buffer4, -toleft - 1);
	      if (sample.right > maxval)
		/* if so, the `while' will continue anyway, so maxval may
		   be adjusted here already (if necessary) */
		maxval = sample.right;
	    }
	}
      while (toleft < parampointer->postlength4 &&
	     sample.right > parampointer->long1);

      toright = -1;
      sample.right = 0;
      do
	{
	  toright++;
	  if (toright < parampointer->prelength4)
	    {
	      sample = get_from_buffer (&parampointer->buffer4, toright + 1);
	      if (sample.right > maxval)
		/* if so, the `while' will continue anyway, so maxval may
		   be adjusted here already (if necessary) */
		maxval = sample.right;
	    }
	}
      while (toright < parampointer->prelength4 &&
	     sample.right > parampointer->long1);

      /* total length of gate is toleft+toright+1, so a median with
         size 2*(toleft+toright+1)+1 is needed. */

      /* only interpolate if there really is a tick */
      if (maxval > parampointer->long2)
	{

#ifdef VIEW_INTERNALS
	  returnval.right = (toright + toleft + 1) * 500;
#else
#ifdef FANCY_FILL
	  for (i = 0; i <= 2 * (toleft + toright + 1); i++)
	    {
	      list3[i] = get_from_buffer (&parampointer->buffer,
					  i - (toleft + toright + 1)).right;
	      list4[i] = i - (toleft + toright + 1);
	    }
	  l1fit (list4, list3, 2 * (toleft + toright + 1), &a1, &b1);
	  returnval.right = (signed short) rint (a1);
#else
	  for (i = 0; i <= 2 * (toleft + toright + 1); i++)
	    list3[i] = get_from_buffer (&parampointer->buffer,
					i - (toleft + toright + 1)).right;

	  returnval.right = median (list3, 2 * (toleft + toright + 1) + 1);
#endif
#endif
	}
    }

  return returnval;
}
