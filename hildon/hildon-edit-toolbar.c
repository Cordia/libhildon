/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008, 2009 Nokia Corporation, all rights reserved.
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
 * SECTION:hildon-edit-toolbar
 * @short_description: Toolbar for edition of user data.
 *
 * The #HildonEditToolbar is a toolbar which contains a label and two
 * buttons, one of them being an arrow pointing backwards.
 *
 * The label is a description of the action that the user is supposed
 * to do. The button is to be pressed when the user completes the
 * action. The arrow is used to go back to the previous view
 * discarding any changes.
 *
 * Note that those widgets don't do anything themselves by default. To
 * actually peform actions the developer must provide callbacks for
 * them.
 *
 * To add a #HildonEditToolbar to a window use
 * hildon_window_set_edit_toolbar().
 *
 * This toolbar is meant to replace the window title bar by emulating
 * its appearance, so you should put the window in full screen mode
 * with gtk_window_fullscreen().
 *
 * <example>
 * <title>HildonEditToolbar example</title>
 * <programlisting>
 * GtkWidget *window;
 * GtkWidget *toolbar;
 * // Declare more widgets here ...
 * <!-- -->
 * window = hildon_stackable_window_new ();
 * toolbar = hildon_edit_toolbar_new_with_text ("Choose items to delete", "Delete");
 * // Create more widgets here ...
 * <!-- -->
 * // Add toolbar to window
 * hildon_window_set_edit_toolbar (HILDON_WINDOW (window), HILDON_EDIT_TOOLBAR (toolbar));
 * <!-- -->
 * // Add other widgets ...
 * <!-- -->
 * g_signal_connect (toolbar, "button-clicked", G_CALLBACK (delete_button_clicked), someparameter);
 * g_signal_connect_swapped (toolbar, "arrow-clicked", G_CALLBACK (gtk_widget_destroy), window);
 * <!-- -->
 * gtk_widget_show_all (window);
 * gtk_window_fullscreen (GTK_WINDOW (window));
 * </programlisting>
 * </example>
 */

#include                                        "hildon-edit-toolbar.h"
#include                                        "hildon-edit-toolbar-private.h"
#include                                        "hildon-private.h"
#include                                        "hildon-gtk.h"

G_DEFINE_TYPE                                   (HildonEditToolbar, hildon_edit_toolbar, GTK_TYPE_HBOX);

#define                                         TOOLBAR_LEFT_PADDING 24
#define                                         TOOLBAR_RIGHT_PADDING 8

#define                                         HILDON_EDIT_TOOLBAR_ANIMATION_FRAMERATE ((float)1000/150)
#define                                         HILDON_EDIT_TOOLBAR_ANIMATION_TMPL "indicator_update%d"
#define                                         HILDON_EDIT_TOOLBAR_ANIMATION_NFRAMES 8

#define                                         HILDON_EDIT_TOOLBAR_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                HILDON_TYPE_EDIT_TOOLBAR, HildonEditToolbarPrivate));

typedef struct                                  _HildonEditToolbarPrivate HildonEditToolbarPrivate;

struct                                          _HildonEditToolbarPrivate
{
    GtkBox *hbox;
    GtkLabel *label;
    GtkWidget *animation;
    GtkButton *button;
    GtkButton *arrow;
};

enum {
    BUTTON_CLICKED,
    ARROW_CLICKED,
    N_SIGNALS
};

static guint                                    toolbar_signals [N_SIGNALS] = { 0 };

static void
hildon_edit_toolbar_style_set                   (GtkWidget *widget,
                                                 GtkStyle  *previous_style)
{
    guint width, height;
    HildonEditToolbarPrivate *priv = HILDON_EDIT_TOOLBAR_GET_PRIVATE (widget);

    if (GTK_WIDGET_CLASS (hildon_edit_toolbar_parent_class)->style_set)
        GTK_WIDGET_CLASS (hildon_edit_toolbar_parent_class)->style_set (widget, previous_style);

    gtk_widget_style_get (widget,
                          "arrow-width", &width,
                          "arrow-height", &height,
                          NULL);

    gtk_widget_set_size_request (GTK_WIDGET (priv->arrow), width, height);
}

