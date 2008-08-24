/* GramoFile - Main

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "tracksplit.h"
#include "signpr_main.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  char filename[250];
  char startdir[250];
  char *helpcharptr;

  /* options defaults */
  /* FIXME: no default for filename */
  int make_use_rms = 1;
  int make_graphs = 0;
  long blocklen = 4410;
  int global_silence_factor = 150;
  int local_silence_threshold = 5;
  int min_silence_blocks = 20;
  int min_track_blocks = 50;
  int extra_blocks_start = 3;
  int extra_blocks_end = 6;
  char *charptr;

  char outfilename[250];
  char usage[1024];

  helpcharptr = getcwd (startdir, 250);
  if (helpcharptr == NULL)
    strcpy (startdir, "/");
  else
    strcat (startdir, "/");

  /* uber-ghetto cmdline processing -- I just want to do the minimum to automate this shit. */
  strcpy(usage, "gramofile <infile.wav> <global_silence_factor=150> <local_silence_thresh=5> <min_silent_blocks=20> <min_track_blocks=50> <extra_blocks_start=3> <extra_blocks_end=6>");
fprintf(stderr, "args[%d]\n", argc);
  if (8 != argc) {
	  fprintf(stderr, "%s\n", usage);
	  return 1;
  }

  // grab commandline args
  strncpy(filename, argv[1], 250);
  global_silence_factor = atoi(argv[2]);
  local_silence_threshold = atoi(argv[3]);
  min_silence_blocks = atoi(argv[4]);
  min_track_blocks = atoi(argv[5]);
  extra_blocks_start = atoi(argv[6]);
  extra_blocks_end = atoi(argv[7]);

  fprintf(stderr, "args: [%s] [%d] [%d] [%d] [%d] [%d] [%d]\n", filename, global_silence_factor, local_silence_threshold, min_silence_blocks, min_track_blocks, extra_blocks_start, extra_blocks_end);
 
  /* build a reasonable outfilename from filename */
  strncpy(outfilename, filename, 240);
  if ((charptr = strrchr(outfilename, '.')) != (char *)NULL) {
	  charptr[0] = '\0';
	  strcat(outfilename, "-part-.wav");
	  fprintf(stderr, "filename[%s] ->[%s]\n", filename, outfilename);
  } else {
	  fprintf(stderr, "Bad .wav filename [%s]\n", filename);
	  return(1);
  }
  
  /* split up tracks */
  cmdline_tracksplit_main (startdir, filename, make_use_rms, make_graphs,
	  blocklen, global_silence_factor, local_silence_threshold,
	  min_silence_blocks, min_track_blocks, extra_blocks_start,
	  extra_blocks_end);

  /* and write out result files */
  signproc_main (startdir, filename, outfilename);

  return 0;
}
