/* Signal Processing Main

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#include "signpr_main.h"
#include "signpr_general.h"
#include "signpr_wav.h"
#include "fmtheaders.h"
#include "secshms.h"

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

long totalsize_samples;
long current_total_sample;
long tracksize_samples;
long current_sample;

#define TRACKS_TEXT		"[Tracks]"
#define NUM_OF_TRACKS_TEXT	"Number_of_tracks="

#define MORE_VERBOSE

int
load_track_times (char *filename, beginendsample_t * tracktimes,
		  int *number_of_tracks)
/* Returns: 0: error, 1: OK */
{
  char tempstring[250];
  char filestring[250];
  char tempstring2[250];
  FILE *tracksfile;
  long tracks_pos;
  int i;
  double seconds;

  strcpy (filestring, filename);
  strcat (filestring, ".tracks");

  tracksfile = fopen (filestring, "r");

  if (tracksfile == NULL)
    return 0;

  while (!feof (tracksfile) &&
	 strncasecmp (tempstring, TRACKS_TEXT, strlen (TRACKS_TEXT)))
    fgets (tempstring, 250, tracksfile);

  if (feof (tracksfile))	/* [Tracks] on last line => stop reading */
    {
#ifdef MORE_VERBOSE
      fprintf (stderr, "[Tracks] marker not found in tracks file [%s].\n", filestring);
#endif
      fclose (tracksfile);
      return 0;
    }

  tracks_pos = ftell (tracksfile);

  tempstring[0] = '\0';
  while (!feof (tracksfile) && strncasecmp (tempstring,
			   NUM_OF_TRACKS_TEXT, strlen (NUM_OF_TRACKS_TEXT)))
    fgets (tempstring, 250, tracksfile);

  if (strncasecmp (tempstring, NUM_OF_TRACKS_TEXT,
		   strlen (NUM_OF_TRACKS_TEXT)))
    {
#ifdef MORE_VERBOSE
      fprintf (stderr, "Number_of_tracks not found in tracks file [%s].\n", filestring);
#endif
      fclose (tracksfile);
      return 0;
    }

  *number_of_tracks = atoi (tempstring + strlen (NUM_OF_TRACKS_TEXT));

  if (*number_of_tracks < 1 || *number_of_tracks > 99999)
    {
#ifdef MORE_VERBOSE
      fprintf (stderr, "Track number [%d] out of bounds.\n", *number_of_tracks);
#endif
      fclose (tracksfile);
      return 0;
    }

  for (i = 1; i <= *number_of_tracks; i++)
    {
      sprintf (tempstring2, "Track%02dstart=", i);	/* Start time */
      fseek (tracksfile, tracks_pos, SEEK_SET);

      tempstring[0] = '\0';
      while (!feof (tracksfile) && strncasecmp (tempstring,
					 tempstring2, strlen (tempstring2)))
	fgets (tempstring, 250, tracksfile);

      if (strncasecmp (tempstring, tempstring2, strlen (tempstring2)))
	{
#ifdef MORE_VERBOSE
	  fprintf (stderr, "TrackXXstart not found in tracks file [%s].\n", filestring);
#endif
	  fclose (tracksfile);
	  return 0;
	}

      tempstring[strlen (tempstring) - 1] = '\0';	/* Del \n */
      if (!hmsf2fsec (tempstring + strlen (tempstring2), &seconds))
	{
#ifdef MORE_VERBOSE
	  fprintf (stderr, "Could not convert start time [%lf] in tracks file [%s].\n", seconds, filestring);
#endif
	  fclose (tracksfile);
	  return 0;
	}

      tracktimes[i].begin = 44100 * seconds;

      sprintf (tempstring2, "Track%02dend=", i);	/* End time */
      fseek (tracksfile, tracks_pos, SEEK_SET);

      tempstring[0] = '\0';
      while (!feof (tracksfile) && strncasecmp (tempstring,
					 tempstring2, strlen (tempstring2)))
	fgets (tempstring, 250, tracksfile);

      if (strncasecmp (tempstring, tempstring2, strlen (tempstring2)))
	{
#ifdef MORE_VERBOSE
	  fprintf (stderr, "TrackXXend not found in tracks file [%s].\n", filestring);
#endif
	  fclose (tracksfile);
	  return 0;
	}

      tempstring[strlen (tempstring) - 1] = '\0';	/* Del \n */
      if (!hmsf2fsec (tempstring + strlen (tempstring2), &seconds))
	{
#ifdef MORE_VERBOSE
	  fprintf(stderr, "Could not convert end time [%lf] in tracks file [%s].\n", seconds, filestring);
#endif
	  fclose (tracksfile);
	  return 0;
	}

      tracktimes[i].end = 44100 * seconds;
    }

  fclose (tracksfile);
  return 1;
}


