/* Determining Starts and Ends of Tracks

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "tracksplit.h"
#include "signpr_wav.h"
#include "fmtheaders.h"
#include "secshms.h"
#include "signpr_general.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void
tracksplit_merge (short *typearray, long *startarray, long *endarray,
		  long *num_of_parts)
{
  long l;
  long l2 = 0;

  for (l = 1; l < *num_of_parts; l++)
    if (typearray[l] == typearray[l2])
      /* same type as previous -> merge */
      endarray[l2] = endarray[l];
    else
      {
	/* different */
	l2++;
	typearray[l2] = typearray[l];
	startarray[l2] = startarray[l];
	endarray[l2] = endarray[l];
      }

  *num_of_parts = l2 + 1;
}


void
tracksplit_findtracks (double *medarray,	/* inputs */
		       long total_blocks,
		       double global_silence_threshold,
		       int local_silence_threshold,
		       int min_silence_blocks,
		       int min_track_blocks,
		       int extra_blocks_start,
		       int extra_blocks_end,

		       long *trackstarts,	/* outputs */
		       long *trackends,
		       int *detected_tracks)
{
  double *above_threshold;
  double *above_th_rms;

  short *part_type;
  long *part_start;
  long *part_end;
  long num_parts;

  short type_now;
  short type_left, type_right;

  double local_mean;

  register long l;
  register long l2;
  double tempdouble;


/* These modes must be >0 and all different */
#define POSSIBLE_TRACK     1
#define POSSIBLE_SILENCE   2
#define CERTAIN_TRACK      3
#define CERTAIN_SILENCE    4

#define SECOND_THRESHOLD   500


  /* reserve space for arrays */
  above_threshold = (double *) malloc (total_blocks * sizeof (double));
  above_th_rms = (double *) malloc (total_blocks * sizeof (double));
  part_type = (short *) malloc (total_blocks * sizeof (short));
  part_start = (long *) malloc (total_blocks * sizeof (long));
  part_end = (long *) malloc (total_blocks * sizeof (long));

  /* apply global threshold */
  for (l = 0; l < total_blocks; l++)
    {
      if (medarray[l] > global_silence_threshold)
	above_threshold[l] = 1000;
      else
	above_threshold[l] = 0;

      above_th_rms[l] = 0;
    }

  /* smooth the data a little by `running RMS' of length 11 */
  for (l = 0; l < total_blocks; l++)
    {
      tempdouble = 0;
      for (l2 = l - 5; l2 <= l + 5; l2++)
	if (l2 >= 0 && l2 < total_blocks)
	  tempdouble += above_threshold[l2] * above_threshold[l2];
      /* else tempdouble += 0 */
      above_th_rms[l] = sqrt (tempdouble / (2 * 5 + 1));
    }

  /* before first track is certainly nothing (this is necessary for the
     computation later on) */
  part_type[0] = CERTAIN_SILENCE;
  part_start[0] = -2;
  part_end[0] = -2;

  type_now = POSSIBLE_SILENCE;
  num_parts = 1;
  l2 = -1;			/* l2 = start of current mode */

  for (l = 0; l < total_blocks; l++)
    {
      if (type_now == POSSIBLE_SILENCE &&
	  above_th_rms[l] > SECOND_THRESHOLD)
	{
	  part_type[num_parts] = type_now;
	  part_start[num_parts] = l2;
	  part_end[num_parts] = l - 1;
	  l2 = l;
	  type_now = POSSIBLE_TRACK;
	  num_parts++;
	}

      if (type_now == POSSIBLE_TRACK &&
	  above_th_rms[l] <= SECOND_THRESHOLD)
	{
	  part_type[num_parts] = type_now;
	  part_start[num_parts] = l2;
	  part_end[num_parts] = l - 1;
	  l2 = l;
	  type_now = POSSIBLE_SILENCE;
	  num_parts++;
	}
    }

  /* close last track/silence */
  part_type[num_parts] = type_now;
  part_start[num_parts] = l2;
  part_end[num_parts] = l - 1;
  l2 = l;
  num_parts++;

  /* after last block there's certainly nothing */
  part_type[num_parts] = CERTAIN_SILENCE;
  part_start[num_parts] = l2;
  part_end[num_parts] = l2;
  num_parts++;


  /* search for certain tracks/silences */
  for (l = 0; l < num_parts; l++)
    {
      if (part_type[l] == POSSIBLE_SILENCE
	  && part_end[l] - part_start[l] + 1 >= min_silence_blocks)
	part_type[l] = CERTAIN_SILENCE;

      if (part_type[l] == POSSIBLE_TRACK
	  && part_end[l] - part_start[l] + 1 >= min_track_blocks)
	part_type[l] = CERTAIN_TRACK;
    }

#define notPART_DEBUG
#ifdef PART_DEBUG
  for (l = 0; l < num_parts; l++)
    fprintf (stderr, "\n\rBlk: %ld  Typ: %hd  Start: %ld  End: %ld",
	     l, part_type[l], part_start[l], part_end[l]);
  sleep (5);
#endif

  /* If first silence (blocks -1 .. ??) shorter than min_sil, it would
     be added in front of the first track. That's not what we want, so
     force it to be certain silence. Likewise with the last silence
     (if there is one). */

  /* Part 0, block -2 only, already was certain silence;
     part 1 always starts at block -1 and is either possible or certain
     silence. */
  if (part_type[1] == POSSIBLE_SILENCE)
    part_type[1] = CERTAIN_SILENCE;
  /* At this point, both _blocks_ -2 and -1 are certain silence. */

  /* Part num_parts-1 is block total_blocks only, and is certain silence;
     part num_parts-2 may be all modes. */
  if (part_type[num_parts - 2] == POSSIBLE_SILENCE)
    part_type[num_parts - 2] = CERTAIN_SILENCE;
  /* At this point, _block_ total_blocks is certain silence. */

  /* Concluding, now all `virtual' blocks (outside range 0...total_blocks-1)
     are certain silence, and won't be (illegal) parts of tracks. */


  /* Search for poss_sil poss_tr poss_sil, to account for ticks within
     silences; convert to (poss_sil)^3

     This works, but may do some strange things. For example: c_t p_s p_t
     p_s c_t might become c_t p_s p_s p_s c_t -> c_t p_s c_t -> c_t c_s c_t.
     In words: a more-or-less common occurrence within one (1) track could
     get interpreted as a inter-track silence.

     Actually, this is not really a problem, because it is much easier to
     join two parts of one track than to manually split one `track' in two
     pieces at the right spot. So, it doesn't matter if there are a few
     silences too much. */
  for (l = 1; l < num_parts - 1; l++)
    {
      if (part_type[l] == POSSIBLE_TRACK
	  && part_type[l - 1] == POSSIBLE_SILENCE
	  && part_type[l + 1] == POSSIBLE_SILENCE
	  && part_end[l] - part_start[l] < 10)
	part_type[l] = POSSIBLE_SILENCE;
    }

  tracksplit_merge (part_type, part_start, part_end,
		    &num_parts);

  /* now some (poss_sil)^3 sequences may be > min_silence_blocks, so
     check again */
  for (l = 0; l < num_parts; l++)
    {
      if (part_type[l] == POSSIBLE_SILENCE
	  && part_end[l] - part_start[l] + 1 >= min_silence_blocks)
	part_type[l] = CERTAIN_SILENCE;
    }


  /* More rules here? */


#ifdef PART_DEBUG
  for (l = 0; l < num_parts; l++)
    fprintf (stderr, "\n\rBlk: %ld  Typ: %hd  Start: %ld  End: %ld",
	     l, part_type[l], part_start[l], part_end[l]);
  sleep (5);
#endif

  /* now resolve all other possible_* */
  for (l = 0; l < num_parts; l++)
    if (part_type[l] == POSSIBLE_SILENCE || part_type[l] == POSSIBLE_TRACK)
      {
	/* search what is certain on left side */
	l2 = l;
	type_left = 0;
	while (l2 >= 0 && type_left == 0)
	  {
	    if (part_type[l2] == CERTAIN_SILENCE
		|| part_type[l2] == CERTAIN_TRACK)
	      type_left = part_type[l2];
	    l2--;
	  }
	if (type_left == 0)
	  type_left = CERTAIN_SILENCE;

	/* search what is certain on right side */
	l2 = l;
	type_right = 0;
	while (l2 < num_parts && type_right == 0)
	  {
	    if (part_type[l2] == CERTAIN_SILENCE
		|| part_type[l2] == CERTAIN_TRACK)
	      type_right = part_type[l2];
	    l2++;
	  }
	if (type_right == 0)
	  type_right = CERTAIN_SILENCE;

	/* decide what this one is going to be */
	if (type_left == CERTAIN_SILENCE && type_right == CERTAIN_SILENCE)
	  part_type[l] = CERTAIN_SILENCE;
	else			/* CERTAIN_TRACK on either or both sides */
	  part_type[l] = CERTAIN_TRACK;
      }

  tracksplit_merge (part_type, part_start, part_end,
		    &num_parts);

  /* Get rid of (illegal) blocks -2, -1 and total_blocks */
  if (part_end[0] >= 0)
    part_start[0] = 0;
  else
    {
      for (l = 0; l < num_parts - 1; l++)
	{
	  part_type[l] = part_type[l + 1];
	  part_start[l] = part_start[l + 1];
	  part_end[l] = part_end[l + 1];
	}
      num_parts--;
    }

  if (part_start[num_parts - 1] <= total_blocks - 1)
    part_end[num_parts - 1] = total_blocks - 1;
  else
    num_parts--;

#ifdef PART_DEBUG
  for (l = 0; l < num_parts; l++)
    fprintf (stderr, "\n\rBlk: %ld  Typ: %hd  Start: %ld  End: %ld",
	     l, part_type[l], part_start[l], part_end[l]);
  sleep (5);
#endif

  /* fine-search for track starts/ends */
  for (l = 0; l < num_parts; l++)
    if (part_type[l] == CERTAIN_SILENCE &&
	part_end[l] - part_start[l] + 1 >= 10)
      {
	/* find local mean; exclude silence start/end */
	local_mean = 0;
	for (l2 = part_start[l] + 3; l2 <= part_end[l] - 3; l2++)
	  local_mean += medarray[l2];
	local_mean /= (part_end[l] - part_start[l] + 1 - 6);

	while (medarray[part_start[l]] > local_mean *
	       (1 + (local_silence_threshold / 100.))
	       && part_start[l] < part_end[l])
	  part_start[l]++;

	while (medarray[part_end[l]] > local_mean *
	       (1 + (local_silence_threshold / 100.))
	       && part_end[l] > part_start[l])
	  part_end[l]--;

	/* minimize lost fade-in/out by adding extra blocks to track
	   start/end (if possible) */
	if (part_end[l] - part_start[l] + 1 >=
	    extra_blocks_end + extra_blocks_start + 1)
	  {
	    part_start[l] += extra_blocks_end;
	    part_end[l] -= extra_blocks_start;
	  }

	/* Now silence may be too short, regard it as silence-in-a-track;
	   except first and last silence. */
	if (part_end[l] - part_start[l] + 1 < min_silence_blocks
	    && l > 0 && l < num_parts - 1)
	  part_type[l] = CERTAIN_TRACK;

	/* Adjust adjecent tracks */
	if (l > 0)
	  part_end[l - 1] = part_start[l] - 1;
	if (l < num_parts - 1)
	  part_start[l + 1] = part_end[l] + 1;
      }

  /* if too-short-c_s -> c_t, we should merge c_t's */
  tracksplit_merge (part_type, part_start, part_end,
		    &num_parts);

#ifdef PART_DEBUG
  for (l = 0; l < num_parts; l++)
    fprintf (stderr, "\n\rBlk: %ld  Typ: %hd  Start: %ld  End: %ld",
	     l, part_type[l], part_start[l], part_end[l]);
  sleep (5);
#endif

  /* Make an array with tracks only */
  *detected_tracks = 0;
  for (l = 0; l < num_parts && *detected_tracks < 99999; l++)
    if (part_type[l] == CERTAIN_TRACK)
      {
	trackstarts[*detected_tracks] = part_start[l];
	trackends[*detected_tracks] = part_end[l];
	(*detected_tracks)++;
      }

#if 0
/* TESTING: determine mean(localmeans) */
/* NB: what if NO certain silences? -> divide by 0 ?? */
  tempdouble = 0;
  for (l = 0; l < num_parts; l++)
    if (part_type[l] == CERTAIN_SILENCE)
      {
	local_mean = 0;
	for (l2 = part_start[l]; l2 <= part_end[l]; l2++)
	  local_mean += medarray[l2];
	local_mean /= (part_end[l] - part_start[l] + 1);
	tempdouble += local_mean;
      }
  l2 = 0;
  for (l = 0; l < num_parts; l++)
    if (part_type[l] == CERTAIN_SILENCE)
      l2++;
  tempdouble /= l2;
  fprintf (stderr, "mean(locmean)= %f\n", tempdouble);
/* END TESTING */
#endif /* 0 */

  free (above_threshold);
  free (above_th_rms);
  free (part_type);
  free (part_start);
  free (part_end);
}


