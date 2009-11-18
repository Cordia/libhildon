/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
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
 * SECTION:hildon-window-stack
 * @short_description: A stack of windows in Hildon applications.
 * @see_also: #HildonStackableWindow
 *
 * The #HildonWindowStack is a stack of top-level windows.
 *
 * Stacks contain all #HildonStackableWindow<!-- -->s that are being
 * shown. The user can only interact with the topmost window from each
 * stack (as it covers all the others), but all of them are mapped and
 * visible from the Gtk point of view.
 *
 * Each window can only be in one stack at a time. All stacked windows
 * are visible and all visible windows are stacked.
 *
 * Each application has a default stack, and windows are automatically
 * added to it when they are shown with gtk_widget_show().
 *
 * Additional stacks can be created at any time using
 * hildon_window_stack_new(). To add a window to a specific stack, use
 * hildon_window_stack_push_1() (remember that, for the default stack,
 * gtk_widget_show() can be used instead).
 *
 * To remove a window from a stack use hildon_window_stack_pop_1(), or
 * simply gtk_widget_hide().
 *
 * For more complex layout changes, applications can push and/or pop
 * several windows at the same time in a single step. See
 * hildon_window_stack_push(), hildon_window_stack_pop() and
 * hildon_window_stack_pop_and_push() for more details.
 */

#include                                        "hildon-window-stack.h"
#include                                        "hildon-window-stack-private.h"
#include                                        "hildon-stackable-window-private.h"

struct                                          _HildonWindowStackPrivate
{
    GList *list;
    GtkWindowGroup *group;
    GdkWindow *leader; /* X Window group hint for all windows in a group */
};

#define                                         HILDON_WINDOW_STACK_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
                                                HILDON_TYPE_WINDOW_STACK, HildonWindowStackPrivate))

G_DEFINE_TYPE (HildonWindowStack, hildon_window_stack, G_TYPE_OBJECT);

enum {
    PROP_GROUP = 1,
};

static void
hildon_window_stack_set_window_group             (HildonWindowStack *stack,
                                                  GtkWindowGroup    *group)
{
    g_return_if_fail (HILDON_IS_WINDOW_STACK (stack));
    g_return_if_fail (!group || GTK_IS_WINDOW_GROUP (group));

    /* The window group is only to be set once during construction */
    g_return_if_fail (stack->priv->group == NULL);

    if (!group)
        group = gtk_window_group_new ();

    stack->priv->group = group;
}

static GtkWindowGroup *
hildon_window_stack_get_window_group             (HildonWindowStack *stack)
{
    g_return_val_if_fail (HILDON_IS_WINDOW_STACK (stack), NULL);

    return stack->priv->group;
}

/**
 * hildon_window_stack_get_default:
 *
 * Returns the default window stack. This stack always exists and
 * doesn't need to be created by the application.
 *
 * Return value: the default #HildonWindowStack
 *
 * Since: 2.2
 **/
HildonWindowStack *
hildon_window_stack_get_default                 (void)
{
    static HildonWindowStack *stack = NULL;
    if (G_UNLIKELY (stack == NULL)) {
        stack = g_object_new (HILDON_TYPE_WINDOW_STACK,
                              "window-group", gtk_window_get_group (NULL),
                              NULL);
    }
    return stack;
}

/**
 * hildon_window_stack_new:
 *
 * Creates a new #HildonWindowStack. The stack is initially empty.
 *
 * Return value: a new #HildonWindowStack
 *
 * Since: 2.2
 **/
HildonWindowStack *
hildon_window_stack_new                         (void)
{
    HildonWindowStack *stack = g_object_new (HILDON_TYPE_WINDOW_STACK, NULL);
    return stack;
}

/**
 * hildon_window_stack_size:
 * @stack: A #HildonWindowStack
 *
 * Returns the number of windows in @stack
 *
 * Return value: Number of windows in @stack
 *
 * Since: 2.2
 **/
gint
hildon_window_stack_size                        (HildonWindowStack *stack)
{
    g_return_val_if_fail (HILDON_IS_WINDOW_STACK (stack), 0);

    return g_list_length (stack->priv->list);
}

