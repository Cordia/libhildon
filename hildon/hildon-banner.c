/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006, 2007 Nokia Corporation, all rights reserved.
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
 * SECTION:hildon-banner
 * @short_description: A widget used to display timed notifications.
 *
 * #HildonBanner is a small, pop-up window that can be used to display
 * a short, timed notification or information to the user. It can
 * communicate that a task has been finished or that the application
 * state has changed.
 *
 * Hildon provides convenient funtions to create and show banners. To
 * create and show information banners you can use
 * hildon_banner_show_information(), hildon_banner_show_informationf()
 * or hildon_banner_show_information_with_markup().
 *
 * If the application window has set the _HILDON_DO_NOT_DISTURB flag (using
 * hildon_gtk_window_set_do_not_disturb() for example), the banner will not
 * be shown. If you need to override this flag for important information,
 * you can use the method hildon_banner_show_information_override_dnd().
 * Please, take into account that this is only for important information.
 *
 *
 * Two more kinds of banners are maintained for backward compatibility
 * but are no longer recommended in Hildon 2.2. These are the animated
 * banner (created with hildon_banner_show_animation()) and the
 * progress banner (created with hildon_banner_show_progress()). See
 * hildon_gtk_window_set_progress_indicator() for the preferred way of
 * showing progress notifications in Hildon 2.2.
 *
 * Information banners are automatically destroyed after a certain
 * period. This is stored in the #HildonBanner:timeout property (in
 * miliseconds), and can be changed using hildon_banner_set_timeout().
 *
 * Note that #HildonBanner<!-- -->s should only be used to display
 * non-critical pieces of information.
 *
 * <example>
 * <title>Using the HildonBanner widget</title>
 * <programlisting>
 * void show_info_banner (GtkWidget *parent)
 * {
 *   GtkWidget *banner;
 * <!-- -->
 *   banner = hildon_banner_show_information (widget, NULL, "Information banner");
 *   hildon_banner_set_timeout (HILDON_BANNER (banner), 9000);
 * <!-- -->
 *   return;
 * }
 * </programlisting>
 * </example>
 */

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <string.h>
#include                                        <X11/Xatom.h>
#include                                        <gdk/gdkx.h>

#undef                                          HILDON_DISABLE_DEPRECATED

#include                                        "hildon-banner.h"
#include                                        "hildon-private.h"
#include                                        "hildon-defines.h"
#include                                        "hildon-gtk.h"

/* max widths */

#define                                         HILDON_BANNER_LABEL_MAX_TIMED \
                                                (gdk_screen_width() - ((HILDON_MARGIN_TRIPLE) * 2))

#define                                         HILDON_BANNER_LABEL_MAX_PROGRESS 375 /*265*/

/* default timeout */

#define                                         HILDON_BANNER_DEFAULT_TIMEOUT 3000

/* default icons */

#define                                         HILDON_BANNER_DEFAULT_PROGRESS_ANIMATION "indicator_update"

/* animation related stuff */

#define                                         HILDON_BANNER_ANIMATION_FRAMERATE ((float)1000/150)

#define                                         HILDON_BANNER_ANIMATION_TMPL "indicator_update%d"

#define                                         HILDON_BANNER_ANIMATION_NFRAMES 8

enum 
{
    PROP_0,
    PROP_PARENT_WINDOW, 
    PROP_IS_TIMED,
    PROP_TIMEOUT
};

static GtkWidget*                               global_timed_banner = NULL;

static GQuark 
hildon_banner_timed_quark                       (void);

static void 
hildon_banner_bind_style                        (HildonBanner *self);

static gboolean 
hildon_banner_timeout                           (gpointer data);

static gboolean 
hildon_banner_clear_timeout                     (HildonBanner *self);

static void 
hildon_banner_ensure_timeout                    (HildonBanner *self);

static void 
hildon_banner_set_property                      (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec);
    
static void 
hildon_banner_get_property                      (GObject *object,
                                                 guint prop_id,
                                                 GValue *value,
                                                 GParamSpec *pspec);

static void
hildon_banner_destroy                           (GtkObject *object);
        
static GObject*
hildon_banner_real_get_instance                 (GObject *window, 
                                                 gboolean timed);

static GObject* 
hildon_banner_constructor                       (GType type,
                                                 guint n_construct_params,
                                                 GObjectConstructParam *construct_params);

static void
hildon_banner_dispose                           (GObject *object);

static void
hildon_banner_finalize                          (GObject *object);

static gboolean
hildon_banner_button_press_event                (GtkWidget* widget,
						 GdkEventButton* event);

static gboolean 
hildon_banner_map_event                         (GtkWidget *widget, 
                                                 GdkEventAny *event);

static void 
force_to_wrap_truncated                         (HildonBanner *banner);

static void
hildon_banner_realize                           (GtkWidget *widget);

static void 
hildon_banner_class_init                        (HildonBannerClass *klass);

static void 
hildon_banner_init                              (HildonBanner *self);

static void
hildon_banner_ensure_child                      (HildonBanner *self, 
                                                 GtkWidget *user_widget,
                                                 guint pos,
                                                 GType type,
                                                 const gchar *first_property, 
                                                 ...);

static HildonBanner*
hildon_banner_get_instance_for_widget           (GtkWidget *widget, 
                                                 gboolean timed);

