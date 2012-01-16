/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006 Nokia Corporation, all rights reserved.
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
 * SECTION:hildon-caption
 * @short_description: A single-child container widget that precedes the 
 * contained widget with a field label and an optional icon.
 *
 * #HildonCaption is a single-child container widget that precedes the 
 * contained widget with a field label and an optional icon. It allows 
 * grouping of several controls together. When a captioned widget has focus, 
 * both widget and caption label are displayed with active focus. 
 */

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <libintl.h>
#include                                        <gtk/gtk.h>

#include                                        "hildon-defines.h"
#include                                        "hildon-caption.h"
#include                                        "hildon-caption-private.h"

#define                                         _(String)\
                                                dgettext("hildon-libs", String)

#define                                         HILDON_CAPTION_SPACING 6

static GtkEventBox*                             parent_class = NULL;

static void 
hildon_caption_class_init                       (HildonCaptionClass *caption_class);

static void
hildon_caption_init                             (HildonCaption *caption);

static void
hildon_caption_get_preferred_width              (GtkWidget *widget,
                                                 gint      *minimal_width,
                                                 gint      *natural_width);

static void
hildon_caption_get_preferred_height             (GtkWidget *widget,
                                                 gint      *minimal_height,
                                                 gint      *natural_height);

static void 
hildon_caption_size_allocate                    (GtkWidget *widget, 
                                                 GtkAllocation *allocation);

static void 
hildon_caption_forall                           (GtkContainer *container,
                                                 gboolean include_internals,
                                                 GtkCallback callback, 
                                                 gpointer data );

static void
hildon_caption_hierarchy_changed                (GtkWidget *widget,
                                                 GtkWidget *previous_toplevel);

static void 
hildon_caption_set_focus                        (GtkWindow *window, 
                                                 GtkWidget *widget,
                                                 GtkWidget *caption);

static void 
hildon_caption_grab_focus                       (GtkWidget *widget);

static void 
hildon_caption_activate                         (GtkWidget *widget);

static void
hildon_caption_set_property                     (GObject *object, 
                                                 guint param_id,
                                                 const GValue *value, 
                                                 GParamSpec *pspec );

static void
hildon_caption_get_property                     (GObject *object, 
                                                 guint param_id,
                                                 GValue *value, 
                                                 GParamSpec *pspec);

static void 
hildon_caption_destroy                          (GtkWidget *self);

static gboolean 
hildon_caption_button_press                     (GtkWidget *widget, 
                                                 GdkEventButton *event);

static void
hildon_caption_set_label_text                   (HildonCaptionPrivate *priv, 
                                                 gboolean markup);

static void 
hildon_caption_set_child_property               (GtkContainer *container,
                                                 GtkWidget *child,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec);

static void 
hildon_caption_get_child_property               (GtkContainer *container,
                                                 GtkWidget *child,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec);

enum
{
    PROP_0,
    PROP_LABEL,
    PROP_MARKUP,
    PROP_ICON,
    PROP_STATUS,
    PROP_SEPARATOR,
    PROP_SIZE_GROUP,
    PROP_ICON_POSITION,
};

enum
{
    CHILD_PROP_NONE,
    CHILD_PROP_EXPAND
};

/**
 * hildon_caption_get_type:
 *
 * Initializes and returns the type of a hildon caption.
 *
 * Returns: GType of #HildonCaption
 */
GType G_GNUC_CONST 
hildon_caption_get_type                         (void)
{
    static GType caption_type = 0;

    if (!caption_type)
    {
        static const GTypeInfo caption_info = {
            sizeof (HildonCaptionClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) hildon_caption_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof (HildonCaption),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_caption_init,
        };
        caption_type = g_type_register_static (GTK_TYPE_EVENT_BOX,
                "HildonCaption", &caption_info, 0 );
    }
    return caption_type;
}

/*
 * Initialises the caption class.
 */