static GdkWindow *
hildon_window_stack_get_leader_window           (HildonWindowStack *stack,
                                                 GtkWidget         *win)
{
    /* Create the X Window group (leader) if we haven't. */
    if (!stack->priv->leader) {
        if (stack == hildon_window_stack_get_default ()) {
            GdkDisplay *dpy;

            /* We're the default stack, use the default group. */
            dpy = gtk_widget_get_display (win);
            stack->priv->leader = gdk_display_get_default_group (dpy);
        } else {
            static GdkWindowAttr attr = {
                .window_type = GDK_WINDOW_TOPLEVEL,
                .x = 10, .y = 10, .width = 10, .height = 10,
                .wclass = GDK_INPUT_OUTPUT, .event_mask = 0,
            };
            GdkWindow *root;

            /* Create a new X Window group. */
            root = gtk_widget_get_root_window (win);
            stack->priv->leader = gdk_window_new (root, &attr, GDK_WA_X | GDK_WA_Y);
        }
    }

    return stack->priv->leader;
}

/* Set the X Window group of a window when it is realized. */
static void
hildon_window_stack_window_realized             (GtkWidget         *win,
                                                 HildonWindowStack *stack)
{
    GdkWindow *leader = hildon_window_stack_get_leader_window (stack, win);
    gdk_window_set_group (win->window, leader);
}

/* Remove a window from its stack, no matter its position */
void G_GNUC_INTERNAL
hildon_window_stack_remove                      (HildonStackableWindow *win)
{
    HildonWindowStack *stack = hildon_stackable_window_get_stack (win);

    /* If the window is stacked */
    if (stack) {
        GList *pos;

        hildon_stackable_window_set_stack (win, NULL, -1);
        gtk_window_set_transient_for (GTK_WINDOW (win), NULL);
        if (GTK_WIDGET (win)->window) {
            gdk_window_set_group (GTK_WIDGET (win)->window, NULL);
        }

        /* If the window removed is in the middle of the stack, update
         * transiency of other windows */
        pos = g_list_find (stack->priv->list, win);
        g_assert (pos != NULL);
        if (pos->prev) {
            GtkWindow *upper = GTK_WINDOW (pos->prev->data);
            GtkWindow *lower = pos->next ? GTK_WINDOW (pos->next->data) : NULL;
            gtk_window_set_transient_for (upper, lower);
        }

        stack->priv->list = g_list_remove (stack->priv->list, win);

        g_signal_handlers_disconnect_by_func (win, hildon_window_stack_window_realized, stack);
    }
}

/**
 * hildon_window_stack_get_windows:
 * @stack: a #HildonWindowStack
 *
 * Returns the list of windows on this stack (topmost first). The
 * widgets in the list are not individually referenced. Once you are
 * done with the list you must call g_list_free().
 *
 * Returns: a newly-allocated list of #HildonStackableWindow<!-- -->s
 *
 * Since: 2.2
 **/
GList *
hildon_window_stack_get_windows                 (HildonWindowStack *stack)
{
    g_return_val_if_fail (HILDON_IS_WINDOW_STACK (stack), NULL);

    return g_list_copy (stack->priv->list);
}

/**
 * hildon_window_stack_peek:
 * @stack: A %HildonWindowStack
 *
 * Returns the window on top of @stack. The stack is never modified.
 *
 * Return value: the window on top of the stack, or %NULL if the stack
 * is empty.
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_window_stack_peek                        (HildonWindowStack *stack)
{
    GtkWidget *win = NULL;

    g_return_val_if_fail (HILDON_IS_WINDOW_STACK (stack), NULL);

    if (stack->priv->list != NULL) {
        win = GTK_WIDGET (stack->priv->list->data);
    }

    return win;
}

/* This function does everything to push a window to the stack _but_
 * actually calling gtk_widget_show().
 * It's up to each specific push function to decide the order in which
 * to show windows. */
