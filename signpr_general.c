/* General functions for signal processing

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */


#include "signpr_general.h"
#include "signpr_median.h"
#include "signpr_wav.h"
#include "signpr_cmf.h"
#include "signpr_cmf2.h"
#include "signpr_cmf3.h"
#include "signpr_mean.h"
#include "signpr_doubmed.h"
#include "signpr_rms.h"
#include "signpr_copy.h"
#include "signpr_exper.h"
#include "signpr_mono.h"

#include <stdio.h>
#include <stdlib.h>

void
write_sample_to_screen (sample_t data)
{
  printf ("Output, left & right: %d %d\n", data.left, data.right);
}

parampointer_t parampointerarray[MAX_FILTERS];

int filter_type[MAX_FILTERS];

int number_of_filters;

/* ----- BUFFER ------------------------------------------------------------ */

buffer_t
init_buffer (long post_length, long pre_length)
{
  buffer_t newbuffer;
  long bufferlength;

  bufferlength = pre_length + post_length + 1;

  newbuffer.array = (sample_t *) malloc (bufferlength *
					 sizeof (sample_t));
  newbuffer.currpos = -1;
  newbuffer.arraylength = bufferlength;
  newbuffer.pre_length = pre_length;
  newbuffer.post_length = post_length;

#ifdef TURBO_BUFFER
  {
    int i;
    int tablesize = bufferlength * 3;
    newbuffer.indextable = (int *) malloc (tablesize *
					   sizeof (int));
    newbuffer.indextable += bufferlength;
    for (i = -bufferlength; i < 2 * bufferlength; i++)
      {
	newbuffer.indextable[i] = (i + bufferlength) % bufferlength;
      }
  }
#endif

  return newbuffer;
}

void
delete_buffer (buffer_t * buffer)
{
#ifdef TURBO_BUFFER
  free (buffer->indextable - buffer->arraylength);
#endif
  buffer->arraylength = 0;
  buffer->pre_length = 0;
  buffer->post_length = 0;
  free (buffer->array);
}

#ifndef TURBO_BUFFER

sample_t
get_from_buffer (buffer_t * buffer, long offset)
{
  long realpos;

  if ((offset > buffer->pre_length)
      || (-offset > buffer->post_length))
    {
      fprintf (stderr, "ERROR: get_from_buffer #1 - offset=%ld\n", offset);
    }

  realpos = buffer->currpos + offset;

  if (realpos < 0)
    realpos += buffer->arraylength;

  if (realpos >= buffer->arraylength)
    realpos -= buffer->arraylength;

  return (buffer->array)[realpos];
}

void
put_in_buffer (buffer_t * buffer, long offset, sample_t sample)
{
  long realpos;

  if ((offset > buffer->pre_length)
      || (-offset > buffer->post_length))
    {
      fprintf (stderr, "ERROR (put_in_buffer #1)\n");
    }

  realpos = buffer->currpos + offset;

  if (realpos < 0)
    realpos += buffer->arraylength;

  if (realpos >= buffer->arraylength)
    realpos -= buffer->arraylength;

  (buffer->array)[realpos] = sample;
}

#endif /* ndef TURBO_BUFFER */

void
advance_current_pos (buffer_t * buffer, int filterno)
{
  long i;

  if (buffer->currpos < 0)	/* just newly created */
    {
      for (i = 0; i <= buffer->post_length; i++)
	{
	  /* fill first part with zeroes */
	  (buffer->array)[i].left = 0;
	  (buffer->array)[i].right = 0;
	}

      buffer->currpos = buffer->post_length;

      for (i = 0; i <= buffer->pre_length; i++)
	{
	  (buffer->array)[buffer->currpos + i] =
	    get_sample_from_filter (filterno - 1);
	}

    }
  else
    {
      buffer->currpos++;

      if (buffer->currpos >= buffer->arraylength)
	buffer->currpos -= buffer->arraylength;

      i = buffer->currpos + buffer->pre_length;

      if (i >= buffer->arraylength)
	i -= buffer->arraylength;

      (buffer->array)[i] = get_sample_from_filter (filterno - 1);
    }
}

