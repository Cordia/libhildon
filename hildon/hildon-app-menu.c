/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
 *
 * Contact: Rodrigo Novo <rodrigo.novo@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 */

/**
 * SECTION:hildon-app-menu
 * @short_description: Application menu for Hildon applications.
 *
 * #HildonAppMenu is an application menu for applications in the Hildon
 * framework.
 *
 * This menu opens from the top of the screen and contains a number of
 * entries (#GtkButton) organized in one or two columns, depending on
 * the size of the screen (the number of columns changes automatically
 * if the screen is resized). Entries are added left to right and top
 * to bottom.
 *
 * Besides that, #HildonAppMenu can contain a group of filter buttons
 * (#GtkToggleButton or #GtkRadioButton). Filters are meant to change
 * the way data is presented in the application, rather than change
 * the layout of the menu itself. For example, a file manager can have
 * filters to decide the order used to display a list of files (name,
 * date, size, etc.).
 *
 * To use a #HildonAppMenu, add it to a #HildonWindow using
 * hildon_window_set_app_menu(). The menu will appear when the user
 * presses the window title bar. Alternatively, you can show it by
 * hand using hildon_app_menu_popup().
 *
 * The menu will be automatically hidden when one of its buttons is
 * clicked. Use g_signal_connect_after() when connecting callbacks to
 * buttons to make sure that they're called after the menu
 * disappears. Alternatively, you can add the button to the menu
 * before connecting any callback.
 *
 * Although implemented with a #GtkWindow, #HildonAppMenu behaves like
 * a normal ref-counted widget, so g_object_ref(), g_object_unref(),
 * g_object_ref_sink() and friends will behave just like with any
 * other non-toplevel widget.
 *
 * <example>
 * <title>Creating a HildonAppMenu</title>
 * <programlisting>
 * GtkWidget *win;
 * HildonAppMenu *menu;
 * GtkWidget *button;
 * GtkWidget *filter;
 * <!-- -->
 * win = hildon_stackable_window_new ();
 * menu = HILDON_APP_MENU (hildon_app_menu_new ());
 * <!-- -->
 * // Create a button and add it to the menu
 * button = gtk_button_new_with_label ("Menu command one");
 * g_signal_connect_after (button, "clicked", G_CALLBACK (button_one_clicked), userdata);
 * hildon_app_menu_append (menu, GTK_BUTTON (button));
 * <!-- -->
 * // Another button
 * button = gtk_button_new_with_label ("Menu command two");
 * g_signal_connect_after (button, "clicked", G_CALLBACK (button_two_clicked), userdata);
 * hildon_app_menu_append (menu, GTK_BUTTON (button));
 * <!-- -->
 * // Create a filter and add it to the menu
 * filter = gtk_radio_button_new_with_label (NULL, "Filter one");
 * gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (filter), FALSE);
 * g_signal_connect_after (filter, "clicked", G_CALLBACK (filter_one_clicked), userdata);
 * hildon_app_menu_add_filter (menu, GTK_BUTTON (filter));
 * <!-- -->
 * // Add a new filter
 * filter = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (filter), "Filter two");
 * gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (filter), FALSE);
 * g_signal_connect_after (filter, "clicked", G_CALLBACK (filter_two_clicked), userdata);
 * hildon_app_menu_add_filter (menu, GTK_BUTTON (filter));
 * <!-- -->
 * // Show all menu items
 * gtk_widget_show_all (GTK_WIDGET (menu));
 * <!-- -->
 * // Add the menu to the window
 * hildon_window_set_app_menu (HILDON_WINDOW (win), menu);
 * </programlisting>
 * </example>
 *
 */

#include                                        <string.h>
#include                                        <X11/Xatom.h>
#include                                        <gdk/gdkx.h>

#include                                        "hildon-gtk.h"
#include                                        "hildon-app-menu.h"
#include                                        "hildon-app-menu-private.h"
#include                                        "hildon-window.h"
#include                                        "hildon-banner.h"
#include                                        "hildon-animation-actor.h"

static void
hildon_app_menu_repack_items                    (HildonAppMenu *menu,
                                                 gint           start_from);

static void
hildon_app_menu_repack_filters                  (HildonAppMenu *menu);

static gboolean
can_activate_accel                              (GtkWidget *widget,
                                                 guint      signal_id,
                                                 gpointer   user_data);

static void
item_visibility_changed                         (GtkWidget     *item,
                                                 GParamSpec    *arg1,
                                                 HildonAppMenu *menu);

static void
filter_visibility_changed                       (GtkWidget     *item,
                                                 GParamSpec    *arg1,
                                                 HildonAppMenu *menu);

static void
remove_item_from_list                           (GList    **list,
                                                 gpointer   item);