gboolean G_GNUC_INTERNAL
_hildon_window_stack_do_push                    (HildonWindowStack     *stack,
                                                 HildonStackableWindow *win)
{
    HildonWindowStack *current_stack;

    g_return_val_if_fail (HILDON_IS_WINDOW_STACK (stack), FALSE);
    g_return_val_if_fail (HILDON_IS_STACKABLE_WINDOW (win), FALSE);

    current_stack = hildon_stackable_window_get_stack (win);

    if (current_stack == NULL) {
        GtkWidget *parent = hildon_window_stack_peek (stack);
        gint pos = 0;

        if (parent) {
            pos = HILDON_STACKABLE_WINDOW_GET_PRIVATE (parent)->stack_position + 1;
        }

        /* Push the window */
        hildon_stackable_window_set_stack (win, stack, pos);
        stack->priv->list = g_list_prepend (stack->priv->list, win);

        /* Make the window part of the same group as its parent */
        if (parent) {
            gtk_window_set_transient_for (GTK_WINDOW (win), GTK_WINDOW (parent));
        } else {
            gtk_window_group_add_window (stack->priv->group, GTK_WINDOW (win));
        }

        /* Set window group */
        if (GTK_WIDGET_REALIZED (win)) {
            hildon_window_stack_window_realized (GTK_WIDGET (win), stack);
        } else {
            g_signal_connect (win, "realize",
                              G_CALLBACK (hildon_window_stack_window_realized),
                              stack);
        }

        return TRUE;
    } else {
        g_warning ("Trying to push a window that is already on a stack");
        return FALSE;
    }
}

static GtkWidget *
_hildon_window_stack_do_pop                     (HildonWindowStack *stack)
{
    GtkWidget *win = hildon_window_stack_peek (stack);

    if (win)
        hildon_window_stack_remove (HILDON_STACKABLE_WINDOW (win));

    return win;
}

/**
 * hildon_window_stack_push_1:
 * @stack: A %HildonWindowStack
 * @win: A %HildonStackableWindow
 *
 * Adds @win to the top of @stack, and shows it. The window must not
 * be already stacked.
 *
 * Since: 2.2
 **/
void
hildon_window_stack_push_1                      (HildonWindowStack     *stack,
                                                 HildonStackableWindow *win)
{
    if (_hildon_window_stack_do_push (stack, win))
        gtk_widget_show (GTK_WIDGET (win));
}

/**
 * hildon_window_stack_pop_1:
 * @stack: A %HildonWindowStack
 *
 * Removes the window on top of @stack, and hides it. If the stack is
 * empty nothing happens.
 *
 * Return value: the window on top of the stack, or %NULL if the stack
 * is empty.
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_window_stack_pop_1                       (HildonWindowStack *stack)
{
    GtkWidget *win = _hildon_window_stack_do_pop (stack);
    if (win)
        gtk_widget_hide (win);
    return win;
}

/**
 * hildon_window_stack_push_list:
 * @stack: A %HildonWindowStack
 * @list: A list of %HildonStackableWindow<!-- -->s to push
 *
 * Pushes all windows in @list to the top of @stack, and shows
 * them. Everything is done in a single transition, so the user will
 * only see the last window in @list during this operation. None of
 * the windows must be already stacked.
 *
 * Since: 2.2
 **/
void
hildon_window_stack_push_list                   (HildonWindowStack *stack,
                                                 GList             *list)
{
    HildonStackableWindow *win;
    GList *l;
    GList *pushed = NULL;

    g_return_if_fail (HILDON_IS_WINDOW_STACK (stack));

    /* Stack all windows */
    for (l = list; l != NULL; l = g_list_next (l)) {
        win = HILDON_STACKABLE_WINDOW (l->data);
        if (win) {
            _hildon_window_stack_do_push (stack, win);
            pushed = g_list_prepend (pushed, win);
        } else {
            g_warning ("Trying to stack a non-stackable window!");
        }
    }

    /* Show windows in reverse order (topmost first) */
    g_list_foreach (pushed, (GFunc) gtk_widget_show, NULL);

    g_list_free (pushed);
}

/**
 * hildon_window_stack_push:
 * @stack: A %HildonWindowStack
 * @win1: The first window to push
 * @Varargs: A %NULL-terminated list of additional #HildonStackableWindow<!-- -->s to push.
 *
 * Pushes all windows to the top of @stack, and shows them. Everything
 * is done in a single transition, so the user will only see the last
 * window. None of the windows must be already stacked.
 *
 * Since: 2.2
 **/
void
hildon_window_stack_push                        (HildonWindowStack     *stack,
                                                 HildonStackableWindow *win1,
                                                 ...)
{
    HildonStackableWindow *win = win1;
    GList *list = NULL;
    va_list args;

    va_start (args, win1);

    while (win != NULL) {
        list = g_list_prepend (list, win);
        win = va_arg (args, HildonStackableWindow *);
    }

    va_end (args);

    list = g_list_reverse (list);

    hildon_window_stack_push_list (stack, list);
    g_list_free (list);
}

