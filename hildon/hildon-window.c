/*
 * This file is a part of hildon
 *
 * Copyright (C) 2006 Nokia Corporation, all rights reserved.
 *
 * Contact: Rodrigo Novo <rodrigo.novo@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/**
 * SECTION:hildon-window
 * @short_description: Top-level window in the Hildon framework.
 * @see_also: #HildonProgram, #HildonStackableWindow
 *
 * #HildonWindow is a top-level
 * window in the Hildon framework. It is derived from #GtkWindow
 * and provides additional commodities specific to the Hildon
 * framework.
 *
 * #HildonWindow<!-- -->s can have a menu attached, which is toggled
 * with a hardware key or by tapping on the window frame. This menu
 * can be either a #GtkMenu or a #HildonAppMenu (set with
 * hildon_window_set_main_menu() and hildon_window_set_app_menu()
 * respectively). Only one type of menu can be used at the same time.
 * In Hildon 2.2, #HildonAppMenu is the recommended menu to use.
 *
 * Similarly, a #HildonWindow can have several toolbars
 * attached. These can be added with hildon_window_add_toolbar(). In
 * addition to those, a #HildonWindow can also have a
 * #HildonEditToolbar. To add it to the window use
 * hildon_window_set_edit_toolbar().
 *
 * <example>
 * <title>Creating a HildonWindow</title>
 * <programlisting>
 * HildonWindow *window;
 * GtkToolbar *toolbar;
 * HildonAppMenu *menu;
 * GdkPixbuf *icon_pixbuf;
 * <!-- -->
 * window = HILDON_WINDOW (hildon_window_new());
 * <!-- -->
 * toolbar = create_toolbar();
 * <!-- -->
 * menu = create_menu();
 * <!-- -->
 * icon_pixbuf = create_icon();
 * <!-- -->
 * hildon_window_set_app_menu (window, menu);
 * <!-- -->
 * hildon_window_add_toolbar (window, toolbar);
 * <!-- -->
 * // Can be used to set the window fullscreen
 * gtk_window_fullscreen (GTK_WINDOW (window));
 * <!-- -->
 * // Used to trigger the blinking of the window's icon in the task navigator
 * gtk_window_set_urgency_hint (GTK_WINDOW (window), TRUE);
 * <!-- -->
 * // Change the window's icon in the task navigator
 * gtk_window_set_icon (GTK_WINDOW (window), icon_pixbuf);
 * </programlisting>
 * </example>
 */

#undef                                          HILDON_DISABLE_DEPRECATED

#include                                        <memory.h>
#include                                        <string.h>
#include                                        <strings.h>
#include                                        <stdio.h>
#include                                        <libintl.h>
#include                                        <X11/X.h>
#include                                        <X11/Xatom.h>
#include                                        <gdk/gdkkeysyms.h>
#include                                        <gdk/gdkx.h>
#include                                        <gtk/gtkprivate.h>

#include                                        "hildon-window.h"
#include                                        "hildon-window-private.h"
#include                                        "hildon-app-menu-private.h"
#include                                        "hildon-find-toolbar.h"
#include                                        "hildon-defines.h"
#include                                        "hildon-private.h"

#define                                         _(String) gettext(String)

#define                                         TOOLBAR_HEIGHT 70

#define                                         TOOLBAR_MIDDLE 0

/*FIXME*/
#define                                         CAN_HIBERNATE "CANKILL"

#define                                         CAN_HIBERNATE_LENGTH 7

#define                                         CAN_HIBERNATE_PROPERTY "_HILDON_ABLE_TO_HIBERNATE"

#define                                         LEGACY_MENU_PROPERTY_NAME  "_HILDON_WM_WINDOW_TYPE"
#define                                         LEGACY_MENU_PROPERTY_VALUE "_HILDON_WM_WINDOW_TYPE_LEGACY_MENU"

#define TITLE_SEPARATOR                         " - "

typedef void                                    (*HildonWindowSignal) (HildonWindow *, gint, gpointer);

static void
hildon_window_init                              (HildonWindow * self);

static void
hildon_window_class_init                        (HildonWindowClass * window_class);

static void
hildon_window_menu_popup_func                   (GtkMenu *menu, 
                                                 gint *x, 
                                                 gint *y,
                                                 gboolean *push_in,
                                                 GtkWidget *widget);
static void
hildon_window_menu_popup_func_full              (GtkMenu *menu, 
                                                 gint *x, 
                                                 gint *y,
                                                 gboolean *push_in,
                                                 GtkWidget *widget);
static gboolean
hildon_window_expose                            (GtkWidget *widget, 
                                                 GdkEventExpose *event);
static void 
hildon_window_forall                            (GtkContainer *container,
                                                 gboolean include_internals,
                                                 GtkCallback callback,
                                                 gpointer callback_data);
static void
hildon_window_show_all                          (GtkWidget *widget);

static void
hildon_window_size_allocate                     (GtkWidget * widget,
                                                 GtkAllocation *allocation);
static void
hildon_window_size_request                      (GtkWidget * widget,
                                                 GtkRequisition *requisition);
static void
hildon_window_finalize                          (GObject *obj_self);

static void
hildon_window_get_property                      (GObject *object,
                                                 guint property_id,
                                                 GValue *value, 
                                                 GParamSpec *pspec);

static void
hildon_window_set_property                      (GObject      *object,
                                                 guint         property_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec);

static void
hildon_window_update_markup                     (HildonWindow *window);

static void
hildon_window_destroy                           (GtkObject *obj);

static void
hildon_window_realize                           (GtkWidget *widget);

static void
hildon_window_unrealize                         (GtkWidget *widget);

static void
hildon_window_map                               (GtkWidget *widget);

static void
hildon_window_unmap                             (GtkWidget *widget);

static gboolean
hildon_window_key_press_event                   (GtkWidget *widget,
                                                 GdkEventKey *event);

static gboolean
hildon_window_key_release_event                 (GtkWidget *widget, 
                                                 GdkEventKey *event);
static gboolean
hildon_window_window_state_event                (GtkWidget *widget, 
                                                 GdkEventWindowState *event);
static gboolean
hildon_window_focus_out_event                   (GtkWidget *widget, 
                                                 GdkEventFocus *event);

static void
hildon_window_notify                            (GObject *gobject, 
                                                 GParamSpec *param);

static void
hildon_window_is_topmost_notify                 (HildonWindow *window);

static gboolean
hildon_window_toggle_menu                       (HildonWindow * self,
						 guint button,
						 guint32 time);

static gboolean
hildon_window_toggle_menu_real                  (HildonWindow * self,
						 guint button,
						 guint32 time);

static gboolean
hildon_window_escape_timeout                    (gpointer data);

static GdkFilterReturn
hildon_window_event_filter                      (GdkXEvent *xevent, 
                                                 GdkEvent *event, 
                                                 gpointer data);

static void
hildon_window_get_borders                       (HildonWindow *window);

static void
visible_toolbar                                 (gpointer data, 
                                                 gpointer user_data);

static void
paint_toolbar                                   (GtkWidget *widget, 
                                                 GtkBox *box, 
                                                 GdkEventExpose * event, 
                                                 gboolean fullscreen);

static void
paint_edit_toolbar                              (GtkWidget *widget,
                                                 GtkWidget *toolbar,
                                                 GdkEventExpose *event,
                                                 gboolean fullscreen);

enum
{
    PROP_0,
    PROP_IS_TOPMOST,
    PROP_MARKUP
};

enum
{
    WIN_TYPE = 0,
    WIN_TYPE_MESSAGE,
    MAX_WIN_MESSAGES
};

G_DEFINE_TYPE (HildonWindow, hildon_window, GTK_TYPE_WINDOW);