static void
emit_menu_changed                               (HildonAppMenu *menu,
                                                 gpointer item);

static void
hildon_app_menu_apply_style                     (GtkWidget *widget);

G_DEFINE_TYPE (HildonAppMenu, hildon_app_menu, GTK_TYPE_WINDOW);

enum
{
    CHANGED,
    LAST_SIGNAL
};

static guint app_menu_signals[LAST_SIGNAL] = { 0 };

/**
 * hildon_app_menu_new:
 *
 * Creates a new #HildonAppMenu.
 *
 * Return value: A #HildonAppMenu.
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_app_menu_new                             (void)
{
    GtkWidget *menu = g_object_new (HILDON_TYPE_APP_MENU, NULL);
    return menu;
}

/**
 * hildon_app_menu_insert:
 * @menu : A #HildonAppMenu
 * @item : A #GtkButton to add to the #HildonAppMenu
 * @position : The position in the item list where @item is added (from 0 to n-1).
 *
 * Adds @item to @menu at the position indicated by @position.
 *
 * Since: 2.2
 */
void
hildon_app_menu_insert                          (HildonAppMenu *menu,
                                                 GtkButton     *item,
                                                 gint           position)
{
    HildonAppMenuPrivate *priv;

    g_return_if_fail (HILDON_IS_APP_MENU (menu));
    g_return_if_fail (GTK_IS_BUTTON (item));

    priv = HILDON_APP_MENU_GET_PRIVATE(menu);

    /* Force widget size */
    hildon_gtk_widget_set_theme_size (GTK_WIDGET (item),
                                      HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);

    /* Add the item to the menu */
    g_object_ref_sink (item);
    priv->buttons = g_list_insert (priv->buttons, item, position);
    if (GTK_WIDGET_VISIBLE (item))
        hildon_app_menu_repack_items (menu, position);

    /* Enable accelerators */
    g_signal_connect (item, "can-activate-accel", G_CALLBACK (can_activate_accel), NULL);

    /* Close the menu when the button is clicked */
    g_signal_connect_swapped (item, "clicked", G_CALLBACK (gtk_widget_hide), menu);
    g_signal_connect (item, "notify::visible", G_CALLBACK (item_visibility_changed), menu);

    /* Remove item from list when it is destroyed */
    g_object_weak_ref (G_OBJECT (item), (GWeakNotify) remove_item_from_list, &(priv->buttons));
    g_object_weak_ref (G_OBJECT (item), (GWeakNotify) emit_menu_changed, menu);

    g_signal_emit (menu, app_menu_signals[CHANGED], 0);
}

/**
 * hildon_app_menu_append:
 * @menu : A #HildonAppMenu
 * @item : A #GtkButton to add to the #HildonAppMenu
 *
 * Adds @item to the end of the menu's item list.
 *
 * Since: 2.2
 */
void
hildon_app_menu_append                          (HildonAppMenu *menu,
                                                 GtkButton     *item)
{
    hildon_app_menu_insert (menu, item, -1);
}

/**
 * hildon_app_menu_prepend:
 * @menu : A #HildonAppMenu
 * @item : A #GtkButton to add to the #HildonAppMenu
 *
 * Adds @item to the beginning of the menu's item list.
 *
 * Since: 2.2
 */
void
hildon_app_menu_prepend                         (HildonAppMenu *menu,
                                                 GtkButton     *item)
{
    hildon_app_menu_insert (menu, item, 0);
}

/**
 * hildon_app_menu_reorder_child:
 * @menu : A #HildonAppMenu
 * @item : A #GtkButton to move
 * @position : The new position to place @item (from 0 to n-1).
 *
 * Moves a #GtkButton to a new position within #HildonAppMenu.
 *
 * Since: 2.2
 */
void
hildon_app_menu_reorder_child                   (HildonAppMenu *menu,
                                                 GtkButton     *item,
                                                 gint           position)
{
    HildonAppMenuPrivate *priv;
    gint old_position;

    g_return_if_fail (HILDON_IS_APP_MENU (menu));
    g_return_if_fail (GTK_IS_BUTTON (item));
    g_return_if_fail (position >= 0);

    priv = HILDON_APP_MENU_GET_PRIVATE (menu);
    old_position = g_list_index (priv->buttons, item);

    g_return_if_fail (old_position >= 0);

    /* Move the item */
    priv->buttons = g_list_remove (priv->buttons, item);
    priv->buttons = g_list_insert (priv->buttons, item, position);

    hildon_app_menu_repack_items (menu, MIN (old_position, position));
}

