/* Conditional Median Filter - Better Version
 *                             Using fft and eliminating high frequencies 
 *                             to fill over the ticks.
 *
 * Copyright (C) 1998 J.A. Bezemer
 *               2001 S.J. Tappin
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

#define noDEBUGFILE

#include "signpr_cmf3.h"
#include "signpr_general.h"
#include "signpr_l1fit.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef DEBUGFILE
static FILE *debugf=NULL;
#endif

#ifndef M_PIl
# define M_PIl          3.1415926535897932384626433832795029L  /* pi */ 
#endif

void
cond_median3_param_defaults (parampointer_t parampointer)
{
  parampointer->postlength2 = 4;
  parampointer->prelength2 = 4;
  parampointer->postlength3 = 5;
  parampointer->prelength3 = 5;
  parampointer->postlength4 = 256;
  parampointer->prelength4 = 255;

  parampointer->int1 = 12;
  parampointer->int2 = 9;      /* This could be derived from
				  postlength4 and prelength4, 
				  but it's messy */
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
#error A Highpass version must be defined (signpr_cmf3.c)
#endif

}


#ifdef FOURTH_ORDER
#undef SIGNPR_CMF3_PARAMSCR_HEADERTEXT
#define SIGNPR_CMF3_PARAMSCR_HEADERTEXT "CMF IIF [FOURTH ORDER] - Parameters"
#endif

#ifdef SIXTH_ORDER
#undef SIGNPR_CMF3_PARAMSCR_HEADERTEXT
#define SIGNPR_CMF3_PARAMSCR_HEADERTEXT "CMF IIF [SIXTH ORDER] - Parameters"
#endif


void
init_cond_median3_filter (int filterno, parampointer_t parampointer)
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

  /* Set up the FFT plans here. Since we expect to do lots of FFTs,
     we take the time to MEASURE the best way to do them [SJT] */

  parampointer->planf = rfftw_create_plan(parampointer->postlength4 +
					  parampointer->prelength4 + 1,
					  FFTW_REAL_TO_COMPLEX,
					  FFTW_MEASURE);
  parampointer->planr = rfftw_create_plan(parampointer->postlength4 +
					  parampointer->prelength4 + 1,
					  FFTW_COMPLEX_TO_REAL,
					  FFTW_MEASURE);

#ifdef DEBUGFILE
  debugf = fopen("./gram.txt","w");
#endif


}


void
delete_cond_median3_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
  delete_buffer (&parampointer->buffer2);
  delete_buffer (&parampointer->buffer3);
  delete_buffer (&parampointer->buffer4);

  rfftw_destroy_plan(parampointer->planf);
  rfftw_destroy_plan(parampointer->planr);

#ifdef DEBUGFILE
  fclose(debugf);
#endif
}