/**
 * hildon_window_stack_pop:
 * @stack: A %HildonWindowStack
 * @nwindows: Number of windows to pop
 * @popped_windows: if non-%NULL, the list of popped windows is stored here
 *
 * Pops @nwindows windows from @stack, and hides them. Everything is
 * done in a single transition, so the user will not see any of the
 * windows being popped in this operation.
 *
 * If @popped_windows is not %NULL, the list of popped windows is
 * stored there (ordered bottom-up). That list must be freed by the
 * user.
 *
 * Since: 2.2
 **/
void
hildon_window_stack_pop                         (HildonWindowStack  *stack,
                                                 gint                nwindows,
                                                 GList             **popped_windows)
{
    gint i;
    GList *popped = NULL;

    g_return_if_fail (HILDON_IS_WINDOW_STACK (stack));
    g_return_if_fail (nwindows > 0);
    g_return_if_fail (g_list_length (stack->priv->list) >= nwindows);

    /* Pop windows */
    for (i = 0; i < nwindows; i++) {
        GtkWidget *win = _hildon_window_stack_do_pop (stack);
        popped = g_list_prepend (popped, win);
    }

    /* Hide windows in reverse order (topmost last) */
    g_list_foreach (popped, (GFunc) gtk_widget_hide, NULL);

    if (popped_windows) {
        *popped_windows = popped;
    } else {
        g_list_free (popped);
    }
}

/**
 * hildon_window_stack_pop_and_push_list:
 * @stack: A %HildonWindowStack
 * @nwindows: Number of windows to pop.
 * @popped_windows: if non-%NULL, the list of popped windows is stored here
 * @list: A list of %HildonStackableWindow<!-- -->s to push
 *
 * Pops @nwindows windows from @stack (and hides them), then pushes
 * all windows in @list (and shows them). Everything is done in a
 * single transition, so the user will only see the last window from
 * @list. None of the pushed windows must be already stacked.
 *
 * If @popped_windows is not %NULL, the list of popped windows is
 * stored there (ordered bottom-up). That list must be freed by the
 * user.
 *
 * Since: 2.2
 **/
void
hildon_window_stack_pop_and_push_list           (HildonWindowStack  *stack,
                                                 gint                nwindows,
                                                 GList             **popped_windows,
                                                 GList              *list)
{
    gint i, topmost_index;
    GList *l;
    GList *popped = NULL;
    GList *pushed = NULL;
    HildonStackableWindowPrivate *priv;

    g_return_if_fail (HILDON_IS_WINDOW_STACK (stack));
    g_return_if_fail (nwindows > 0);
    g_return_if_fail (g_list_length (stack->priv->list) >= nwindows);

    /*
     * We need to call gdk_flush() because the application that called us might
     * just shown some of the windows we manipulate, and if the GTK+ has not
     * processed the MapNotify yet it will end up calling the XWithdrawWindow
     * when we hide the window (and will actually not unmap it).
     */
    gdk_flush ();

    /* Store the index of the topmost window */
    priv = HILDON_STACKABLE_WINDOW_GET_PRIVATE (hildon_window_stack_peek (stack));
    topmost_index = priv->stack_position;

    /* Pop windows */
    for (i = 0; i < nwindows; i++) {
        GtkWidget *win = _hildon_window_stack_do_pop (stack);
        popped = g_list_prepend (popped, win);
        /* Hide now windows that are popped and then pushed back.
           This way all the windows that has a changed stack index
           will be unmapped and mapped again. */
        if (g_list_find (list, win) != NULL) {
            gtk_widget_hide (win);
        }
    }

    /* Push windows */
    for (l = list; l != NULL; l = g_list_next (l)) {
        HildonStackableWindow *win = HILDON_STACKABLE_WINDOW (l->data);
        if (win) {
            _hildon_window_stack_do_push (stack, win);
            pushed = g_list_prepend (pushed, win);
        } else {
            g_warning ("Trying to stack a non-stackable window!");
        }
    }

    if (pushed != NULL) {
        /* The WM will be confused if the old topmost window and the new
         * one have the same index, so make sure that they're different */
        priv = HILDON_STACKABLE_WINDOW_GET_PRIVATE (hildon_window_stack_peek (stack));
        if (priv->stack_position == topmost_index) {
            priv->stack_position++;
        }
    }

    /* Show windows in reverse order (topmost first) */
    g_list_foreach (pushed, (GFunc) gtk_widget_show, NULL);

    /* Hide windows that are popped but not pushed back (topmost last) */
    for (l = popped; l != NULL; l = l->next) {
        if (g_list_find (list, l->data) == NULL) {
            gtk_widget_hide (GTK_WIDGET (l->data));
        }
    }

    g_list_free (pushed);
    if (popped_windows) {
        *popped_windows = popped;
    } else {
        g_list_free (popped);
    }
}