void
cmdline_tracksplit_main (char *startdir, char *filename, int make_use_rms, 
		int make_graphs, long blocklen, int global_silence_factor, 
		int local_silence_threshold, int min_silence_blocks, 
		int min_track_blocks, int extra_blocks_start, 
		int extra_blocks_end)
{
  sample_t sample;
  double *rmsarray = NULL;	/* Otherwise gcc complains */
  double *medarray;
  double *sortarray;
  long total_samples, total_samples_read;
  struct stat buf;
  register long l;
  long samples_read;
  long current_block, total_blocks;
  double sum_left, sum_right;
  int i;
  FILE *tempfile;
  FILE *tempfile2;

#define dontCOMPUTE_GLOBAL_MEAN
#ifdef COMPUTE_GLOBAL_MEAN
  double global_mean;
#endif

  long trackstarts[10000];
  long trackends[10000];
  int detected_tracks;
  char tempstring[250];
  double global_silence_threshold;
  double min_poss_threshold, max_poss_threshold;
  int compute_rms_now;
  long templong;
  compute_rms_now = 0;		/* Should we compute RMS? */

  if (!make_use_rms)
    compute_rms_now = 1;

  if (!compute_rms_now)		/* RMS must exist */
    {
      strcpy (tempstring, filename);
      strcat (tempstring, ".rms");
      tempfile = fopen (tempstring, "r");
      if (tempfile == NULL)
		  compute_rms_now = 1;
    }

  if (!compute_rms_now)		/* Right RMS format */
    {
      fgets (tempstring, 100, tempfile);
      if (strncasecmp (tempstring, "GramoFile Binary RMS Data", 25))
	{
	  fclose (tempfile);
	  compute_rms_now = 1;
	}
    }

  if (!compute_rms_now)		/* Read blocklen */
    {
      if (fread (&templong, sizeof (long), 1, tempfile) < 1)
	{
	  fclose (tempfile);
	  compute_rms_now = 1;
	}
    }

  if (!compute_rms_now)		/* blocklen same? */
    {
      if (templong != blocklen)
	{
	  fclose (tempfile);
	  compute_rms_now = 1;
	}
    }

  if (!compute_rms_now)		/* Read total_blocks */
    {
      if (fread (&templong, sizeof (long), 1, tempfile) < 1)
	{
	  fclose (tempfile);
	  compute_rms_now = 1;
	}
    }

  if (!compute_rms_now)		/* total_blocks reasonable? */
    {
      if (templong < 1)
	{
	  fclose (tempfile);
	  compute_rms_now = 1;
	}
      else
	/* NOW we're satisfied */
	{
	  total_blocks = templong;

	  rmsarray = (double *) malloc (total_blocks * sizeof (double));

	  fread (rmsarray, sizeof (double), total_blocks, tempfile);
	  fclose (tempfile);
	}
    }

  if (compute_rms_now)		/* Well, we can't be lazy always... */
    {
      if (stat (filename, &buf))
	{
	  fprintf (stderr, "Sound file [%s] could not be opened (stat).\n", filename);
	  return;
	}

      total_samples = (buf.st_size - sizeof (wavhead)) / (2 * 2);

      total_blocks = (total_samples / blocklen) + 1;

      if (!openwavsource (filename))
	{
	  fprintf (stderr, "Sound file [%s] could not be opened (openwavsource).\n", filename);
	  return;
	}

      fprintf (stderr, "Computing signal power (RMS)...\n");

      rmsarray = (double *) malloc (total_blocks * sizeof (double));

      total_samples_read = 0;
      current_block = 0;

      while (total_samples_read < total_samples)
	{
	  if (!(current_block % 5))
	    {
			fprintf(stderr, "File [%s] -> %3d %%\n", filename, (int) (current_block * 100. / total_blocks));
	    }

	  samples_read = 0;	/* Compute RMS */
	  sum_left = 0;
	  sum_right = 0;
	  for (l = 0; l < blocklen; l++)
	    if (total_samples_read < total_samples)
	      {
		sample = readsamplesource ();
		sum_left += sample.left * (long) sample.left;
		sum_right += sample.right * (long) sample.right;
		/* (long) => faster & accurate */
		samples_read++;
		total_samples_read++;
	      }

	  sum_left = sqrt (fabs (sum_left / samples_read));
	  sum_right = sqrt (fabs (sum_right / samples_read));

	  if (sum_right > sum_left)
	    sum_left = sum_right;
	  /* now left = max */

	  /* sum_left: 0..32767 but doublecheck */
	  if (sum_left < 0)
	    sum_left = 0;
	  if (sum_left > 32767)
	    sum_left = 32767;

	  rmsarray[current_block] = sum_left;	/* store in array */

	  current_block++;
	}

      closewavsource ();

      if (make_use_rms)		/* Write .RMS if requested */
	{
	  strcpy (tempstring, filename);
	  strcat (tempstring, ".rms");
	  tempfile = fopen (tempstring, "w");
	  fprintf (tempfile, "GramoFile Binary RMS Data\n");
	  fwrite (&blocklen, sizeof (long), 1, tempfile);
	  fwrite (&total_blocks, sizeof (long), 1, tempfile);
	  fwrite (rmsarray, sizeof (double), total_blocks, tempfile);
	  fclose (tempfile);
	}

    }				/* if compute_rms */


  /* RMS data available now. Start real work... */

  medarray = (double *) malloc (total_blocks * sizeof (double));
  sortarray = (double *) malloc (total_blocks * sizeof (double));

  medarray[0] = rmsarray[0];	/* Take Median-3 */
  medarray[total_blocks - 1] = rmsarray[total_blocks - 1];

  /* A more or less optimized version of a median computation... */
  for (l = 1; l < total_blocks - 1; l++)
    if (rmsarray[l - 1] < rmsarray[l])
      /* a < b */
      {
	if (rmsarray[l] < rmsarray[l + 1])
	  /* b < c */
	  medarray[l] = rmsarray[l];
	else
	  /* b > c */
	  {
	    if (rmsarray[l - 1] < rmsarray[l + 1])
	      /* a < c */
	      medarray[l] = rmsarray[l + 1];
	    else
	      /* c > a */
	      medarray[l] = rmsarray[l - 1];
	  }
      }
    else
      /* a > b */
      {
	if (rmsarray[l] < rmsarray[l + 1])
	  /* b < c */
	  {
	    if (rmsarray[l - 1] < rmsarray[l + 1])
	      /* a < c */
	      medarray[l] = rmsarray[l - 1];
	    else
	      /* a > c */
	      medarray[l] = rmsarray[l + 1];
	  }
	else
	  /* b > c */
	  medarray[l] = rmsarray[l];
      }

/* #define: up, up, up! */
#ifdef COMPUTE_GLOBAL_MEAN
  global_mean = 0;		/* Global Mean - Not used any more */
  for (l = 0; l < total_blocks; l++)
    global_mean += medarray[l];
  global_mean /= total_blocks;

  /* fprintf(stderr, "\nGlobMean:%f\n",global_mean); sleep(5); */
#endif

  for (l = 0; l < total_blocks; l++)	/* Fill arrays */
    sortarray[l] = medarray[l];

  qsort2double (sortarray, total_blocks);	/* Sort meds. Fast! */


  min_poss_threshold = sortarray[10];
  max_poss_threshold = sortarray[total_blocks / 2];


#define dontDISPLAY_SOME_THRESHOLDS
#ifdef DISPLAY_SOME_THRESHOLDS
  fprintf (stderr, "\n\r%f to %f\n\r", min_poss_threshold, max_poss_threshold);
#define NUMTRIES 50
  for (l = 0; l < NUMTRIES; l++)
    {
      /* Try some different thresholds between min_poss en max_poss;
         you need a looong xterm to see all of it ;-) */
      global_silence_threshold = ((max_poss_threshold - min_poss_threshold)
				  / (NUMTRIES - 1))
	* l
	+ min_poss_threshold;
      tracksplit_findtracks (medarray,
			     total_blocks,
			     global_silence_threshold,
			     local_silence_threshold,
			     min_silence_blocks,
			     min_track_blocks,
			     extra_blocks_start,
			     extra_blocks_end,

			     trackstarts,
			     trackends,
			     &detected_tracks);
      fprintf (stderr, "Glob.sil.thr.: %f  Tracks: %d\n\r",
	       global_silence_threshold, detected_tracks);
    }
#endif /* DISPLAY_SOME_THRESHOLDS */


  global_silence_threshold = (max_poss_threshold - min_poss_threshold)
    * (global_silence_factor / 1000.)
    + min_poss_threshold;

  /* fprintf(stderr, "New global threshold: %f\n\r",
     global_silence_threshold); */

  tracksplit_findtracks (medarray,
			 total_blocks,
			 global_silence_threshold,
			 local_silence_threshold,
			 min_silence_blocks,
			 min_track_blocks,
			 extra_blocks_start,
			 extra_blocks_end,

			 trackstarts,
			 trackends,
			 &detected_tracks);

  /* sleep(5); */


  /* Write the .tracks file */
  strcpy (tempstring, filename);
  strcat (tempstring, ".tracks");
  tempfile = fopen (tempstring, "w");
  if (tempfile == NULL)
    {
      fprintf (stderr, "The .tracks file could not be written.");
      return;
    }

  fprintf (tempfile, "\
# GramoFile Tracks File\n\
#\n\
# This file contains information on track starts/ends. It is automatically\n\
# generated and will be overwritten completely by subsequent track-\n\
# splitting actions on the same audio file.\n\
\n\
# Blank lines and lines starting with `#' are ignored.\n\
\n\
[Tracks]\n\
# These values are not used (yet), but are included for reference /\n\
# regeneration purposes.\n\
");
  fprintf (tempfile, "Blocklen=%ld\n", blocklen);
  fprintf (tempfile, "Global_silence_factor=%d\n",
	   global_silence_factor);
  fprintf (tempfile, "Local_silence_factor=%d\n",
	   local_silence_threshold);
  fprintf (tempfile, "Min_silence_blocks=%d\n", min_silence_blocks);
  fprintf (tempfile, "Min_track_blocks=%d\n", min_track_blocks);
  fprintf (tempfile, "Extra_blocks_start=%d\n", extra_blocks_start);
  fprintf (tempfile, "Extra_blocks_end=%d\n", extra_blocks_end);
  fprintf (tempfile, "\
\n\
# Below are start/end times of tracks. These are used to create separate\n\
# soundfiles during signal processing. You may modify the computed values\n\
# if you disagree... The block-numbers are those used in the .med file.\n\
\n\
");

  fprintf (tempfile, "Number_of_tracks=%d\n", detected_tracks);

  for (l = 0; l < detected_tracks; l++)
    {
      fsec2hmsf ((trackends[l] - trackstarts[l] + 1) *
		 (double) blocklen / 44100.,
		 tempstring);
      fprintf (tempfile,
	       "\n# Track %ld - blocks %ld to %ld - length: %s\n",
	       l + 1, trackstarts[l], trackends[l], tempstring);
      fsec2hmsf (trackstarts[l] * (double) blocklen / 44100.,
		 tempstring);
      fprintf (tempfile, "Track%02ldstart=%s\n", l + 1, tempstring);
      fsec2hmsf ((trackends[l] + 1) * (double) blocklen / 44100.,
		 tempstring);
      fprintf (tempfile, "Track%02ldend=%s\n", l + 1, tempstring);
    }

  fprintf (tempfile, "\n");
  fclose (tempfile);


  if (make_graphs)		/* Write graphs */
    {
      strcpy (tempstring, filename);
      strcat (tempstring, ".med");
      tempfile2 = fopen (tempstring, "w");
      fprintf (tempfile2, "Med(RMS(signal))\n");
      fprintf (tempfile2, "Threshold: %f\n\n", global_silence_threshold);
      for (l = 0; l < total_blocks; l++)
	{
	  fprintf (tempfile2, "%5ld:%8.2f ", l, medarray[l]);
	  for (i = 0; i < fabs (medarray[l] / 80); i++)
	    fprintf (tempfile2, "=");
	  fprintf (tempfile2, "\n");
	}
      fclose (tempfile2);

      strcpy (tempstring, filename);
      strcat (tempstring, ".sor");
      tempfile2 = fopen (tempstring, "w");
      fprintf (tempfile2, "Sort(Med(RMS(signal)))\n");
      fprintf (tempfile2, "Threshold: %f\n\n", global_silence_threshold);
      for (l = 0; l < total_blocks; l++)
	{
	  fprintf (tempfile2, "%5ld:%8.2f ", l, sortarray[l]);
	  for (i = 0; i < fabs (sortarray[l] / 80); i++)
	    fprintf (tempfile2, "=");
	  fprintf (tempfile2, "\n");
	}
      fclose (tempfile2);

    }


  sprintf (tempstring, "%d tracks have been detected. More information is in the `.tracks' file.\n", detected_tracks);
  fprintf (stderr, tempstring);

  free (rmsarray);
  free (medarray);
  free (sortarray);
}