static void
hildon_banner_set_override_flag                 (HildonBanner *banner);

static void
reshow_banner                                   (HildonBanner *banner);

static GtkWidget*
hildon_banner_real_show_information             (GtkWidget *widget,
                                                 const gchar *text,
                                                 gboolean override_dnd);

G_DEFINE_TYPE (HildonBanner, hildon_banner, GTK_TYPE_WINDOW)

typedef struct                                  _HildonBannerPrivate HildonBannerPrivate;

#define                                         HILDON_BANNER_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                HILDON_TYPE_BANNER, HildonBannerPrivate));

struct                                          _HildonBannerPrivate
{
    GtkWidget   *main_item;
    GtkWidget   *alignment;
    GtkWidget   *label;
    GtkWidget   *layout;
    GtkWindow   *parent;
    const gchar *name_suffix;
    guint        timeout;
    guint        timeout_id;
    guint        is_timed             : 1;
    guint        require_override_dnd : 1;
    guint        overrides_dnd        : 1;
};

static GQuark 
hildon_banner_timed_quark                       (void)
{
    static GQuark quark = 0;

    if (G_UNLIKELY(quark == 0))
        quark = g_quark_from_static_string ("hildon-banner-timed");

    return quark;
}

/* Set the widget and label name to make the correct rc-style attached into them */
static void 
hildon_banner_bind_style                  (HildonBanner *self)
{
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (self);
    GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (self));
    gboolean portrait = gdk_screen_get_width (screen) < gdk_screen_get_height (screen);
    const gchar *portrait_suffix = portrait ? "-portrait" : NULL;
    gchar *name;

    g_assert (priv);

    name = g_strconcat ("HildonBannerLabel-", priv->name_suffix, NULL);
    gtk_widget_set_name (priv->label, name);
    g_free (name);

    name = g_strconcat ("HildonBanner-", priv->name_suffix, portrait_suffix, NULL);
    gtk_widget_set_name (GTK_WIDGET (self), name);
    g_free (name);
}

/* In timeout function we automatically destroy timed banners */
static gboolean
simulate_close (GtkWidget* widget)
{
    gboolean result = FALSE;

    /* If the banner is currently visible (it normally should), 
       we simulate clicking the close button of the window.
       This allows applications to reuse the banner by prevent
       closing it etc */
    if (GTK_WIDGET_DRAWABLE (widget))
    {
        GdkEvent *event = gdk_event_new (GDK_DELETE);
        event->any.window = g_object_ref (widget->window);
        event->any.send_event = FALSE;
        result = gtk_widget_event (widget, event);
        gdk_event_free (event);
    }

    return result;
}

static void
hildon_banner_size_request                      (GtkWidget      *self,
                                                 GtkRequisition *req)
{
    GTK_WIDGET_CLASS (hildon_banner_parent_class)->size_request (self, req);
    req->width = gdk_screen_get_width (gtk_widget_get_screen (self));
}

static gboolean 
hildon_banner_timeout                           (gpointer data)
{
    GtkWidget *widget;
    gboolean continue_timeout = FALSE;

    g_assert (HILDON_IS_BANNER (data));

    widget = GTK_WIDGET (data);
    g_object_ref (widget);

    continue_timeout = simulate_close (widget);

    if (! continue_timeout) {
        HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (data);
        if (priv->timeout_id) {
            g_source_remove (priv->timeout_id);
            priv->timeout_id = 0;
        }
        gtk_widget_destroy (widget);
    }

    g_object_unref (widget);

    return continue_timeout;
}

static gboolean 
hildon_banner_clear_timeout                     (HildonBanner *self)
{
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (self);
    g_assert (priv);

    if (priv->timeout_id != 0) {
        g_source_remove (priv->timeout_id);
        priv->timeout_id = 0;
        return TRUE;
    }

    return FALSE;
}

static void 
hildon_banner_ensure_timeout                    (HildonBanner *self)
{
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (self);
    g_assert (priv);

    if (priv->timeout_id == 0 && priv->is_timed && priv->timeout > 0)
        priv->timeout_id = gdk_threads_add_timeout (priv->timeout,
                hildon_banner_timeout, self);
}