/**
 * hildon_window_stack_pop_and_push:
 * @stack: A %HildonWindowStack
 * @nwindows: Number of windows to pop.
 * @popped_windows: if non-%NULL, the list of popped windows is stored here
 * @win1: The first window to push
 * @Varargs: A %NULL-terminated list of additional #HildonStackableWindow<!-- -->s to push.
 *
 * Pops @nwindows windows from @stack (and hides them), then pushes
 * all passed windows (and shows them). Everything is done in a single
 * transition, so the user will only see the last pushed window. None
 * of the pushed windows must be already stacked.
 *
 * If @popped_windows is not %NULL, the list of popped windows is
 * stored there (ordered bottom-up). That list must be freed by the
 * user.
 *
 * Since: 2.2
 **/
void
hildon_window_stack_pop_and_push                (HildonWindowStack      *stack,
                                                 gint                    nwindows,
                                                 GList                 **popped_windows,
                                                 HildonStackableWindow  *win1,
                                                 ...)
{
    HildonStackableWindow *win = win1;
    GList *list = NULL;
    va_list args;

    va_start (args, win1);

    while (win != NULL) {
        list = g_list_prepend (list, win);
        win = va_arg (args, HildonStackableWindow *);
    }

    va_end (args);

    list = g_list_reverse (list);

    hildon_window_stack_pop_and_push_list (stack, nwindows, popped_windows, list);
    g_list_free (list);
}

static void
hildon_window_stack_finalize (GObject *object)
{
    HildonWindowStack *stack = HILDON_WINDOW_STACK (object);

    if (stack->priv->list)
        hildon_window_stack_pop (stack, hildon_window_stack_size (stack), NULL);

    if (stack->priv->group)
        g_object_unref (stack->priv->group);

    /* Since the default group stack shouldn't be finalized,
     * it's safe to destroy the X Window group we created. */
    if (stack->priv->leader)
        gdk_window_destroy (stack->priv->leader);

    G_OBJECT_CLASS (hildon_window_stack_parent_class)->finalize (object);
}

static void
hildon_window_stack_set_property                (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
    HildonWindowStack *stack = HILDON_WINDOW_STACK (object);

    switch (prop_id)
    {
    case PROP_GROUP:
        hildon_window_stack_set_window_group (stack, g_value_get_object (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
hildon_window_stack_get_property                (GObject    *object,
                                                 guint       prop_id,
                                                 GValue     *value,
                                                 GParamSpec *pspec)
{
    HildonWindowStack *stack = HILDON_WINDOW_STACK (object);

    switch (prop_id)
    {
    case PROP_GROUP:
        g_value_set_object (value, hildon_window_stack_get_window_group (stack));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
hildon_window_stack_class_init (HildonWindowStackClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;

    gobject_class->set_property = hildon_window_stack_set_property;
    gobject_class->get_property = hildon_window_stack_get_property;
    gobject_class->finalize = hildon_window_stack_finalize;

    g_object_class_install_property (
        gobject_class,
        PROP_GROUP,
        g_param_spec_object (
            "window-group",
            "GtkWindowGroup for this stack",
            "GtkWindowGroup that all windows on this stack belong to",
            GTK_TYPE_WINDOW_GROUP,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_type_class_add_private (klass, sizeof (HildonWindowStackPrivate));
}

static void
hildon_window_stack_init (HildonWindowStack *self)
{
    HildonWindowStackPrivate *priv;

    priv = self->priv = HILDON_WINDOW_STACK_GET_PRIVATE (self);

    priv->list = NULL;
    priv->group = NULL;
}
