/* Determining Track Start/End - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_TRACKSPLIT_H
#define HAVE_TRACKSPLIT_H


#define TRACKSPLIT_COMPUTE_HEADERTEXT "Track Location"

void tracksplit_main (char *startdir);
void cmdline_tracksplit_main (char *startdir, char *filename, int make_use_rms, 
		int make_graphs, long blocklen, int global_silence_factor, 
		int local_silence_threshold, int min_silence_blocks, 
		int min_track_blocks, int extra_blocks_start, 
		int extra_blocks_end);

#endif /* HAVE_TARACKSPLIT_H */