static void 
hildon_window_class_init                        (HildonWindowClass * window_class)
{
    /* Get convenience variables */
    GtkWidgetClass *widget_class        = GTK_WIDGET_CLASS (window_class);
    GObjectClass *object_class          = G_OBJECT_CLASS (window_class);
    GtkContainerClass *container_class  = GTK_CONTAINER_CLASS (window_class);

    object_class->get_property          = hildon_window_get_property;
    object_class->set_property          = hildon_window_set_property;
    object_class->notify                = hildon_window_notify;
    widget_class->size_allocate         = hildon_window_size_allocate;
    widget_class->size_request          = hildon_window_size_request;
    widget_class->expose_event          = hildon_window_expose;
    widget_class->show_all              = hildon_window_show_all;
    widget_class->realize               = hildon_window_realize;
    widget_class->unrealize             = hildon_window_unrealize;
    widget_class->key_press_event       = hildon_window_key_press_event;
    widget_class->key_release_event     = hildon_window_key_release_event;
    widget_class->window_state_event    = hildon_window_window_state_event;
    widget_class->focus_out_event       = hildon_window_focus_out_event;
    widget_class->map                   = hildon_window_map;
    widget_class->unmap                 = hildon_window_unmap;

    /* now the object stuff */
    object_class->finalize              = hildon_window_finalize;

    /* To the container */
    container_class->forall             = hildon_window_forall;

    /* To this class */
    window_class->toggle_menu           = hildon_window_toggle_menu_real;

    /* gtkobject stuff*/
    GTK_OBJECT_CLASS (window_class)->destroy = hildon_window_destroy; 

    g_type_class_add_private (window_class,
            sizeof (struct _HildonWindowPrivate));

    /* Install properties */

    g_object_class_install_property (object_class, PROP_IS_TOPMOST,
            g_param_spec_boolean ("is-topmost",
                "Is top-most",
                "Whether the window is currently activated by the window "
                "manager",
                FALSE,
                G_PARAM_READABLE));

    g_object_class_install_property (object_class, PROP_MARKUP,
            g_param_spec_string ("markup",
                "Marked up text for the window title",
                "Marked up text for the window title",
                NULL,
                G_PARAM_READWRITE));

    gtk_widget_class_install_style_property (widget_class,
            g_param_spec_boxed ("borders",
                "Graphical borders",
                "Size of graphical window borders",
                GTK_TYPE_BORDER,
                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
            g_param_spec_boxed ("toolbar-borders",
                "Graphical toolbar borders",
                "Size of graphical toolbar borders",
                GTK_TYPE_BORDER,
                G_PARAM_READABLE));

    /* opera hack, install clip operation signal */
    g_signal_new ("clipboard_operation",
            G_OBJECT_CLASS_TYPE (object_class),
            G_SIGNAL_RUN_FIRST,
            G_STRUCT_OFFSET (HildonWindowClass, clipboard_operation),
            NULL, NULL,
            g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1,
            G_TYPE_INT);
}

static void
hildon_window_init                              (HildonWindow *self)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (self);
    g_assert (priv != NULL);

    priv->vbox = gtk_vbox_new (TRUE, TOOLBAR_MIDDLE);
    gtk_widget_set_parent (priv->vbox, GTK_WIDGET(self));
    priv->menu = NULL;
    priv->app_menu = NULL;
    priv->edit_toolbar = NULL;
    priv->visible_toolbars = 0;
    priv->is_topmost = FALSE;
    priv->borders = NULL;
    priv->toolbar_borders = NULL;
    priv->escape_timeout = 0;
    priv->markup = NULL;

    priv->fullscreen = FALSE;

    priv->program = NULL;
}

static void
hildon_window_finalize                          (GObject * obj_self)
{
    HildonWindowPrivate *priv; 
      
    g_return_if_fail (HILDON_WINDOW (obj_self));

    priv = HILDON_WINDOW_GET_PRIVATE (obj_self);
    g_assert (priv != NULL);
    
    g_free (priv->markup);

    if (priv->escape_timeout) {
      g_source_remove (priv->escape_timeout);
      priv->escape_timeout = 0;
    }

    if (priv->borders)
        gtk_border_free (priv->borders);

    if (priv->toolbar_borders)
        gtk_border_free (priv->toolbar_borders);

    if (G_OBJECT_CLASS (hildon_window_parent_class)->finalize)
        G_OBJECT_CLASS (hildon_window_parent_class)->finalize (obj_self);

}

static void
hildon_window_realize                           (GtkWidget *widget)
{
    Atom *old_atoms, *new_atoms;
    Display *disp;
    Window window;
    gint atom_count;
    Window active_window;
    HildonWindowPrivate *priv;

    GTK_WIDGET_CLASS (hildon_window_parent_class)->realize (widget);

    priv = HILDON_WINDOW_GET_PRIVATE (widget);
    g_assert (priv != NULL);

    gtk_widget_realize (GTK_WIDGET (priv->vbox));

    if (priv->edit_toolbar != NULL)
        gtk_widget_realize (priv->edit_toolbar);

    /* catch the custom button signal from mb to display the menu */
    gdk_window_add_filter (widget->window, hildon_window_event_filter, widget);

    window = GDK_WINDOW_XID (widget->window);
    disp = GDK_WINDOW_XDISPLAY (widget->window);

    /* Enable custom button that is used for menu */
    XGetWMProtocols (disp, window, &old_atoms, &atom_count);
    new_atoms = g_new (Atom, atom_count + 1);

    memcpy (new_atoms, old_atoms, sizeof(Atom) * atom_count);

    new_atoms[atom_count++] =
        XInternAtom (disp, "_NET_WM_CONTEXT_CUSTOM", False);

    XSetWMProtocols (disp, window, new_atoms, atom_count);

    XFree(old_atoms);
    g_free(new_atoms);

    /* rely on GDK to set the window group to its default */
    gdk_window_set_group (widget->window, NULL);

    if (priv->program) {
        gboolean can_hibernate = hildon_program_get_can_hibernate (priv->program);

        hildon_window_set_can_hibernate_property (HILDON_WINDOW (widget),
                &can_hibernate);
    }

    if (priv->markup)
        hildon_window_update_markup (HILDON_WINDOW (widget));

    /* Update the topmost status */
    active_window = hildon_window_get_active_window();
    hildon_window_update_topmost (HILDON_WINDOW (widget), active_window);
}

static void
hildon_window_unrealize                         (GtkWidget *widget)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);
    g_assert (priv != NULL);

    gdk_window_remove_filter (widget->window, hildon_window_event_filter,
            widget);

    hildon_window_update_topmost (HILDON_WINDOW (widget), 0);

    gtk_widget_unrealize (GTK_WIDGET (priv->vbox));

    if (priv->edit_toolbar != NULL)
        gtk_widget_unrealize (priv->edit_toolbar);

    GTK_WIDGET_CLASS(hildon_window_parent_class)->unrealize(widget);
}

static void
hildon_window_map                             (GtkWidget *widget)
{
  HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);
  g_assert (priv != NULL);

  if (GTK_WIDGET_CLASS (hildon_window_parent_class)->map)
    GTK_WIDGET_CLASS (hildon_window_parent_class)->map (widget);

  if (priv->vbox != NULL && GTK_WIDGET_VISIBLE (priv->vbox))
    gtk_widget_map (priv->vbox);

  if (priv->edit_toolbar != NULL && GTK_WIDGET_VISIBLE (priv->edit_toolbar))
    gtk_widget_map (priv->edit_toolbar);
}

static void
hildon_window_unmap                             (GtkWidget *widget)
{
  HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);
  g_assert (priv != NULL);

  gtk_widget_unmap (priv->vbox);

  if (priv->edit_toolbar != NULL)
    gtk_widget_unmap (priv->edit_toolbar);

  if (GTK_WIDGET_CLASS (hildon_window_parent_class)->unmap)
    GTK_WIDGET_CLASS (hildon_window_parent_class)->unmap (widget);
}