void
signproc_main (char *startdir, char *infilename, char *outfilename)
{
  char baseoutfilename[250];
  char outfileextension[250];

  /* The menu settings are made static to allow the selected 
     filters and settings to be remembered between invocations */

  static int first_entry = 1;
  static scrollmenu_t filtlist;
  static char *filtlist_items[MAX_FILTERS + 10];
  static int filtnumbers[MAX_FILTERS + 10];
  static char *helptexts[MAX_FILTERS + 10];
  static scrollmenu_t selectedfilts;
  static char *selectedfilts_items[MAX_FILTERS];
  static int usebeginendtime = 0, usetracktimes = 1;
  static double begintime = 0, endtime = 0;

  struct stat buf;
  int i;
  char *charptr;
  beginendsample_t tracktimes[10000];	/* max. 99999 tracks: 1 (!) - 99999 */
  int number_of_tracks;

  if (first_entry)
    {
      filtlist.items = filtlist_items;	/* malloc also works :) */
      make_filterlist (&filtlist, filtnumbers, helptexts);

      selectedfilts.items = selectedfilts_items;
      number_of_filters = 0;
      selectedfilts.number = 0;

/* Default filter: Conditional Median Filter II:

 ******* NOTE:  The exact number must be changed when new filters are added!
 */
      selectedfilts.items[number_of_filters] = filtlist.items[7];
      filter_type[number_of_filters] = filtnumbers[7];
      parampointerarray[number_of_filters] =
	(parampointer_t) malloc (sizeof (param_t));
      param_defaults (parampointerarray[number_of_filters], filtnumbers[7]);
      number_of_filters++;
      selectedfilts.number = number_of_filters;
      selectedfilts.selected = number_of_filters - 1;
/* ----- End (default filter) */

      first_entry = 0;
    }

  strcpy (baseoutfilename, outfilename);
  strcpy (outfileextension, ".wav");
  /* IF there is a last '.' AND it's after
     the last '/' THEN make a new basename
     and extension */
  if ((charptr = strrchr (outfilename, '.')) != NULL &&
      strchr (charptr, '/') == NULL
    )
    {
      baseoutfilename[charptr - outfilename] = '\0';
      strcpy (outfileextension, charptr);
    }

  if (usebeginendtime)
    {
      number_of_tracks = 1;
      tracktimes[1].begin = begintime * 44100;
      tracktimes[1].end = endtime * 44100;
    }
  else if (usetracktimes)
    {
      if (!load_track_times (infilename, tracktimes, &number_of_tracks))
	{
	  fprintf (stderr, "No (correct) track information is available for \
the specified source file [%s].\n", infilename);
	  return;
	}
    }
  else
    /* entire file */
    {
      number_of_tracks = 1;
      stat (infilename, &buf);
      tracktimes[1].begin = 0;
      tracktimes[1].end = (buf.st_size - sizeof (wavhead)) / (2 * 2);
    }

  totalsize_samples = 0;	/* calculate sample totals */
  current_total_sample = 0;
  for (i = 1; i <= number_of_tracks; i++)
    totalsize_samples += tracktimes[i].end - tracktimes[i].begin;

  if (!openwavsource (infilename)) {
	  fprintf(stderr, "Could not open infile [%s]\n", infilename);
    return;			/* open source */
   }
 

  for (i = 1; i <= number_of_tracks; i++)
    {
      fprintf (stderr, "File: [%s]  Track:   %2d of %d.\n", infilename, i, number_of_tracks);

      /* calculate #samples for this track */
      tracksize_samples = tracktimes[i].end - tracktimes[i].begin + 1;

      /* seek to beginsample */
      if (!seeksamplesource (tracktimes[i].begin))
	{
	  fprintf(stderr, "The start position of the track could not be found. This track will be skipped.\n");
	  break;
	}

      if (number_of_tracks > 1)	/* make outfile name */
	sprintf (outfilename, "%s%02d%s", baseoutfilename, i, outfileextension);

      /* open destination file */
      if (!openwavdest (outfilename, tracksize_samples * 4))
	{
	  fprintf (stderr, "The destination file [%s] could not be opened. This track will be skipped.\n", outfilename);
	  break;
	}

      init_filters ();

      /* process the signal */
      for (current_sample = 0; current_sample < tracksize_samples;
	   current_sample++)
	{

	  if (!(current_sample % 4000))
	    /* progress indicator */
	    {
	      fprintf (stderr, "File: [%s] %d/%d: ... %3ld%%\n", infilename, i, number_of_tracks, (long) ((100. * current_sample) / tracksize_samples));

	    }
	  /* process one sample */
	  writesampledest (get_sample_from_filter (number_of_filters - 1));

	  current_total_sample++;	/* update total count */
	}

      closewavdest ();		/* close destination */
      delete_filters ();	/* delete filters */
    }

  closewavsource ();		/* close source */
}