static void 
hildon_caption_class_init                       (HildonCaptionClass *caption_class)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (caption_class);
    GObjectClass *gobject_class = G_OBJECT_CLASS (caption_class);
    GtkContainerClass *container_class = GTK_CONTAINER_CLASS (caption_class);

    parent_class = g_type_class_peek_parent (caption_class);

    g_type_class_add_private (caption_class, sizeof (HildonCaptionPrivate));

    /* Override virtual functions */
    gobject_class->get_property                 = hildon_caption_get_property;
    gobject_class->set_property                 = hildon_caption_set_property;
    caption_class->activate                     = (gpointer) hildon_caption_activate;

    container_class->forall                     = hildon_caption_forall;
    container_class->set_child_property         = hildon_caption_set_child_property;
    container_class->get_child_property         = hildon_caption_get_child_property;

    widget_class->hierarchy_changed             = hildon_caption_hierarchy_changed;
    widget_class->get_preferred_width           = hildon_caption_get_preferred_width;
    widget_class->get_preferred_height          = hildon_caption_get_preferred_height;
    widget_class->size_allocate                 = hildon_caption_size_allocate;
    widget_class->button_press_event            = hildon_caption_button_press;
    widget_class->grab_focus                    = hildon_caption_grab_focus;
    widget_class->destroy                       = hildon_caption_destroy;

    /* Create new signals and properties */
    widget_class->activate_signal = g_signal_new ("activate",
            G_OBJECT_CLASS_TYPE (
                gobject_class),
            G_SIGNAL_RUN_FIRST |
            G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (HildonCaptionClass,
                activate), NULL, NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    /**
     * HildonCaption:label:
     *
     * Caption label.
     */
    g_object_class_install_property (gobject_class, PROP_LABEL,
            g_param_spec_string ("label",
                "Current label", "Caption label",
                NULL, G_PARAM_READABLE | G_PARAM_WRITABLE) );
    
    /**
     * HildonCaption:markup:
     *
     * Caption markup. Mutually exclusive with label.
     */
    g_object_class_install_property (gobject_class, PROP_MARKUP,
            g_param_spec_string ("markup",
                "Current markup", "Caption markup",
                NULL, G_PARAM_WRITABLE) );

    /**
     * HildonCaption:icon:
     *
     * The icon shown on the caption area.
     */
    g_object_class_install_property (gobject_class, PROP_ICON,
            g_param_spec_object ("icon",
                "Current icon",
                "The icon shown on the caption area",
                GTK_TYPE_WIDGET, G_PARAM_READABLE | 
                G_PARAM_WRITABLE) );
    /**
     * HildonCaption:status:
     *
     * Mandatory or optional status.
     */
    g_object_class_install_property (gobject_class, PROP_STATUS,
            g_param_spec_enum ("status",
                "Current status",
                "Mandatory or optional status",
                HILDON_TYPE_CAPTION_STATUS,
                HILDON_CAPTION_OPTIONAL,
                G_PARAM_READABLE | G_PARAM_WRITABLE) );
    /**
     * HildonCaption:icon-position:
     *
     * If the icon is positioned on the left or right side.
     */
    g_object_class_install_property (gobject_class, PROP_ICON_POSITION,
            g_param_spec_enum ("icon-position",
                "Icon position",
                "Whether the icon is on the left or right side",
                HILDON_TYPE_CAPTION_ICON_POSITION,
                HILDON_CAPTION_POSITION_RIGHT,
                G_PARAM_READABLE | G_PARAM_WRITABLE) );

    /**
     * HildonCaption:size_group:
     *
     * Current size group the caption is in.
     */
    g_object_class_install_property (gobject_class, PROP_SIZE_GROUP,
            g_param_spec_object ("size_group",
                "Current size group",
                "Current size group the caption is in",
                GTK_TYPE_SIZE_GROUP, G_PARAM_READABLE | 
                G_PARAM_WRITABLE) );

    /**
     * HildonCaption:separator:
     *
     * The current separator.
     */
    g_object_class_install_property (gobject_class, PROP_SEPARATOR,
            g_param_spec_string ("separator",
                "Current separator", "Current separator",
                _("ecdg_ti_caption_separator"),
                G_PARAM_READABLE | G_PARAM_WRITABLE) );

    /* Create child properties. These are related to
       child <-> parent relationship, not to either of objects itself */
    gtk_container_class_install_child_property (container_class,
            CHILD_PROP_EXPAND,
            g_param_spec_boolean ("expand",
                "Same as GtkBox expand.",
                "Same as GtkBox expand. Wheter the child should be expanded or not.",
                FALSE,
                G_PARAM_READWRITE));
}

/* Destroy can be called multiple times, remember to set pointers to NULL */
static void 
hildon_caption_destroy                          (GtkWidget *self)
{
    HildonCaptionPrivate *priv = HILDON_CAPTION_GET_PRIVATE (self);

    /* Free our internal child */
    if (priv && priv->caption_area)
    {
        gtk_widget_unparent (priv->caption_area);
        priv->caption_area = NULL;
    }

    /* Free user provided strings */
    if (priv && priv->text)
    {
        g_free (priv->text);
        priv->text = NULL;
    }

    if (priv && priv->separator)
    {
        g_free (priv->separator);
        priv->separator = NULL;
    }

    /* Parent classes destroy takes care of user packed contents */
    if (GTK_WIDGET_CLASS (parent_class)->destroy)
        GTK_WIDGET_CLASS (parent_class)->destroy (self);
}