static void
hildon_window_get_property                      (GObject *object, 
                                                 guint property_id,
                                                 GValue *value, 
                                                 GParamSpec * pspec)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (object);
    g_assert (priv != NULL);

    switch (property_id) {

        case PROP_IS_TOPMOST:
            g_value_set_boolean (value, priv->is_topmost);
            break;

        case PROP_MARKUP:
            g_value_set_string (value, priv->markup);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void
hildon_window_set_property                      (GObject      *object,
                                                 guint         property_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
    switch (property_id) {

        case PROP_MARKUP:
            hildon_window_set_markup (HILDON_WINDOW (object), g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

/*
 * Retrieve the graphical borders size used by the themes
 */
static void
hildon_window_get_borders                       (HildonWindow *window)
{
    GtkBorder zero = {0, 0, 0, 0};
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (window);
    g_assert (priv);

    GtkBorder *borders = NULL;
    GtkBorder *toolbar_borders = NULL;

    if (priv->borders)
        gtk_border_free (priv->borders);
    if (priv->toolbar_borders)
        gtk_border_free (priv->toolbar_borders);

    priv->borders = NULL;
    priv->toolbar_borders = NULL;

    gtk_widget_style_get (GTK_WIDGET (window), "borders",&borders,
            "toolbar-borders", &toolbar_borders,
            NULL);

    // We're doing a copy here instead of reusing the pointer, 
    // as we don't know where it comes from (has it been allocated using 
    // malloc or slices... and we want to free it sanely. Blowing on 
    // cold probbably.

    if (borders) {
        priv->borders = gtk_border_copy (borders);
        gtk_border_free (borders);
    } else
        priv->borders = g_boxed_copy (GTK_TYPE_BORDER, &zero);

    if (toolbar_borders) {
        priv->toolbar_borders = gtk_border_copy (toolbar_borders);
        gtk_border_free (toolbar_borders);
    } else
        priv->toolbar_borders = g_boxed_copy (GTK_TYPE_BORDER, &zero);
}

static gboolean
hildon_window_expose                            (GtkWidget *widget, 
                                                 GdkEventExpose * event)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);
    g_assert (priv);

    GtkWidget *bx = priv->vbox;
    GtkBox *box = GTK_BOX(bx);
    GtkBorder *b = priv->borders;
    GtkBorder *tb = priv->toolbar_borders;
    gint tb_height = 0;

    if (! priv->borders) {
        hildon_window_get_borders (HILDON_WINDOW (widget));
        b = priv->borders;
        tb = priv->toolbar_borders;
    }

    tb_height = bx->allocation.height + tb->top + tb->bottom;

    paint_toolbar (widget, box,
            event, priv->fullscreen);

    if (priv->edit_toolbar != NULL)
    {
        paint_edit_toolbar (widget, priv->edit_toolbar,
                            event, priv->fullscreen);
    }

    if (! priv->fullscreen) {

        /* Draw the left and right window border */
        gint side_borders_height = widget->allocation.height - b->top;

        if (priv->visible_toolbars)
            side_borders_height -= tb_height;
        else
            side_borders_height -= b->bottom;

        if (b->left > 0) 
        {
            gtk_paint_box (widget->style, widget->window,
                    GTK_WIDGET_STATE(widget), GTK_SHADOW_OUT,
                    &event->area, widget, "left-border",
                    widget->allocation.x, widget->allocation.y +
                    b->top, b->left, side_borders_height);
        } 

        if (b->right > 0)
        {
            gtk_paint_box (widget->style, widget->window,
                    GTK_WIDGET_STATE(widget), GTK_SHADOW_OUT,
                    &event->area, widget, "right-border",
                    widget->allocation.x + widget->allocation.width -
                    b->right, widget->allocation.y + b->top,
                    b->right, side_borders_height);
        }

        /* If no toolbar, draw the bottom window border */
        if (! priv->visible_toolbars && b->bottom > 0)
        {
            gtk_paint_box (widget->style, widget->window,
                    GTK_WIDGET_STATE(widget), GTK_SHADOW_OUT,
                    &event->area, widget, "bottom-border",
                    widget->allocation.x, widget->allocation.y +
                    (widget->allocation.height - b->bottom),
                    widget->allocation.width, b->bottom);
        }

        /* Draw the top border */
        if (b->top > 0)
        {
            gtk_paint_box (widget->style, widget->window,
                    GTK_WIDGET_STATE(widget), GTK_SHADOW_OUT,
                    &event->area, widget, "top-border",
                    widget->allocation.x, widget->allocation.y,
                    widget->allocation.width, b->top);
        } 


    }

    /* don't draw the window stuff as it overwrites our borders with a blank
     * rectangle. Instead start with the drawing of the GtkBin */
    GTK_WIDGET_CLASS (g_type_class_peek_parent (hildon_window_parent_class))->expose_event (widget, event);

    /* FIXME Not sure why this is commented out 
     * GTK_WIDGET_CLASS (hildon_window_parent_class))->
     *  expose_event (widget, event); 
     */

    return FALSE;
}

static void
hildon_window_size_request                      (GtkWidget *widget, 
                                                 GtkRequisition *requisition)
{
    GdkScreen *screen = gtk_widget_get_screen (widget);
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);
    GtkWidget *child;
    GtkRequisition req;
    g_assert (priv);

    child = gtk_bin_get_child (GTK_BIN (widget));

    if (child != NULL && GTK_WIDGET_VISIBLE (child))
        gtk_widget_size_request (child, &req);

    if (priv->vbox != NULL && GTK_WIDGET_VISIBLE (priv->vbox))
        gtk_widget_size_request (priv->vbox, &req);

    if (priv->edit_toolbar != NULL && GTK_WIDGET_VISIBLE (priv->edit_toolbar))
        gtk_widget_size_request (priv->edit_toolbar, &req);

    /* Request the full size of the screen */
    requisition->width = gdk_screen_get_width (screen);
    requisition->height = gdk_screen_get_height (screen);

    if (! priv->fullscreen)
        requisition->height -= HILDON_WINDOW_TITLEBAR_HEIGHT;
}

static void
hildon_window_size_allocate                     (GtkWidget *widget, 
                                                 GtkAllocation *allocation)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);
    g_assert (priv);

    GtkAllocation box_alloc = { 0 };
    GtkAllocation edittb_alloc = { 0 };
    GtkAllocation alloc = *allocation;

    GtkWidget *child = gtk_bin_get_child (GTK_BIN (widget));
    GtkBorder *tb;

    if (!priv->borders)
        hildon_window_get_borders (HILDON_WINDOW (widget));

    tb = priv->toolbar_borders;

    widget->allocation = *allocation;

    /* Calculate allocation of edit toolbar */
    if (priv->edit_toolbar != NULL && GTK_WIDGET_VISIBLE (priv->edit_toolbar))
    {
        GtkRequisition req;
        gtk_widget_get_child_requisition (priv->edit_toolbar, &req);
        edittb_alloc.width = alloc.width - tb->left - tb->right;
        edittb_alloc.height = MIN (req.height, alloc.height);
        edittb_alloc.x = alloc.x + tb->left;
        edittb_alloc.y = alloc.y + tb->top;

        if (edittb_alloc.height > 0)
        {
            alloc.y += tb->top + tb->bottom + edittb_alloc.height;
            alloc.height -= tb->top + tb->bottom + edittb_alloc.height;
            gtk_widget_size_allocate (priv->edit_toolbar, &edittb_alloc);
        }
    }

    /* Calculate allocation of normal toolbars */
    if (priv->vbox != NULL && GTK_WIDGET_VISIBLE (priv->vbox))
    {
        GtkRequisition req;
        gtk_widget_get_child_requisition (priv->vbox, &req);
        box_alloc.width = alloc.width - tb->left - tb->right;
        box_alloc.height = MIN (req.height, alloc.height);
        box_alloc.x = alloc.x + tb->left;
        box_alloc.y = alloc.y + alloc.height - box_alloc.height - tb->bottom;

        if (box_alloc.height > 0)
        {
            alloc.height -= tb->top + tb->bottom + box_alloc.height;
            gtk_widget_size_allocate (priv->vbox, &box_alloc);
        }
    }

    /* Calculate allocation of the child widget */
    if (child != NULL && GTK_WIDGET_VISIBLE (child))
    {
        guint border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));
        alloc.x += border_width;
        alloc.y += border_width;
        alloc.width -= (border_width * 2);
        alloc.height -= (border_width * 2);

        if (! priv->fullscreen)
        {
            GtkBorder *b = priv->borders;
            alloc.x += b->left;
            alloc.width -= (b->left + b->right);

            /* Use the top border if there's no edit toolbar */
            if (edittb_alloc.height <= 0)
            {
                alloc.y += b->top;
                alloc.height -= b->top;
            }

            /* Use the top border if there are no standard toolbars */
            if (box_alloc.height <= 0)
                alloc.height -= b->bottom;
        }

        gtk_widget_size_allocate (child, &alloc);
    }

    if (priv->previous_vbox_y != box_alloc.y)
    {
        /* The size of the VBox has changed, we need to redraw part
         * of the window borders */
        gint draw_from_y = MIN (priv->previous_vbox_y, box_alloc.y) - tb->top;

        gtk_widget_queue_draw_area (widget, 0, draw_from_y, 
                widget->allocation.width,
                widget->allocation.height - draw_from_y);

        priv->previous_vbox_y = box_alloc.y;
    }

}

static void
hildon_window_forall                            (GtkContainer *container, 
                                                 gboolean include_internals,
                                                 GtkCallback callback, 
                                                 gpointer callback_data)
{
    HildonWindow *self = HILDON_WINDOW (container);
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (self);

    g_return_if_fail (callback != NULL);
    g_assert (priv);

    GTK_CONTAINER_CLASS (hildon_window_parent_class)->forall (container, include_internals,
            callback, callback_data);

    if (include_internals && priv->vbox != NULL)
        (* callback)(GTK_WIDGET (priv->vbox), callback_data);

    if (include_internals && priv->edit_toolbar != NULL)
        (* callback)(GTK_WIDGET (priv->edit_toolbar), callback_data);
}