/**
 * hildon_app_menu_add_filter:
 * @menu : A #HildonAppMenu
 * @filter : A #GtkButton to add to the #HildonAppMenu.
 *
 * Adds the @filter to @menu.
 *
 * Since: 2.2
 */
void
hildon_app_menu_add_filter                      (HildonAppMenu *menu,
                                                 GtkButton *filter)
{
    HildonAppMenuPrivate *priv;

    g_return_if_fail (HILDON_IS_APP_MENU (menu));
    g_return_if_fail (GTK_IS_BUTTON (filter));

    priv = HILDON_APP_MENU_GET_PRIVATE(menu);

    /* Force widget size */
    hildon_gtk_widget_set_theme_size (GTK_WIDGET (filter),
                                      HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);

    /* Add the filter to the menu */
    g_object_ref_sink (filter);
    priv->filters = g_list_append (priv->filters, filter);
    if (GTK_WIDGET_VISIBLE (filter))
        hildon_app_menu_repack_filters (menu);

    /* Enable accelerators */
    g_signal_connect (filter, "can-activate-accel", G_CALLBACK (can_activate_accel), NULL);

    /* Close the menu when the button is clicked */
    g_signal_connect_swapped (filter, "clicked", G_CALLBACK (gtk_widget_hide), menu);
    g_signal_connect (filter, "notify::visible", G_CALLBACK (filter_visibility_changed), menu);

    /* Remove filter from list when it is destroyed */
    g_object_weak_ref (G_OBJECT (filter), (GWeakNotify) remove_item_from_list, &(priv->filters));
    g_object_weak_ref (G_OBJECT (filter), (GWeakNotify) emit_menu_changed, menu);

    g_signal_emit (menu, app_menu_signals[CHANGED], 0);
}

static void
hildon_app_menu_set_columns                     (HildonAppMenu *menu,
                                                 guint          columns)
{
    HildonAppMenuPrivate *priv;

    g_return_if_fail (HILDON_IS_APP_MENU (menu));
    g_return_if_fail (columns > 0);

    priv = HILDON_APP_MENU_GET_PRIVATE (menu);

    if (columns != priv->columns) {
        priv->columns = columns;
        hildon_app_menu_repack_items (menu, 0);
    }
}

static void
parent_window_topmost_notify                   (HildonWindow *parent_win,
                                                GParamSpec   *arg1,
                                                GtkWidget    *menu)
{
    if (!hildon_window_get_is_topmost (parent_win))
        gtk_widget_hide (menu);
}

static void
parent_window_unmapped                         (HildonWindow *parent_win,
                                                GtkWidget    *menu)
{
    gtk_widget_hide (menu);
}

void G_GNUC_INTERNAL
hildon_app_menu_set_parent_window              (HildonAppMenu *self,
                                                GtkWindow     *parent_window)
{
    HildonAppMenuPrivate *priv;

    g_return_if_fail (HILDON_IS_APP_MENU (self));
    g_return_if_fail (parent_window == NULL || GTK_IS_WINDOW (parent_window));

    priv = HILDON_APP_MENU_GET_PRIVATE(self);

    /* Disconnect old handlers, if any */
    if (priv->parent_window) {
        g_signal_handlers_disconnect_by_func (priv->parent_window, parent_window_topmost_notify, self);
        g_signal_handlers_disconnect_by_func (priv->parent_window, parent_window_unmapped, self);
    }

    /* Connect a new handler */
    if (parent_window) {
        g_signal_connect (parent_window, "notify::is-topmost", G_CALLBACK (parent_window_topmost_notify), self);
        g_signal_connect (parent_window, "unmap", G_CALLBACK (parent_window_unmapped), self);
    }

    priv->parent_window = parent_window;

    if (parent_window == NULL && GTK_WIDGET_VISIBLE (self))
        gtk_widget_hide (GTK_WIDGET (self));
}

gpointer G_GNUC_INTERNAL
hildon_app_menu_get_parent_window              (HildonAppMenu *self)
{
    HildonAppMenuPrivate *priv;

    g_return_val_if_fail (HILDON_IS_APP_MENU (self), NULL);

    priv = HILDON_APP_MENU_GET_PRIVATE (self);

    return priv->parent_window;
}

static void
screen_size_changed                            (GdkScreen     *screen,
                                                HildonAppMenu *menu)
{
    hildon_app_menu_apply_style (GTK_WIDGET (menu));

    if (gdk_screen_get_width (screen) > gdk_screen_get_height (screen)) {
        hildon_app_menu_set_columns (menu, 2);
    } else {
        hildon_app_menu_set_columns (menu, 1);
    }
}

static gboolean
can_activate_accel                              (GtkWidget *widget,
                                                 guint      signal_id,
                                                 gpointer   user_data)
{
    return GTK_WIDGET_VISIBLE (widget);
}

