/* Convert to mono `Filter'

 * Copyright (C) 1999 S.J. Tappin
 * Derived from the Copy only filter (J.A. Bezemer 1998)
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_mono.h"
#include "signpr_general.h"


void
monoize_param_defaults (parampointer_t parampointer)
{
}

void
init_monoize_filter (int filterno, parampointer_t parampointer)
{
  parampointer->buffer = init_buffer (0, 0);

  parampointer->filterno = filterno;
}

void
delete_monoize_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
}


sample_t
monoize_filter (parampointer_t parampointer)
{
  sample_t sample;
  longsample_t sum;

  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  sample = get_from_buffer (&parampointer->buffer, 0);
  sum.left = (sample.left + sample.right) / 2;
  sample.left = sum.left;
  sample.right = sum.left;

  return sample;
}
