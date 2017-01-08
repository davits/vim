/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *			Overlay windows by Davit Samvelyan
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 */

#if !defined(GUI_OVERLAY_H) && defined(FEAT_OVERLAY)
#define GUI_OVERLAY_H

typedef struct OverlayStruct
{
    long   row;
    long   col;
    int    markup;

    list_T      *items;
    listwatch_T selected;
    int         sel_index;

    GtkWidget *window;
    GtkWidget *counter;
    GtkWidget *nav;
    GtkWidget *info;
} Overlay;

#include "gui_overlay.pro"

#endif /* GUI_OVERLAY_H and FEAT_GUI_GTK */