static void
item_visibility_changed                         (GtkWidget     *item,
                                                 GParamSpec    *arg1,
                                                 HildonAppMenu *menu)
{
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE (menu);

    if (! priv->inhibit_repack)
        hildon_app_menu_repack_items (menu, g_list_index (priv->buttons, item));
    g_signal_emit (menu, app_menu_signals[CHANGED], 0);
}

static void
filter_visibility_changed                       (GtkWidget     *item,
                                                 GParamSpec    *arg1,
                                                 HildonAppMenu *menu)
{
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE (menu);

    if (! priv->inhibit_repack)
        hildon_app_menu_repack_filters (menu);
    g_signal_emit (menu, app_menu_signals[CHANGED], 0);
}

static void
remove_item_from_list                           (GList    **list,
                                                 gpointer   item)
{
    *list = g_list_remove (*list, item);
}

static void
emit_menu_changed                               (HildonAppMenu *menu,
                                                 gpointer item)
{
    g_signal_emit (menu, app_menu_signals[CHANGED], 0);
}

static void
hildon_app_menu_show_all                        (GtkWidget *widget)
{
    HildonAppMenu *menu = HILDON_APP_MENU (widget);
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE (widget);

    priv->inhibit_repack = TRUE;

    /* Show children, but not self. */
    g_list_foreach (priv->buttons, (GFunc) gtk_widget_show_all, NULL);
    g_list_foreach (priv->filters, (GFunc) gtk_widget_show_all, NULL);

    priv->inhibit_repack = FALSE;

    hildon_app_menu_repack_items (menu, 0);
    hildon_app_menu_repack_filters (menu);
}


static void
hildon_app_menu_hide_all                        (GtkWidget *widget)
{
    HildonAppMenu *menu = HILDON_APP_MENU (widget);
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE (widget);

    priv->inhibit_repack = TRUE;

    /* Hide children, but not self. */
    g_list_foreach (priv->buttons, (GFunc) gtk_widget_hide_all, NULL);
    g_list_foreach (priv->filters, (GFunc) gtk_widget_hide_all, NULL);

    priv->inhibit_repack = FALSE;

    hildon_app_menu_repack_items (menu, 0);
    hildon_app_menu_repack_filters (menu);
}

/*
 * There's a race condition that can freeze the UI if a dialog appears
 * between a HildonAppMenu and its parent window, see NB#100468
 */
static gboolean
hildon_app_menu_find_intruder                   (gpointer data)
{
    GtkWidget *widget = GTK_WIDGET (data);
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE (widget);

    priv->find_intruder_idle_id = 0;

    /* If there's a modal window between the menu and its parent window, hide the menu */
    if (priv->parent_window) {
        gboolean intruder_found = FALSE;
        GdkScreen *screen = gtk_widget_get_screen (widget);
        GList *stack = gdk_screen_get_window_stack (screen);
        GList *parent_pos = g_list_find (stack, GTK_WIDGET (priv->parent_window)->window);
        GList *toplevels = gtk_window_list_toplevels ();
        GList *i;

        for (i = toplevels; i != NULL && !intruder_found; i = i->next) {
            if (i->data != widget && i->data != priv->parent_window) {
                if (g_list_find (parent_pos, GTK_WIDGET (i->data)->window)) {
                    /* HildonBanners are not closed automatically when
                     * a new window appears, so we must close them by
                     * hand to make the AppMenu work as expected.
                     * Yes, this is a hack. See NB#111027 */
                    if (HILDON_IS_BANNER (i->data)) {
                        gtk_widget_hide (i->data);
                    } else if (GTK_IS_WINDOW (i->data)) {
                        intruder_found = gtk_window_get_modal (i->data);
                    }
                }
            }
        }

        g_list_foreach (stack, (GFunc) g_object_unref, NULL);
        g_list_free (stack);
        g_list_free (toplevels);

        if (intruder_found)
            gtk_widget_hide (widget);
    }

    return FALSE;
}

static void
hildon_app_menu_map                             (GtkWidget *widget)
{
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE(widget);

    GTK_WIDGET_CLASS (hildon_app_menu_parent_class)->map (widget);

    if (priv->find_intruder_idle_id == 0)
        priv->find_intruder_idle_id = gdk_threads_add_timeout_full (
            G_PRIORITY_DEFAULT_IDLE, 100, hildon_app_menu_find_intruder,
            g_object_ref (widget), g_object_unref);
}

static void
hildon_app_menu_grab_notify                     (GtkWidget *widget,
                                                 gboolean   was_grabbed)
{
    if (GTK_WIDGET_CLASS (hildon_app_menu_parent_class)->grab_notify)
        GTK_WIDGET_CLASS (hildon_app_menu_parent_class)->grab_notify (widget, was_grabbed);

    if (!was_grabbed && GTK_WIDGET_VISIBLE (widget))
        gtk_widget_hide (widget);
}

