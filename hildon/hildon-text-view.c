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
 * SECTION:hildon-text-view
 * @short_description: Text view within the Hildon framework.
 *
 * #HildonTextView is a text view derived from the #GtkTextView widget
 * but with a slightly different appearance designed for the Hildon
 * 2.2 framework.
 *
 * Text views in Hildon 2.2 can have a placeholder text, set with
 * hildon_gtk_text_view_set_placeholder_text(). This text will be
 * shown if the text view is empty and doesn't have the input focus,
 * but it's otherwise ignored. Thus, calls to
 * gtk_text_view_get_buffer() will never return the placeholder text,
 * not even when it's being displayed.
 *
 * <example>
 * <title>Creating a HildonTextView with a placeholder</title>
 * <programlisting>
 * GtkWidget *
 * create_text_view (void)
 * {
 *     GtkWidget *text_view;
 * <!-- -->
 *     text_view = hildon_text_view_new ();
 *     hildon_gtk_text_view_set_placeholder_text (GTK_TEXT_VIEW (text_view),
 *                                                "Type some text here");
 * <!-- -->
 *     return text_view;
 * }
 * </programlisting>
 * </example>
 */

#undef                                          HILDON_DISABLE_DEPRECATED

#include                                        "hildon-text-view.h"
#include <math.h>

#define HILDON_TEXT_VIEW_DRAG_THRESHOLD 16.0

G_DEFINE_TYPE                                   (HildonTextView, hildon_text_view, GTK_TYPE_TEXT_VIEW);

#define                                         HILDON_TEXT_VIEW_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                HILDON_TYPE_TEXT_VIEW, HildonTextViewPrivate));

typedef struct                                  _HildonTextViewPrivate HildonTextViewPrivate;

struct                                          _HildonTextViewPrivate
{
    gdouble x;                                                      /* tap x position */
    gdouble y;                                                      /* tap y position */
    gboolean selection_movement;                                    /* selection in progress */
};


/**
 * hildon_text_view_set_buffer:
 * @text_view: a #HildonTextView
 * @buffer: a #GtkTextBuffer
 *
 * Sets @buffer as the buffer being displayed by @text_view. The
 * previous buffer displayed by the text view is unreferenced, and a
 * reference is added to @buffer. If you owned a reference to @buffer
 * before passing it to this function, you must remove that reference
 * yourself
 *
 * Since: 2.2
 *
 * Deprecated: use gtk_text_view_set_buffer() instead
 */
void
hildon_text_view_set_buffer                     (HildonTextView *text_view,
                                                 GtkTextBuffer  *buffer)
{
    g_return_if_fail (HILDON_IS_TEXT_VIEW (text_view));
    g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
    gtk_text_view_set_buffer (GTK_TEXT_VIEW (text_view), buffer);
}

/**
 * hildon_text_view_get_buffer:
 * @text_view: a #HildonTextView
 *
 * Returns the text buffer in @text_view. The reference count is not
 * incremented; the caller of this function won't own a new reference.
 *
 * Note that the placeholder text (set using
 * hildon_gtk_text_view_set_placeholder_text()) is never contained in
 * this buffer.
 *
 * Returns: a #GtkTextBuffer
 *
 * Since: 2.2
 *
 * Deprecated: use gtk_text_view_get_buffer() instead
 */
GtkTextBuffer *
hildon_text_view_get_buffer                     (HildonTextView *text_view)
{
    g_return_val_if_fail (HILDON_IS_TEXT_VIEW (text_view), NULL);
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
}

/**
 * hildon_text_view_set_placeholder:
 * @text_view: a #HildonTextView
 * @text: the new text
 *
 * Sets the placeholder text in @text_view to @text.
 *
 * Since: 2.2
 *
 * Deprecated: use hildon_gtk_text_view_set_placeholder_text() instead
 */
void
hildon_text_view_set_placeholder                (HildonTextView *text_view,
                                                 const gchar    *text)
{
    g_return_if_fail (HILDON_IS_TEXT_VIEW (text_view) && text != NULL);
    hildon_gtk_text_view_set_placeholder_text (GTK_TEXT_VIEW (text_view), text);
}

/**
 * hildon_text_view_new:
 *
 * Creates a new text view.
 *
 * Returns: a new #HildonTextView
 *
 * Since: 2.2
 */