static void
hildon_window_show_all                          (GtkWidget *widget)
{
    HildonWindow *self = HILDON_WINDOW (widget);
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (self);

    g_assert (priv != NULL);

    GTK_WIDGET_CLASS (hildon_window_parent_class)->show_all (widget);

    gtk_widget_show_all (priv->vbox);

    if (priv->edit_toolbar)
        gtk_widget_show_all (priv->edit_toolbar);
}

static void
hildon_window_destroy                           (GtkObject *obj)
{
    HildonWindow *self = HILDON_WINDOW (obj);
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (obj);
    GList *menu_list = NULL;
    GList *menu_node = NULL;

    g_assert (priv != NULL);

    if (priv->vbox != NULL)
    {
        if (priv->program)
        {
            GtkWidget * common_toolbar = 
                GTK_WIDGET (hildon_program_get_common_toolbar (priv->program));
            if (common_toolbar && common_toolbar->parent == priv->vbox)
            {
                gtk_container_remove (GTK_CONTAINER (priv->vbox),
                        common_toolbar);
            }
        }

        gtk_widget_unparent (priv->vbox);
        priv->vbox = NULL;    

    }

    if (priv->edit_toolbar != NULL)
    {
        gtk_widget_unparent (priv->edit_toolbar);
        priv->edit_toolbar = NULL;
    }

    if (priv->app_menu)
    {
        hildon_app_menu_set_parent_window (priv->app_menu, NULL);
        g_object_unref (priv->app_menu);
        priv->app_menu = NULL;
    }

    menu_list = g_list_copy (gtk_menu_get_for_attach_widget (GTK_WIDGET (obj)));
    menu_node = menu_list;

    while (menu_node)
    {
        if (GTK_IS_MENU (menu_node->data))
        {
            if (GTK_WIDGET_VISIBLE (GTK_WIDGET (menu_node->data)))
            {
                gtk_menu_popdown (GTK_MENU (menu_node->data));
                gtk_menu_shell_deactivate (GTK_MENU_SHELL (menu_node->data));
            }
            gtk_menu_detach (GTK_MENU (menu_node->data));

            /* Destroy it, but only if it's not a common menu */
            if (priv->program && 
                hildon_program_get_common_menu (priv->program) != menu_node->data) {
                    gtk_object_destroy (GTK_OBJECT (menu_node->data));
                    g_object_unref (menu_node->data);
            }
        }
        menu_node = menu_node->next;
    }

    g_list_free (menu_list);
    menu_list = NULL;

    if (priv->program)
    {
        hildon_program_remove_window (priv->program, self);
    }

    gtk_widget_set_events (GTK_WIDGET(obj), 0);

    GTK_OBJECT_CLASS (hildon_window_parent_class)->destroy (obj);
}

static void
hildon_window_notify                            (GObject *gobject, 
                                                 GParamSpec *param)
{
    HildonWindow *window = HILDON_WINDOW (gobject);

    if (g_str_equal (param->name, "is-topmost"))
    {
        hildon_window_is_topmost_notify (window);
    }

    if (G_OBJECT_CLASS(hildon_window_parent_class)->notify)
        G_OBJECT_CLASS(hildon_window_parent_class)->notify (gobject, param);
}


static void
visible_toolbar                                 (gpointer data, 
                                                 gpointer user_data)
{
    if (GTK_WIDGET_VISIBLE (((GtkBoxChild *)data)->widget))
        (*((gint *)user_data))++;
}

static void
paint_toolbar                                   (GtkWidget *widget, 
                                                 GtkBox *box, 
                                                 GdkEventExpose * event, 
                                                 gboolean fullscreen)
{
    gint toolbar_num = 0; 
    gint count;

    /* collect info to help on painting the boxes */
    g_list_foreach (box->children, visible_toolbar, 
            (gpointer) &toolbar_num);

    if(toolbar_num <= 0)
        return;

    /*top most toolbar painting*/
    gtk_paint_box (widget->style, widget->window,
            GTK_WIDGET_STATE(widget), GTK_SHADOW_OUT,
            &event->area, widget, "toolbar-primary",
            widget->allocation.x,
            GTK_WIDGET(box)->allocation.y,
            widget->allocation.width,
            TOOLBAR_HEIGHT);

    /*multi toolbar painting*/
    for (count = 0; count < toolbar_num - 1; count++)
    {
            gtk_paint_box (widget->style, widget->window,
                           GTK_WIDGET_STATE(widget), GTK_SHADOW_OUT,
                           &event->area, widget, "toolbar-secondary",
                           widget->allocation.x,
                           GTK_WIDGET(box)->allocation.y +
                           (1 + count) * (TOOLBAR_HEIGHT),
                           widget->allocation.width,
                           TOOLBAR_HEIGHT);
    }
}

static void
paint_edit_toolbar                              (GtkWidget *widget,
                                                 GtkWidget *toolbar,
                                                 GdkEventExpose *event,
                                                 gboolean fullscreen)
{
    if (!GTK_WIDGET_VISIBLE (toolbar))
        return;

    gtk_paint_box (widget->style, widget->window,
                   GTK_WIDGET_STATE (widget), GTK_SHADOW_OUT,
                   &event->area, widget, "toolbar-edit-mode",
                   toolbar->allocation.x,
                   toolbar->allocation.y,
                   toolbar->allocation.width,
                   toolbar->allocation.height);
}

/*
 * Checks the root window to know which is the topped window
 */
Window
hildon_window_get_active_window                 (void)
{
    Atom realtype;
    gint xerror;
    int format;
    int status;
    Window ret;
    unsigned long n;
    unsigned long extra;
    union
    {
        Window *win;
        unsigned char *char_pointer;
    } win;
    Atom active_app_atom = 
        XInternAtom (GDK_DISPLAY (), "_MB_CURRENT_APP_WINDOW", False);

    win.win = NULL;

    gdk_error_trap_push ();
    status = XGetWindowProperty (GDK_DISPLAY(), GDK_ROOT_WINDOW(),
            active_app_atom, 0L, 16L,
            0, XA_WINDOW, &realtype, &format,
            &n, &extra, &win.char_pointer);
    xerror = gdk_error_trap_pop ();
    if (xerror || !(status == Success && realtype == XA_WINDOW && format == 32
                && n == 1 && win.win != NULL))
    {
        if (win.win != NULL)
            XFree (win.char_pointer);
        return None;
    }

    ret = win.win[0];

    if (win.win != NULL)
        XFree(win.char_pointer);

    /* 0xFFFFFFFF is not an actual window ID, but a magic value to
     * indicate that the task switcher is visible */
    return (ret != 0xFFFFFFFF) ? ret : None;
}

static int
xclient_message_type_check                      (XClientMessageEvent *cm, 
                                                 const gchar *name)
{
    return cm->message_type == XInternAtom(GDK_DISPLAY(), name, FALSE);
}

/*
 * Handle the window border custom button, which toggles the menu,
 * and the Hildon input method copy paste messages
 */
static GdkFilterReturn
hildon_window_event_filter                      (GdkXEvent *xevent, 
                                                 GdkEvent *event, 
                                                 gpointer data)
{
    XAnyEvent *eventti = xevent;

    if (eventti->type == ClientMessage)
    {
        XClientMessageEvent *cm = xevent;

        if (xclient_message_type_check (cm, "_MB_GRAB_TRANSFER"))
        {
            hildon_window_toggle_menu (HILDON_WINDOW ( data ), cm->data.l[2], cm->data.l[0]);
            return GDK_FILTER_REMOVE;
        }
        /* opera hack clipboard client message */
        else if (xclient_message_type_check (cm, "_HILDON_IM_CLIPBOARD_COPY"))
        {
            g_signal_emit_by_name(G_OBJECT(data), "clipboard_operation",
                    HILDON_WINDOW_CO_COPY);
            return GDK_FILTER_REMOVE;
        }
        else if (xclient_message_type_check(cm, "_HILDON_IM_CLIPBOARD_CUT"))
        {
            g_signal_emit_by_name(G_OBJECT(data), "clipboard_operation",
                    HILDON_WINDOW_CO_CUT);
            return GDK_FILTER_REMOVE;
        }
        else if (xclient_message_type_check(cm, "_HILDON_IM_CLIPBOARD_PASTE"))
        {
            g_signal_emit_by_name(G_OBJECT(data), "clipboard_operation",
                    HILDON_WINDOW_CO_PASTE);
            return GDK_FILTER_REMOVE;
        }
    }

    if (eventti->type == PropertyNotify)
    {
        XPropertyEvent *pevent = xevent;
        Atom active_app_atom = 
            XInternAtom (GDK_DISPLAY (), "_MB_CURRENT_APP_WINDOW", False);

        if (pevent->atom == active_app_atom)
        {
            HildonWindow *window = HILDON_WINDOW (data);
            Window active_window = hildon_window_get_active_window();

            hildon_window_update_topmost (window, active_window);
        }
    }

    return GDK_FILTER_CONTINUE;
}