static gboolean
hildon_app_menu_hide_idle                       (gpointer widget)
{
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE (widget);
    gtk_widget_hide (GTK_WIDGET (widget));
    priv->hide_idle_id = 0;
    return FALSE;
}

/* Send keyboard accelerators to the parent window, if necessary.
 * This code is heavily based on gtk_menu_key_press ()
 */
static gboolean
hildon_app_menu_key_press                       (GtkWidget   *widget,
                                                 GdkEventKey *event)
{
    GtkWindow *parent_window;
    HildonAppMenuPrivate *priv;

    g_return_val_if_fail (HILDON_IS_APP_MENU (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    if (GTK_WIDGET_CLASS (hildon_app_menu_parent_class)->key_press_event (widget, event))
        return TRUE;

    priv = HILDON_APP_MENU_GET_PRIVATE (widget);
    parent_window = priv->parent_window;

    if (parent_window) {
        guint accel_key, accel_mods;
        GdkModifierType consumed_modifiers;
        GdkDisplay *display;
        GSList *accel_groups;
        GSList *list;

        display = gtk_widget_get_display (widget);

        /* Figure out what modifiers went into determining the key symbol */
        gdk_keymap_translate_keyboard_state (gdk_keymap_get_for_display (display),
                                             event->hardware_keycode, event->state, event->group,
                                             NULL, NULL, NULL, &consumed_modifiers);

        accel_key = gdk_keyval_to_lower (event->keyval);
        accel_mods = event->state & gtk_accelerator_get_default_mod_mask () & ~consumed_modifiers;

        /* If lowercasing affects the keysym, then we need to include SHIFT in the modifiers,
         * We re-upper case when we match against the keyval, but display and save in caseless form.
         */
        if (accel_key != event->keyval)
            accel_mods |= GDK_SHIFT_MASK;

        accel_groups = gtk_accel_groups_from_object (G_OBJECT (parent_window));

        for (list = accel_groups; list; list = list->next) {
            GtkAccelGroup *accel_group = list->data;

            if (gtk_accel_group_query (accel_group, accel_key, accel_mods, NULL)) {
                gtk_window_activate_key (parent_window, event);
                priv->hide_idle_id = gdk_threads_add_idle (hildon_app_menu_hide_idle, widget);
                break;
            }
        }
    }

    return TRUE;
}

static gboolean
hildon_app_menu_delete_event_handler            (GtkWidget   *widget,
                                                 GdkEventAny *event)
{
    /* Hide the menu if it receives a delete-event, but don't destroy it */
    gtk_widget_hide (widget);
    return TRUE;
}

static void
hildon_app_menu_size_request                    (GtkWidget      *widget,
                                                 GtkRequisition *requisition)
{
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE (widget);

    GTK_WIDGET_CLASS (hildon_app_menu_parent_class)->size_request (widget, requisition);

    requisition->width = priv->width_request;
}

static void
hildon_app_menu_realize                         (GtkWidget *widget)
{
    Atom property, window_type;
    Display *xdisplay;
    GdkDisplay *gdkdisplay;
    GdkScreen *screen;

    GTK_WIDGET_CLASS (hildon_app_menu_parent_class)->realize (widget);

    gdk_window_set_decorations (widget->window, GDK_DECOR_BORDER);

    gdkdisplay = gdk_drawable_get_display (widget->window);
    xdisplay = GDK_WINDOW_XDISPLAY (widget->window);

    property = gdk_x11_get_xatom_by_name_for_display (gdkdisplay, "_NET_WM_WINDOW_TYPE");
    window_type = XInternAtom (xdisplay, "_HILDON_WM_WINDOW_TYPE_APP_MENU", False);
    XChangeProperty (xdisplay, GDK_WINDOW_XID (widget->window), property,
                     XA_ATOM, 32, PropModeReplace, (guchar *) &window_type, 1);

    /* Detect any screen changes */
    screen = gtk_widget_get_screen (widget);
    g_signal_connect (screen, "size-changed", G_CALLBACK (screen_size_changed), widget);

    /* Force menu to set the initial layout */
    screen_size_changed (screen, HILDON_APP_MENU (widget));
}

static void
hildon_app_menu_unrealize                       (GtkWidget *widget)
{
    GdkScreen *screen = gtk_widget_get_screen (widget);
    /* Disconnect "size-changed" signal handler */
    g_signal_handlers_disconnect_by_func (screen, G_CALLBACK (screen_size_changed), widget);

    GTK_WIDGET_CLASS (hildon_app_menu_parent_class)->unrealize (widget);
}

static void
hildon_app_menu_apply_style                     (GtkWidget *widget)
{
    GdkScreen *screen;
    gint filter_group_width;
    guint horizontal_spacing, vertical_spacing, filter_vertical_spacing;
    guint inner_border, external_border;
    HildonAppMenuPrivate *priv;

    priv = HILDON_APP_MENU_GET_PRIVATE (widget);

    gtk_widget_style_get (widget,
                          "horizontal-spacing", &horizontal_spacing,
                          "vertical-spacing", &vertical_spacing,
                          "filter-group-width", &filter_group_width,
                          "filter-vertical-spacing", &filter_vertical_spacing,
                          "inner-border", &inner_border,
                          "external-border", &external_border,
                          NULL);

    /* Set spacings */
    gtk_table_set_row_spacings (priv->table, vertical_spacing);
    gtk_table_set_col_spacings (priv->table, horizontal_spacing);
    gtk_box_set_spacing (priv->vbox, filter_vertical_spacing);

    /* Set inner border */
    gtk_container_set_border_width (GTK_CONTAINER (widget), inner_border);

    /* Set width of the group of filter buttons */
    gtk_widget_set_size_request (GTK_WIDGET (priv->filters_hbox), filter_group_width, -1);

    /* Compute width request */
    screen = gtk_widget_get_screen (widget);
    if (gdk_screen_get_width (screen) < gdk_screen_get_height (screen)) {
        external_border = 0;
    }
    priv->width_request = gdk_screen_get_width (screen) - external_border * 2;

    if (widget->window)
      gdk_window_move_resize (widget->window,
                              external_border, 0, 1, 1);

    gtk_widget_queue_resize (widget);
}

static void
hildon_app_menu_style_set                       (GtkWidget *widget,
                                                 GtkStyle  *previous_style)
{
    if (GTK_WIDGET_CLASS (hildon_app_menu_parent_class)->style_set)
        GTK_WIDGET_CLASS (hildon_app_menu_parent_class)->style_set (widget, previous_style);

    hildon_app_menu_apply_style (widget);
}

static void
hildon_app_menu_repack_filters                  (HildonAppMenu *menu)
{
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE(menu);
    GList *iter;

    for (iter = priv->filters; iter != NULL; iter = iter->next) {
        GtkWidget *filter = GTK_WIDGET (iter->data);
        GtkWidget *parent = gtk_widget_get_parent (filter);
        if (parent) {
            g_object_ref (filter);
            gtk_container_remove (GTK_CONTAINER (parent), filter);
        }
    }

    for (iter = priv->filters; iter != NULL; iter = iter->next) {
        GtkWidget *filter = GTK_WIDGET (iter->data);
        if (GTK_WIDGET_VISIBLE (filter)) {
            gtk_box_pack_start (GTK_BOX (priv->filters_hbox), filter, TRUE, TRUE, 0);
            g_object_unref (filter);
            /* GtkButton must be realized for accelerators to work */
            gtk_widget_realize (filter);
        }
    }
}

/*
 * When items displayed in the menu change (e.g, a new item is added,
 * an item is hidden or the list is reordered), the layout must be
 * updated. To do this we repack all items starting from a given one.
 */
static void
hildon_app_menu_repack_items                    (HildonAppMenu *menu,
                                                 gint           start_from)
{
    HildonAppMenuPrivate *priv;
    gint row, col, nvisible, i;
    GList *iter;

    priv = HILDON_APP_MENU_GET_PRIVATE(menu);

    i = nvisible = 0;
    for (iter = priv->buttons; iter != NULL; iter = iter->next) {
        /* Count number of visible items */
        if (GTK_WIDGET_VISIBLE (iter->data))
            nvisible++;
        /* Remove buttons from their parent */
        if (start_from != -1 && i >= start_from) {
            GtkWidget *item = GTK_WIDGET (iter->data);
            GtkWidget *parent = gtk_widget_get_parent (item);
            if (parent) {
                g_object_ref (item);
                gtk_container_remove (GTK_CONTAINER (parent), item);
            }
        }
        i++;
    }

    /* If items have been removed, recalculate the size of the menu */
    if (start_from != -1)
        gtk_window_resize (GTK_WINDOW (menu), 1, 1);

    /* Set the final size now to avoid unnecessary resizes later */
    if (nvisible > 0)
        gtk_table_resize (priv->table, ((nvisible - 1) / priv->columns) + 1, priv->columns);

    /* Add buttons */
    row = col = 0;
    for (iter = priv->buttons; iter != NULL; iter = iter->next) {
        GtkWidget *item = GTK_WIDGET (iter->data);
        if (GTK_WIDGET_VISIBLE (item)) {
            /* Don't add an item to the table if it's already there */
            if (gtk_widget_get_parent (item) == NULL) {
                gtk_table_attach_defaults (priv->table, item, col, col + 1, row, row + 1);
                g_object_unref (item);
                /* GtkButton must be realized for accelerators to work */
                gtk_widget_realize (item);
            }
            if (++col == priv->columns) {
                col = 0;
                row++;
            }
        }
    }

    gtk_widget_queue_draw (GTK_WIDGET (menu));
}

/**
 * hildon_app_menu_has_visible_children:
 * @menu: a #HildonAppMenu
 *
 * Returns whether this menu has any visible items
 * and/or filters. If this is %FALSE, the menu will
 * not be popped up.
 *
 * Returns: whether there are visible items or filters
 *
 * Since: 2.2
 **/
gboolean
hildon_app_menu_has_visible_children (HildonAppMenu *menu)
{
   HildonAppMenuPrivate *priv;
   GList *i;
   gboolean show_menu = FALSE;

    priv = HILDON_APP_MENU_GET_PRIVATE (menu);

    /* Don't show menu if it doesn't contain visible items */
    for (i = priv->buttons; i && !show_menu; i = i->next)
        show_menu = GTK_WIDGET_VISIBLE (i->data);

    for (i = priv->filters; i && !show_menu; i = i->next)
	    show_menu = GTK_WIDGET_VISIBLE (i->data);

    return show_menu;
}

/**
 * hildon_app_menu_popup:
 * @menu: a #HildonAppMenu
 * @parent_window: a #GtkWindow
 *
 * Displays a menu on top of a window and makes it available for
 * selection.
 *
 * Since: 2.2
 **/
void
hildon_app_menu_popup                           (HildonAppMenu *menu,
                                                 GtkWindow     *parent_window)
{
    g_return_if_fail (HILDON_IS_APP_MENU (menu));
    g_return_if_fail (GTK_IS_WINDOW (parent_window));

    if (hildon_app_menu_has_visible_children (menu)) {
        GtkWindowGroup *group;
        hildon_app_menu_set_parent_window (menu, parent_window);
        group = gtk_window_get_group (parent_window);
        gtk_window_group_add_window (group, GTK_WINDOW (menu));
        gtk_widget_show (GTK_WIDGET (menu));
    }

}

/**
 * hildon_app_menu_get_items:
 * @menu: a #HildonAppMenu
 *
 * Returns a list of all items (regular items, not filters) contained
 * in @menu.
 *
 * Returns: a newly-allocated list containing the items in @menu
 *
 * Since: 2.2
 **/
GList *
hildon_app_menu_get_items                       (HildonAppMenu *menu)
{
    HildonAppMenuPrivate *priv;

    g_return_val_if_fail (HILDON_IS_APP_MENU (menu), NULL);

    priv = HILDON_APP_MENU_GET_PRIVATE (menu);

    return g_list_copy (priv->buttons);
}

/**
 * hildon_app_menu_get_filters:
 * @menu: a #HildonAppMenu
 *
 * Returns a list of all filters contained in @menu.
 *
 * Returns: a newly-allocated list containing the filters in @menu
 *
 * Since: 2.2
 **/
GList *
hildon_app_menu_get_filters                     (HildonAppMenu *menu)
{
    HildonAppMenuPrivate *priv;

    g_return_val_if_fail (HILDON_IS_APP_MENU (menu), NULL);

    priv = HILDON_APP_MENU_GET_PRIVATE (menu);

    return g_list_copy (priv->filters);
}

static void
hildon_app_menu_init                            (HildonAppMenu *menu)
{
    GtkWidget *alignment;
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE(menu);

    /* Initialize private variables */
    priv->parent_window = NULL;
    priv->transfer_window = NULL;
    priv->pressed_outside = FALSE;
    priv->inhibit_repack = FALSE;
    priv->last_pressed_button = NULL;
    priv->buttons = NULL;
    priv->filters = NULL;
    priv->columns = 2;
    priv->width_request = -1;
    priv->find_intruder_idle_id = 0;
    priv->hide_idle_id = 0;

    /* Create boxes and tables */
    priv->filters_hbox = GTK_BOX (gtk_hbox_new (TRUE, 0));
    priv->vbox = GTK_BOX (gtk_vbox_new (FALSE, 0));
    priv->table = GTK_TABLE (gtk_table_new (1, priv->columns, TRUE));

    /* Align the filters to the center */
    alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment), GTK_WIDGET (priv->filters_hbox));

    /* Pack everything */
    gtk_container_add (GTK_CONTAINER (menu), GTK_WIDGET (priv->vbox));
    gtk_box_pack_start (priv->vbox, alignment, TRUE, TRUE, 0);
    gtk_box_pack_start (priv->vbox, GTK_WIDGET (priv->table), TRUE, TRUE, 0);

    /* This should be treated like a normal, ref-counted widget */
    g_object_force_floating (G_OBJECT (menu));
    GTK_WINDOW (menu)->has_user_ref_count = FALSE;

    gtk_widget_show_all (GTK_WIDGET (priv->vbox));
}