void
advance_current_pos_custom (buffer_t * buffer, fillfuncpointer_t fillfunc,
			    long offset_zero, parampointer_t parampointer)
{
  long i;

  if (buffer->currpos < 0)	/* just newly created */
    {
      for (i = 0; i <= buffer->post_length; i++)
	{
	  /* fill first part with zeroes */
	  (buffer->array)[i].left = 0;
	  (buffer->array)[i].right = 0;
	}

      buffer->currpos = buffer->post_length;

      for (i = 0; i <= buffer->pre_length; i++)
	{
	  (buffer->array)[buffer->currpos + i] =
	    fillfunc (i, offset_zero, parampointer);
	}

    }
  else
    {
      buffer->currpos++;

      if (buffer->currpos >= buffer->arraylength)
	buffer->currpos -= buffer->arraylength;

      i = buffer->currpos + buffer->pre_length;

      if (i >= buffer->arraylength)
	i -= buffer->arraylength;

      (buffer->array)[i] = fillfunc (buffer->pre_length, offset_zero,
				     parampointer);
    }
}

/* ----- QUICK SORT -------------------------------------------------------- */

/* For arrays of `signed short' with max. length of 32767. */

/* Adapted from Ammeraal, L., `Ansi C', 2nd ed., Schoonhoven (The
   Netherlands): Academic Service, 1993.  */

void
qsort2 (signed short *a, int n)	/* a: pointer to start of array      */
{				/* n: # elements in array            */
  int i, j;
  signed short x, w;

  do
    {
      i = 0;
      j = n - 1;
      x = a[j / 2];
      do
	{
	  while (a[i] < x)
	    i++;
	  while (a[j] > x)
	    j--;
	  if (i > j)
	    break;
	  w = a[i];
	  a[i] = a[j];
	  a[j] = w;
	}
      while (++i <= --j);
      if (j + 1 < n - i)
	{
	  if (j > 0)
	    qsort2 (a, j + 1);
	  a += i;
	  n -= i;
	}
      else
	{
	  if (i < n - 1)
	    qsort2 (a + i, n - i);
	  n = j + 1;
	}
    }
  while (n > 1);
}

/* And one for doubles, max size 2G */

void
qsort2double (double *a, long n)
					/* a: pointer to start of array      */
{				/* n: # elements in array            */
  long i, j;
  double x, w;

  do
    {
      i = 0;
      j = n - 1;
      x = a[j / 2];
      do
	{
	  while (a[i] < x)
	    i++;
	  while (a[j] > x)
	    j--;
	  if (i > j)
	    break;
	  w = a[i];
	  a[i] = a[j];
	  a[j] = w;
	}
      while (++i <= --j);
      if (j + 1 < n - i)
	{
	  if (j > 0)
	    qsort2double (a, j + 1);
	  a += i;
	  n -= i;
	}
      else
	{
	  if (i < n - 1)
	    qsort2double (a + i, n - i);
	  n = j + 1;
	}
    }
  while (n > 1);
}

/* ----- MEDIAN FUNCTION --------------------------------------------------- */

/* For arrays of 'signed short' with max. length of 32767.

   NOTE: The array will be affected (most likely: sorted). Do not use
   the array after calling median() !

   Examples: {2,5,4,1,3}  ==sort==>  {1,2,3,4,5}   ==returns==>  3

   {2,5,4,1,6,3}  ==sort==>  {1,2,3,4,5,6}  ==returns==>  3
 */

#ifdef TURBO_MEDIAN

#ifndef MEDIAN_THRESHOLD
#define MEDIAN_THRESHOLD 5
#endif

signed short
median (signed short *a, int n)
{
  int i, j, k;
  signed short x, w, t1, t2;
  int low, mid, high;

  low = 0;
  mid = n / 2;
  high = n - 1;

  while (high - low > MEDIAN_THRESHOLD)
    {
      i = low;
      j = high;
      t1 = a[i];
      t2 = a[j];
      if (t1 > t2)
	{
	  x = t1;
	  t1 = t2;
	  t2 = x;
	}
      x = a[(low + high) / 2];
      if (x < t1)
	{
	  x = t1;
	}
      else if (x > t2)
	{
	  x = t2;
	}
      do
	{
	  while (a[i] < x)
	    i++;
	  while (a[j] > x)
	    j--;
	  if (i > j)
	    break;
	  w = a[i];
	  a[i] = a[j];
	  a[j] = w;
	}
      while (++i <= --j);
      if (i <= mid)
	{
	  low = i;
	}
      else
	{
	  high = j;
	}
    }
  for (i = low; i <= mid; i++)
    {
      k = i;
      w = a[i];
      for (j = i + 1; j <= high; j++)
	{
	  if (a[j] < w)
	    {
	      k = j;
	      w = a[j];
	    }
	}
      if (k != i)
	{
	  a[k] = a[i];
	  a[i] = w;
	}
    }
  return a[mid];
}