/*
 * Handle the menu hardware key here
 */
static gboolean
hildon_window_key_press_event                   (GtkWidget *widget, 
                                                 GdkEventKey *event)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);

    g_return_val_if_fail (HILDON_IS_WINDOW (widget),FALSE);
    g_assert (priv);

    switch (event->keyval)
    {
        case HILDON_HARDKEY_MENU:
            if (hildon_window_toggle_menu (HILDON_WINDOW (widget), 0, GDK_CURRENT_TIME))
                return TRUE;
            break;
        case HILDON_HARDKEY_ESC:
            if (!priv->escape_timeout)
            {
                priv->escape_timeout = gdk_threads_add_timeout
                    (HILDON_WINDOW_LONG_PRESS_TIME,
                     hildon_window_escape_timeout, widget);
            }
            break;
    }

    return GTK_WIDGET_CLASS (hildon_window_parent_class)->key_press_event (widget, event);
}

static gboolean
hildon_window_key_release_event                 (GtkWidget *widget, 
                                                 GdkEventKey *event)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);

    g_return_val_if_fail (HILDON_IS_WINDOW (widget), FALSE);
    g_assert (priv);

    switch (event->keyval)
    {
        case HILDON_HARDKEY_ESC:
            if (priv->escape_timeout)
            {
                g_source_remove (priv->escape_timeout);
                priv->escape_timeout = 0;
            }
            break;
    }

    return GTK_WIDGET_CLASS (hildon_window_parent_class)->key_release_event (widget, event);

}

/*
 * We keep track of the window state changes, because the drawing
 * (borders) differs whether we are in fullscreen mode or not
 */
static gboolean
hildon_window_window_state_event                (GtkWidget *widget, 
                                                 GdkEventWindowState *event)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);
    g_assert (priv != NULL);

    if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
        priv->fullscreen = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;

    if (GTK_WIDGET_CLASS (hildon_window_parent_class)->window_state_event)
    {
        return GTK_WIDGET_CLASS (hildon_window_parent_class)->window_state_event (
                widget,
                event);
    }
    else
    {
        return FALSE;
    }
}

/*
 * If the window lost focus while the user started to press the ESC key, we
 * won't get the release event. We need to stop the timeout.
 */
static gboolean
hildon_window_focus_out_event                   (GtkWidget *widget, 
                                                 GdkEventFocus *event)
{
  HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (widget);

  if (priv->escape_timeout)
  {
      g_source_remove (priv->escape_timeout);
      priv->escape_timeout = 0;
  }

  return GTK_WIDGET_CLASS (hildon_window_parent_class)->focus_out_event (widget, event);
}

static void
set_legacy_menu_type                            (GtkMenu  *menu,
                                                 gboolean  set)
{
    GdkWindow *gdkwin = GTK_WIDGET (menu->toplevel)->window;
    GdkAtom property = gdk_atom_intern_static_string (LEGACY_MENU_PROPERTY_NAME);
    if (set) {
        GdkAtom type = gdk_x11_xatom_to_atom (XA_ATOM);
        GdkAtom value = gdk_atom_intern_static_string (LEGACY_MENU_PROPERTY_VALUE);
        gdk_property_change (gdkwin, property, type, 32,
                             GDK_PROP_MODE_REPLACE, (const guchar *) &value, 1);
    } else {
        gdk_property_delete (gdkwin, property);
    }
}

static void
legacy_menu_realized                            (GtkMenu *menu)
{
    set_legacy_menu_type (menu, TRUE);
    g_signal_handlers_disconnect_by_func (menu, legacy_menu_realized, NULL);
}

static void
legacy_menu_unmapped                            (GtkMenu *menu)
{
    set_legacy_menu_type (menu, FALSE);
    g_signal_handlers_disconnect_by_func (menu, legacy_menu_unmapped, NULL);
}

/*
 * The menu popuping needs a menu popup-function
 */
static void
hildon_window_menu_popup_func                   (GtkMenu *menu, 
                                                 gint *x, 
                                                 gint *y,
                                                 gboolean *push_in, 
                                                 GtkWidget *widget)
{
    gint window_x = 0;
    gint window_y = 0;
    GdkWindow *window = GTK_WIDGET(widget)->window;

    if (window)
    {
        gdk_window_get_origin (window, &window_x, &window_y);
    }

    gtk_widget_style_get (GTK_WIDGET (menu), "horizontal-offset", x,
            "vertical-offset", y, NULL);

    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    {
        *x = GTK_WIDGET (widget)->allocation.width + window_x - GTK_WIDGET (menu)->allocation.width - *x;
    }
    else
        *x += window_x;

    *y += window_y;

}

static void
hildon_window_menu_popup_func_full              (GtkMenu *menu, 
                                                 gint *x, 
                                                 gint *y,
                                                 gboolean *push_in, 
                                                 GtkWidget *widget)
{
    gtk_widget_style_get (GTK_WIDGET (menu), "horizontal-offset", x,
            "vertical-offset", y, NULL);

    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        *x = GTK_WIDGET (widget)->allocation.width - GTK_WIDGET (menu)->allocation.width - *x;
    else
        *x = MAX (0, *x);

    *y = MAX (0, *y);
}


/*
 * Takes the common toolbar when we acquire the top-most status
 */
static void
hildon_window_is_topmost_notify                 (HildonWindow *window)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (window);

    g_assert (priv);

    if (priv->is_topmost)
    {
        hildon_window_take_common_toolbar (window);
    }
}

static void
hildon_window_update_menu_flag (HildonWindow *self,
                                gboolean is_app_menu)
{
    HildonWindowPrivate *priv;

    priv = HILDON_WINDOW_GET_PRIVATE (self);

    if (is_app_menu)
    {
        /* Change the menu flag only if there is no program or common
           application menu. */
        if (!priv->program ||
            !hildon_program_get_common_app_menu (priv->program))
        {
            hildon_window_set_menu_flag (self, priv->app_menu != NULL &&
                                         hildon_app_menu_has_visible_children (priv->app_menu));
        }
    } else {
        if (!priv->program || !hildon_program_get_common_menu (priv->program))
        {
            GList *menu_children = gtk_container_get_children (GTK_CONTAINER (priv->menu));
            hildon_window_set_menu_flag (self, priv->menu != NULL && menu_children != NULL);
            g_list_free (menu_children);
        }
    }
}

static void
on_menu_changed (HildonAppMenu *menu,
                 HildonWindow *window)
{
    hildon_window_update_menu_flag (window, TRUE);
}

/*
 * Sets the program to which the window belongs. This should only be called
 * by hildon_program_add_window
 */
void G_GNUC_INTERNAL
hildon_window_set_program                       (HildonWindow *self, 
                                                 GObject *program)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (self);

    g_return_if_fail (HILDON_IS_WINDOW (self));
    g_assert (priv != NULL);

    if (priv->program)
    {
        g_object_unref (priv->program);
    }

    priv->program = HILDON_PROGRAM (program);
    g_object_ref (program);
}

/*
 * Unsets the program to which the window belongs. This should only be called
 * by hildon_program_remove_window
 */
void G_GNUC_INTERNAL
hildon_window_unset_program                     (HildonWindow *self)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (self);

    g_return_if_fail(HILDON_IS_WINDOW (self));
    g_assert (priv != NULL);

    if (priv->program)
    {
        g_object_unref (priv->program);
        priv->program = NULL;
    }

    priv->program = NULL;
}

/*
 * Sets whether or not the program to which this window belongs is
 * killable. This is used by the HildonProgram to signify to the
 * Task Navigator whether or not it can hibernate in memory-low situations
 **/    