static void 
hildon_caption_set_property                     (GObject *object, 
                                                 guint param_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec)
{
    HildonCaptionPrivate *priv = HILDON_CAPTION_GET_PRIVATE (object);

    switch (param_id) 
    {
        case PROP_ICON_POSITION:
            hildon_caption_set_icon_position (HILDON_CAPTION (object), g_value_get_enum (value));
            break;

        case PROP_LABEL:
            /* Free old label string */
            if (priv->text)
            {
                g_free (priv->text);
                priv->text = NULL;
            }

            /* Update label */
            priv->text = g_value_dup_string (value);
            hildon_caption_set_label_text (priv, FALSE);
            break;

        case PROP_MARKUP:
            /* Free old label string */
            if (priv->text)
            {
                g_free (priv->text);
                priv->text = NULL;
            }

            /* Update label */
            priv->text = g_value_dup_string (value);
            hildon_caption_set_label_text (priv, TRUE);
            break;

        case PROP_ICON:
            /* Remove old icon */
            if (priv->icon)
                gtk_container_remove (GTK_CONTAINER (priv->icon_align), priv->icon);

            /* Pack and display new icon */
            priv->icon = g_value_get_object (value);
            if (priv->icon)
            {
                gtk_container_add (GTK_CONTAINER (priv->icon_align), priv->icon);
                gtk_widget_show_all (priv->caption_area);
            }
            break;

        case PROP_STATUS:
            priv->status = g_value_get_enum (value);
            break;

        case PROP_SIZE_GROUP:
            /* Detach from previous size group */
            if (priv->group)
                gtk_size_group_remove_widget (priv->group, priv->caption_area);

            priv->group = g_value_get_object (value);

            /* Attach to new size group */
            if (priv->group)
                gtk_size_group_add_widget (priv->group, priv->caption_area);

            gtk_widget_queue_draw (GTK_WIDGET(object));
            break;

        case PROP_SEPARATOR:

            /* Free old separator */
            if (priv->separator)
            {
                g_free (priv->separator);
                priv->separator = NULL;
            }

            priv->separator = g_value_dup_string (value);
            hildon_caption_set_label_text (priv, FALSE);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}

static void 
hildon_caption_get_property                     (GObject *object, 
                                                 guint param_id,
                                                 GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonCaptionPrivate *priv = HILDON_CAPTION_GET_PRIVATE (object);

    switch (param_id) 
    {
        case PROP_LABEL:
            g_value_set_string (value, priv->text);
            break;

        case PROP_ICON:
            g_value_set_object (value, priv->icon);
            break;

        case PROP_STATUS:
            g_value_set_enum (value, priv->status);
            break;

        case PROP_ICON_POSITION:
            g_value_set_enum (value, priv->icon_position);
            break;

        case PROP_SIZE_GROUP:
            g_value_set_object (value, priv->group);
            break;

        case PROP_SEPARATOR:
            g_value_set_string (value, priv->separator);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
            break;
    }
}

static void 
hildon_caption_set_child_property               (GtkContainer *container,
                                                 GtkWidget *child,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec)
{
    switch (property_id)
    {
        case CHILD_PROP_EXPAND:
            hildon_caption_set_child_expand (HILDON_CAPTION (container),
                    g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (container, property_id, pspec);
            break;
    }
}

static void 
hildon_caption_get_child_property               (GtkContainer *container,
                                                 GtkWidget *child,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec)
{
    switch (property_id)
    {
        case CHILD_PROP_EXPAND:
            g_value_set_boolean (value, hildon_caption_get_child_expand(
                        HILDON_CAPTION (container)));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(container, property_id, pspec);
            break;
    }
}

/* We want to activate out child control on button press */
static gboolean
hildon_caption_button_press                     (GtkWidget *widget, 
                                                 GdkEventButton *event)
{
    gtk_widget_grab_focus (gtk_bin_get_child (GTK_BIN (widget)));

    /* we'll update our focused state in set-focus when/if the child receives
     * focus */

    return FALSE;
}

static void 
hildon_caption_init                             (HildonCaption *caption)
{
    HildonCaptionPrivate *priv = NULL;

    /* Initialize startup state */
    priv = HILDON_CAPTION_GET_PRIVATE (caption);

    priv->status = HILDON_CAPTION_OPTIONAL;
    priv->icon = NULL;
    priv->group = NULL;
    priv->is_focused = FALSE;
    priv->text = NULL;

    priv->separator = g_strdup(_("ecdg_ti_caption_separator"));

    gtk_widget_push_composite_child();

    /* Create caption text */
    priv->caption_area = gtk_hbox_new (FALSE, HILDON_CAPTION_SPACING); 
    priv->label = gtk_label_new (NULL);
    priv->icon_align = gtk_alignment_new (0.5f, 0.5f, 0.0f, 0.0f);
    priv->icon_position = HILDON_CAPTION_POSITION_RIGHT;

    /* We want to receive button presses for child widget activation */
    gtk_event_box_set_above_child (GTK_EVENT_BOX (caption), FALSE);
    gtk_widget_add_events (GTK_WIDGET (caption), GDK_BUTTON_PRESS_MASK);

    /* Pack text label caption layout */
    gtk_box_pack_end (GTK_BOX (priv->caption_area), priv->icon_align, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (priv->caption_area), priv->label, FALSE, FALSE, 0);
    gtk_widget_set_parent (priv->caption_area, GTK_WIDGET (caption));

    gtk_widget_pop_composite_child ();

    hildon_caption_set_child_expand (caption, TRUE);

    gtk_widget_show_all (priv->caption_area);
}

static void 
hildon_caption_set_focus                        (GtkWindow *window, 
                                                 GtkWidget *widget,
                                                 GtkWidget *caption)
{
    HildonCaptionPrivate *priv = HILDON_CAPTION_GET_PRIVATE (caption);

    /* check if ancestor gone */
    if (! widget)
    {
        return;
    }

    /* Try to find caption among the ancestors of widget */
    if (gtk_widget_is_ancestor (widget, caption))
    {
        priv->is_focused = TRUE;
        gtk_widget_queue_draw (caption);
        return;
    }

    if (priv->is_focused == TRUE)
    {
        /* Caption wasn't found, so cannot focus */
        priv->is_focused = FALSE;
        gtk_widget_queue_draw (caption);
    }
}

/* We need to connect/disconnect signal handlers to toplevel window
   in which we reside. Ww want to update connected signals if our
   parent changes */
static void 
hildon_caption_hierarchy_changed                (GtkWidget *widget,
                                                 GtkWidget *previous_toplevel)
{
    GtkWidget *current_ancestor;

    if (GTK_WIDGET_CLASS (parent_class)->hierarchy_changed)
        GTK_WIDGET_CLASS (parent_class)->hierarchy_changed (widget, previous_toplevel);

    /* If we already were inside a window, remove old handler */
    if (previous_toplevel) {
        /* This is a compilation workaround for gcc > 3.3 since glib is buggy */
        /* see http://bugzilla.gnome.org/show_bug.cgi?id=310175 */
#ifdef __GNUC__
        __extension__
#endif
            g_signal_handlers_disconnect_by_func
            (previous_toplevel, (gpointer) hildon_caption_set_focus, widget);
    }

    current_ancestor = gtk_widget_get_ancestor (widget, GTK_TYPE_WINDOW);

    /* Install new handler for focus movement */
    if (current_ancestor)
        g_signal_connect( current_ancestor, "set-focus", 
                G_CALLBACK (hildon_caption_set_focus), widget );
}

static void
hildon_caption_get_preferred_height             (GtkWidget *widget,
                                                 gint      *minimal_height,
                                                 gint      *natural_height)
{
    gint caption_minimal, caption_natural;
    HildonCaptionPrivate *priv = NULL;
    GtkStyle *style;
    g_return_if_fail (HILDON_IS_CAPTION(widget));

    priv = HILDON_CAPTION_GET_PRIVATE (widget);

    /* Use the height for the main box of the caption */
    gtk_widget_get_preferred_height (priv->caption_area, &caption_minimal, &caption_natural);

    if (GTK_WIDGET_CLASS (parent_class)->get_preferred_height)
        GTK_WIDGET_CLASS (parent_class)->get_preferred_height (widget, minimal_height, natural_height);

    g_object_get (widget, "style", &style, NULL);

    if ((caption_minimal + (2 * style->ythickness)) > *minimal_height)
        *minimal_height = caption_minimal + (2 * style->ythickness);
    if ((caption_natural + (2 * style->ythickness)) > *natural_height)
        *natural_height = caption_natural + (2 * style->ythickness);

    g_object_unref (style);
}

static void
hildon_caption_get_preferred_width              (GtkWidget *widget,
                                                 gint      *minimal_width,
                                                 gint      *natural_width)
{
    gint caption_minimal, caption_natural;
    HildonCaptionPrivate *priv = NULL;
    g_return_if_fail (HILDON_IS_CAPTION(widget));

    priv = HILDON_CAPTION_GET_PRIVATE (widget);

    /* Use the width for the main box of the caption */
    gtk_widget_get_preferred_width (priv->caption_area, &caption_minimal, &caption_natural);

    if (GTK_WIDGET_CLASS (parent_class)->get_preferred_width)
        GTK_WIDGET_CLASS (parent_class)->get_preferred_width (widget, minimal_width, natural_width);

    *minimal_width += caption_minimal + HILDON_CAPTION_SPACING * 3;
    *natural_width += caption_natural + HILDON_CAPTION_SPACING * 3;
}

/* We use HILDON_CAPTION_SPACING to make it look a bit nicer */
static void 
hildon_caption_size_allocate                    (GtkWidget *widget,
                                                 GtkAllocation *allocation)
{
    GtkAllocation child_alloc;
    GtkAllocation caption_alloc;
    GtkRequisition req, child_req = {0};
    GtkWidget *child = NULL;
    HildonCaptionPrivate *priv = NULL;
    gboolean rtl;
    guint border_width;

    g_assert (HILDON_IS_CAPTION (widget));
    priv = HILDON_CAPTION_GET_PRIVATE (widget);

    /* Get the rtl status */
    rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);

    /* Position the caption to its allocated location */
    if (gtk_widget_get_realized (widget))
    {
        border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));
        gdk_window_move_resize (gtk_widget_get_window (widget),
                allocation->x + border_width,
                allocation->y + border_width,
                MAX (allocation->width - border_width * 2, 0),
                MAX (allocation->height - border_width * 2, 0));
    }

    child = gtk_bin_get_child (GTK_BIN (widget));
    if (child)
        gtk_widget_get_child_requisition (child, &child_req);

    gtk_widget_size_allocate (widget, allocation);
    gtk_widget_get_child_requisition (priv->caption_area, &req);

    child_alloc.height = caption_alloc.height = allocation->height;
    child_alloc.width  = caption_alloc.width  = allocation->width;
    child_alloc.x = caption_alloc.x = caption_alloc.y = child_alloc.y = 0;

    /* Center the captioned widget */
    if (rtl)
    {
        if (caption_alloc.width > child_req.width + HILDON_CAPTION_SPACING)
        {
            caption_alloc.x = caption_alloc.width - req.width;
            child_alloc.width = child_req.width;
        }
        caption_alloc.width -= child_req.width + HILDON_CAPTION_SPACING * 2;
    }
    else
    {
        if (child_alloc.width > req.width + HILDON_CAPTION_SPACING)
        {
            child_alloc.x += req.width + HILDON_CAPTION_SPACING * 2;
            caption_alloc.width = req.width;
        }
        /* Leave at least the space of the HILDON_CAPTION_SPACING in the left */
        caption_alloc.x = HILDON_CAPTION_SPACING;

        /* Leave room for the other drawable parts of the caption control */
        child_alloc.width -= req.width + HILDON_CAPTION_SPACING * 2;
    }

    /* Give the child at least its minimum requisition, unless it is expandable */
    if (! priv->expand && child && gtk_widget_get_visible (child))
    {
        child_alloc.width  = MIN (child_alloc.width,  child_req.width);
        child_alloc.height = MIN (child_alloc.height, child_req.height);
	/* Center the child */
	child_alloc.y = (allocation->height - child_alloc.height -
			 2 * gtk_container_get_border_width (GTK_CONTAINER (widget)))/2;
    }

    /* Ensure there are no negative dimensions */
    if (child_alloc.width < 0)
    {
        caption_alloc.width = req.width + child_alloc.width;
        child_alloc.width = 0;
        caption_alloc.width = MAX (caption_alloc.width, 0);
    }

    if (rtl)
        child_alloc.x = caption_alloc.x - child_req.width - HILDON_CAPTION_SPACING * 2;

    child_alloc.height = MAX (child_alloc.height, 0);
    caption_alloc.height = MAX (caption_alloc.height, 0);

    if (child && gtk_widget_get_visible (child) )
        gtk_widget_size_allocate (child, &child_alloc );

    gtk_widget_size_allocate (priv->caption_area, &caption_alloc);
}

