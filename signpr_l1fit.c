/* 
   signpr_l1fit.c

   Does L1 norm fit to supplied data, using integer arithmetic.

   Copyright (C) 1999 S.J. Tappin

   Licensed under the terms of the GNU General Public License.
   ABSOLUTELY NO WARRANTY.
   See the file `COPYING' in this directory.
 */


#include <stdlib.h>
#include <math.h>
#include "signpr_general.h"
#include "signpr_l1fit.h"

long
mdfunc (b, x, y, a, n, tmp1)
     double b, *a;
     signed short *x, *y, *tmp1;
     int n;
{
  int i;
  long result;
  double dtmp;

  for (i = 0; i < n; i++)
    {
      tmp1[i] = (signed short) rint (y[i] - b * x[i]);
    }

  *a = (double) median (tmp1, n);

  result = 0;
  for (i = 0; i < n; i++)
    {
      dtmp = (y[i] - (b * x[i] + *a));
      if (y[i] != 0)
	dtmp = dtmp / abs (y[i]);
      if (dtmp > 0)
	{
	  result += x[i];
	}
      else if (dtmp < 0)
	{
	  result -= x[i];
	}
    }


  return (result);
}

void
l1fit (x, y, n, a, b)
     signed short *x, *y;
     double *a, *b;
     int n;
{
  int sx, sy, sxx, sxy;
  int del;
  double aa, bb, c2, sigb;
  double b1, b2, delb;
  long f1, f2, f;
  int i;

  signed short *tmp1;

  sx = 0;
  sy = 0;
  sxx = 0;
  sxy = 0;

  for (i = 0; i < n; i++)
    {
      sx += x[i];
      sy += y[i];
      sxx += x[i] * x[i];
      sxy += x[i] * y[i];
    }

  del = n * sxx - sx * sx;

  if (del == 0)
    {				/* All X's the same only fit horizontal */
      *a = (double) median (y, n);
      *b = 0.;
      return;
    }

  /*
     Use the normal least squares (L2) fit as a starting point.
   */

  aa = (sxx * sy - sx * sxy) / (double) del;
  bb = (n * sxy - sx * sy) / (double) del;

  c2 = 0.;
  for (i = 0; i < n; i++)
    c2 += (y[i] - (aa + bb * x[i])) * (y[i] - (aa + bb * x[i]));
  sigb = sqrt (c2 / del);

  /* Perfect fit, L1 must give the same answer so go straight back */

  if (c2 == 0.)
    {
      *a = aa;
      *b = bb;
      return;
    }

  tmp1 = malloc (n * sizeof (short));

  b1 = bb;
  f1 = mdfunc (b1, x, y, &aa, n, tmp1);
  if (f1 >= 0)
    {
      delb = sigb * 3.;
    }
  else
    {
      delb = -sigb * 3.;
    }

  b2 = b1 + delb;
  f2 = mdfunc (b2, x, y, &aa, n, tmp1);

  while (f1 * f2 > 0)
    {
      b1 = b2;
      f1 = f2;
      b2 = b1 + delb;
      f2 = mdfunc (b2, x, y, &aa, n, tmp1);
    }

  sigb *= 0.01;

  while (fabs (b1 - b2) > sigb)
    {
      bb = (b1 + b2) / 2.;
      if (bb == b1 || bb == b2)
	break;
      f = mdfunc (bb, x, y, &aa, n, tmp1);
      if (f * f1 >= 0)
	{
	  f1 = f;
	  b1 = bb;
	}
      else
	{
	  f2 = f;
	  b2 = bb;
	}
    }

  free (tmp1);

  *a = aa;
  *b = bb;
}