void G_GNUC_INTERNAL
hildon_window_set_can_hibernate_property        (HildonWindow *self, 
                                                 gpointer _can_hibernate)
{
    GdkAtom killable_atom;
    gboolean can_hibernate;

    g_return_if_fail(self && HILDON_IS_WINDOW (self));

    if (!GTK_WIDGET_REALIZED ((GTK_WIDGET (self))))
    {
        return;
    }

    can_hibernate = * ((gboolean *)_can_hibernate);

    killable_atom = gdk_atom_intern (CAN_HIBERNATE_PROPERTY, FALSE);

    if (can_hibernate)
    {
        gdk_property_change (GTK_WIDGET (self)->window, killable_atom,
                (GdkAtom)31/* XA_STRING */, 8,
                GDK_PROP_MODE_REPLACE, (const guchar *)CAN_HIBERNATE,
                CAN_HIBERNATE_LENGTH);
    }
    else
    {
        gdk_property_delete (GTK_WIDGET (self)->window, killable_atom);
    }

}

/*
 * If a common toolbar was set to the program, reparent it to
 * us
 */
void G_GNUC_INTERNAL
hildon_window_take_common_toolbar               (HildonWindow *self)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (self);

    g_return_if_fail(HILDON_IS_WINDOW (self));
    g_assert (priv);

    if (priv->program)
    {
        GtkWidget *common_toolbar =  
            GTK_WIDGET (hildon_program_get_common_toolbar (priv->program));

        if (common_toolbar && common_toolbar->parent != priv->vbox)
        {
            g_object_ref (common_toolbar);
            if (common_toolbar->parent)
            {
                gtk_container_remove (GTK_CONTAINER (common_toolbar->parent),
                        common_toolbar);
            }

            gtk_box_pack_end (GTK_BOX(priv->vbox), common_toolbar,
                    TRUE, TRUE, 0);
            g_object_unref (common_toolbar);

            gtk_widget_set_size_request (common_toolbar, -1, TOOLBAR_HEIGHT);

            gtk_widget_show  (priv->vbox);

        }
    }
}

/*
 * Compare the window that was last topped, and act consequently
 */
void
hildon_window_update_topmost                    (HildonWindow *self, 
                                                 Window window_id)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (self);

    GdkWindow *my_window;

    g_return_if_fail (HILDON_IS_WINDOW (self));
    g_assert (priv);

    my_window = GTK_WIDGET (self)->window;

    if (my_window && window_id == GDK_WINDOW_XID (my_window))
    {
        if (! priv->is_topmost)
        {
            priv->is_topmost = TRUE;
            hildon_window_is_topmost_notify (self);
            g_object_notify (G_OBJECT (self), "is-topmost");
        }
    }
    else if (priv->is_topmost)
    {
        /* Should this go in the signal handler? */
        GtkWidget *focus = gtk_window_get_focus (GTK_WINDOW (self));

        if (GTK_IS_ENTRY (focus))
            gtk_im_context_focus_out (GTK_ENTRY (focus)->im_context);
        if (GTK_IS_TEXT_VIEW (focus))
            gtk_im_context_focus_out (GTK_TEXT_VIEW (focus)->im_context);

        priv->is_topmost = FALSE;
        hildon_window_is_topmost_notify (self);
        g_object_notify (G_OBJECT (self), "is-topmost");
    }
}

static void
detach_menu_func                                (GtkWidget *attach_widget, 
                                                 GtkMenu *menu)
{
    /* FIXME Why is this even needed here? */
}

static gboolean
hildon_window_toggle_menu                       (HildonWindow *self,
						 guint button,
						 guint32 time)
{
    g_return_val_if_fail (HILDON_IS_WINDOW (self), FALSE);

    if (HILDON_WINDOW_GET_CLASS (self)->toggle_menu != NULL)
    {
        return HILDON_WINDOW_GET_CLASS (self)->toggle_menu (self, button, time);
    }
    else
    {
        return FALSE;
    }
}


static gboolean
hildon_window_toggle_gtk_menu                   (HildonWindow *self,
						 GtkMenu      *menu,
						 guint         button,
						 guint32       time)
{
    gboolean retvalue = FALSE;

    g_return_val_if_fail (HILDON_IS_WINDOW (self), FALSE);
    g_return_val_if_fail (GTK_IS_MENU (menu), FALSE);

    if (gtk_menu_get_attach_widget (menu) != GTK_WIDGET (self))
    {
        g_object_ref (menu);
        if (gtk_menu_get_attach_widget (menu))
        {
            gtk_menu_detach (menu);
        }
        gtk_menu_attach_to_widget (menu, GTK_WIDGET (self), &detach_menu_func);
        g_object_unref (menu);
    }

    if (GTK_WIDGET_MAPPED (menu))
    {
        gtk_menu_popdown (menu);
        gtk_menu_shell_deactivate (GTK_MENU_SHELL (menu));
        retvalue = TRUE;
    }
    else
    {
        /* Check if the menu has items */
        GList *menu_children = gtk_container_get_children (GTK_CONTAINER (menu));

        if (menu_children)
        {
            HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (self);
            g_list_free (menu_children);

            /* Set the 'legacy app menu' property when the widget is realized */
            if (GTK_WIDGET_REALIZED (menu)) {
                set_legacy_menu_type (menu, TRUE);
            } else {
                g_signal_connect (menu, "realize",
                                  G_CALLBACK (legacy_menu_realized), NULL);
            }

            /* Remove it when it's unmapped */
            g_signal_connect (menu, "unmap",
                              G_CALLBACK (legacy_menu_unmapped), NULL);

            /* Apply right theming */
            gtk_widget_set_name (GTK_WIDGET (menu), "menu_force_with_corners");

            if (priv->fullscreen)
            {
                gtk_menu_popup (menu, NULL, NULL,
                                (GtkMenuPositionFunc)
                                hildon_window_menu_popup_func_full,
                                self, button, time);
            }
            else
            {
                gtk_menu_popup (menu, NULL, NULL,
                                (GtkMenuPositionFunc)
                                hildon_window_menu_popup_func,
                                self, button, time);
            }
            gtk_menu_shell_select_first (GTK_MENU_SHELL (menu), TRUE);
            retvalue = TRUE;
        }
    }

    return retvalue;
}

static gboolean
hildon_window_toggle_app_menu                   (HildonWindow  *self,
						 HildonAppMenu *menu)
{
    g_return_val_if_fail (HILDON_IS_WINDOW (self), FALSE);
    g_return_val_if_fail (HILDON_IS_APP_MENU (menu), FALSE);

    if (self != hildon_app_menu_get_parent_window (menu))
    {
        gtk_widget_hide (GTK_WIDGET (menu));
    }

    if (GTK_WIDGET_MAPPED (menu))
    {
        gtk_widget_hide (GTK_WIDGET (menu));
    }
    else
    {
        hildon_app_menu_popup (menu, GTK_WINDOW (self));
    }

    return TRUE;
}

/*
 * Toggles the display of the HildonWindow menu.
 * Returns whether or not something was done (whether or not we had a menu
 * to toggle)
 */
static gboolean
hildon_window_toggle_menu_real                  (HildonWindow * self,
						 guint button,
						 guint32 time)
{
    gboolean retvalue = FALSE;
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (self);

    g_return_val_if_fail (HILDON_IS_WINDOW (self), FALSE);

    /* Select which menu to use, Window specific has highest priority,
     * then program specific */
    if (priv->menu)
    {
        retvalue = hildon_window_toggle_gtk_menu (self, priv->menu, button, time);
    }
    else if (priv->app_menu)
    {
        retvalue = hildon_window_toggle_app_menu (self, priv->app_menu);
    }
    else if (priv->program)
    {
        GtkMenu *gtkmenu = hildon_program_get_common_menu (priv->program);
        HildonAppMenu *appmenu = hildon_program_get_common_app_menu (priv->program);

        if (gtkmenu)
        {
            retvalue = hildon_window_toggle_gtk_menu (self, gtkmenu, button, time);
        }
        else if (appmenu)
        {
            retvalue = hildon_window_toggle_app_menu (self, appmenu);
        }
    }

    return retvalue;
}

/*
 * If the ESC key was not released when the timeout expires,
 * close the window
 */
static gboolean
hildon_window_escape_timeout                    (gpointer data)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (data);
    GdkEvent *event;

    g_assert (priv);

    /* Send fake event, simulation a situation that user
       pressed 'x' from the corner */
    event = gdk_event_new(GDK_DELETE);
    ((GdkEventAny *)event)->window = GDK_WINDOW (g_object_ref (GTK_WIDGET(data)->window));
    gtk_main_do_event(event);

    /* That unrefs the window, so we're reffing it above */
    gdk_event_free(event);

    priv->escape_timeout = 0;

    return FALSE;
}

/**
 * hildon_window_new: 
 * 
 * Creates a new #HildonWindow.
 * 
 * Return value: A #HildonWindow.
 **/
