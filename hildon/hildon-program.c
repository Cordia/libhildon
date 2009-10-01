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
 * SECTION:hildon-program
 * @short_description: Application abstraction in the Hildon framework.
 * @see_also: #HildonWindow, #HildonStackableWindow
 *
 * #HildonProgram is an object used to represent an application running
 * in the Hildon framework.
 *
 * Applications can have one or more #HildonWindow<!-- -->s. These
 * can be registered in the #HildonProgram with hildon_program_add_window(),
 * and can be unregistered similarly with hildon_program_remove_window().
 *
 * #HildonProgram provides the programmer with commodities such
 * as applying a common toolbar and menu to all registered
 * #HildonWindow<!-- -->s. This is done with hildon_program_set_common_menu(),
 * hildon_program_set_common_app_menu() and hildon_program_set_common_toolbar().
 *
 * #HildonProgram is also used to apply program-wide properties that
 * are specific to the Hildon framework. For instance
 * hildon_program_set_can_hibernate() sets whether or not an application
 * can be set to hibernate by the Hildon task navigator, in situations of
 * low memory.
 *
 * <example>
 * <programlisting>
 * HildonProgram *program;
 * HildonWindow *window1;
 * HildonWindow *window2;
 * GtkToolbar *common_toolbar, *window_specific_toolbar;
 * HildonAppMenu *menu;
 * <!-- -->
 * program = HILDON_PROGRAM (hildon_program_get_instance ());
 * <!-- -->
 * window1 = HILDON_WINDOW (hildon_window_new ());
 * window2 = HILDON_WINDOW (hildon_window_new ());
 * <!-- -->
 * common_toolbar = create_common_toolbar ();
 * window_specific_toolbar = create_window_specific_toolbar ();
 * <!-- -->
 * menu = create_menu ();
 * <!-- -->
 * hildon_program_add_window (program, window1);
 * hildon_program_add_window (program, window2);
 * <!-- -->
 * hildon_program_set_common_app_menu (program, menu);
 * <!-- -->
 * hildon_program_set_common_toolbar (program, common_toolbar);
 * hildon_window_add_toolbar (window1, window_specific_toolbar);
 * <!-- -->
 * hildon_program_set_can_hibernate (program, TRUE);
 * </programlisting>
 * </example>
 */

#undef                                          HILDON_DISABLE_DEPRECATED

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <gdk/gdkx.h>
#include                                        <X11/Xatom.h>

#include                                        "hildon-program.h"
#include                                        "hildon-program-private.h"
#include                                        "hildon-window-private.h"
#include                                        "hildon-window-stack.h"
#include                                        "hildon-app-menu-private.h"

static void
hildon_program_init                             (HildonProgram *self);

static void
hildon_program_finalize                         (GObject *self);

static void
hildon_program_class_init                       (HildonProgramClass *self);

static void
hildon_program_get_property                     (GObject *object, 
                                                 guint property_id,
                                                 GValue *value, 
                                                 GParamSpec *pspec);
static void
hildon_program_set_property                     (GObject *object, 
                                                 guint property_id,
                                                 const GValue *value, 
                                                 GParamSpec *pspec);

enum
{
    PROP_0,
    PROP_IS_TOPMOST,
    PROP_KILLABLE
};

GType G_GNUC_CONST
hildon_program_get_type                         (void)
{
    static GType program_type = 0;

    if (! program_type)
    {
        static const GTypeInfo program_info =
        {
            sizeof (HildonProgramClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) hildon_program_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof (HildonProgram),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_program_init,
        };
        program_type = g_type_register_static(G_TYPE_OBJECT,
                "HildonProgram", &program_info, 0);
    }
    return program_type;
}

static void
hildon_program_init                             (HildonProgram *self)
{
    HildonProgramPrivate *priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);
    
    priv->killable = FALSE;
    priv->window_count = 0;
    priv->is_topmost = FALSE;
    priv->common_menu = NULL;
    priv->common_app_menu = NULL;
    priv->common_toolbar = NULL;
    priv->windows = NULL;
}

static void
hildon_program_finalize                         (GObject *self)
{
    HildonProgramPrivate *priv = HILDON_PROGRAM_GET_PRIVATE (HILDON_PROGRAM (self));
    g_assert (priv);
    
    if (priv->common_toolbar)
    {
        g_object_unref (priv->common_toolbar);
        priv->common_toolbar = NULL;
    }

    if (priv->common_menu)
    {
        g_object_unref (priv->common_menu);
        priv->common_menu = NULL;
    }
}