static void
disconnect_weak_refs (gpointer data,
		      gpointer user_data)
{
	g_object_weak_unref (G_OBJECT (data),
			     (GWeakNotify) emit_menu_changed,
			     user_data);
}

static void
hildon_app_menu_dispose                         (GObject *object)

{
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE(object);

    g_list_foreach (priv->buttons, (GFunc) disconnect_weak_refs, object);
    g_list_foreach (priv->filters, (GFunc) disconnect_weak_refs, object);

    G_OBJECT_CLASS (hildon_app_menu_parent_class)->dispose (object);
}

static void
hildon_app_menu_finalize                        (GObject *object)
{
    HildonAppMenuPrivate *priv = HILDON_APP_MENU_GET_PRIVATE(object);

    if (priv->find_intruder_idle_id) {
        g_source_remove (priv->find_intruder_idle_id);
        priv->find_intruder_idle_id = 0;
    }

    if (priv->hide_idle_id) {
        g_source_remove (priv->hide_idle_id);
        priv->hide_idle_id = 0;
    }

    if (priv->parent_window) {
        g_signal_handlers_disconnect_by_func (priv->parent_window, parent_window_topmost_notify, object);
        g_signal_handlers_disconnect_by_func (priv->parent_window, parent_window_unmapped, object);
    }

    if (priv->transfer_window)
        gdk_window_destroy (priv->transfer_window);

    g_list_foreach (priv->buttons, (GFunc) g_object_unref, NULL);
    g_list_foreach (priv->filters, (GFunc) g_object_unref, NULL);

    g_list_free (priv->buttons);
    g_list_free (priv->filters);

    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (hildon_app_menu_parent_class)->finalize (object);
}

