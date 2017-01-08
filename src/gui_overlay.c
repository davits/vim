/* vi:set ts=8 sts=4 sw=4 noet:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *			Overlay windows by Davit Samvelyan
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

#include "vim.h"

#ifdef FEAT_OVERLAY

#include <gtk/gtk.h>
#include "gui_overlay.h"

static Overlay *overlay = 0;

# if GTK_CHECK_VERSION(3,0,0)
    gboolean
draw_overlay_window(GtkWidget* widget, cairo_t* cr, gpointer data)
{
    if (widget == NULL)
	return TRUE;

    GtkStyleContext *context = NULL;
    gint width = -1, height = -1;

    context = gtk_widget_get_style_context(widget);
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    gtk_style_context_save(context);

    gtk_style_context_add_class(context, "tooltip");
    gtk_style_context_set_state(context, GTK_STATE_FLAG_NORMAL);

    cairo_save(cr);
    gtk_render_frame(context, cr, 0, 0, width, height);
    gtk_render_background(context, cr, 0, 0, width, height);
    cairo_restore(cr);

    gtk_style_context_restore(context);

    return FALSE;
    return FALSE;
}

# else

    static gint
overlay_expose_event(GtkWidget *widget,
		     GdkEventExpose *event,
		     gpointer data UNUSED)
{
    gtk_paint_flat_box(widget->style, widget->window,
		       GTK_STATE_NORMAL, GTK_SHADOW_OUT,
		       &event->area, widget, "tooltip",
		       0, 0, -1, -1);

    return FALSE;
}
# endif

    static const char*
get_counter_text(int i, int count)
{
    static char buf[64];
    sprintf(buf, "%d of %d", i, count);
    return buf;
}

    static const char*
get_info_text(listitem_T* li)
{
    char* r = (char*)get_tv_string(&(li->li_tv));
    return r;
}

    static void
set_overlay_info()
{
    if (overlay->counter != 0) {
	gtk_label_set_text(GTK_LABEL(overlay->counter),
			   get_counter_text(overlay->sel_index,
					    overlay->items->lv_len));
    }
    if (overlay->markup) {
	gtk_label_set_markup(GTK_LABEL(overlay->info),
			     get_info_text(overlay->selected.lw_item));
    } else {
	gtk_label_set_text(GTK_LABEL(overlay->info),
			   get_info_text(overlay->selected.lw_item));
    }
}

    static void
get_text_area_origin_on_screen(int* x, int* y)
{
    int win_x;
    int win_y;
    gdk_window_get_origin(gtk_widget_get_window(gui.mainwin),
		          &win_x, &win_y);

    int draw_x;
    int draw_y;
    gtk_widget_translate_coordinates(gui.drawarea, gui.mainwin,
				     0, 0, &draw_x, &draw_y);

    *x = win_x + draw_x;
    *y = win_y + draw_y;
}

    static void
get_row_col_coordinates(int row, int col, int* x, int* y)
{
    if (curbuf->b_signlist != NULL) {
	// if sign bar is visible adjust column
	col += 2;
    }
    *x = gui.char_width * (col - curwin->w_leftcol - 1);
    *y = gui.char_height * (row - curwin->w_topline);
}

    static void
get_row_col_screen_coordinates(int row, int col, int* x, int* y)
{
    get_row_col_coordinates(row, col, x, y);
    int tx;
    int ty;
    get_text_area_origin_on_screen(&tx, &ty);
    *x += tx;
    *y += ty;
}

    static void
get_screen_size(Overlay* o, int* w, int* h)
{
# ifdef HAVE_GTK_MULTIHEAD
#  if GTK_CHECK_VERSION(3,22,2)
    GdkRectangle rect;
    GdkMonitor * const mon = gdk_display_get_monitor_at_window(
	    gtk_widget_get_display(o->window),
	    gtk_widget_get_window(o->window));
    gdk_monitor_get_geometry(mon, &rect);

    *w = rect.width;
    *h = rect.height;
#  else
    GdkScreen *screen;

    screen = gtk_widget_get_screen(o->window);
    gtk_window_set_screen(GTK_WINDOW(o->window), screen);
    *w = gdk_screen_get_width(screen);
    *h = gdk_screen_get_height(screen);
#  endif
# else
    *w = gdk_screen_width();
    *h = gdk_screen_height();
# endif
}

    static void
calc_overlay_coordinates(Overlay* o, int* x, int* y)
{
    int screen_w;
    int screen_h;
    get_screen_size(o, &screen_w, &screen_h);

    GtkRequisition	requisition;
# if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_get_preferred_size(o->window, &requisition, NULL);
# else
    gtk_widget_size_request(o->window, &requisition);
# endif

    get_row_col_screen_coordinates(o->row, o->col, x, y);
    *y -= requisition.height + 2;
    *x = CLAMP(*x, 0, MAX(0, screen_w - requisition.width));
}

    static void
reposition_overlay()
{
    int x;
    int y;
    calc_overlay_coordinates(overlay, &x, &y);
# if GTK_CHECK_VERSION(3,0,0)
	gtk_window_move(GTK_WINDOW(overlay->window), x, y);
# else
	gtk_widget_set_uposition(overlay->window, x, y);
# endif
}

    static void
show_prev(GtkWidget* w, gpointer data)
{
    if (overlay == 0 || overlay->items->lv_len <=1) {
	return;
    }
    if (overlay->selected.lw_item->li_prev == NULL) {
        overlay->selected.lw_item = overlay->items->lv_last;
	overlay->sel_index = overlay->items->lv_len;
    } else {
	overlay->selected.lw_item = overlay->selected.lw_item->li_prev;
	--overlay->sel_index;
    }
    set_overlay_info();
    reposition_overlay();
}

    static void
show_next(GtkWidget* w, gpointer data)
{
    if (overlay == 0 || overlay->items->lv_len <=1) {
	return;
    }
    if (overlay->selected.lw_item->li_next == NULL) {
	overlay->selected.lw_item = overlay->items->lv_first;
	overlay->sel_index = 1;
    } else {
	overlay->selected.lw_item = overlay->selected.lw_item->li_next;
	++overlay->sel_index;
    }
    set_overlay_info();
    reposition_overlay();
}

    static void
build_overlay()
{
    overlay->window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_widget_set_app_paintable(overlay->window, TRUE);

    gtk_widget_set_name(overlay->window, "gtk-overlay");

# if GTK_CHECK_VERSION(3,0,0)
    gtk_window_set_resizable(GTK_WINDOW(overlay->window), FALSE);
    g_signal_connect(G_OBJECT(overlay->window), "draw",
		     G_CALLBACK(draw_overlay_window), NULL);

    GtkWidget* info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_start(info_box, 5);
    gtk_widget_set_margin_end(info_box, 5);
    gtk_widget_set_margin_top(info_box, 5);
    gtk_widget_set_margin_bottom(info_box, 5);

    overlay->nav = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(overlay->nav, GTK_ALIGN_START);
    gtk_widget_set_valign(overlay->nav, GTK_ALIGN_END);

    GtkWidget* backward = gtk_button_new_from_icon_name("back", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_halign(backward, GTK_ALIGN_START);
    gtk_button_set_relief(GTK_BUTTON(backward), GTK_RELIEF_NONE);
    g_signal_connect(G_OBJECT(backward), "clicked",
		     G_CALLBACK(show_prev), NULL);

    overlay->counter = gtk_label_new(NULL);
    gtk_widget_set_halign(overlay->counter, GTK_ALIGN_START);

    GtkWidget* forward = gtk_button_new_from_icon_name("forward", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_halign(forward, GTK_ALIGN_START);
    gtk_button_set_relief(GTK_BUTTON(forward), GTK_RELIEF_NONE);
    g_signal_connect(G_OBJECT(forward), "clicked",
		     G_CALLBACK(show_next), NULL);

    gtk_box_pack_start(GTK_BOX(overlay->nav), backward, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(overlay->nav), overlay->counter, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(overlay->nav), forward, FALSE, FALSE, 0);

    overlay->info = gtk_label_new(NULL);
    gtk_widget_set_halign(overlay->info, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(info_box), overlay->nav, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(info_box), overlay->info, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(overlay->window), info_box);

#else
    gtk_widget_set_name(overlay->window, "gtk-tooltips"); // required for style
    gtk_window_set_policy(GTK_WINDOW(overlay->window), FALSE, FALSE, TRUE);
    gtk_signal_connect((GtkObject*)(overlay->window), "expose_event",
		       GTK_SIGNAL_FUNC(overlay_expose_event), NULL);
    gtk_container_border_width(GTK_CONTAINER(overlay->window), 5);

    GtkWidget* info_box = gtk_hbox_new(FALSE, 0);

    overlay->nav = gtk_hbox_new(FALSE, 0);

    GtkWidget* backward = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(backward), GTK_RELIEF_NONE);
    g_signal_connect(G_OBJECT(backward), "clicked",
		     G_CALLBACK(show_prev), NULL);
    gtk_button_set_image(GTK_BUTTON(backward),
			 gtk_image_new_from_stock(GTK_STOCK_GO_BACK,
						  GTK_ICON_SIZE_BUTTON));

    overlay->counter = gtk_label_new(NULL);

    GtkWidget* forward = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(forward), GTK_RELIEF_NONE);
    g_signal_connect(G_OBJECT(forward), "clicked",
		     G_CALLBACK(show_next), NULL);
    gtk_button_set_image(GTK_BUTTON(forward),
			 gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD,
						  GTK_ICON_SIZE_BUTTON));

    gtk_box_pack_start(GTK_BOX(overlay->nav), backward, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(overlay->nav), overlay->counter, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(overlay->nav), forward, FALSE, FALSE, 0);

    overlay->info = gtk_label_new(NULL);

    GtkWidget* nav_align = gtk_alignment_new(0, 1, 0, 0);
    gtk_container_add(GTK_CONTAINER(nav_align), overlay->nav);

    GtkWidget* info_align = gtk_alignment_new(0, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(info_align), overlay->info);

    gtk_box_pack_start(GTK_BOX(info_box), nav_align, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(info_box), info_align, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(overlay->window), info_box);
# endif

    gtk_widget_show_all(info_box);
}

    static void
window_moved(GtkWindow* w, GdkEventConfigure* e, gpointer data)
{
    overlay_close();
}

    void
overlay_show(long row, long col, list_T* items, int markup)
{
    if (overlay == NULL) {
	overlay = (Overlay*)alloc_clear(sizeof(Overlay));
    } else {
	// free old list of items
	list_unref(overlay->items);
    }

    overlay->row = row;
    overlay->col = col;
    ++items->lv_refcount;
    overlay->items = items;
    overlay->markup = markup;

    // hacky, add watch to prevent garbage collector from removing the list
    list_add_watch(overlay->items, &(overlay->selected));
    overlay->selected.lw_item = overlay->items->lv_first;
    overlay->sel_index = 1;

    if (overlay->window == NULL) {
	build_overlay();
    }
    if (overlay->items->lv_len > 1) {
	gtk_widget_show(overlay->nav);
    } else {
	gtk_widget_hide(overlay->nav);
    }

    set_overlay_info();
    reposition_overlay();

    gtk_widget_show(overlay->window);
}

    void
overlay_next()
{
    show_next(NULL, NULL);
}

    void
overlay_prev()
{
    show_prev(NULL, NULL);
}

    void
overlay_close()
{
    if (overlay != NULL) {
	gtk_widget_destroy(overlay->window);
	list_unref(overlay->items);
	vim_free(overlay);
	overlay = NULL;
    }
}

#endif // FEAT_OVERLAY