static void
hildon_program_class_init                       (HildonProgramClass *self)
{
    GObjectClass *object_class = G_OBJECT_CLASS (self);

    g_type_class_add_private (self, sizeof (HildonProgramPrivate));

    /* Set up object virtual functions */
    object_class->finalize      = hildon_program_finalize;
    object_class->set_property  = hildon_program_set_property;
    object_class->get_property  = hildon_program_get_property;

    /* Install properties */

    /**
     * HildonProgram:is-topmost:
     *
     * Whether one of the program's window or dialog currently
     * is activated by window manager. 
     */
    g_object_class_install_property (object_class, PROP_IS_TOPMOST,
                g_param_spec_boolean ("is-topmost",
                "Is top-most",
                "Whether one of the program's window or dialog currently "
                "is activated by window manager",
                FALSE,
                G_PARAM_READABLE)); 

    /**
     * HildonProgram:can-hibernate:
     *
     * Whether the program should be set to hibernate by the Task
     * Navigator in low memory situation.
     */
    g_object_class_install_property (object_class, PROP_KILLABLE,
                g_param_spec_boolean ("can-hibernate",
                "Can hibernate",
                "Whether the program should be set to hibernate by the Task "
                "Navigator in low memory situation",
                FALSE,
                G_PARAM_READWRITE)); 
    return;
}

static void
hildon_program_set_property                     (GObject *object, 
                                                 guint property_id,
                                                 const GValue *value, 
                                                 GParamSpec *pspec)
{
    switch (property_id) {

        case PROP_KILLABLE:
            hildon_program_set_can_hibernate (HILDON_PROGRAM (object), g_value_get_boolean (value));
            break;
            
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }

}