GtkWidget*
hildon_window_new                               (void)
{
    HildonWindow *newwindow = g_object_new (HILDON_TYPE_WINDOW, NULL);

    return GTK_WIDGET (newwindow);
}

/**
 * hildon_window_add_with_scrollbar:
 * @self: A #HildonWindow
 * @child: A #GtkWidget
 *
 * Adds @child to the #HildonWindow and creates a scrollbar for
 * it. Similar to adding first a #GtkScrolledWindow and then @child to
 * it.
 */
void
hildon_window_add_with_scrollbar                (HildonWindow *self,
                                                 GtkWidget *child)
{
    GtkScrolledWindow *scrolledw;

    g_return_if_fail (HILDON_IS_WINDOW (self));
    g_return_if_fail (GTK_IS_WIDGET (child));
    g_return_if_fail (child->parent == NULL);

    scrolledw = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
    gtk_scrolled_window_set_policy (scrolledw, GTK_POLICY_NEVER,
            GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (scrolledw, GTK_SHADOW_NONE);

    if (GTK_IS_VIEWPORT (child))
        gtk_container_add (GTK_CONTAINER (scrolledw), child);
    else
    {
        if (GTK_IS_CONTAINER (child) )
            gtk_container_set_focus_vadjustment (GTK_CONTAINER(child),
                    gtk_scrolled_window_get_vadjustment (scrolledw) );
        gtk_scrolled_window_add_with_viewport (scrolledw, child);
    }

    gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (scrolledw));
}

static void
calculate_visible_toolbars                      (gpointer data,
                                                 gpointer user_data)
{
  if (GTK_WIDGET_VISIBLE (GTK_WIDGET (((GtkBoxChild *)data)->widget)))
    (*((gint *)user_data)) ++;
}

static void
toolbar_visible_notify                          (GtkWidget *toolbar, GParamSpec *pspec,
                                                 HildonWindow *window)
{
  HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (window);

  g_assert (priv);

  /* Recalculate from scratch the value just in case */
  priv->visible_toolbars = 0;

  g_list_foreach (GTK_BOX (priv->vbox)->children, calculate_visible_toolbars, 
                  &priv->visible_toolbars);

  if (priv->visible_toolbars == 0)
    gtk_widget_hide (priv->vbox);
  else
    gtk_widget_show (priv->vbox);
}

/**
 * hildon_window_add_toolbar:
 * @self: A #HildonWindow
 * @toolbar: A #GtkToolbar to add to the #HildonWindow
 *
 * Adds a toolbar to the window. Note that the toolbar is not automatically
 * shown. You need to call gtk_widget_show_all() on it to make it visible.
 * It's also possible to hide the toolbar (without removing it) by calling
 * gtk_widget_hide()
 **/
void 
hildon_window_add_toolbar                       (HildonWindow *self, 
                                                 GtkToolbar *toolbar)
{
    GtkBox *vbox;
    HildonWindowPrivate *priv;

    g_return_if_fail (HILDON_IS_WINDOW (self));
    g_return_if_fail (toolbar && GTK_IS_TOOLBAR (toolbar));

    priv = HILDON_WINDOW_GET_PRIVATE (self);

    vbox = GTK_BOX (priv->vbox);

    gtk_box_pack_start (vbox, GTK_WIDGET (toolbar), TRUE, TRUE, 0);
    gtk_box_reorder_child (vbox, GTK_WIDGET (toolbar), 0);
    gtk_widget_set_size_request (GTK_WIDGET (toolbar), -1, TOOLBAR_HEIGHT);

    g_signal_connect (G_OBJECT (toolbar), "notify::visible",
                      G_CALLBACK (toolbar_visible_notify), self);

    if (GTK_WIDGET_VISIBLE (toolbar))
      {
        priv->visible_toolbars++;
        gtk_widget_show (priv->vbox);
      }

    gtk_widget_queue_resize (GTK_WIDGET (self));
}

/**
 * hildon_window_remove_toolbar:
 * @self: A #HildonWindow
 * @toolbar: A #GtkToolbar to remove from the #HildonWindow
 *
 * Removes a toolbar from the window. Note that this decreases the refference
 * count on the widget. If you want to keep the toolbar alive call g_object_ref()
 * before calling this function.
 **/
void
hildon_window_remove_toolbar                    (HildonWindow *self, 
                                                 GtkToolbar *toolbar)
{
    HildonWindowPrivate *priv;

    g_return_if_fail (HILDON_IS_WINDOW (self));
    
    priv = HILDON_WINDOW_GET_PRIVATE (self);

    if (GTK_WIDGET_VISIBLE (toolbar))
      {
        if (--(priv->visible_toolbars) == 0)
          gtk_widget_hide (priv->vbox);
      }

    g_signal_handlers_disconnect_by_func (toolbar, toolbar_visible_notify, self);

    gtk_container_remove (GTK_CONTAINER (priv->vbox), GTK_WIDGET (toolbar));
}

/**
 * hildon_window_set_edit_toolbar:
 * @self: A #HildonWindow
 * @toolbar: A #HildonEditToolbar, or %NULL to remove the current one.
 *
 * Adds a #HildonEditToolbar to the window. Note that the toolbar is
 * not automatically shown. You need to call gtk_widget_show() on it
 * to make it visible. It's also possible to hide the toolbar (without
 * removing it) by calling gtk_widget_hide().
 *
 * A window can only have at most one edit toolbar at a time, so the
 * previous toolbar (if any) is replaced after calling this function.
 *
 * Since: 2.2
 **/
void
hildon_window_set_edit_toolbar                  (HildonWindow      *self,
                                                 HildonEditToolbar *toolbar)
{
    HildonWindowPrivate *priv;

    g_return_if_fail (HILDON_IS_WINDOW (self));
    g_return_if_fail (toolbar == NULL || HILDON_IS_EDIT_TOOLBAR (toolbar));

    priv = HILDON_WINDOW_GET_PRIVATE (self);

    if (priv->edit_toolbar != GTK_WIDGET (toolbar))
    {
        GtkWidget *old_toolbar = priv->edit_toolbar;
        priv->edit_toolbar = GTK_WIDGET (toolbar);

        if (priv->edit_toolbar)
            gtk_widget_set_parent (priv->edit_toolbar, GTK_WIDGET (self));

        if (old_toolbar)
            gtk_widget_unparent (old_toolbar);
    }
}

/**
 * hildon_window_get_main_menu:
 * @self: a #HildonWindow
 *
 * Gets the #GtkMenu assigned to the #HildonWindow. Note that the
 * window is still the owner of the menu.
 *
 * Note that if you're using a #HildonAppMenu rather than a #GtkMenu
 * you should use hildon_window_get_app_menu() instead.
 *
 * Return value: The #GtkMenu assigned to this application view. 
 *
 * Since: 2.2
 **/
GtkMenu*
hildon_window_get_main_menu                     (HildonWindow * self)
{
    HildonWindowPrivate *priv;

    g_return_val_if_fail (HILDON_IS_WINDOW (self), NULL);

    priv = HILDON_WINDOW_GET_PRIVATE (self);

    return priv->menu;
}

/**
 * hildon_window_get_menu:
 * @self: a #HildonWindow
 *
 * Gets the #GtkMenu assigned to @self
 *
 * Return value: a #GtkMenu
 *
 * Deprecated: In Hildon 2.2 this function has been renamed to
 * hildon_window_get_main_menu() for consistency
 **/
GtkMenu*
hildon_window_get_menu                          (HildonWindow * self)
{
    return hildon_window_get_main_menu (self);
}

/* Since we've been asking developers to call gtk_window_add_accel_group()
 * themselves, do not trigger criticals by trying it again.
 */
static void
hildon_window_add_accel_group (HildonWindow *self,
			       GtkAccelGroup *accel_group)
{
    GSList *groups, *l;

    groups = gtk_accel_groups_from_object (G_OBJECT (self));
    for (l = groups; l != NULL; l = l->next)
      if (l->data == (gpointer)accel_group)
	/* Maybe print a warning here? */
	return;

    gtk_window_add_accel_group (GTK_WINDOW (self), accel_group);
}

static void
do_set_has_menu (GtkWindow *window,
                 gpointer   boolptr)
{
    gboolean has_menu = GPOINTER_TO_INT (boolptr);
    hildon_gtk_window_set_clear_window_flag (window, "_HILDON_WM_WINDOW_MENU_INDICATOR",
                                             XA_INTEGER, has_menu);
    g_signal_handlers_disconnect_matched (window, G_SIGNAL_MATCH_FUNC,
                                          0, 0, NULL, do_set_has_menu, NULL);
}