static void 
hildon_banner_set_property                      (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec)
{
    GtkWidget *window;
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (object);
    g_assert (priv);

    switch (prop_id) {

        case PROP_TIMEOUT:
             priv->timeout = g_value_get_uint (value);
             break;
 
        case PROP_IS_TIMED:
            priv->is_timed = g_value_get_boolean (value);
            break;

        case PROP_PARENT_WINDOW:
            window = g_value_get_object (value);         
            if (priv->parent) {
                g_object_remove_weak_pointer(G_OBJECT (priv->parent), (gpointer) &priv->parent);
            }

            gtk_window_set_transient_for (GTK_WINDOW (object), (GtkWindow *) window);
            priv->parent = (GtkWindow *) window;

            if (window) {
                gtk_window_set_destroy_with_parent (GTK_WINDOW (object), TRUE);
                g_object_add_weak_pointer(G_OBJECT (window), (gpointer) &priv->parent);
            }

            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void 
hildon_banner_get_property                      (GObject *object,
                                                 guint prop_id,
                                                 GValue *value,
                                                 GParamSpec *pspec)
{
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (object);
    g_assert (priv);

    switch (prop_id)
    {
        case PROP_TIMEOUT:
             g_value_set_uint (value, priv->timeout);
             break;
 
        case PROP_IS_TIMED:
            g_value_set_boolean (value, priv->is_timed);
            break;

        case PROP_PARENT_WINDOW:
            g_value_set_object (value, gtk_window_get_transient_for (GTK_WINDOW (object)));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_banner_destroy                           (GtkObject *object)
{
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (object);
    g_assert (priv);

    HildonBanner *self;
    GObject *parent_window = (GObject *) priv->parent;

    g_assert (HILDON_IS_BANNER (object));
    self = HILDON_BANNER (object);

    /* Drop possible global pointer. That can hold reference to us */
    if ((gpointer) object == (gpointer) global_timed_banner) {
        global_timed_banner = NULL;
        g_object_unref (object);
    }

    /* Remove the data from parent window for timed banners. Those hold reference */
    if (priv->is_timed && parent_window != NULL) {
        g_object_set_qdata (parent_window, hildon_banner_timed_quark (), NULL);
    }

    if (!priv->is_timed && priv->parent) {
        hildon_gtk_window_set_progress_indicator (priv->parent, 0);
    }

    (void) hildon_banner_clear_timeout (self);

    if (GTK_OBJECT_CLASS (hildon_banner_parent_class)->destroy)
        GTK_OBJECT_CLASS (hildon_banner_parent_class)->destroy (object);
}

/* Search a previous banner instance */
static GObject*
hildon_banner_real_get_instance                 (GObject *window, 
                                                 gboolean timed)
{
    if (timed) {
        /* If we have a parent window, the previous instance is stored there */
        if (window) {
            return g_object_get_qdata(window, hildon_banner_timed_quark ());
        }

        /* System notification instance is stored into global pointer */
        return (GObject *) global_timed_banner;
    }

    /* Non-timed banners are normal (non-singleton) objects */
    return NULL;
}

/* By overriding constructor we force timed banners to be
   singletons for each window */
static GObject* 
hildon_banner_constructor                       (GType type,
                                                 guint n_construct_params,
                                                 GObjectConstructParam *construct_params)
{
    GObject *banner, *window = NULL;
    gboolean timed = FALSE;
    guint i;

    /* Search banner type information from parameters in order
       to locate the possible previous banner instance. */
    for (i = 0; i < n_construct_params; i++)
    {
        if (strcmp(construct_params[i].pspec->name, "parent-window") == 0)
            window = g_value_get_object (construct_params[i].value);       
        else if (strcmp(construct_params[i].pspec->name, "is-timed") == 0)
            timed = g_value_get_boolean (construct_params[i].value);
    }

    /* Try to get a previous instance if such exists */
    banner = hildon_banner_real_get_instance (window, timed);
    if (! banner)
    {
        /* We have to create a new banner */
        banner = G_OBJECT_CLASS (hildon_banner_parent_class)->constructor (type, n_construct_params, construct_params);

        /* Store the newly created singleton instance either into parent 
           window data or into global variables. */
        if (timed) {
            if (window) {
                g_object_set_qdata_full (G_OBJECT (window), hildon_banner_timed_quark (), 
                        g_object_ref (banner), g_object_unref); 
            } else {
                g_assert (global_timed_banner == NULL);
                global_timed_banner = g_object_ref (banner);
            }
        }
    }
    else {
        /* FIXME: This is a hack! We have to manually freeze
           notifications. This is normally done by g_object_init, but we
           are not going to call that. g_object_newv will otherwise give
           a critical like this:

           GLIB CRITICAL ** GLib-GObject - g_object_notify_queue_thaw: 
           assertion `nqueue->freeze_count > 0' failed */

        g_object_freeze_notify (banner);
    }

    /* We restart possible timeouts for each new timed banner request */
    if (timed && hildon_banner_clear_timeout (HILDON_BANNER (banner)))
        hildon_banner_ensure_timeout (HILDON_BANNER(banner));

    return banner;
}

static void
hildon_banner_dispose                           (GObject *object)
{
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (object);

    if (priv->label) {
        g_object_unref (priv->label);
        priv->label = NULL;
    }

    G_OBJECT_CLASS (hildon_banner_parent_class)->dispose (object);
}

static void
hildon_banner_finalize                          (GObject *object)
{
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (object);

    if (priv->parent) {
        g_object_remove_weak_pointer(G_OBJECT (priv->parent), (gpointer) &priv->parent);
    }

    G_OBJECT_CLASS (hildon_banner_parent_class)->finalize (object);
}

static gboolean
hildon_banner_button_press_event                (GtkWidget* widget,
						 GdkEventButton* event)
{
    gboolean result = simulate_close (widget);

    if (!result) {
        /* signal emission not stopped - basically behave like
         * gtk_main_do_event() for a delete event, but just hide the
         * banner instead of destroying it, as it is already meant to
         * be destroyed by hildon_banner_timeout() (if it's timed) or
         * the application (if it's not). */
        gtk_widget_hide (widget);
    }

    return result;
}

#if defined(MAEMO_GTK)
static void
hildon_banner_map                               (GtkWidget *widget)
{
    if (GTK_WIDGET_CLASS (hildon_banner_parent_class)->map) {
        /* Make the banner temporary _before_ mapping it, to avoid closing
         * other temporary windows */
        gtk_window_set_is_temporary (GTK_WINDOW (widget), TRUE);

        GTK_WIDGET_CLASS (hildon_banner_parent_class)->map (widget);

        /* Make the banner non-temporary _after_ mapping it, to avoid
         * being closed by other non-temporary windows */
        gtk_window_set_is_temporary (GTK_WINDOW (widget), FALSE);

        gtk_window_move (GTK_WINDOW (widget), 0, HILDON_WINDOW_TITLEBAR_HEIGHT);
    }
}
#endif

/* We start the timer for timed notifications after the window appears on screen */
static gboolean 
hildon_banner_map_event                         (GtkWidget *widget, 
                                                 GdkEventAny *event)
{
    gboolean result = FALSE;

    if (GTK_WIDGET_CLASS (hildon_banner_parent_class)->map_event)
        result = GTK_WIDGET_CLASS (hildon_banner_parent_class)->map_event (widget, event);

    hildon_banner_ensure_timeout (HILDON_BANNER(widget));

    return result;
}  

static void
banner_do_set_text                              (HildonBanner *banner,
                                                 const gchar  *text,
                                                 gboolean      is_markup)
{
    HildonBannerPrivate *priv;
    GtkRequisition req;

    priv = HILDON_BANNER_GET_PRIVATE (banner);

    if (is_markup) {
        gtk_label_set_markup (GTK_LABEL (priv->label), text);
    } else {
        gtk_label_set_text (GTK_LABEL (priv->label), text);
    }
    gtk_widget_set_size_request (priv->label, -1, -1);
    gtk_widget_size_request (priv->label, &req);

    force_to_wrap_truncated (banner);
}

/* force to wrap truncated label by setting explicit size request
 * see N#27000 and G#329646 */
static void 
force_to_wrap_truncated                         (HildonBanner *banner)
{
    PangoLayout *layout;
    int lines;
    int width;
    int height = -1;
    PangoRectangle logical;
    GtkRequisition requisition;
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (banner);

    g_return_if_fail (priv);

    width = priv->is_timed ? HILDON_BANNER_LABEL_MAX_TIMED
        : HILDON_BANNER_LABEL_MAX_PROGRESS;

    /* Force the label to compute its layout using the maximum
     * available width rather than its default one.
     */
    gtk_widget_set_size_request (priv->label, width, height);
    gtk_widget_size_request (priv->label, &requisition);

    layout = gtk_label_get_layout (GTK_LABEL (priv->label));
    pango_layout_get_extents (layout, NULL, &logical);

    /* Now get the actual width needed by the pango layout */
    width = PANGO_PIXELS (logical.width);

    /* If the layout has now been wrapped and exceeds 3 lines, we truncate
     * the rest of the label according to spec.
     */
    lines = pango_layout_get_line_count (layout);
    if (pango_layout_is_wrapped (layout) && lines > 3) {
        /* This calculation assumes that the same font is used
         * throughout the banner -- this is usually the case on maemo
         *
         * FIXME: Pango >= 1.20 has pango_layout_set_height().
         */
        height = (PANGO_PIXELS (logical.height) * 3) / lines + 1;
    }

    /* Set the final width/height */
    gtk_widget_set_size_request (priv->label, width, height);
}

static void
screen_size_changed                            (GdkScreen *screen,
                                                GtkWindow *banner)

{
    if (GTK_WIDGET_VISIBLE (banner)) {
        HildonBanner *hbanner = HILDON_BANNER (banner);
        hildon_banner_bind_style (hbanner);
        reshow_banner (hbanner);
    }
}

static void
hildon_banner_realize                           (GtkWidget *widget)
{
    GdkWindow *gdkwin;
    GdkScreen *screen;
    GdkAtom atom;
    guint32 portrait = 1;
    const gchar *notification_type = "_HILDON_NOTIFICATION_TYPE_BANNER";
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (widget);
    g_assert (priv);

    /* We let the parent to init widget->window before we need it */
    if (GTK_WIDGET_CLASS (hildon_banner_parent_class)->realize)
        GTK_WIDGET_CLASS (hildon_banner_parent_class)->realize (widget);

    /* We use special hint to turn the banner into information notification. */
    gdk_window_set_type_hint (widget->window, GDK_WINDOW_TYPE_HINT_NOTIFICATION);
    gtk_window_set_transient_for (GTK_WINDOW (widget), (GtkWindow *) priv->parent);

    gdkwin = widget->window;

    /* Set the _HILDON_NOTIFICATION_TYPE property so Matchbox places the window correctly */
    atom = gdk_atom_intern ("_HILDON_NOTIFICATION_TYPE", FALSE);
    gdk_property_change (gdkwin, atom, gdk_x11_xatom_to_atom (XA_STRING), 8, GDK_PROP_MODE_REPLACE,
                         (gpointer) notification_type, strlen (notification_type));

    /* HildonBanner supports portrait mode */
    atom = gdk_atom_intern ("_HILDON_PORTRAIT_MODE_SUPPORT", FALSE);
    gdk_property_change (gdkwin, atom, gdk_x11_xatom_to_atom (XA_CARDINAL), 32,
                         GDK_PROP_MODE_REPLACE, (gpointer) &portrait, 1);

    /* Manage override flag */
    if ((priv->require_override_dnd)&&(!priv->overrides_dnd)) {
      hildon_banner_set_override_flag (HILDON_BANNER (widget));
        priv->overrides_dnd = TRUE;
    }

    screen = gtk_widget_get_screen (widget);
    g_signal_connect (screen, "size-changed", G_CALLBACK (screen_size_changed), widget);
}

static void
hildon_banner_unrealize                         (GtkWidget *widget)
{
    GdkScreen *screen = gtk_widget_get_screen (widget);
    g_signal_handlers_disconnect_by_func (screen, G_CALLBACK (screen_size_changed), widget);

    GTK_WIDGET_CLASS (hildon_banner_parent_class)->unrealize (widget);
}

static void 
hildon_banner_class_init                        (HildonBannerClass *klass)
{
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = G_OBJECT_CLASS (klass);
    widget_class = GTK_WIDGET_CLASS (klass);

    /* Append private structure to class. This is more elegant than
       on g_new based approach */
    g_type_class_add_private (klass, sizeof (HildonBannerPrivate));

    /* Override virtual methods */
    object_class->constructor = hildon_banner_constructor;
    object_class->dispose = hildon_banner_dispose;
    object_class->finalize = hildon_banner_finalize;
    object_class->set_property = hildon_banner_set_property;
    object_class->get_property = hildon_banner_get_property;
    GTK_OBJECT_CLASS (klass)->destroy = hildon_banner_destroy;
    widget_class->size_request = hildon_banner_size_request;
    widget_class->map_event = hildon_banner_map_event;
    widget_class->realize = hildon_banner_realize;
    widget_class->unrealize = hildon_banner_unrealize;
    widget_class->button_press_event = hildon_banner_button_press_event;
#if defined(MAEMO_GTK)
    widget_class->map = hildon_banner_map;
#endif

    /* Install properties.
       We need construct properties for singleton purposes */

    /**
     * HildonBanner:parent-window:
     *
     * The window for which the banner will be singleton. 
     *                      
     */
    g_object_class_install_property (object_class, PROP_PARENT_WINDOW,
            g_param_spec_object ("parent-window",
                "Parent window",
                "The window for which the banner will be singleton",
                GTK_TYPE_WINDOW, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    /**
     * HildonBanner:is-timed:
     *
     * Whether the banner is timed and goes away automatically.
     *                      
     */
    g_object_class_install_property (object_class, PROP_IS_TIMED,
            g_param_spec_boolean ("is-timed",
                "Is timed",
                "Whether or not the notification goes away automatically "
                "after the specified time has passed",
                FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    /**
     * HildonBanner:timeout:
     *
     * The time before destroying the banner. This needs
     * to be adjusted before the banner is mapped to the screen.
     *                      
     */
    g_object_class_install_property (object_class, PROP_TIMEOUT,
            g_param_spec_uint ("timeout",
                "Timeout",
                "The time before making the banner banner go away",
                0,
                10000,
                HILDON_BANNER_DEFAULT_TIMEOUT,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void 
hildon_banner_init                              (HildonBanner *self)
{
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (self);
    g_assert (priv);

    priv->parent = NULL;
    priv->overrides_dnd = FALSE;
    priv->require_override_dnd = FALSE;
    priv->name_suffix = NULL;
    priv->main_item = NULL;

    /* Initialize the common layout inside banner */
    priv->alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
    priv->layout = gtk_hbox_new (FALSE, HILDON_MARGIN_DEFAULT);

    priv->label = g_object_new (GTK_TYPE_LABEL, NULL);
    gtk_label_set_line_wrap (GTK_LABEL (priv->label), TRUE);
    gtk_label_set_line_wrap_mode (GTK_LABEL (priv->label), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_justify (GTK_LABEL (priv->label), GTK_JUSTIFY_CENTER);

    gtk_container_set_border_width (GTK_CONTAINER (priv->layout), HILDON_MARGIN_DEFAULT);
    gtk_container_add (GTK_CONTAINER (self), priv->alignment);
    gtk_container_add (GTK_CONTAINER (priv->alignment), priv->layout);
    g_object_ref (priv->label);
    gtk_box_pack_start (GTK_BOX (priv->layout), priv->label, FALSE, FALSE, 0);

    gtk_window_set_accept_focus (GTK_WINDOW (self), FALSE);

    gtk_widget_add_events (GTK_WIDGET (self), GDK_BUTTON_PRESS_MASK);
}

/* Makes sure that icon/progress item contains the desired type
   of item. If possible, tries to avoid creating a new widget but
   reuses the existing one */
static void
hildon_banner_ensure_child                      (HildonBanner *self, 
                                                 GtkWidget *user_widget,
                                                 guint pos,
                                                 GType type,
                                                 const gchar *first_property, 
                                                 ...)
{
    GtkWidget *widget;
    va_list args;
    HildonBannerPrivate *priv = HILDON_BANNER_GET_PRIVATE (self);

    g_assert (priv);

    widget = priv->main_item;
    va_start (args, first_property);

    /* Reuse existing widget if possible */
    if (! user_widget && G_TYPE_CHECK_INSTANCE_TYPE (widget, type))
    {
        g_object_set_valist (G_OBJECT (widget), first_property, args);
    }
    else
    {
        /* We have to abandon old content widget */
        if (widget)
            gtk_container_remove (GTK_CONTAINER (priv->layout), widget);
        
        /* Use user provided widget or create a new one */
        priv->main_item = widget = user_widget ? 
            user_widget : GTK_WIDGET (g_object_new_valist(type, first_property, args));
        gtk_box_pack_start (GTK_BOX (priv->layout), widget, FALSE, FALSE, 0);
    }

    /* We make sure that the widget exists in desired position. Different
       banners place this child widget to different places */
    gtk_box_reorder_child (GTK_BOX (priv->layout), widget, pos);
    va_end (args);
}

/* Creates a new banner instance or uses an existing one */
static HildonBanner*
hildon_banner_get_instance_for_widget           (GtkWidget *widget, 
                                                 gboolean timed)
{
    GtkWidget *window;

    window = widget ? gtk_widget_get_ancestor (widget, GTK_TYPE_WINDOW) : NULL;
    return g_object_new (HILDON_TYPE_BANNER, "parent-window", window, "is-timed", timed, NULL);
}

/**
 * hildon_banner_show_information:
 * @widget: the #GtkWidget that is the owner of the banner
 * @icon_name: since Hildon 2.2 this parameter is not used anymore and
 * any value that you pass will be ignored
 * @text: Text to display
 *
 * This function creates and displays an information banner that is
 * automatically destroyed after a certain time period (see
 * hildon_banner_set_timeout()). For each window in your application
 * there can only be one timed banner, so if you spawn a new banner
 * before the earlier one has timed out, the previous one will be
 * replaced.
 *
 * Returns: The newly created banner
 *
 */
GtkWidget*
hildon_banner_show_information                  (GtkWidget *widget, 
                                                 const gchar *icon_name,
                                                 const gchar *text)
{
    return hildon_banner_real_show_information (widget, text, FALSE);
}

/**
 * hildon_banner_show_information_override_dnd:
 * @widget: the #GtkWidget that is the owner of the banner
 * @text: Text to display
 *
 * Equivalent to hildon_banner_show_information(), but overriding the
 * "do not disturb" flag.
 *
 * Returns: The newly created banner
 *
 * Since: 2.2
 *
 */
GtkWidget*
hildon_banner_show_information_override_dnd     (GtkWidget *widget,
                                                 const gchar *text)
{
    return hildon_banner_real_show_information (widget, text, TRUE);
}

static void
hildon_banner_set_override_flag                 (HildonBanner *banner)
{
    guint32 state = 1;

    gdk_property_change (GTK_WIDGET (banner)->window,
                         gdk_atom_intern_static_string ("_HILDON_DO_NOT_DISTURB_OVERRIDE"),
                         gdk_x11_xatom_to_atom (XA_INTEGER),
                         32,
                         GDK_PROP_MODE_REPLACE,
                         (const guchar*) &state,
                         1);
}

static void
reshow_banner                                   (HildonBanner *banner)
{
    gint width = gdk_screen_get_width (gtk_widget_get_screen (GTK_WIDGET (banner)));
    gtk_window_resize (GTK_WINDOW (banner), width, 1);
    force_to_wrap_truncated (banner);
    gtk_widget_show_all (GTK_WIDGET (banner));
}

static void
unpack_main_widget_pack_label                   (HildonBanner *banner)
{
    HildonBannerPrivate *priv = NULL;

    priv = HILDON_BANNER_GET_PRIVATE (banner);

    if (priv->main_item) {
        gtk_container_remove (GTK_CONTAINER (priv->layout), priv->main_item);
        priv->main_item = NULL;
        gtk_box_pack_start (GTK_BOX (priv->layout), priv->label, FALSE, FALSE,
                            0);
    }
}

static GtkWidget*
hildon_banner_real_show_information             (GtkWidget *widget,
                                                 const gchar *text,
                                                 gboolean override_dnd)
{
    HildonBanner *banner;
    HildonBannerPrivate *priv = NULL;

    g_return_val_if_fail (text != NULL, NULL);

    /* Prepare banner */
    banner = hildon_banner_get_instance_for_widget (widget, TRUE);
    priv = HILDON_BANNER_GET_PRIVATE (banner);

    priv->name_suffix = "information";
    unpack_main_widget_pack_label (banner);
    banner_do_set_text (banner, text, FALSE);
    hildon_banner_bind_style (banner);

    if (override_dnd) {
      /* so on the realize it will set the property */
      priv->require_override_dnd = TRUE;
    }

    /* Show the banner, since caller cannot do that */
    reshow_banner (banner);

    return GTK_WIDGET (banner);
}

/**
 * hildon_banner_show_informationf:
 * @widget: the #GtkWidget that is the owner of the banner
 * @icon_name: since Hildon 2.2 this parameter is not used anymore and
 * any value that you pass will be ignored
 * @format: a printf-like format string
 * @Varargs: arguments for the format string
 *
 * A helper function for hildon_banner_show_information() with
 * string formatting.
 *
 * Returns: the newly created banner
 */
GtkWidget* 
hildon_banner_show_informationf                 (GtkWidget *widget, 
                                                 const gchar *icon_name,
                                                 const gchar *format, 
                                                 ...)
{
    g_return_val_if_fail (format != NULL, NULL);

    gchar *message;
    va_list args;
    GtkWidget *banner;

    va_start (args, format);
    message = g_strdup_vprintf (format, args);
    va_end (args);

    banner = hildon_banner_show_information (widget, icon_name, message);

    g_free (message);

    return banner;
}

/**
 * hildon_banner_show_information_with_markup:
 * @widget: the #GtkWidget that wants to display banner
 * @icon_name: since Hildon 2.2 this parameter is not used anymore and
 * any value that you pass will be ignored
 * @markup: a markup string to display (see <link linkend="PangoMarkupFormat">Pango markup format</link>)
 *
 * This function creates and displays an information banner that is
 * automatically destroyed after certain time period (see
 * hildon_banner_set_timeout()). For each window in your application
 * there can only be one timed banner, so if you spawn a new banner
 * before the earlier one has timed out, the previous one will be
 * replaced.
 *
 * Returns: the newly created banner
 *
 */
GtkWidget*
hildon_banner_show_information_with_markup      (GtkWidget *widget, 
                                                 const gchar *icon_name, 
                                                 const gchar *markup)
{
    HildonBanner *banner;
    HildonBannerPrivate *priv;

    g_return_val_if_fail (icon_name == NULL || icon_name[0] != 0, NULL);
    g_return_val_if_fail (markup != NULL, NULL);

    /* Prepare banner */
    banner = hildon_banner_get_instance_for_widget (widget, TRUE);
    priv = HILDON_BANNER_GET_PRIVATE (banner);

    priv->name_suffix = "information";
    banner_do_set_text (banner, markup, TRUE);
    hildon_banner_bind_style (banner);

    /* Show the banner, since caller cannot do that */
    reshow_banner (banner);

    return (GtkWidget *) banner;
}

/**
 * hildon_banner_show_animation:
 * @widget: the #GtkWidget that wants to display banner
 * @animation_name: since Hildon 2.2 this parameter is not used
 *                  anymore and any value that you pass will be
 *                  ignored
 * @text: the text to display.
 *
 * Shows an animated progress notification. It's recommended not to try
 * to show more than one progress notification at a time, since
 * they will appear on top of each other. You can use progress
 * notifications with timed banners. In this case the banners are
 * located so that you can somehow see both.
 *
 * Unlike information banners (created with
 * hildon_banner_show_information()), animation banners are not
 * destroyed automatically using a timeout. You have to destroy them
 * yourself.
 *
 * Please note also that these banners are destroyed automatically if the
 * window they are attached to is closed. The pointer that you receive
 * with this function does not contain additional references, so it
 * can become invalid without warning (this is true for all toplevel
 * windows in gtk). To make sure that the banner does not disappear
 * automatically, you can separately ref the return value (this
 * doesn't prevent the banner from disappearing, just the object from
 * being finalized). In this case you have to call
 * gtk_widget_destroy() followed by g_object_unref().
 *
 * Returns: a #HildonBanner widget. You must call gtk_widget_destroy()
 *          once you are done with the banner.
 *
 * Deprecated: Hildon 2.2: use
 * hildon_gtk_window_set_progress_indicator() instead.
 */
GtkWidget*
hildon_banner_show_animation                    (GtkWidget *widget, 
                                                 const gchar *animation_name, 
                                                 const gchar *text)
{
    HildonBanner *banner;
    GtkWidget *image_widget;
    HildonBannerPrivate *priv;

    g_return_val_if_fail (text != NULL, NULL);

    image_widget = hildon_private_create_animation (
        HILDON_BANNER_ANIMATION_FRAMERATE,
        HILDON_BANNER_ANIMATION_TMPL,
        HILDON_BANNER_ANIMATION_NFRAMES);

    /* Prepare banner */
    banner = hildon_banner_get_instance_for_widget (widget, FALSE);
    hildon_banner_ensure_child (banner, image_widget, 0,
            GTK_TYPE_IMAGE, "yalign", 0.0, NULL);

    priv = HILDON_BANNER_GET_PRIVATE (banner);
    priv->name_suffix = "animation";
    banner_do_set_text (banner, text, FALSE);
    hildon_banner_bind_style (banner);

    /* And show it */
    reshow_banner (banner);

    return (GtkWidget *) banner;
}

/**
 * hildon_banner_show_progress:
 * @widget: the #GtkWidget that wants to display banner
 * @bar: since Hildon 2.2 this parameter is not used anymore and
 * any value that you pass will be ignored
 * @text: text to display.
 *
 * Shows progress notification. See hildon_banner_show_animation()
 * for more information.
 * 
 * Returns: a #HildonBanner widget. You must call #gtk_widget_destroy
 *          once you are done with the banner.
 *
 * Deprecated: Hildon 2.2: use hildon_gtk_window_set_progress_indicator() instead.
 */
GtkWidget*
hildon_banner_show_progress                     (GtkWidget *widget, 
                                                 GtkProgressBar *bar, 
                                                 const gchar *text)
{
    HildonBanner *banner;
    HildonBannerPrivate *priv;

    g_return_val_if_fail (text != NULL, NULL);

    /* Prepare banner */
    banner = hildon_banner_get_instance_for_widget (widget, FALSE);
    priv = HILDON_BANNER_GET_PRIVATE (banner);
    g_assert (priv);

    priv->name_suffix = "progress";
    unpack_main_widget_pack_label (banner);
    banner_do_set_text (banner, text, FALSE);
    hildon_banner_bind_style (banner);

    if (priv->parent)
        hildon_gtk_window_set_progress_indicator (priv->parent, 1);

    /* Show the banner */
    reshow_banner (banner);

    return GTK_WIDGET (banner);   
}


/**
 * hildon_banner_show_custom_widget:
 * @widget: the #GtkWidget that wants to display a banner
 * @custom_widget: a #GtkWidget to be placed inside the banner.
 *
 * Shows a banner displaying a user-defined widget.
 *
 * Returns: a new #HildonBanner
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_banner_show_custom_widget                (GtkWidget *widget,
                                                 GtkWidget *custom_widget)
{
    HildonBanner *banner;
    HildonBannerPrivate *priv;

    g_return_val_if_fail (GTK_IS_WIDGET (custom_widget), NULL);

    banner = hildon_banner_get_instance_for_widget (widget, TRUE);
    priv = HILDON_BANNER_GET_PRIVATE (banner);
    g_assert (priv);

    g_return_val_if_fail (gtk_widget_get_parent (custom_widget) == NULL ||
                          priv->main_item == custom_widget, NULL);

    if (priv->main_item == NULL) {
        gtk_container_remove (GTK_CONTAINER (priv->layout), priv->label);
    }

    if (priv->main_item != custom_widget) {
        GtkWidget *old_item = priv->main_item;

        /* Remove old item */
        if (old_item) {
            g_object_ref (old_item);
            gtk_container_remove (GTK_CONTAINER (priv->layout), old_item);
        }

        /* Add new item */
        gtk_box_pack_start (GTK_BOX (priv->layout), custom_widget, FALSE, FALSE, 0);
        priv->main_item = custom_widget;

        if (old_item)
            g_object_unref (old_item);
    }

    priv->name_suffix = "information";
    hildon_banner_bind_style (banner);

    reshow_banner (banner);

    return GTK_WIDGET (banner);
}

/**
 * hildon_banner_set_text:
 * @self: a #HildonBanner widget
 * @text: a new text to display in banner
 *
 * Sets the text that is displayed in the banner.
 *
 */
void 
hildon_banner_set_text                          (HildonBanner *self, 
                                                 const gchar *text)
{
    g_return_if_fail (HILDON_IS_BANNER (self));

    banner_do_set_text (self, text, FALSE);

    if (GTK_WIDGET_VISIBLE (self))
        reshow_banner (self);
}

/**
 * hildon_banner_set_markup:
 * @self: a #HildonBanner widget
 * @markup: a new text with Pango markup to display in the banner
 *
 * Sets the text with markup that is displayed in the banner.
 *
 */
void 
hildon_banner_set_markup                        (HildonBanner *self, 
                                                 const gchar *markup)
{
    g_return_if_fail (HILDON_IS_BANNER (self));

    banner_do_set_text (self, markup, TRUE);

    if (GTK_WIDGET_VISIBLE (self))
        reshow_banner (self);
}

/**
 * hildon_banner_set_fraction:
 * @self: a #HildonBanner widget
 * @fraction: #gdouble
 *
 * The fraction is the completion of progressbar, 
 * the scale is from 0.0 to 1.0.
 * Sets the amount of fraction the progressbar has.
 *
 * Note that this method only has effect if @self was created with
 * hildon_banner_show_progress()
 *
 * Deprecated: This function does nothing. As of Hildon 2.2, hildon
 * banners don't have progress bars.
 */
void 
hildon_banner_set_fraction                      (HildonBanner *self, 
                                                 gdouble fraction)
{
}

/**
 * hildon_banner_set_timeout:
 * @self: a #HildonBanner widget
 * @timeout: timeout to set in miliseconds.
 *
 * Sets the timeout on the banner. After the given amount of miliseconds
 * has elapsed the banner will be destroyed. Setting this only makes
 * sense on banners that are timed and that have not been yet displayed
 * on the screen.
 *
 * Note that this method only has effect if @self is an information
 * banner (created using hildon_banner_show_information() and
 * friends).
 */
void
hildon_banner_set_timeout                       (HildonBanner *self,
                                                 guint timeout)
{
    HildonBannerPrivate *priv;

    g_return_if_fail (HILDON_IS_BANNER (self));
    priv = HILDON_BANNER_GET_PRIVATE (self);
    g_assert (priv);

    priv->timeout = timeout;
}

/**
 * hildon_banner_set_icon:
 * @self: a #HildonBanner widget
 * @icon_name: the name of icon to use. Can be %NULL for default icon
 *
 * Sets the icon to be used in the banner.
 *
 * Deprecated: This function does nothing. As of hildon 2.2, hildon
 * banners don't allow changing their icons.
 */
void 
hildon_banner_set_icon                          (HildonBanner *self, 
                                                 const gchar  *icon_name)
{
}

/**
 * hildon_banner_set_icon_from_file:
 * @self: a #HildonBanner widget
 * @icon_file: the filename of icon to use. Can be %NULL for default icon
 *
 * Sets the icon from its filename to be used in the banner.
 *
 * Deprecated: This function does nothing. As of hildon 2.2, hildon
 * banners don't allow changing their icons.
 */
void 
hildon_banner_set_icon_from_file                (HildonBanner *self, 
                                                 const gchar  *icon_file)
{
}