static void
hildon_program_get_property                     (GObject *object, 
                                                 guint property_id,
                                                 GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonProgramPrivate *priv = HILDON_PROGRAM_GET_PRIVATE (object);
    g_assert (priv);

    switch (property_id)
    {
        case PROP_KILLABLE:
            g_value_set_boolean (value, priv->killable);
            break;

        case PROP_IS_TOPMOST:
            g_value_set_boolean (value, priv->is_topmost);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

/**
 * hildon_program_pop_window_stack:
 * @self: A #HildonProgram
 *
 * Pops a window from the stack.
 *
 * Deprecated: Use hildon_window_stack_pop() instead
 *
 * Returns: A #HildonStackableWindow, or %NULL
 *
 * Since: 2.2
 */
HildonStackableWindow *
hildon_program_pop_window_stack                 (HildonProgram *self)
{
    HildonWindowStack *stack = hildon_window_stack_get_default ();
    GtkWidget *win = hildon_window_stack_pop_1 (stack);
    g_warning ("%s: this function is deprecated. Use hildon_window_stack_pop() instead", __FUNCTION__);
    return win ? HILDON_STACKABLE_WINDOW (win) : NULL;
}

/**
 * hildon_program_peek_window_stack:
 * @self: A #HildonProgram
 *
 * Deprecated: Use hildon_window_stack_peek() instead
 *
 * Returns: A #HildonStackableWindow, or %NULL
 *
 * Since: 2.2
 */
HildonStackableWindow *
hildon_program_peek_window_stack                (HildonProgram *self)
{
    HildonWindowStack *stack = hildon_window_stack_get_default ();
    GtkWidget *win = hildon_window_stack_peek (stack);
    g_warning ("%s: this function is deprecated. Use hildon_window_stack_peek() instead", __FUNCTION__);
    return win ? HILDON_STACKABLE_WINDOW (win) : NULL;
}

/* Utilities */
static gint 
hildon_program_window_list_compare              (gconstpointer window_a, 
                                                 gconstpointer window_b)
{
    g_return_val_if_fail (HILDON_IS_WINDOW(window_a) && 
                          HILDON_IS_WINDOW(window_b), 1);

    return window_a != window_b;
}

/*
 * foreach function, checks if a window is topmost and acts consequently
 */
static void
hildon_program_window_list_is_is_topmost        (gpointer data, 
                                                 gpointer window_id_)
{
    if (data && HILDON_IS_WINDOW (data))
    {
        HildonWindow *window = HILDON_WINDOW (data);
        Window window_id = * (Window*)window_id_;

        hildon_window_update_topmost (window, window_id);
    }
}

/*
 * Check the _MB_CURRENT_APP_WINDOW on the root window, and update
 * the top_most status accordingly
 */
static void
hildon_program_update_top_most                  (HildonProgram *program)
{
    gboolean is_topmost;
    Window active_window;
    HildonProgramPrivate *priv;

    priv = HILDON_PROGRAM_GET_PRIVATE (program);
    g_assert (priv);
    
    active_window = hildon_window_get_active_window();
    is_topmost = FALSE;

    if (active_window)
    {
      gint xerror;
      XWMHints *wm_hints;
      
      gdk_error_trap_push ();
      wm_hints = XGetWMHints (GDK_DISPLAY (), active_window);
      xerror = gdk_error_trap_pop ();
      if (xerror && xerror != BadWindow)
      {
        if (wm_hints)
          XFree (wm_hints);
        return;
      }

      if (wm_hints)
      {
        GSList *iter;
        for (iter = priv->windows ; iter && !is_topmost; iter = iter->next)
          {
            GdkWindow *gdkwin = GTK_WIDGET (iter->data)->window;
            GdkWindow *group = gdkwin ? gdk_window_get_group (gdkwin) : NULL;
            if (group)
              is_topmost = wm_hints->window_group == GDK_WINDOW_XID (group);
          }
        XFree (wm_hints);
      }
    }

    /* Send notification if is_topmost has changed */
    if (!priv->is_topmost != !is_topmost)
    {
      priv->is_topmost = is_topmost;
      g_object_notify (G_OBJECT (program), "is-topmost");
    }

    /* Check each window if it was is_topmost */
    g_slist_foreach (priv->windows, 
            (GFunc)hildon_program_window_list_is_is_topmost, &active_window);
}

/*
 * We keep track of the _MB_CURRENT_APP_WINDOW property on the root window,
 * to detect when a window belonging to this program was is_topmost. This
 * is based on the window group WM hint.
 */
static GdkFilterReturn
hildon_program_root_window_event_filter         (GdkXEvent *xevent,
                                                 GdkEvent *event,
                                                 gpointer data)
{
    XAnyEvent *eventti = xevent;
    HildonProgram *program = HILDON_PROGRAM (data);
    Atom active_app_atom =
            XInternAtom (GDK_DISPLAY (), "_MB_CURRENT_APP_WINDOW", False);

    if (eventti->type == PropertyNotify)
    {
        XPropertyEvent *pevent = xevent;

        if (pevent->atom == active_app_atom)
        {
            hildon_program_update_top_most (program);
        }
    }

    return GDK_FILTER_CONTINUE;
}

static void
hildon_program_window_set_common_menu_flag (HildonWindow *window,
                                            gboolean common_menu)
{
    if (HILDON_IS_WINDOW (window))
    {
        gboolean has_menu = hildon_window_get_app_menu (window) ||
            hildon_window_get_main_menu (window);

        if (!has_menu) {
            hildon_window_set_menu_flag (window, common_menu);
        }
    }
}

static void
hildon_program_set_common_menu_flag (HildonProgram *self,
                                     gboolean common_menu)
{
    HildonProgramPrivate *priv = HILDON_PROGRAM_GET_PRIVATE (self);

    g_slist_foreach (priv->windows,
                     (GFunc) hildon_program_window_set_common_menu_flag,
                     GINT_TO_POINTER (common_menu));
}

/* 
 * Checks if the window is the topmost window of the program and in
 * that case forces the window to take the common toolbar.
 */
static void
hildon_program_common_toolbar_topmost_window    (gpointer window, 
                                                 gpointer data)
{
    if (HILDON_IS_WINDOW (window) && hildon_window_get_is_topmost (HILDON_WINDOW (window)))
        hildon_window_take_common_toolbar (HILDON_WINDOW (window));
}

/**
 * hildon_program_get_instance:
 *
 * Returns the #HildonProgram for the current process. The object is
 * created on the first call. Note that you're not supposed to unref
 * the returned object since it's not reffed in the first place.
 *
 * Return value: the #HildonProgram.
 **/
HildonProgram*
hildon_program_get_instance                     (void)
{
    static HildonProgram *program = NULL;

    if (! program)
    {
        program = g_object_new (HILDON_TYPE_PROGRAM, NULL);
    }

    return program;
}

/**
 * hildon_program_add_window:
 * @self: The #HildonProgram to which the window should be registered
 * @window: A #HildonWindow to be added
 *
 * Registers a #HildonWindow as belonging to a given #HildonProgram. This
 * allows to apply program-wide settings as all the registered windows,
 * such as hildon_program_set_common_menu(), hildon_program_set_common_app_menu()
 * and hildon_program_set_common_toolbar().
 **/
void
hildon_program_add_window                       (HildonProgram *self, 
                                                 HildonWindow *window)
{
    HildonProgramPrivate *priv;
    
    g_return_if_fail (HILDON_IS_PROGRAM (self));
    g_return_if_fail (HILDON_IS_WINDOW (window));
    
    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    if (g_slist_find_custom (priv->windows, window,
           hildon_program_window_list_compare) )
    {
        /* We already have that window */
        return;
    }

    if (!priv->window_count)
    {
        hildon_program_update_top_most (self);
        
        /* Now that we have a window we should start keeping track of
         * the root window */
        gdk_window_set_events (gdk_get_default_root_window (),
                gdk_window_get_events (gdk_get_default_root_window ()) | GDK_PROPERTY_CHANGE_MASK);

        gdk_window_add_filter (gdk_get_default_root_window (),
                hildon_program_root_window_event_filter, self );
    }
    
    hildon_window_set_can_hibernate_property (window, &priv->killable);

    hildon_window_set_program (window, G_OBJECT (self));

    if (priv->common_menu || priv->common_app_menu)
        hildon_program_window_set_common_menu_flag (window, TRUE);

    priv->windows = g_slist_append (priv->windows, window);
    priv->window_count ++;
}

/**
 * hildon_program_remove_window:
 * @self: The #HildonProgram to which the window should be unregistered
 * @window: The #HildonWindow to unregister
 *
 * Used to unregister a window from the program. Subsequent calls to
 * hildon_program_set_common_menu(), hildon_program_set_common_app_menu()
 * and hildon_program_set_common_toolbar() will not affect the window.
 **/
void
hildon_program_remove_window                    (HildonProgram *self, 
                                                 HildonWindow *window)
{
    HildonProgramPrivate *priv;
    
    g_return_if_fail (HILDON_IS_PROGRAM (self));
    g_return_if_fail (HILDON_IS_WINDOW (window));
    
    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);
    
    g_return_if_fail (g_slist_find (priv->windows, window));

    hildon_window_unset_program (window);

    priv->windows = g_slist_remove (priv->windows, window);

    priv->window_count --;

    if (! priv->window_count)
        gdk_window_remove_filter (gdk_get_default_root_window(),
                hildon_program_root_window_event_filter,
                self);

    if (priv->common_menu || priv->common_app_menu)
        hildon_program_window_set_common_menu_flag (window, FALSE);
}

/**
 * hildon_program_set_can_hibernate:
 * @self: The #HildonProgram which can hibernate or not
 * @can_hibernate: whether or not the #HildonProgram can hibernate
 *
 * Used to set whether or not the Hildon task navigator should
 * be able to set the program to hibernation in case of low memory
 **/
void
hildon_program_set_can_hibernate                (HildonProgram *self, 
                                                 gboolean can_hibernate)
{
    HildonProgramPrivate *priv;
    
    g_return_if_fail (HILDON_IS_PROGRAM (self));
    
    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    if (priv->killable != can_hibernate)
    {
        g_slist_foreach (priv->windows, 
                (GFunc) hildon_window_set_can_hibernate_property, &can_hibernate);
    }

    priv->killable = can_hibernate;
}

/**
 * hildon_program_get_can_hibernate:
 * @self: The #HildonProgram which can hibernate or not
 *
 * Returns whether the #HildonProgram is set to be support hibernation
 * from the Hildon task navigator
 *
 * Return value: %TRUE if the program can hibernate, %FALSE otherwise.
 **/
gboolean
hildon_program_get_can_hibernate                (HildonProgram *self)
{
    HildonProgramPrivate *priv;
    
    g_return_val_if_fail (HILDON_IS_PROGRAM (self), FALSE);
   
    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    return priv->killable;
}

/**
 * hildon_program_set_common_menu:
 * @self: The #HildonProgram in which the common menu should be used
 * @menu: A #GtkMenu to use as common menu for the program
 *
 * Sets a #GtkMenu that will appear in all #HildonWindow<!-- -->s
 * registered with the #HildonProgram. Only one common #GtkMenu can be
 * set, further calls will detach the previous common #GtkMenu. A
 * #HildonWindow can use its own #GtkMenu with
 * hildon_window_set_menu()
 *
 * This method does not support #HildonAppMenu<!-- -->s. See
 * hildon_program_set_common_app_menu() for that.
 **/
void
hildon_program_set_common_menu                  (HildonProgram *self, 
                                                 GtkMenu *menu)
{
    HildonProgramPrivate *priv;

    g_return_if_fail (HILDON_IS_PROGRAM (self));

    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    if (priv->common_menu)
    {
        if (GTK_WIDGET_VISIBLE (priv->common_menu))
        {
            gtk_menu_popdown (priv->common_menu);
            gtk_menu_shell_deactivate (GTK_MENU_SHELL (priv->common_menu));
        }

        if (gtk_menu_get_attach_widget (priv->common_menu))
        {
            gtk_menu_detach (priv->common_menu);
        }
        else
        {
            g_object_unref (priv->common_menu);
        }
    }

    /* Only set the menu flag if there was no common menu and
       we are setting one. If we are unsetting the current common menu,
       remove the commmon menu flag. Otherwise, nothing to do. */

    GList *menu_children = gtk_container_get_children (GTK_CONTAINER (menu));
    if (!priv->common_menu
        && menu && menu_children != NULL) {
        hildon_program_set_common_menu_flag (self, TRUE);
    } else if (priv->common_menu &&
               (!menu || menu_children == NULL))
    {
        hildon_program_set_common_menu_flag (self, FALSE);
    }
    g_list_free (menu_children);

    priv->common_menu = menu;

    if (priv->common_menu)
    {
        g_object_ref (menu);
        gtk_object_sink (GTK_OBJECT (menu));
        gtk_widget_show_all (GTK_WIDGET (menu));
    }
}

/**
 * hildon_program_get_common_menu:
 * @self: The #HildonProgram from which to retrieve the common menu
 *
 * Returns the #GtkMenu that was set as common menu for this
 * #HildonProgram.
 *
 * Return value: the #GtkMenu or %NULL of no common menu was set.
 **/
GtkMenu*
hildon_program_get_common_menu                  (HildonProgram *self)
{
    HildonProgramPrivate *priv;

    g_return_val_if_fail (HILDON_IS_PROGRAM (self), NULL);

    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    return priv->common_menu;
}

static void
hildon_program_on_common_app_menu_changed       (HildonAppMenu *menu,
                                                 HildonProgram *program)
{
    hildon_program_set_common_menu_flag (program,
                                         hildon_app_menu_has_visible_children (menu));
}

/**
 * hildon_program_set_common_app_menu:
 * @self: The #HildonProgram in which the common menu should be used
 * @menu: A #HildonAppMenu to use as common menu for the program
 *
 * Sets a #HildonAppMenu that will appear in all
 * #HildonWindow<!-- -->s registered with the #HildonProgram. Only
 * one common #HildonAppMenu can be set, further calls will detach the
 * previous common #HildonAppMenu. A #HildonWindow can use its own
 * #HildonAppMenu with hildon_window_set_app_menu()
 *
 * This method does not support #GtkMenu<!-- -->s. See
 * hildon_program_set_common_menu() for that.
 *
 * Since: 2.2
 **/
void
hildon_program_set_common_app_menu              (HildonProgram *self,
                                                 HildonAppMenu *menu)
{
    HildonProgramPrivate *priv;
    HildonAppMenu *old_menu;

    g_return_if_fail (HILDON_IS_PROGRAM (self));
    g_return_if_fail (menu == NULL || HILDON_IS_APP_MENU (menu));

    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    old_menu = priv->common_app_menu;

    /* Only set the menu flag if there was no common menu and
       we are setting one. If we are unsetting the current common menu,
       remove the commmon menu flag. Otherwise, nothing to do. */
    if (!priv->common_app_menu
        && menu && hildon_app_menu_has_visible_children (menu)) {
        hildon_program_set_common_menu_flag (self, TRUE);
    } else if (priv->common_app_menu &&
               (!menu || !hildon_app_menu_has_visible_children (menu))) {
        hildon_program_set_common_menu_flag (self, FALSE);
    }

    /* Set new menu */
    priv->common_app_menu = menu;
    if (menu) {
        g_signal_connect (menu, "changed",
                          G_CALLBACK (hildon_program_on_common_app_menu_changed), self);
        g_object_ref_sink (menu);
    }

    /* Hide and unref old menu */
    if (old_menu) {
        hildon_app_menu_set_parent_window (old_menu, NULL);
        g_signal_handlers_disconnect_by_func (old_menu,
                                              hildon_program_on_common_app_menu_changed,
                                              self);
        g_object_unref (old_menu);
    }
}

/**
 * hildon_program_get_common_app_menu:
 * @self: The #HildonProgram from which to retrieve the common app menu
 *
 * Returns the #HildonAppMenu that was set as common menu for this
 * #HildonProgram.
 *
 * Return value: the #HildonAppMenu or %NULL of no common app menu was
 * set.
 *
 * Since: 2.2
 **/
HildonAppMenu*
hildon_program_get_common_app_menu              (HildonProgram *self)
{
    HildonProgramPrivate *priv;

    g_return_val_if_fail (HILDON_IS_PROGRAM (self), NULL);

    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    return priv->common_app_menu;
}

/**
 * hildon_program_set_common_toolbar:
 * @self: The #HildonProgram in which the common toolbar should be used
 * @toolbar: A #GtkToolbar to use as common toolbar for the program
 *
 * Sets a #GtkToolbar that will appear in all the #HildonWindow registered
 * to the #HildonProgram. Only one common #GtkToolbar can be set, further
 * call will detach the previous common #GtkToolbar. A #HildonWindow
 * can use its own #GtkToolbar with hildon_window_add_toolbar(). Both
 * #HildonProgram and #HildonWindow specific toolbars will be shown
 **/
void
hildon_program_set_common_toolbar               (HildonProgram *self, 
                                                 GtkToolbar *toolbar)
{
    HildonProgramPrivate *priv;

    g_return_if_fail (HILDON_IS_PROGRAM (self));

    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    if (priv->common_toolbar)
    {
        if (priv->common_toolbar->parent)
        {
            gtk_container_remove (GTK_CONTAINER (priv->common_toolbar->parent), 
                                  priv->common_toolbar);
        }
        
        g_object_unref (priv->common_toolbar);
    }

    priv->common_toolbar = GTK_WIDGET (toolbar);

    if (priv->common_toolbar)
    {
        g_object_ref (priv->common_toolbar);
        gtk_object_sink (GTK_OBJECT (priv->common_toolbar) );
    }

    /* if the program is the topmost we have to update the common
       toolbar right now for the topmost window */
    if (priv->is_topmost)
      {
        g_slist_foreach (priv->windows, 
                         (GFunc) hildon_program_common_toolbar_topmost_window, NULL);
      }
}

/**
 * hildon_program_get_common_toolbar:
 * @self: The #HildonProgram from which to retrieve the common toolbar
 *
 * Returns the #GtkToolbar that was set as common toolbar for this
 * #HildonProgram.
 *
 * Return value: the #GtkToolbar or %NULL of no common toolbar was
 * set.
 **/
GtkToolbar*
hildon_program_get_common_toolbar               (HildonProgram *self)
{
    HildonProgramPrivate *priv;

    g_return_val_if_fail (HILDON_IS_PROGRAM (self), NULL);

    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    return priv->common_toolbar ? GTK_TOOLBAR (priv->common_toolbar) : NULL;
}

/**
 * hildon_program_get_is_topmost:
 * @self: A #HildonWindow
 *
 * Returns whether one of the program's windows or dialogs is
 * currently activated by the window manager.
 *
 * Return value: %TRUE if a window or dialog is topmost, %FALSE
 * otherwise.
 **/
gboolean
hildon_program_get_is_topmost                   (HildonProgram *self)
{
    HildonProgramPrivate *priv;

    g_return_val_if_fail (HILDON_IS_PROGRAM (self), FALSE);
    
    priv = HILDON_PROGRAM_GET_PRIVATE (self);
    g_assert (priv);

    return priv->is_topmost;
}

/**
 * hildon_program_go_to_root_window:
 * @self: A #HildonProgram
 *
 * Goes to the root window of the stack.
 *
 * Deprecated: See #HildonWindowStack
 *
 * Since: 2.2
 */
void
hildon_program_go_to_root_window                (HildonProgram *self)
{
    HildonWindowStack *stack = hildon_window_stack_get_default ();
    gint n = hildon_window_stack_size (stack);
    g_warning ("%s: this function is deprecated. Use hildon_window_stack_pop() instead.", __FUNCTION__);
    if (n > 1) {
        hildon_window_stack_pop (stack, n-1, NULL);
    }
}