void
hildon_window_set_menu_flag (HildonWindow *window,
                             gboolean   has_menu)
{
    hildon_gtk_window_set_flag (GTK_WINDOW (window), (HildonFlagFunc) do_set_has_menu,
                                GUINT_TO_POINTER (has_menu));
}

/**
 * hildon_window_set_main_menu:
 * @self: A #HildonWindow
 * @menu: The #GtkMenu to be used for this #HildonWindow
 *
 * Sets the menu to be used for this window. This menu overrides
 * a program-wide menu that may have been set with
 * hildon_program_set_common_menu(). Pass %NULL to remove the current
 * menu. #HildonWindow takes ownership of the passed menu and you're
 * not supposed to free it yourself anymore.
 *
 * Note that if you're using a #HildonAppMenu rather than a #GtkMenu
 * you should use hildon_window_set_app_menu() instead.
 **/
void
hildon_window_set_main_menu (HildonWindow* self,
			     GtkMenu     * menu)
{
    HildonWindowPrivate *priv;
    GtkAccelGroup *accel_group;

    g_return_if_fail (HILDON_IS_WINDOW (self));

    priv = HILDON_WINDOW_GET_PRIVATE (self);

    if (priv->menu != NULL)
    {
	accel_group = gtk_menu_get_accel_group (priv->menu);
	if (accel_group != NULL)
	    gtk_window_remove_accel_group (GTK_WINDOW (self), accel_group);

        gtk_menu_detach (priv->menu);
        g_object_unref (priv->menu);
    }

    priv->menu = menu;
    if (priv->menu != NULL)
    {
        gtk_widget_set_name (GTK_WIDGET (priv->menu), "menu_force_with_corners");
        gtk_menu_attach_to_widget (priv->menu, GTK_WIDGET (self), &detach_menu_func);
        g_object_ref (priv->menu);

	accel_group = gtk_menu_get_accel_group (priv->menu);
	if (accel_group != NULL)
	    hildon_window_add_accel_group (self, accel_group);
    }

    hildon_window_update_menu_flag (self, FALSE);
}

/**
 * hildon_window_set_menu:
 * @self: A #HildonWindow
 * @menu: The #GtkMenu to be used for this #HildonWindow
 *
 * Sets the menu to be used for this window. This menu overrides
 * a program-wide menu that may have been set with
 * hildon_program_set_common_menu(). Pass %NULL to remove the current
 * menu. HildonWindow takes ownership of the passed menu and you're
 * not supposed to free it yourself anymore.
 *
 * Note: hildon_window_set_menu() calls gtk_widget_show_all() for the
 * #GtkMenu. To pass control about visibility to the application
 * developer, hildon_window_set_main_menu() was introduced, which
 * doesn't do this.
 *
 * Deprecated: Hildon 2.2: use hildon_window_set_main_menu()
 **/
void
hildon_window_set_menu                          (HildonWindow *self, 
                                                 GtkMenu *menu)
{
    HildonWindowPrivate *priv;

    g_return_if_fail (HILDON_IS_WINDOW (self));

    hildon_window_set_main_menu (self, menu);

    priv = HILDON_WINDOW_GET_PRIVATE (self);

    if (priv->menu != NULL)
        gtk_widget_show_all (GTK_WIDGET (priv->menu));
}

/**
 * hildon_window_get_is_topmost:
 * @self: A #HildonWindow
 *
 * Returns whether the #HildonWindow is currenty activated by the
 * window manager.
 *
 * Return value: %TRUE if @self is currently activated, %FALSE otherwise.
 **/
gboolean
hildon_window_get_is_topmost                    (HildonWindow *self)
{
    HildonWindowPrivate *priv;

    g_return_val_if_fail (HILDON_IS_WINDOW (self), FALSE);

    priv = HILDON_WINDOW_GET_PRIVATE (self);
    return priv->is_topmost;
}

/**
 * hildon_window_set_app_menu:
 * @self: a #HildonWindow
 * @menu: a #HildonAppMenu to be used for this window
 *
 * Sets the menu to be used for this window. Pass %NULL to remove the
 * current menu. Any reference to a previous menu will be dropped.
 * #HildonWindow takes ownership of the passed menu and
 * you're not supposed to free it yourself anymore.
 *
 * Note that if you're using a #GtkMenu rather than a #HildonAppMenu
 * you should use hildon_window_set_main_menu() instead.
 *
 * Since: 2.2
 **/
void
hildon_window_set_app_menu                      (HildonWindow  *self,
                                                 HildonAppMenu *menu)
{
    HildonWindowPrivate *priv;
    HildonAppMenu *old_menu;

    g_return_if_fail (HILDON_IS_WINDOW (self));
    g_return_if_fail (!menu || HILDON_IS_APP_MENU (menu));
    priv = HILDON_WINDOW_GET_PRIVATE (self);

    old_menu = priv->app_menu;

    /* Add new menu */
    priv->app_menu = menu;
    if (menu)
    {
        g_object_ref_sink (menu);
        g_signal_connect (menu, "changed", G_CALLBACK (on_menu_changed), self);
    }

    /* Unref old menu */
    if (old_menu)
    {
        g_signal_handlers_disconnect_by_func (old_menu, on_menu_changed, self);
        g_object_unref (old_menu);
    }

    hildon_window_update_menu_flag (self, TRUE);
}

/**
 * hildon_window_get_app_menu:
 * @self: a #HildonWindow
 *
 * Returns the #HildonAppMenu assigned to @self, or %NULL if it's
 * unset. Note that the window is still the owner of the menu.
 *
 * Note that if you're using a #GtkMenu rather than a #HildonAppMenu
 * you should use hildon_window_get_main_menu() instead.
 *
 * Returns: a #HildonAppMenu
 *
 * Since: 2.2
 **/
HildonAppMenu *
hildon_window_get_app_menu                      (HildonWindow *self)
{
    HildonWindowPrivate *priv;

    g_return_val_if_fail (HILDON_IS_WINDOW (self), NULL);

    priv = HILDON_WINDOW_GET_PRIVATE (self);

    return priv->app_menu;
}

static void
hildon_window_update_markup                     (HildonWindow *window)
{
    HildonWindowPrivate *priv = HILDON_WINDOW_GET_PRIVATE (window);
    GdkAtom markup_atom = gdk_atom_intern ("_HILDON_WM_NAME", FALSE);
    GdkAtom utf8_atom = gdk_atom_intern ("UTF8_STRING", FALSE);
    GdkWindow *gdkwin = GTK_WIDGET (window)->window;

    if (priv->markup) {
        gdk_property_change (gdkwin, markup_atom, utf8_atom, 8,
                             GDK_PROP_MODE_REPLACE, (const guchar *) priv->markup,
                             strlen (priv->markup));
    } else {
        gdk_property_delete (gdkwin, markup_atom);
    }
}

/**
 * hildon_window_get_markup:
 * @window: a #HildonWindow
 *
 * Gets the marked up title of the window title. See hildon_window_set_markup()
 *
 * Returns: the marked up title of the window, or %NULL if none has
 * been set explicitely. The returned string is owned by the widget
 * and must not be modified or freed.
 *
 * Since: 2.2
 **/
const gchar *
hildon_window_get_markup                        (HildonWindow *window)
{
    HildonWindowPrivate *priv;

    g_return_val_if_fail (HILDON_IS_WINDOW (window), NULL);

    priv = HILDON_WINDOW_GET_PRIVATE (window);

    return priv->markup;
}

/**
 * hildon_window_set_markup:
 * @window: a #HildonWindow
 * @markup: the marked up title of the window, or %NULL to unset the
 * current one
 *
 * Sets the marked up title of @window. The accepted format is the one
 * used in Pango (see #PangoMarkupFormat) with the exception of
 * &lt;span&gt;.
 *
 * Note that you need support from the window manager for this title
 * to be used. See gtk_window_set_title() for the standard way of
 * setting the title of a window.
 *
 * Since: 2.2
 **/
void
hildon_window_set_markup                        (HildonWindow *window,
                                                 const gchar  *markup)
{
    HildonWindowPrivate *priv;
    gchar *new_markup;

    g_return_if_fail (HILDON_IS_WINDOW (window));

    priv = HILDON_WINDOW_GET_PRIVATE (window);

    new_markup = g_strdup (markup);
    g_free (priv->markup);
    priv->markup = new_markup;

    if (GTK_WIDGET_REALIZED (window))
        hildon_window_update_markup (window);

    g_object_notify (G_OBJECT (window), "markup");
}