static void 
hildon_caption_forall                           (GtkContainer *container,
                                                 gboolean include_internals,
                                                 GtkCallback callback, 
                                                 gpointer data)
{
    HildonCaptionPrivate *priv = NULL;

    g_assert (HILDON_IS_CAPTION (container));
    g_assert (callback != NULL);

    priv = HILDON_CAPTION_GET_PRIVATE (container);

    /* Execute callback for the child widgets */
    if (GTK_CONTAINER_CLASS (parent_class)->forall)
        GTK_CONTAINER_CLASS (parent_class)->forall (container, include_internals, callback, data);

    if (include_internals)
        /* Execute callback for the parent box as well */
        (*callback) (priv->caption_area, data);
}

/**
 * hildon_caption_set_size_group:
 * @caption: a #HildonCaption
 * @new_group: a #GtkSizeGroup
 *
 * Sets a #GtkSizeGroup of a given captioned control.
 *
 */
void 
hildon_caption_set_size_group                   (const HildonCaption *self,
                                                 GtkSizeGroup *group)
{
    g_object_set (G_OBJECT(self), "size_group", group, NULL);
}

/**
 * hildon_caption_get_size_group:
 * @caption: a #HildonCaption
 *
 * Queries given captioned control for the #GtkSizeGroup assigned to it.
 *
 * Returns: a #GtkSizeGroup
 * 
 */
