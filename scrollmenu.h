/* Scrolling Menus - Header

 * Copyright (C) 1998 J.A. Bezemer
 *
 * Licensed under the terms of the GNU General Public License.
 * ABSOLUTELY NO WARRANTY.
 * See the file `COPYING' in this directory.
 */

#ifndef HAVE_SCROLLMENU_H
#define HAVE_SCROLLMENU_H


typedef struct
  {
    char **items;
    int number;
    int y;
    int x;			/* (y,x): upper left corner */
    int h;
    int w;			/* h=height, w=width */
    int selected;
    int firstonscreen;
    int last_of_1st_part;	/* 'private' for dirfile-menu */
    int hasfocus;
  }
scrollmenu_t;


extern void scrollmenu_display (scrollmenu_t * menu);

extern int scrollmenu_stdkeys (int key, scrollmenu_t * menu);
				/* Returns >0: item was selected;
				   Returns -1 if nothing serious has happened.
				 */

extern void scrollmenu_delete_menu (scrollmenu_t * menu);


#endif /* HAVE_SCROLLMENU_H */
