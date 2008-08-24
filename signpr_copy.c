/* Copy Only `Filter'

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_copy.h"
#include "signpr_general.h"


void
copyonly_param_defaults (parampointer_t parampointer)
{
}

void
init_copyonly_filter (int filterno, parampointer_t parampointer)
{
  parampointer->buffer = init_buffer (0, 0);

  parampointer->filterno = filterno;
}

void
delete_copyonly_filter (parampointer_t parampointer)
{
  delete_buffer (&parampointer->buffer);
}


sample_t
copyonly_filter (parampointer_t parampointer)
{
  advance_current_pos (&parampointer->buffer, parampointer->filterno);

  return get_from_buffer (&parampointer->buffer, 0);
}