GtkWidget *
hildon_text_view_new                            (void)
{
    GtkWidget *entry = g_object_new (HILDON_TYPE_TEXT_VIEW, NULL);

    return entry;
}

static gint
hildon_text_view_button_press_event             (GtkWidget        *widget,
                                                 GdkEventButton   *event)
{
    HildonTextViewPrivate *priv = HILDON_TEXT_VIEW_GET_PRIVATE (widget);

    gtk_widget_grab_focus (widget);

    if (GTK_TEXT_VIEW (widget)->editable &&
        hildon_gtk_im_context_filter_event (GTK_TEXT_VIEW (widget)->im_context, (GdkEvent*)event)) {
        GTK_TEXT_VIEW (widget)->need_im_reset = TRUE;
        return TRUE;
    }

    if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
        priv->x = event->x;
        priv->y = event->y;

        if (event->state & GDK_SHIFT_MASK) {
            GtkTextBuffer *buffer;
            GtkTextWindowType window_type;
            GtkTextIter iter;
            gint x, y;
            GtkTextView *text_view = GTK_TEXT_VIEW (widget);

            priv->selection_movement = TRUE;

            window_type = gtk_text_view_get_window_type (text_view,
                                                         event->window);
            gtk_text_view_window_to_buffer_coords (text_view,
                                                   window_type,
                                                   event->x, event->y,
                                                   &x, &y);
            gtk_text_view_get_iter_at_location (text_view, &iter, x, y);
            buffer = gtk_text_view_get_buffer (text_view);
            if (gtk_text_buffer_get_char_count (buffer)) {
                gtk_text_buffer_place_cursor (buffer, &iter);
            }

            return GTK_WIDGET_CLASS (hildon_text_view_parent_class)->
                button_press_event (widget, event);
        }

        return TRUE;
    }

    return FALSE;
}

static gint
hildon_text_view_button_release_event           (GtkWidget        *widget,
                                                 GdkEventButton   *event)
{
    GtkTextView *text_view = GTK_TEXT_VIEW (widget);
    HildonTextViewPrivate *priv = HILDON_TEXT_VIEW_GET_PRIVATE (widget);
    GtkTextIter iter;
    gint x, y;

    if (text_view->editable &&
        hildon_gtk_im_context_filter_event (text_view->im_context, (GdkEvent*)event)) {
        text_view->need_im_reset = TRUE;
        return TRUE;
    }

    if (event->button == 1 && event->type == GDK_BUTTON_RELEASE &&
        !priv->selection_movement) {
        if (fabs (priv->x - event->x) < HILDON_TEXT_VIEW_DRAG_THRESHOLD &&
            fabs (priv->y - event->y) < HILDON_TEXT_VIEW_DRAG_THRESHOLD) {
            GtkTextWindowType window_type;
            GtkTextBuffer *buffer;

            window_type = gtk_text_view_get_window_type (text_view, event->window);
            gtk_text_view_window_to_buffer_coords (text_view,
                                                   window_type,
                                                   event->x, event->y,
                                                   &x, &y);
            gtk_text_view_get_iter_at_location (text_view, &iter, x, y);
            buffer = gtk_text_view_get_buffer (text_view);
            if (gtk_text_buffer_get_char_count (buffer))
                gtk_text_buffer_place_cursor (buffer, &iter);

            gtk_widget_grab_focus (GTK_WIDGET (text_view));

            return TRUE;
        }
    }

    if (priv->selection_movement) {
        gboolean result;

        result = GTK_WIDGET_CLASS (hildon_text_view_parent_class)->
            button_release_event (widget, event);

        if (result) {
            priv->selection_movement = FALSE;
        }

        return result;
    }

    return FALSE;
}

static void
hildon_text_view_class_init                     (HildonTextViewClass *klass)
{
    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;

    widget_class->motion_notify_event = NULL;
    widget_class->button_press_event = hildon_text_view_button_press_event;
    widget_class->button_release_event = hildon_text_view_button_release_event;

    g_type_class_add_private (klass, sizeof (HildonTextViewPrivate));
}
static void
hildon_text_view_init                           (HildonTextView *self)
{
    HildonTextViewPrivate *priv = HILDON_TEXT_VIEW_GET_PRIVATE (self);

    priv->selection_movement = FALSE;
}
