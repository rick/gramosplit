/* Experimenting Filter

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

/* You can experiment with this filter to your hearts desire. Currently,
   samples x[t-100] through x[t+100] are accessible. An example
   smoothing filter is presented below, so you can get an idea how
   to program things. */

#include "signpr_exper.h"
#include "signpr_general.h"
#include <math.h>


void
experiment_param_defaults (parampointer_t parampointer)
{
}

void
init_experiment_filter (int filterno, parampointer_t parampointer)
{
  parampointer->buffer = init_buffer (100, 100);

  parampointer->filterno = filterno;
}

void
delete_experiment_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
}


sample_t
experiment_filter (parampointer_t parampointer)
{
  sample_t sample;
  longsample_t longsample;
/* doublesample_t doublesample; */

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

/* Example: a smoothing filter (lowpass, that is):

   y[t] = { x[t-2] + 5*x[t-1] + 13*x[t] + 5*x[t+1] + x[t+2] } / 25
 */

  /* zero totals */
  longsample.left = 0;
  longsample.right = 0;

  /* compute the weighted sum */
  sample = get_from_buffer (&parampointer->buffer, -2);
  longsample.left += sample.left;
  longsample.right += sample.right;

  sample = get_from_buffer (&parampointer->buffer, -1);
  longsample.left += 5 * sample.left;
  longsample.right += 5 * sample.right;

  sample = get_from_buffer (&parampointer->buffer, 0);
  longsample.left += 13 * sample.left;
  longsample.right += 13 * sample.right;

  sample = get_from_buffer (&parampointer->buffer, 1);
  longsample.left += 5 * sample.left;
  longsample.right += 5 * sample.right;

  sample = get_from_buffer (&parampointer->buffer, 2);
  longsample.left += sample.left;
  longsample.right += sample.right;

  /* devide by the total weight */
  sample.left = longsample.left / 25;
  sample.right = longsample.right / 25;

  /* return the computed sample */
  return sample;
}