#else /* if not TURBO_MEDIAN */

signed short
median (signed short *a, int n)
{				/* a: pointer to start of array      */
  qsort2 (a, n);		/* n: # elements in array            */

  return a[((n + 1) / 2) - 1];	/* (10+1)/2 = 5   (11+1)/2 = 6       */
}

#endif /* (not) TURBO_MEDIAN */



/* ----- BUILDING THE LIST OF FILTERS -------------------------------------- */

void
add_to_filterlist (scrollmenu_t * filtlist, int *filtnumbers,
		   char **helptexts, int filternumber, char *filtername,
		   char *helptext)
{
  filtlist->items[filtlist->number] = filtername;
  filtnumbers[filtlist->number] = filternumber;
  helptexts[filtlist->number] = helptext;

  filtlist->number++;
}

void
make_filterlist (scrollmenu_t * filtlist, int *filtnumbers,
		 char **helptexts)
{
  filtlist->number = 0;

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     COPYONLY_FILTER,
		     COPYONLY_NAME,
		     COPYONLY_HELPTEXT);

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     MONOIZE_FILTER,
		     MONOIZE_NAME,
		     MONOIZE_HELPTEXT);

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     SIMPLE_MEDIAN_FILTER,
		     SIMPLE_MEDIAN_NAME,
		     SIMPLE_MEDIAN_HELPTEXT);

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     DOUBLE_MEDIAN_FILTER,
		     DOUBLE_MEDIAN_NAME,
		     DOUBLE_MEDIAN_HELPTEXT);

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     SIMPLE_MEAN_FILTER,
		     SIMPLE_MEAN_NAME,
		     SIMPLE_MEAN_HELPTEXT);

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     RMS_FILTER,
		     RMS_NAME,
		     RMS_HELPTEXT);

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     COND_MEDIAN_FILTER,
		     COND_MEDIAN_NAME,
		     COND_MEDIAN_HELPTEXT);

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     COND_MEDIAN2_FILTER,
		     COND_MEDIAN2_NAME,
		     COND_MEDIAN2_HELPTEXT);

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     COND_MEDIAN3_FILTER,
		     COND_MEDIAN3_NAME,
		     COND_MEDIAN3_HELPTEXT);

  add_to_filterlist (filtlist,
		     filtnumbers,
		     helptexts,
		     EXPERIMENT_FILTER,
		     EXPERIMENT_NAME,
		     EXPERIMENT_HELPTEXT);

}

/* ----- GET SAMPLE FROM FILTER -------------------------------------------- */

sample_t
get_sample_from_filter (int filterno)
{
  if (filterno == -1)
    return readsamplesource ();
  else
    switch (filter_type[filterno])
      {
      case SIMPLE_MEDIAN_FILTER:
	return simple_median_filter (parampointerarray[filterno]);
	break;

      case SIMPLE_MEAN_FILTER:
	return simple_mean_filter (parampointerarray[filterno]);
	break;

      case COND_MEDIAN_FILTER:
	return cond_median_filter (parampointerarray[filterno]);
	break;

      case DOUBLE_MEDIAN_FILTER:
	return double_median_filter (parampointerarray[filterno]);
	break;

      case COND_MEDIAN2_FILTER:
	return cond_median2_filter (parampointerarray[filterno]);
	break;

      case RMS_FILTER:
	return rms_filter (parampointerarray[filterno]);
	break;

      case COPYONLY_FILTER:
	return copyonly_filter (parampointerarray[filterno]);
	break;

      case MONOIZE_FILTER:
	return monoize_filter (parampointerarray[filterno]);
	break;

      case COND_MEDIAN3_FILTER:
	return cond_median3_filter (parampointerarray[filterno]);
	break;

      case EXPERIMENT_FILTER:
	return experiment_filter (parampointerarray[filterno]);
	break;

      default:
	printf ("Error (get_sample_from_filter): wrong filter [%d]", filterno);
	exit (2);
	break;
      }
}

