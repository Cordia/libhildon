/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008, 2009 Nokia Corporation, all rights reserved.
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
 * SECTION:hildon-stackable-window
 * @short_description: Stackable top-level window in the Hildon framework.
 * @see_also: #HildonWindowStack, #HildonProgram, #HildonWindow
 *
 * #HildonStackableWindow is a top-level window that can be stacked on
 * top of others. It is derived from #HildonWindow. Applications that
 * use stackable windows are organized in a hierarchical way so users
 * can go from any window back to the previous one or directly to the
 * stack's root window.
 *
 * The user can only see and interact with the window on top of the
 * stack. Although all other windows are mapped and visible, they are
 * obscured by the topmost one so in practice the user will see the
 * whole stack as if it was a single window.
 *
 * To add a window to the stack, just use gtk_widget_show(). The
 * previous one will be obscured by the new one. When the new window
 * is destroyed, the previous one will appear again.
 *
 * Alternatively, you can remove a window from the top of the stack
 * without destroying it by using gtk_widget_hide(). The window will
 * be hidden and the previous one will appear automatically.
 *
 * It is important to note that all #HildonStackableWindow<!-- -->s on
 * a stack are always mapped and visible (from the Gtk point of view)
 * and all visible #HildonStackableWindow<!-- -->s are always on a
 * stack.
 *
 * To see how to manage multiple stacks per application and for other
 * advanced details on stack handling, see #HildonWindowStack
 *
 * <example>
 * <title>Basic HildonStackableWindow example</title>
 * <programlisting>
 * static void
 * show_new_window (void)
 * {
 *     GtkWidget *win;
 * <!-- -->
 *     win = hildon_stackable_window_new ();
 * <!-- -->
 *     // ... configure new window
 * <!-- -->
 *     gtk_widget_show (win);
 * }
 * <!-- -->
 * int
 * main (int argc, char **argv)
 * {
 *     GtkWidget *win;
 *     GtkWidget *button;
 * <!-- -->
 *     gtk_init (&amp;argc, &amp;args);
 * <!-- -->
 *     win = hildon_stackable_window_new ();
 *     gtk_window_set_title (GTK_WINDOW (win), "Main window);
 * <!-- -->
 *     // ... add some widgets to the window
 * <!-- -->
 *     g_signal_connect (button, "clicked", G_CALLBACK (show_new_window), NULL);
 *     g_signal_connect (win, "destroy", G_CALLBACK (gtk_main_quit), NULL);
 * <!-- -->
 *     gtk_widget_show_all (win);
 *     gtk_main ();
 * <!-- -->
 *     return 0;
 * }
 * </programlisting>
 * </example>
 */

#include                                        <X11/X.h>
#include                                        <X11/Xatom.h>
#include                                        <gdk/gdkx.h>

#include                                        "hildon-stackable-window.h"
#include                                        "hildon-stackable-window-private.h"
#include                                        "hildon-window-stack.h"
#include                                        "hildon-window-stack-private.h"

G_DEFINE_TYPE (HildonStackableWindow, hildon_stackable_window, HILDON_TYPE_WINDOW);

void G_GNUC_INTERNAL
hildon_stackable_window_set_stack               (HildonStackableWindow *self,
                                                 HildonWindowStack     *stack,
                                                 gint                   position)
{
    HildonStackableWindowPrivate *priv = HILDON_STACKABLE_WINDOW_GET_PRIVATE (self);

    if (stack)
        g_object_ref (stack);

    if (priv->stack)
        g_object_unref (priv->stack);

    priv->stack = stack;
    priv->stack_position = position;
}

/**
 * hildon_stackable_window_get_stack:
 * @self: a #HildonStackableWindow
 *
 * Returns the stack where window @self is on, or %NULL if the window
 * is not stacked.
 *
 * Return value: a #HildonWindowStack, or %NULL
 *
 * Since: 2.2
 **/
HildonWindowStack *
hildon_stackable_window_get_stack               (HildonStackableWindow *self)
{
    HildonStackableWindowPrivate *priv;

    g_return_val_if_fail (HILDON_IS_STACKABLE_WINDOW (self), NULL);

    priv = HILDON_STACKABLE_WINDOW_GET_PRIVATE (self);

    return priv->stack;
}

static void
hildon_stackable_window_map                     (GtkWidget *widget)
{
    GdkDisplay *display;
    Atom atom;
    unsigned long val;
    HildonStackableWindowPrivate *priv = HILDON_STACKABLE_WINDOW_GET_PRIVATE (widget);

    val = priv->stack_position;

    /* Set additional property "_HILDON_STACKABLE_WINDOW", to allow the WM to manage
       it as a stackable window. */
    display = gdk_drawable_get_display (widget->window);
    atom = gdk_x11_get_xatom_by_name_for_display (display, "_HILDON_STACKABLE_WINDOW");
    XChangeProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (widget->window), atom,
                     XA_INTEGER, 32, PropModeReplace,
                     (unsigned char *) &val, 1);

    GTK_WIDGET_CLASS (hildon_stackable_window_parent_class)->map (widget);
}