GtkSizeGroup*
hildon_caption_get_size_group                   (const HildonCaption *self)
{
    HildonCaptionPrivate *priv;
    g_return_val_if_fail (HILDON_IS_CAPTION (self), NULL);

    priv = HILDON_CAPTION_GET_PRIVATE(self);

    return priv->group;
}

/**
 * hildon_caption_new:
 * @group: a #GtkSizeGroup for controlling the size of related captions or %NULL
 * @value: the caption text to accompany the text entry.  The widget makes
 *          a copy of this text.
 * @control: the control that is to be captioned.
 * @icon: an icon to accompany the label or %NULL in case no icon should be displayed.
 * @flag: indicates whether this captioned control is mandatory or
 *         optional.
 *
 * Creates a new instance of #HildonCaption widget, with a specific
 * control and image.
 * Note: Clicking on a focused caption will trigger the activate signal.
 * The default behaviour for the caption's activate signal is to call	 
 * gtk_widget_activate() on its control.
 * 
 * Returns: a new #HildonCaption
 */
GtkWidget*
hildon_caption_new                              (GtkSizeGroup *group, 
                                                 const gchar *value,
                                                 GtkWidget *child, 
                                                 GtkWidget *icon,
                                                 HildonCaptionStatus flag)
{
    GtkWidget *widget;
    g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

    widget = g_object_new (HILDON_TYPE_CAPTION, 
            "label", value,
            "child", child, 
            "size_group", group, 
            "icon", icon, 
            "status", flag,
            NULL);

    /* Do not expand GtkCheckButton by default, we want to reduce its activation area */
    if (GTK_IS_CHECK_BUTTON (child))
      hildon_caption_set_child_expand (HILDON_CAPTION (widget), FALSE);

    return widget;
}