/* ----- INIT FILTERS ------------------------------------------------------ */

void
init_filters ()
{
  int i;

  for (i = 0; i < number_of_filters; i++)
    switch (filter_type[i])
      {
      case 0:
	break;

      case SIMPLE_MEDIAN_FILTER:
	init_simple_median_filter (i, parampointerarray[i]);
	break;

      case SIMPLE_MEAN_FILTER:
	init_simple_mean_filter (i, parampointerarray[i]);
	break;

      case COND_MEDIAN_FILTER:
	init_cond_median_filter (i, parampointerarray[i]);
	break;

      case DOUBLE_MEDIAN_FILTER:
	init_double_median_filter (i, parampointerarray[i]);
	break;

      case COND_MEDIAN2_FILTER:
	init_cond_median2_filter (i, parampointerarray[i]);
	break;

      case RMS_FILTER:
	init_rms_filter (i, parampointerarray[i]);
	break;

      case COPYONLY_FILTER:
	init_copyonly_filter (i, parampointerarray[i]);
	break;

      case MONOIZE_FILTER:
	init_monoize_filter (i, parampointerarray[i]);
	break;

      case COND_MEDIAN3_FILTER:
	init_cond_median3_filter (i, parampointerarray[i]);
	break;

      case EXPERIMENT_FILTER:
	init_experiment_filter (i, parampointerarray[i]);
	break;

      default:
	printf ("Error (init_filters): wrong filter");
	exit (2);
	break;
      }
}

/* ----- DELETE FILTERS ------------------------------------------------------ */

void
delete_filters ()
{
  int i;

  for (i = 0; i < number_of_filters; i++)
    switch (filter_type[i])
      {
      case 0:
	break;

      case SIMPLE_MEDIAN_FILTER:
	delete_simple_median_filter (parampointerarray[i]);
	break;

      case SIMPLE_MEAN_FILTER:
	delete_simple_mean_filter (parampointerarray[i]);
	break;

      case COND_MEDIAN_FILTER:
	delete_cond_median_filter (parampointerarray[i]);
	break;

      case DOUBLE_MEDIAN_FILTER:
	delete_double_median_filter (parampointerarray[i]);
	break;

      case COND_MEDIAN2_FILTER:
	delete_cond_median2_filter (parampointerarray[i]);
	break;

      case RMS_FILTER:
	delete_rms_filter (parampointerarray[i]);
	break;

      case COPYONLY_FILTER:
	delete_copyonly_filter (parampointerarray[i]);
	break;

      case MONOIZE_FILTER:
	delete_monoize_filter (parampointerarray[i]);
	break;

      case COND_MEDIAN3_FILTER:
	delete_cond_median3_filter (parampointerarray[i]);
	break;

      case EXPERIMENT_FILTER:
	delete_experiment_filter (parampointerarray[i]);
	break;

      default:
	printf ("Error (delete_filters): wrong filter");
	exit (2);
	break;
      }
}

/* ----- PARAM DEFAULTS ---------------------------------------------------- */

void
param_defaults (parampointer_t parampointer, int filtertype)
{
  switch (filtertype)
    {
    case 0:			/* nothing needed */
      break;

    case SIMPLE_MEDIAN_FILTER:
      simple_median_param_defaults (parampointer);
      break;

    case SIMPLE_MEAN_FILTER:
      simple_mean_param_defaults (parampointer);
      break;

    case COND_MEDIAN_FILTER:
      cond_median_param_defaults (parampointer);
      break;

    case DOUBLE_MEDIAN_FILTER:
      double_median_param_defaults (parampointer);
      break;

    case COND_MEDIAN2_FILTER:
      cond_median2_param_defaults (parampointer);
      break;

    case RMS_FILTER:
      rms_param_defaults (parampointer);
      break;

    case COPYONLY_FILTER:
      copyonly_param_defaults (parampointer);
      break;

    case MONOIZE_FILTER:
      monoize_param_defaults (parampointer);
      break;

    case COND_MEDIAN3_FILTER:
      cond_median3_param_defaults (parampointer);
      break;

    case EXPERIMENT_FILTER:
      experiment_param_defaults (parampointer);
      break;

    default:
      printf ("Error (praram_defaults): wrong filter");
      exit (2);
      break;
    }
}