sample_t
cond_median3_highpass (long offset, long offset_zero,
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

fillfuncpointer_t cond_median3_highpass_pointer = cond_median3_highpass;

sample_t
cond_median3_rms (long offset, long offset_zero,
		  parampointer_t parampointer)
{
  sample_t sample;
  doublesample_t doublesample;
  doublesample_t sum;
  long i;

  advance_current_pos_custom (&parampointer->buffer2,
			      cond_median3_highpass_pointer,
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

fillfuncpointer_t cond_median3_rms_pointer = cond_median3_rms;

sample_t
cond_median3_gate (long offset, long offset_zero,
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
			      cond_median3_rms_pointer,
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

fillfuncpointer_t cond_median3_gate_pointer = cond_median3_gate;

sample_t
cond_median3_filter (parampointer_t parampointer)
{
  sample_t sample, gate, returnval;
  /* Length of the fft we'll do to get the smoothed interpolate */

  fftw_real list3[parampointer->postlength4 +
		 parampointer->prelength4 + 1];
  fftw_real list4[parampointer->postlength4 +
		 parampointer->prelength4 + 1];

  long fft_l = parampointer->postlength4 + parampointer->prelength4 + 1;
  long i;
  int toleft, toright, nfreq;
  signed short maxval;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  advance_current_pos_custom (&parampointer->buffer4,
			      cond_median3_gate_pointer,
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

      /* only interpolate if there really is a tick */
      if (maxval > parampointer->long2)
	{

#ifdef VIEW_INTERNALS
	  returnval.left = (toright + toleft + 1) * 500;
#else

	  /* Use a HANNING window here for the time being; note that
	     the FFT is centred at the middle of the tick not at the
	     point we are interpolating.  */
	  for (i = 0; i < fft_l; i++)
	    {
	      list3[i] = get_from_buffer(&parampointer->buffer,
					 i - parampointer->prelength4 +
					 (toright - toleft + 1)/2).left * 
		(2.-cos(2.*M_PIl*(double) i/ ((double) fft_l - 1.)))/2.;
	    }
	  rfftw_one(parampointer->planf, list3, list4);
	  nfreq=floor((double) fft_l/(double) (2*(toleft+toright+1)));
	  for (i = 2*nfreq; i <= fft_l - 2*nfreq; i++) list4[i] = 0.;
	  for (i = nfreq; i<2*nfreq && i< fft_l/2; i++) {
	    list4[i] *= (1. - (double) (i-nfreq)/ (double) nfreq);
	    list4[fft_l-i] *= (1. - (double) (i-nfreq)/ (double) nfreq);
	  }
	  rfftw_one(parampointer->planr, list4, list3);
	  returnval.left = (signed short) (list3[parampointer->prelength4 -
						(toright - toleft + 1)/2]/
					   (double) fft_l);

	  /* DON'T ASK !!! -- I have NO idea why I have to MULTIPLY by 
	     the hanning window here. Everything sensible says DIVIDE, but
	     multiply works, divide doesn't */

	  returnval.left *= (2.-cos(2.*M_PIl*(double) ((toright - toleft + 1)/2) /
				    ((double) fft_l - 1.)))/2.;
#ifdef DEBUGFILE
	  fprintf(debugf, "L: %ld %d %ld %ld\n",
		  fft_l, (toleft+toright+1), nfreq,
		  fft_l-nfreq);
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

      /* only interpolate if there really is a tick */
      if (maxval > parampointer->long2)
	{

#ifdef VIEW_INTERNALS
	  returnval.right = (toright + toleft + 1) * 500;
#else
	  /* Use a HANNING window here for the time being; note that
	     the FFT is centred at the middle of the tick not at the
	     point we are interpolating.  */
	  for (i = 0; i < fft_l; i++)
	    {
	      list3[i] = get_from_buffer(&parampointer->buffer,
					 i - parampointer->prelength4 +
					 (toright - toleft + 1)/2).right * 
		(2.-cos(2.*M_PIl*(double) i/ ((double) fft_l - 1.)))/2.;
	    }
	  rfftw_one(parampointer->planf, list3, list4);
	  
	  nfreq=floor((double) fft_l/(double) (2*(toleft+toright+1)));
	  for (i = 2*nfreq; i <= fft_l - 2*nfreq; i++) list4[i] = 0.;
	  for (i = nfreq; i<2*nfreq && i<fft_l/2; i++) {
	    list4[i] *= (1. - (double) (i-nfreq)/ (double) nfreq);
	    list4[fft_l-i] *= (1. - (double) (i-nfreq)/ (double) nfreq);
	  }

	  rfftw_one(parampointer->planr, list4, list3);
	  returnval.right = (signed short) (list3[parampointer->prelength4 -
						 (toright - toleft + 1)/2]/
					    (double) fft_l) ;

	  /* DON'T ASK !!! -- I have NO idea why I have to MULTIPLY by 
	     the hanning window here. Everything sensible says DIVIDE, but
	     multiply works, divide doesn't */

	  returnval.right *= (2.-cos(2.*M_PIl*(double) ((toright - toleft + 1)/2) /
				     ((double) fft_l - 1.)))/2.;
#ifdef DEBUGFILE
	  fprintf(debugf, "R: %ld %d %ld %ld\n",
		  fft_l, (toleft+toright+1), nfreq,
		  fft_l - nfreq);
#endif

#endif
	}
    }

  return returnval;
}