/**
 * hildon_caption_is_mandatory:
 * @caption: a #HildonCaption
 * 
 * Queries whether @caption is mandatory.
 *
 * Returns: Whether this captioned control is mandatory.
 */
gboolean 
hildon_caption_is_mandatory                     (const HildonCaption *caption)
{
    HildonCaptionPrivate *priv;

    g_return_val_if_fail (HILDON_IS_CAPTION (caption), FALSE);

    priv = HILDON_CAPTION_GET_PRIVATE (caption);

    return (priv->status == HILDON_CAPTION_MANDATORY);
}

/**
 * hildon_caption_set_icon_position:
 * @caption: a #HildonCaption
 * @pos: one of the values from #HildonCaptionIconPosition
 *
 * Sets @caption<!-- -->'s icon position.
 *
 */
void 
hildon_caption_set_icon_position                (HildonCaption *caption,
                                                 HildonCaptionIconPosition pos)
{
    g_return_if_fail (HILDON_IS_CAPTION (caption));
    HildonCaptionPrivate *priv = HILDON_CAPTION_GET_PRIVATE (caption);

    g_return_if_fail (priv->caption_area != NULL);
    
    int order = (pos == HILDON_CAPTION_POSITION_LEFT) ? -1 : 0;
    gtk_box_reorder_child (GTK_BOX (priv->caption_area), priv->icon_align, order);

    priv->icon_position = pos;
}

/**
 * hildon_caption_get_icon_position:
 * @caption: a #HildonCaption
 *
 * Gets @caption<!-- -->'s icon position.
 *
 * Returns: one of the values from #HildonCaptionIconPosition.
 *
 */

HildonCaptionIconPosition 
hildon_caption_get_icon_position                (const HildonCaption *caption)
{
    g_return_val_if_fail (HILDON_IS_CAPTION (caption), 0);

    HildonCaptionPrivate *priv = HILDON_CAPTION_GET_PRIVATE (caption);

    return priv->icon_position;
}

/**
 * hildon_caption_set_status:
 * @caption: a #HildonCaption
 * @flag: one of the values from #HildonCaptionStatus
 *
 * Sets @caption<!-- -->'s status.
 */
void 
hildon_caption_set_status                       (HildonCaption *caption,
                                                 HildonCaptionStatus flag)
{
    g_return_if_fail (HILDON_IS_CAPTION(caption));

    g_object_set (G_OBJECT(caption), "status", flag, NULL);
}