static gboolean
hildon_edit_toolbar_draw                        (GtkWidget *widget,
                                                 cairo_t   *cr)
{
    gtk_render_background (gtk_widget_get_style_context (widget),
                           cr,
                           0, 0,
                           gtk_widget_get_allocated_width (widget),
                           gtk_widget_get_allocated_height (widget));

    if (GTK_WIDGET_CLASS (hildon_edit_toolbar_parent_class)->draw)
        return GTK_WIDGET_CLASS (hildon_edit_toolbar_parent_class)->draw (widget, cr);

    return FALSE;
}

static void
hildon_edit_toolbar_class_init                  (HildonEditToolbarClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;

    widget_class->style_set = hildon_edit_toolbar_style_set;
    widget_class->draw = hildon_edit_toolbar_draw;

    g_type_class_add_private (klass, sizeof (HildonEditToolbarPrivate));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "arrow-width",
            "Width of the arrow button",
            "Width of the arrow button",
            0, G_MAXUINT, 112,
            G_PARAM_READABLE));

    gtk_widget_class_install_style_property (
        widget_class,
        g_param_spec_uint (
            "arrow-height",
            "Height of the arrow button",
            "Height of the arrow button",
            0, G_MAXUINT, 56,
            G_PARAM_READABLE));

    /**
     * HildonEditToolbar::button-clicked:
     * @widget: the object which received the signal.
     *
     * Emitted when the toolbar button has been activated (pressed and released).
     *
     * Since: 2.2
     */
    toolbar_signals[BUTTON_CLICKED] =
        g_signal_new ("button_clicked",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0, NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    /**
     * HildonEditToolbar::arrow-clicked:
     * @widget: the object which received the signal.
     *
     * Emitted when the toolbar back button (arrow) has been activated
     * (pressed and released).
     *
     * Since: 2.2
     */
    toolbar_signals[ARROW_CLICKED] =
        g_signal_new ("arrow_clicked",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0, NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}

static void
button_clicked_cb                               (GtkButton *self,
                                                 HildonEditToolbar *toolbar)
{
    g_signal_emit (toolbar, toolbar_signals[BUTTON_CLICKED], 0);
}

static void
arrow_clicked_cb                                (GtkButton *self,
                                                 HildonEditToolbar *toolbar)
{
    g_signal_emit (toolbar, toolbar_signals[ARROW_CLICKED], 0);
}

static void
hildon_edit_toolbar_init                        (HildonEditToolbar *self)
{
    HildonEditToolbarPrivate *priv = HILDON_EDIT_TOOLBAR_GET_PRIVATE (self);
    GtkWidget *separator;
    GtkAlignment *align;
    GtkBox *hbox = GTK_BOX (self);

    priv->hbox = GTK_BOX (gtk_hbox_new (FALSE, 0));
    align = GTK_ALIGNMENT (gtk_alignment_new (0, 0.5, 1, 1));
    priv->label = GTK_LABEL (gtk_label_new (NULL));
    priv->animation = hildon_private_create_animation (
        HILDON_EDIT_TOOLBAR_ANIMATION_FRAMERATE,
        HILDON_EDIT_TOOLBAR_ANIMATION_TMPL,
        HILDON_EDIT_TOOLBAR_ANIMATION_NFRAMES);
    priv->button = GTK_BUTTON (hildon_gtk_button_new (HILDON_SIZE_AUTO));
    separator = gtk_vseparator_new ();
    priv->arrow = GTK_BUTTON (gtk_button_new ());

    g_object_set (priv->button, "can-focus", FALSE, NULL);
    g_object_set (priv->arrow, "can-focus", FALSE, NULL);

    g_signal_connect (priv->button, "clicked", G_CALLBACK (button_clicked_cb), self);
    g_signal_connect (priv->arrow, "clicked", G_CALLBACK (arrow_clicked_cb), self);

    gtk_box_set_spacing (hbox, 0);
    gtk_alignment_set_padding (align, 0, 0, TOOLBAR_LEFT_PADDING, TOOLBAR_RIGHT_PADDING);

    gtk_widget_set_name (GTK_WIDGET (self), "toolbar-edit-mode");
    gtk_widget_set_name (GTK_WIDGET (priv->arrow), "hildon-edit-toolbar-arrow");

    gtk_container_add (GTK_CONTAINER (align), GTK_WIDGET (priv->hbox));
    gtk_box_pack_start (priv->hbox, GTK_WIDGET (priv->label), TRUE, TRUE, 0);
    gtk_box_pack_start (priv->hbox, priv->animation, TRUE, TRUE, 10);
    gtk_box_pack_start (priv->hbox, GTK_WIDGET (priv->button), FALSE, FALSE, 0);

    gtk_widget_set_no_show_all (priv->animation, TRUE);

    gtk_box_pack_start (hbox, GTK_WIDGET (align), TRUE, TRUE, 0);
    gtk_box_pack_start (hbox, separator, FALSE, FALSE, 0);
    gtk_box_pack_start (hbox, GTK_WIDGET (priv->arrow), FALSE, FALSE, 0);

    gtk_misc_set_alignment (GTK_MISC (priv->label), 0, 0.5);
    gtk_misc_set_alignment (GTK_MISC (priv->animation), 0, 0.5);

    gtk_widget_show_all (GTK_WIDGET (align));
    gtk_widget_show_all (separator);
    gtk_widget_show_all (GTK_WIDGET (priv->arrow));
}

/**
 * hildon_edit_toolbar_set_label:
 * @toolbar: a #HildonEditToolbar
 * @label: a new text for the toolbar label
 *
 * Sets the label of @toolbar to @label. This will clear any
 * previously set value.
 *
 * Since: 2.2
 */
void
hildon_edit_toolbar_set_label                   (HildonEditToolbar *toolbar,
                                                 const gchar       *label)
{
    HildonEditToolbarPrivate *priv;
    g_return_if_fail (HILDON_IS_EDIT_TOOLBAR (toolbar));
    priv = HILDON_EDIT_TOOLBAR_GET_PRIVATE (toolbar);
    gtk_label_set_text (priv->label, label);
}

/**
 * hildon_edit_toolbar_set_button_label:
 * @toolbar: a #HildonEditToolbar
 * @label: a new text for the label of the toolbar button
 *
 * Sets the label of the toolbar button to @label. This will clear any
 * previously set value.
 *
 * Since: 2.2
 */
void
hildon_edit_toolbar_set_button_label            (HildonEditToolbar *toolbar,
                                                 const gchar       *label)
{
    HildonEditToolbarPrivate *priv;
    g_return_if_fail (HILDON_IS_EDIT_TOOLBAR (toolbar));
    priv = HILDON_EDIT_TOOLBAR_GET_PRIVATE (toolbar);
    gtk_button_set_label (priv->button, label);
}

/**
 * hildon_edit_toolbar_new:
 *
 * Creates a new #HildonEditToolbar.
 *
 * Returns: a new #HildonEditToolbar
 *
 * Since: 2.2
 */
GtkWidget *
hildon_edit_toolbar_new                         (void)
{
    return g_object_new (HILDON_TYPE_EDIT_TOOLBAR, NULL);
}

/**
 * hildon_edit_toolbar_new_with_text:
 * @label: Text for the toolbar label.
 * @button: Text for the toolbar button.
 *
 * Creates a new #HildonEditToolbar, with the toolbar label set to
 * @label and the button label set to @button.
 *
 * Returns: a new #HildonEditToolbar
 *
 * Since: 2.2
 */
GtkWidget *
hildon_edit_toolbar_new_with_text               (const gchar *label,
                                                 const gchar *button)
{
    GtkWidget *toolbar = g_object_new (HILDON_TYPE_EDIT_TOOLBAR, NULL);

    hildon_edit_toolbar_set_label (HILDON_EDIT_TOOLBAR (toolbar), label);
    hildon_edit_toolbar_set_button_label (HILDON_EDIT_TOOLBAR (toolbar), button);

    return toolbar;
}

void G_GNUC_INTERNAL
hildon_edit_toolbar_set_progress_indicator      (HildonEditToolbar *toolbar,
                                                 gboolean           show)
{
    HildonEditToolbarPrivate *priv;
    g_return_if_fail (HILDON_IS_EDIT_TOOLBAR (toolbar));
    priv = HILDON_EDIT_TOOLBAR_GET_PRIVATE (toolbar);
    if (show) {
        gtk_widget_show (priv->animation);
        gtk_box_set_child_packing (priv->hbox, GTK_WIDGET (priv->label),
                                   FALSE, FALSE, 0, GTK_PACK_START);
    } else {
        gtk_box_set_child_packing (priv->hbox, GTK_WIDGET (priv->label),
                                   TRUE, TRUE, 0, GTK_PACK_START);
        gtk_widget_hide (priv->animation);
    }
}