static void
hildon_stackable_window_show                    (GtkWidget *widget)
{
    HildonStackableWindowPrivate *priv = HILDON_STACKABLE_WINDOW_GET_PRIVATE (widget);

    /* Stack the window if not already stacked */
    if (priv->stack == NULL) {
        HildonWindowStack *stack = hildon_window_stack_get_default ();
        _hildon_window_stack_do_push (stack, HILDON_STACKABLE_WINDOW (widget));
    }

    GTK_WIDGET_CLASS (hildon_stackable_window_parent_class)->show (widget);
}

static void
hildon_stackable_window_hide                    (GtkWidget *widget)
{
    HildonStackableWindowPrivate *priv = HILDON_STACKABLE_WINDOW_GET_PRIVATE (widget);

    if (priv->stack) {
        hildon_window_stack_remove (HILDON_STACKABLE_WINDOW (widget));
    }

    GTK_WIDGET_CLASS (hildon_stackable_window_parent_class)->hide (widget);
}

static gboolean
hildon_stackable_window_delete_event            (GtkWidget   *widget,
                                                 GdkEventAny *event)
{
    HildonStackableWindowPrivate *priv = HILDON_STACKABLE_WINDOW_GET_PRIVATE (widget);
    gboolean retvalue = FALSE;

    if (priv->stack && hildon_window_stack_peek (priv->stack) != widget) {
        /* Ignore the delete event if this window is not the topmost one */
        retvalue = TRUE;
    } else if (GTK_WIDGET_CLASS (hildon_stackable_window_parent_class)->delete_event) {
        retvalue = GTK_WIDGET_CLASS (hildon_stackable_window_parent_class)->delete_event (widget, event);
    }

    return retvalue;
}

static void
hildon_stackable_window_class_init              (HildonStackableWindowClass *klass)
{
    GtkWidgetClass    *widget_class = GTK_WIDGET_CLASS (klass);

    widget_class->map               = hildon_stackable_window_map;
    widget_class->show              = hildon_stackable_window_show;
    widget_class->hide              = hildon_stackable_window_hide;
    widget_class->delete_event      = hildon_stackable_window_delete_event;

    g_type_class_add_private (klass, sizeof (HildonStackableWindowPrivate));
}

static void
hildon_stackable_window_init                    (HildonStackableWindow *self)
{
    HildonStackableWindowPrivate *priv = HILDON_STACKABLE_WINDOW_GET_PRIVATE (self);

    priv->stack = NULL;
    priv->stack_position = -1;
}

/**
 * hildon_stackable_window_new:
 *
 * Creates a new #HildonStackableWindow.
 *
 * Return value: A #HildonStackableWindow
 *
 * Since: 2.2
 **/
GtkWidget*
hildon_stackable_window_new                     (void)
{
    HildonStackableWindow *newwindow = g_object_new (HILDON_TYPE_STACKABLE_WINDOW, NULL);

    return GTK_WIDGET (newwindow);
}