/**
 * hildon_caption_get_status:
 * @caption: a #HildonCaption
 *
 * Gets @caption<!-- -->'s status.
 *
 * Returns: one of the values from #HildonCaptionStatus
 */
HildonCaptionStatus 
hildon_caption_get_status                       (const HildonCaption *caption)
{
    HildonCaptionPrivate *priv;
    g_return_val_if_fail (HILDON_IS_CAPTION (caption), HILDON_CAPTION_OPTIONAL);

    priv = HILDON_CAPTION_GET_PRIVATE(caption);

    return priv->status;
}

/**
 * hildon_caption_set_icon_image:
 * @caption: a #HildonCaption
 * @icon : the #GtkImage to use as the icon. 
 *         Calls gtk_widget_show() on the icon if it is not visible.
 *
 * Sets the icon image widget to be used by @caption.
 */
void 
hildon_caption_set_icon_image                   (HildonCaption *caption, 
                                                 GtkWidget *icon)
{
    g_return_if_fail (HILDON_IS_CAPTION(caption));

    g_object_set (G_OBJECT(caption), "icon", icon, NULL);
}

/**
 * hildon_caption_get_icon_image:
 * @caption: a #HildonCaption
 *
 * Gets the icon image widget used by @caption.
 *
 * Returns: the #GtkImage widget that is being used as the icon by
 *            @caption, or %NULL if no icon image is in use.
 */
GtkWidget*
hildon_caption_get_icon_image                   (const HildonCaption *caption)
{
    HildonCaptionPrivate *priv;

    g_return_val_if_fail (HILDON_IS_CAPTION (caption), NULL);

    priv = HILDON_CAPTION_GET_PRIVATE (caption);

    return priv->icon;
}

/**
 * hildon_caption_set_label:
 * @caption: a #HildonCaption
 * @label: the text to use
 *
 * Sets the label text that appears before the control.  
 * Separator character is added to the end of the label string. By default
 * the separator is ":". See also hildon_caption_set_separator().
 *
*/
void 
hildon_caption_set_label                        (HildonCaption *caption, 
                                                 const gchar *label)
{
    g_return_if_fail (HILDON_IS_CAPTION (caption));

    g_object_set (G_OBJECT(caption), "label", label, NULL);
}

/**
 * hildon_caption_set_label_markup:
 * @caption : a #HildonCaption
 * @markup : the markup text to use
 *
 * Sets the label markup text that appears before the control. It acts like
 * hildon_caption_set_label() but is using the markup text that allows to
 * specify text properties such as bold or italic.
 */
void 
hildon_caption_set_label_markup                 (HildonCaption *caption, 
                                                 const gchar *markup)
{
    g_return_if_fail (HILDON_IS_CAPTION (caption));

    g_object_set (G_OBJECT(caption), "markup", markup, NULL);
}

/**
 * hildon_caption_get_label:
 * @caption : a #HildonCaption
 *
 * Gets the label of @caption
 * 
 * Returns: the text currently being used as the label of @caption.
 * The string is owned by the label and the caller should never
 * free or modify this value.
 */
gchar*
hildon_caption_get_label                        (const HildonCaption *caption)
{
    HildonCaptionPrivate *priv;
    g_return_val_if_fail (HILDON_IS_CAPTION (caption), "");
    priv = HILDON_CAPTION_GET_PRIVATE (caption);

    return (gchar*) gtk_label_get_text (GTK_LABEL (priv->label));
}

/**
 * hildon_caption_set_separator:
 * @caption: a #HildonCaption
 * @separator: the separator to use
 *
 * Sets the separator character that appears after the label.  
 * The default seaparator character is ":".
 */
void 
hildon_caption_set_separator                    (HildonCaption *caption, 
                                                 const gchar *separator)
{
    g_return_if_fail (HILDON_IS_CAPTION (caption));

    g_object_set (G_OBJECT (caption), "separator", separator, NULL);
}

/**
 * hildon_caption_get_separator:
 * @caption: a #HildonCaption
 *
 * Gets the separator string of @caption
 *
 * Returns: the text currently being used as the separator of the caption
 *  control. The string is owned by @caption and the caller should
 *  never free or modify this value.
 */
gchar*
hildon_caption_get_separator                    (const HildonCaption *caption)
{
    HildonCaptionPrivate *priv;
    g_return_val_if_fail (HILDON_IS_CAPTION (caption), "");

    priv = HILDON_CAPTION_GET_PRIVATE (caption);

    return priv->separator;
}

/* Activates the child control
 * We have this signal so that if needed an application can 
 * know when we've been activated (useful for captions with
 * multiple children
 * FIXME: There never are multiple children. Possibly activate
 *  signal could be removed entirely? (does anyone use it?) 
 */