static void
hildon_app_menu_class_init                      (HildonAppMenuClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;

    gobject_class->dispose = hildon_app_menu_dispose;
    gobject_class->finalize = hildon_app_menu_finalize;
    widget_class->show_all = hildon_app_menu_show_all;
    widget_class->hide_all = hildon_app_menu_hide_all;
    widget_class->map = hildon_app_menu_map;
    widget_class->realize = hildon_app_menu_realize;
    widget_class->unrealize = hildon_app_menu_unrealize;
    widget_class->grab_notify = hildon_app_menu_grab_notify;
    widget_class->key_press_event = hildon_app_menu_key_press;
    widget_class->style_set = hildon_app_menu_style_set;
    widget_class->delete_event = hildon_app_menu_delete_event_handler;
    widget_class->size_request = hildon_app_menu_size_request;

    g_type_class_add_private (klass, sizeof (HildonAppMenuPrivate));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "horizontal-spacing",
            "Horizontal spacing on menu items",
            "Horizontal spacing between each menu item. Does not apply to filter buttons.",
            0, G_MAXUINT, 16,
            G_PARAM_READABLE));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "vertical-spacing",
            "Vertical spacing on menu items",
            "Vertical spacing between each menu item. Does not apply to filter buttons.",
            0, G_MAXUINT, 16,
            G_PARAM_READABLE));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_int (
            "filter-group-width",
            "Width of the group of filter buttons",
            "Total width of the group of filter buttons, "
            "or -1 to use the natural size request.",
            -1, G_MAXINT, 444,
            G_PARAM_READABLE));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "filter-vertical-spacing",
            "Vertical spacing between filters and menu items",
            "Vertical spacing between filters and menu items",
            0, G_MAXUINT, 8,
            G_PARAM_READABLE));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "inner-border",
            "Border between menu edges and buttons",
            "Border between menu edges and buttons",
            0, G_MAXUINT, 16,
            G_PARAM_READABLE));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "external-border",
            "Border between menu and screen edges (in horizontal mode)",
            "Border between the right and left edges of the menu and "
            "the screen edges (in horizontal mode)",
            0, G_MAXUINT, 50,
            G_PARAM_READABLE));


    /**
     * HildonAppMenu::changed:
     * @widget: the widget that received the signal
     *
     * The HildonAppMenu::changed signal is emitted whenever an
     * item or filter is added or removed from the menu.
     *
     * Since: 2.2
     */
    app_menu_signals[CHANGED] =
        g_signal_new ("changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
}