static void 
hildon_caption_activate                         (GtkWidget *widget)
{
    GtkWidget *child = gtk_bin_get_child (GTK_BIN (widget));
    gtk_widget_grab_focus (child);
}

static void
hildon_caption_grab_focus                       (GtkWidget *widget)
{
    gtk_widget_grab_focus (gtk_bin_get_child (GTK_BIN (widget)));
}

/**
 * hildon_caption_set_child_expand:
 * @caption: a #HildonCaption
 * @expand: whether the child is expandable.
 *
 * Sets child expandability.
 */
void 
hildon_caption_set_child_expand                 (HildonCaption *caption, 
                                                 gboolean expand)
{
    HildonCaptionPrivate *priv = NULL;
    GtkWidget *child = NULL;
    g_return_if_fail (HILDON_IS_CAPTION (caption));

    priv = HILDON_CAPTION_GET_PRIVATE (caption);

    /* Did the setting really change? */
    if (priv->expand == expand)
        return;

    priv->expand = expand;
    child = gtk_bin_get_child (GTK_BIN (caption));

    /* We do not have a child, nothing to do */
    if (! GTK_IS_WIDGET (child))
        return;

    if (gtk_widget_get_visible (child) && gtk_widget_get_visible (GTK_WIDGET (caption)))
        gtk_widget_queue_resize (child);

    gtk_widget_child_notify (child, "expand");
}

/**
 * hildon_caption_get_child_expand:
 * @caption: a #HildonCaption
 *
 * Gets childs expandability.
 *
 * Returns: wheter the child is expandable or not.
 */
gboolean 
hildon_caption_get_child_expand                 (const HildonCaption *caption)
{
    HildonCaptionPrivate *priv = NULL;
    g_return_val_if_fail (HILDON_IS_CAPTION (caption), FALSE);

    priv = HILDON_CAPTION_GET_PRIVATE (caption);

    return priv->expand;
}

static void
hildon_caption_set_label_text                   (HildonCaptionPrivate *priv, 
                                                 gboolean markup)
{
    gchar *tmp = NULL;
    g_assert (priv != NULL);

    if (priv->text)
    {
        if (priv->separator)
        {
            /* Don't duplicate the separator, if the string already contains one */
            if (g_str_has_suffix (priv->text, priv->separator))
            {
                if (markup)
                    gtk_label_set_markup (GTK_LABEL (priv->label), priv->text);
                else
                    gtk_label_set_text (GTK_LABEL (priv->label), priv->text);
            }
            else
            {
                /* Append separator and set text */
                tmp = g_strconcat( priv->text, priv->separator, NULL );
                
                if (markup)
                    gtk_label_set_markup (GTK_LABEL (priv->label), tmp);
                else
                    gtk_label_set_text (GTK_LABEL (priv->label), tmp);

                g_free (tmp);
            }
        }
        else
        {
            if (markup) 
                gtk_label_set_markup (GTK_LABEL (priv->label), priv->text);
            else
                gtk_label_set_text (GTK_LABEL (priv->label), priv->text);
        }
    }
    else
    {
        /* Clear the label */
        if (markup)
            gtk_label_set_markup (GTK_LABEL (priv->label), "");
        else
            gtk_label_set_text (GTK_LABEL (priv->label), "" );
    }
}

/**
 * hildon_caption_set_label_alignment:
 * @caption: a #HildonCaption widget
 * @alignment: new vertical alignment
 *
 * Sets the vertical alignment to be used for the
 * text part of the caption. Applications need to
 * align the child control themselves.
 *
 */   
void 
hildon_caption_set_label_alignment              (HildonCaption *caption, 
                                                 gfloat alignment)
{
    HildonCaptionPrivate *priv;

    g_return_if_fail (HILDON_IS_CAPTION (caption));

    priv = HILDON_CAPTION_GET_PRIVATE (caption);

    g_object_set (priv->label, "yalign", alignment, NULL);
    g_object_set (priv->icon_align, "yalign", alignment, NULL);

}

/**
 * hildon_caption_get_label_alignment:
 * @caption: a #HildonCaption widget
 *
 * Gets current vertical alignment for the text part.
 *
 * Returns: vertical alignment
 *
 */
gfloat 
hildon_caption_get_label_alignment              (HildonCaption *caption)
{
    HildonCaptionPrivate *priv;
    gfloat result;

    g_return_val_if_fail (HILDON_IS_CAPTION (caption), 0);
    priv = HILDON_CAPTION_GET_PRIVATE (caption);

    g_object_get (priv->label, "yalign", &result, NULL);

    return result;
}
