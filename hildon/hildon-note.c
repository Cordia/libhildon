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
 * SECTION:hildon-note
 * @short_description: A widget to ask confirmation from the user.
 *
 * #HildonNote is a convenient way to prompt users for a small amount of
 * input. A simple note contains an information text and, in case of
 * confirmation notes, it shows buttons to confirm or cancel. It can also
 * include a #GtkProgressBar.
 *
 * This widget provides convenient functions to create either
 * information notes, confirmation notes or cancel notes, which are
 * useful to show the progress of a requested task allowing the user
 * to cancel it.
 *
 * To create information notes you can use
 * hildon_note_new_information(). hildon_note_new_confirmation()
 * creates a note with a text and two buttons to confirm or
 * cancel. Note that it is possible to create a confirmation note with
 * customized buttons by using
 * hildon_note_new_confirmation_add_buttons().
 *
 * To create a note with a text, a progress bar and cancel button,
 * hildon_note_new_cancel_with_progress_bar() can be used.
 *
 * <example>
 * <title>HildonNote example</title>
 * <programlisting>
 * <!-- -->
 * gboolean
 * show_confirmation_note (GtkWindow *parent)
 * {
 *   gint retcode;
 *   GtkWidget *note;
 *   note = hildon_note_new_confirmation (parent, "Confirmation message...");
 * <!-- -->
 *   retcode = gtk_dialog_run (GTK_DIALOG (note));
 *   gtk_widget_destroy (note);
 * <!-- -->
 *   if (retcode == GTK_RESPONSE_OK) {
 *        g_debug ("User pressed 'OK' button'");
 *        return TRUE;
 *   } else {
 *        g_debug ("User pressed 'Cancel' button");
 *        return FALSE;
 *   }
 * }
 * </programlisting>
 * </example>
 */

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <stdio.h>
#include                                        <string.h>
#include                                        <libintl.h>
#include                                        <X11/X.h>
#include                                        <X11/Xatom.h>
#include                                        <gdk/gdkx.h>

#undef HILDON_DISABLE_DEPRECATED

#include                                        "hildon-note.h"
#include                                        "hildon-defines.h"
#include                                        "hildon-sound.h"
#include                                        "hildon-gtk.h"
#include                                        "hildon-enum-types.h"
#include                                        "hildon-note-private.h"

#define                                         HILDON_INFORMATION_NOTE_MIN_HEIGHT 140

#define                                         HILDON_INFORMATION_NOTE_MARGIN 100

#define                                         CONFIRMATION_SOUND_PATH \
                                                "/usr/share/sounds/ui-confirmation_note.wav"

#define                                         INFORMATION_SOUND_PATH \
                                                "/usr/share/sounds/ui-information_note.wav"

#define                                         _(String) dgettext("hildon-libs", String)

static void 
hildon_note_class_init                          (HildonNoteClass *class);

static void
hildon_note_init                                (HildonNote *dialog);

static void 
hildon_note_rebuild                             (HildonNote *note);

#ifdef MAEMO_GTK
static void
hildon_note_set_padding                         (HildonNote *note);
#endif /* MAEMO_GTK */

static void
hildon_note_rename                              (HildonNote *note);

static void
hildon_note_finalize                            (GObject *obj_self);

static void
hildon_note_realize                             (GtkWidget *widget);

static void
hildon_note_unrealize                           (GtkWidget *widget);

static void
hildon_note_size_request                        (GtkWidget      *note,
                                                 GtkRequisition *req);

static void
label_size_request                              (GtkWidget      *label,
                                                 GtkRequisition *req,
                                                 GtkWidget      *note);

static void 
hildon_note_set_property                        (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec);

static void
hildon_note_get_property                        (GObject *object,
                                                 guint prop_id,
                                                 GValue *value, 
                                                 GParamSpec *pspec);

static void
on_show_cb                                      (GtkWidget *widget,
                                                 gpointer data);
static gboolean
sound_handling                                  (gpointer data);

static void
unpack_widget                                   (GtkWidget *widget);

enum 
{
    PROP_0,
    PROP_HILDON_NOTE_TYPE,
    PROP_HILDON_NOTE_DESCRIPTION,
    PROP_HILDON_NOTE_ICON,
    PROP_HILDON_NOTE_PROGRESSBAR,
    PROP_HILDON_NOTE_STOCK_ICON
};

static GtkDialogClass*                          parent_class;

static gboolean
event_box_press_event                           (GtkEventBox    *event_box,
                                                 GdkEventButton *event,
                                                 GtkDialog      *note)
{
    HildonNotePrivate *priv = HILDON_NOTE_GET_PRIVATE (note);

    if (priv->note_n == HILDON_NOTE_TYPE_INFORMATION ||
        priv->note_n == HILDON_NOTE_TYPE_INFORMATION_THEME) {
            gtk_dialog_response (note, GTK_RESPONSE_DELETE_EVENT);
            return TRUE;
    } else {
            return FALSE;
    }
}

static void
hildon_note_set_property                        (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value, 
                                                 GParamSpec * pspec)
{
    HildonNote *note = HILDON_NOTE (object);
    HildonNotePrivate *priv;
    GtkWidget *widget;

    priv = HILDON_NOTE_GET_PRIVATE (note);
    g_assert (priv);

    switch (prop_id) {

        case PROP_HILDON_NOTE_TYPE:
            priv->note_n = g_value_get_enum (value);
	    hildon_note_rename (note);
            hildon_note_rebuild (note);
            break;

        case PROP_HILDON_NOTE_DESCRIPTION:
            if (priv->original_description) 
                    g_free (priv->original_description);
            priv->original_description = g_value_dup_string (value);

            gtk_label_set_text (GTK_LABEL (priv->label), priv->original_description);
            /* FIXME Is the "original_description" used anywhere? */
            
            break;

        case PROP_HILDON_NOTE_ICON:
            if (priv->icon) {
              g_free (priv->icon);
            }
            priv->icon = g_value_dup_string (value);
            break;

        case PROP_HILDON_NOTE_STOCK_ICON:
            if (priv->stock_icon) {
              g_free (priv->stock_icon);
            }
            priv->stock_icon = g_value_dup_string (value);
            break;

        case PROP_HILDON_NOTE_PROGRESSBAR:
            widget = g_value_get_object (value);
            if (widget != priv->progressbar)
            {
                if (priv->progressbar)
                    g_object_unref (priv->progressbar);

                priv->progressbar = widget;

                if (widget)
                {
                    g_object_ref (widget);
                    gtk_object_sink (GTK_OBJECT (widget));
                }

                hildon_note_rebuild (note);
            }
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_note_get_property                        (GObject *object,
                                                 guint prop_id, 
                                                 GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonNote *note = HILDON_NOTE (object);
    HildonNotePrivate *priv;

    priv = HILDON_NOTE_GET_PRIVATE (note);

    switch (prop_id) {

        case PROP_HILDON_NOTE_TYPE:
            g_value_set_enum (value, priv->note_n);
            break;

        case PROP_HILDON_NOTE_DESCRIPTION:
            g_value_set_string (value, priv->original_description);
            break;

        case PROP_HILDON_NOTE_ICON:
            g_value_set_string (value, priv->icon);
            break;

        case PROP_HILDON_NOTE_STOCK_ICON:
            g_value_set_string (value, priv->stock_icon);
            break;

        case PROP_HILDON_NOTE_PROGRESSBAR:
            g_value_set_object (value, priv->progressbar);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

#ifdef MAEMO_GTK
static GObject *
hildon_note_constructor (GType type,
                         guint n_construct_properties,
                         GObjectConstructParam *construct_params)
{
    GObject *object;
    object = (* G_OBJECT_CLASS (parent_class)->constructor)
        (type, n_construct_properties, construct_params);
    hildon_note_set_padding (HILDON_NOTE (object));

    return object;
}
#endif /* MAEMO_GTK */

/**
 * hildon_note_get_type:
 *
 * Returns GType for HildonNote.
 *
 * Returns: HildonNote type
 */
GType G_GNUC_CONST
hildon_note_get_type                            (void)
{
    static GType dialog_type = 0;

    if (! dialog_type) {
        static const GTypeInfo dialog_info = {
            sizeof(HildonNoteClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) hildon_note_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof(HildonNote),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_note_init
        };
        dialog_type = g_type_register_static (GTK_TYPE_DIALOG,
                "HildonNote",
                &dialog_info, 0);
    }
    return dialog_type;
}

static void 
hildon_note_class_init                          (HildonNoteClass *class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

    /* set the global parent_class */
    parent_class = g_type_class_peek_parent (class);

    g_type_class_add_private (class, sizeof (HildonNotePrivate));

    object_class->finalize      = hildon_note_finalize;
    object_class->set_property  = hildon_note_set_property;
    object_class->get_property  = hildon_note_get_property;
#ifdef MAEMO_GTK
    object_class->constructor   = hildon_note_constructor;
#endif /* MAEMO_GTK */
    widget_class->realize       = hildon_note_realize;
    widget_class->unrealize     = hildon_note_unrealize;
    widget_class->size_request  = hildon_note_size_request;

    /**
     * HildonNote:type:
     *
     * The type of the #HildonNote, defining its contents, behavior, and
     * theming.
     */
    g_object_class_install_property (object_class,
            PROP_HILDON_NOTE_TYPE,
            g_param_spec_enum ("note-type",
                "note type",
                "The type of the note dialog",
                hildon_note_type_get_type (),
                HILDON_NOTE_TYPE_CONFIRMATION,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonNote:description:
     *
     * The text that appears in the #HildonNote.
     */
    g_object_class_install_property (object_class,
            PROP_HILDON_NOTE_DESCRIPTION,
            g_param_spec_string ("description",
                "note description",
                "The text that appears in the note dialog",
                "",
                G_PARAM_READWRITE));

    /**
     * HildonNote:icon:
     *
     * Icon for the note.
     *
     * Deprecated: Since 2.2
     */
    g_object_class_install_property (object_class,
            PROP_HILDON_NOTE_ICON,
            g_param_spec_string ("icon",
                "note icon",
                "The name of the icon that appears in the note dialog",
                "",
                G_PARAM_READWRITE));

    /**
     * HildonNote:stock-icon:
     *
     * Stock icon name for the note.
     *
     * Deprecated: Since 2.2
     */
    g_object_class_install_property (object_class,
            PROP_HILDON_NOTE_STOCK_ICON,
            g_param_spec_string ("stock-icon",
                "Stock note icon",
                "The stock name of the icon that appears in the note dialog",
                "",
                G_PARAM_READWRITE));

    /**
     * HildonNote:progressbar:
     *
     * If set, a #GtkProgressBar is displayed in the note.
     */
    g_object_class_install_property (object_class,
            PROP_HILDON_NOTE_PROGRESSBAR,
            g_param_spec_object ("progressbar",
                "Progressbar widget",
                "The progressbar that appears in the note dialog",
                GTK_TYPE_PROGRESS_BAR,
                G_PARAM_READWRITE));
}

static void 
hildon_note_init                                (HildonNote *dialog)
{
    HildonNotePrivate *priv = HILDON_NOTE_GET_PRIVATE (dialog);
    g_assert (priv);

    priv->label = gtk_label_new (NULL);
    gtk_label_set_line_wrap (GTK_LABEL (priv->label), TRUE);
    gtk_label_set_justify (GTK_LABEL (priv->label), GTK_JUSTIFY_LEFT);

    priv->event_box = gtk_event_box_new ();
    priv->icon = NULL;
    priv->stock_icon = NULL;
    priv->idle_handler = 0;
    priv->align = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);

    gtk_container_add (GTK_CONTAINER (priv->event_box), priv->align);

    gtk_event_box_set_visible_window (GTK_EVENT_BOX (priv->event_box), FALSE);
    gtk_event_box_set_above_child (GTK_EVENT_BOX (priv->event_box), TRUE);
    g_signal_connect (priv->event_box, "button-press-event",
                      G_CALLBACK (event_box_press_event), dialog);

    /* Because ESD is synchronous, we wish to play sound after the
       note is already on screen to avoid blocking its appearance */
    g_signal_connect (GTK_WIDGET (dialog), "show",
                      G_CALLBACK (on_show_cb), NULL);

    /* Acquire real references to our internal children, since
       they are not nessecarily packed into container in each
       layout */
    g_object_ref_sink (priv->event_box);
    g_object_ref_sink (priv->label);

    gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    /* We use special hint to turn the note into information notification. */
    gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_NOTIFICATION);

    /* This prevents the window from being too big when the screen is rotated */
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
}


static void 
hildon_note_finalize                            (GObject *obj_self)
{
    HildonNotePrivate *priv = HILDON_NOTE_GET_PRIVATE (obj_self);
    g_assert (priv);

    /* FIXME Some of this stuff should be moved to dispose */

    /* Free internal data */
    if (priv->event_box)
        g_object_unref (priv->event_box);

    if (priv->label)
        g_object_unref (priv->label);

    if (priv->icon) {
        g_free (priv->icon);
        priv->icon = NULL;
    }
    if (priv->stock_icon) {
        g_free (priv->stock_icon);
        priv->stock_icon = NULL;
    }
    if (priv->idle_handler) {
        g_source_remove (priv->idle_handler);
        priv->idle_handler = 0;
    }

    if (priv->progressbar)
        g_object_unref (priv->progressbar);

    if (priv->original_description)
        g_free (priv->original_description);

    G_OBJECT_CLASS (parent_class)->finalize (obj_self);
}

static void
label_size_request                              (GtkWidget      *label,
                                                 GtkRequisition *req,
                                                 GtkWidget      *note)
{
    gint note_height = MAX (HILDON_INFORMATION_NOTE_MIN_HEIGHT, req->height + 2*HILDON_MARGIN_DOUBLE);
    g_object_set (note, "height-request", note_height, NULL);
}

static void
resize_button (GtkWidget *button, gpointer *data)
{
    gint width = GPOINTER_TO_INT (data);
    g_object_set (button, "width-request", width, NULL);
}

static void
hildon_note_orientation_update (HildonNote *note, GdkScreen *screen)
{
    GtkDialog *dialog = GTK_DIALOG (note);
    HildonNotePrivate* priv = HILDON_NOTE_GET_PRIVATE (note);
    GtkWidget *parent;
    gint button_width, padding;
    gboolean portrait = gdk_screen_get_width (screen) < gdk_screen_get_height (screen);

    g_object_ref (dialog->action_area);
    unpack_widget (dialog->action_area);

    if (portrait) {
        parent = dialog->vbox;
        button_width = gdk_screen_get_width (screen) - HILDON_MARGIN_DOUBLE * 2;
        padding = HILDON_MARGIN_DOUBLE;
    } else {
        parent = gtk_widget_get_parent (dialog->vbox);
        button_width = priv->button_width;
        padding = 0;
    }

    gtk_box_pack_end (GTK_BOX (parent), dialog->action_area,
                      portrait, TRUE, 0);
    gtk_box_reorder_child (GTK_BOX (parent), dialog->action_area, 0);
    gtk_container_foreach (GTK_CONTAINER (dialog->action_area),
                           (GtkCallback) resize_button,
                           GINT_TO_POINTER (button_width));
    g_object_unref (dialog->action_area);
    gtk_container_child_set (GTK_CONTAINER (priv->box), priv->label,
                             "padding", padding, NULL);
}

static void
hildon_note_size_request                        (GtkWidget      *note,
                                                 GtkRequisition *req)
{
    GTK_WIDGET_CLASS (parent_class)->size_request (note, req);
    req->width = gdk_screen_get_width (gtk_widget_get_screen (note));
}

static void
screen_size_changed                            (GdkScreen *screen,
                                                GtkWidget *note)
{
    HildonNotePrivate *priv = HILDON_NOTE_GET_PRIVATE (note);

    hildon_note_rename (HILDON_NOTE (note));

    if (priv->note_n == HILDON_NOTE_TYPE_INFORMATION ||
        priv->note_n == HILDON_NOTE_TYPE_INFORMATION_THEME) {
        gint screen_width = gdk_screen_get_width (screen);
        gint text_width = screen_width - HILDON_INFORMATION_NOTE_MARGIN * 2;
        g_object_set (priv->label, "width-request", text_width, NULL);

        return;
    } else if (priv->note_n == HILDON_NOTE_TYPE_CONFIRMATION) {
        hildon_note_orientation_update (HILDON_NOTE (note), screen);
    }
}

static void
hildon_note_realize                             (GtkWidget *widget)
{
    GdkDisplay *display;
    gboolean is_info_note = FALSE;
    Atom atom;
    const gchar *notification_type;
    HildonNotePrivate *priv = HILDON_NOTE_GET_PRIVATE (widget);
    g_assert (priv);

    /* Make widget->window accessible */
    GTK_WIDGET_CLASS (parent_class)->realize (widget);

    /* Border only, no titlebar */
    gdk_window_set_decorations (widget->window, GDK_DECOR_BORDER);

    /* Set the _HILDON_NOTIFICATION_TYPE property so Matchbox places the window correctly */
    display = gdk_drawable_get_display (widget->window);
    atom = gdk_x11_get_xatom_by_name_for_display (display, "_HILDON_NOTIFICATION_TYPE");

    if (priv->note_n == HILDON_NOTE_TYPE_INFORMATION ||
        priv->note_n == HILDON_NOTE_TYPE_INFORMATION_THEME) {
        notification_type = "_HILDON_NOTIFICATION_TYPE_INFO";
        is_info_note = TRUE;
    } else {
        notification_type = "_HILDON_NOTIFICATION_TYPE_CONFIRMATION";
    }

    XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), GDK_WINDOW_XID (widget->window),
                     atom, XA_STRING, 8, PropModeReplace, (guchar *) notification_type,
                     strlen (notification_type));

    if (is_info_note) {
        g_signal_connect (priv->label, "size-request", G_CALLBACK (label_size_request), widget);
    }

    GdkScreen *screen = gtk_widget_get_screen (widget);
    g_signal_connect (screen, "size-changed", G_CALLBACK (screen_size_changed), widget);
    screen_size_changed (screen, widget);

    hildon_gtk_window_set_portrait_flags (GTK_WINDOW (widget), HILDON_PORTRAIT_MODE_SUPPORT);
}

static void
hildon_note_unrealize                           (GtkWidget *widget)
{
    HildonNotePrivate *priv = HILDON_NOTE_GET_PRIVATE (widget);
    GdkScreen *screen = gtk_widget_get_screen (widget);

    g_signal_handlers_disconnect_by_func (screen, G_CALLBACK (screen_size_changed), widget);
    g_signal_handlers_disconnect_by_func (priv->label, G_CALLBACK (label_size_request), widget);

    GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}


/* Helper function for removing a widget from it's container.
   we own a separate reference to each object we try to unpack,
   so extra referencing is not needed. */
static void 
unpack_widget                                   (GtkWidget *widget)
{
    g_assert (widget == NULL || GTK_IS_WIDGET (widget));

    if (widget && widget->parent)
        gtk_container_remove (GTK_CONTAINER (widget->parent), widget);
}

/*
  Name the widget and text label based on the note type. This is used
  by the theme to give proper backgrounds depending on the note type.
*/
static void
hildon_note_rename                              (HildonNote *note)
{
  GEnumValue *value;
  GEnumClass *enum_class;
  gchar *name;
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (note));
  gboolean portrait = gdk_screen_get_width (screen) < gdk_screen_get_height (screen);
  const gchar *portrait_suffix = portrait ? "-portrait" : NULL;

  HildonNotePrivate *priv = HILDON_NOTE_GET_PRIVATE (note);

  enum_class = g_type_class_ref (HILDON_TYPE_NOTE_TYPE);
  value = g_enum_get_value (enum_class, priv->note_n);

  name = g_strconcat ("HildonNote-", value->value_nick, portrait_suffix, NULL);
  gtk_widget_set_name (GTK_WIDGET (note), name);
  g_free (name);

  name = g_strconcat ("HildonNoteLabel-", value->value_nick, NULL);
  gtk_widget_set_name (priv->label, name);
  g_free (name);

  g_type_class_unref (enum_class);
}

#ifdef MAEMO_GTK
static void
hildon_note_set_padding (HildonNote *note)
{
    HildonNotePrivate *priv;

    priv = HILDON_NOTE_GET_PRIVATE (note);

    switch (priv->note_n) {
    case HILDON_NOTE_TYPE_INFORMATION:
    case HILDON_NOTE_TYPE_INFORMATION_THEME:
        gtk_dialog_set_padding (GTK_DIALOG (note),
                                HILDON_MARGIN_DOUBLE,
                                HILDON_MARGIN_DOUBLE,
                                0,
                                0);
        break;

    case HILDON_NOTE_TYPE_CONFIRMATION:
    case HILDON_NOTE_TYPE_CONFIRMATION_BUTTON:
        gtk_dialog_set_padding (GTK_DIALOG (note),
                                HILDON_MARGIN_DOUBLE,
                                HILDON_MARGIN_DEFAULT,
                                HILDON_MARGIN_DOUBLE,
                                HILDON_MARGIN_DOUBLE);
        break;

    default:
        break;
    }
}
#endif /* MAEMO_GTK */

static void
hildon_note_rebuild                             (HildonNote *note)
{
    GtkDialog *dialog;
    HildonNotePrivate *priv;
    gboolean is_info_note = FALSE;

    g_assert (HILDON_IS_NOTE (note));

    priv = HILDON_NOTE_GET_PRIVATE (note);
    g_assert (priv);

    dialog = GTK_DIALOG (note);

    /* Reuse exiting content widgets for new layout */
    unpack_widget (priv->label);
    unpack_widget (priv->progressbar);
    unpack_widget (priv->event_box);

    /* Destroy old layout and buttons */
    if (priv->box) {
        gtk_widget_destroy (priv->box);
        priv->box = NULL;
    }
    if (priv->okButton) {
        gtk_widget_destroy (priv->okButton);
        priv->okButton = NULL;
    }
    if (priv->cancelButton) {
        gtk_widget_destroy (priv->cancelButton);
        priv->cancelButton = NULL;
    }

    /* Add needed buttons and images for each note type */
    switch (priv->note_n)
    {
        case HILDON_NOTE_TYPE_CONFIRMATION:
            priv->okButton = gtk_dialog_add_button (dialog,
                    _("wdgt_bd_yes"), GTK_RESPONSE_OK);
            priv->cancelButton = gtk_dialog_add_button (dialog,
                    _("wdgt_bd_no"), GTK_RESPONSE_CANCEL);
            gtk_widget_show (priv->cancelButton);
            g_object_get (priv->okButton, "width-request",
                          &priv->button_width, NULL);
            gtk_widget_set_no_show_all (priv->cancelButton, FALSE);
#ifdef MAEMO_GTK
	    gtk_dialog_set_padding (dialog,
				    HILDON_MARGIN_DOUBLE,
				    HILDON_MARGIN_DEFAULT,
				    HILDON_MARGIN_DOUBLE,
				    HILDON_MARGIN_DOUBLE);
#endif /* MAEMO_GTK */
            break;

        case HILDON_NOTE_TYPE_PROGRESSBAR:
            priv->cancelButton = gtk_dialog_add_button (dialog,
                    _("wdgt_bd_stop"), GTK_RESPONSE_CANCEL);
            gtk_widget_show (priv->cancelButton);
            gtk_widget_set_no_show_all (priv->cancelButton, FALSE);
            break;

        case HILDON_NOTE_TYPE_INFORMATION_THEME:
        case HILDON_NOTE_TYPE_INFORMATION:
#ifdef MAEMO_GTK
	    gtk_dialog_set_padding (dialog,
				    HILDON_MARGIN_DOUBLE,
				    HILDON_MARGIN_DOUBLE,
				    0,
				    0);
#endif /* MAEMO_GTK */
            is_info_note = TRUE;
            break;

        case HILDON_NOTE_TYPE_CONFIRMATION_BUTTON:
        default:
            break;
    }

    /* Don't display the action area if this is just an information
     * note. This prevents text from being slightly aligned to the
     * left
     */
    if (is_info_note) {
        gtk_widget_hide (dialog->action_area);
    } else {
        gtk_widget_show (dialog->action_area);
    }
    gtk_widget_set_no_show_all (dialog->action_area, is_info_note);

    /* Pack label vertically. Spacing is only necessary for the progressbar note. */
    priv->box = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (priv->align), priv->box);
    gtk_box_pack_start (GTK_BOX (priv->box), priv->label, TRUE, TRUE, 0);

    if (priv->progressbar) {
        gtk_misc_set_alignment (GTK_MISC (priv->label), 0.0, 0.5);
        gtk_alignment_set_padding (GTK_ALIGNMENT (priv->align),
                                   HILDON_MARGIN_DOUBLE, 0, 0, 0);
        gtk_box_pack_start (GTK_BOX (priv->box), priv->progressbar, FALSE, FALSE, 0);
    } else {
        gtk_misc_set_alignment (GTK_MISC (priv->label), 0.5, 0.5);
        gtk_alignment_set_padding (GTK_ALIGNMENT (priv->align), 0, 0, 0, 0);
    }

#ifdef MAEMO_GTK
    hildon_note_set_padding (note);
#endif /* MAEMO_GTK */

    gtk_container_add (GTK_CONTAINER (dialog->vbox), priv->event_box);

    gtk_widget_show_all (priv->event_box);
}

/**
 * hildon_note_new_confirmation_add_buttons:
 * @parent: the parent window. The X window ID of the parent window
 *   has to be the same as the X window ID of the application. This is
 *   important so that the window manager could handle the windows
 *   correctly.
 *   In GTK the X window ID can be checked using
 *   GDK_WINDOW_XID(GTK_WIDGET(parent)->window).
 * @description: the message to confirm
 * @Varargs: arguments pairs for new buttons(label and return value). 
 *   Terminate the list with %NULL value.
 * 
 * Create a new confirmation note with custom buttons. Confirmation
 * note has a text and any number of buttons. It's important to note
 * that even though the name of the function might suggest, the
 * default ok/cancel buttons are not appended but you have to provide
 * all of the buttons.
 *
 * FIXME: This doc seems to be wrong, the two buttons aren't added so
 * it would only contain the "additional" buttons? However, changing
 * this would break those applications that rely on current behaviour.
 *
 * Returns: A new #HildonNote.
 */
GtkWidget*
hildon_note_new_confirmation_add_buttons        (GtkWindow *parent,
                                                 const gchar *description,
                                                 ...)
{
    va_list args;
    char *message;
    int value;
    GtkWidget *button;

    g_return_val_if_fail (description != NULL, NULL);

    GtkWidget *conf_note =
        g_object_new (HILDON_TYPE_NOTE,
                "note-type", HILDON_NOTE_TYPE_CONFIRMATION_BUTTON,
                "description", description,
                NULL);

    if (parent != NULL)
        gtk_window_set_transient_for (GTK_WINDOW (conf_note), parent);

    /* Add the buttons from varargs */
    va_start(args, description);

    while (TRUE) {
        message = va_arg (args, char *);

        if (! message) {
            break;
        }
        value = va_arg (args, int);

        button = gtk_dialog_add_button (GTK_DIALOG (conf_note), message, value);
        /* maemo-gtk is going to set the "no-show-all" property all
           cancel/close-like buttons to TRUE, so that they are not shown. On
           the other hand, this confirmation note with custom buttons should
           not obey this rule, so we need to make sure they are shown. */
        gtk_widget_show (button);
        gtk_widget_set_no_show_all (button, FALSE);
    }

    va_end (args);

    return conf_note;
}


/**
 * hildon_note_new_confirmation:
 * @parent: the parent window. The X window ID of the parent window
 *   has to be the same as the X window ID of the application. This is
 *   important so that the window manager could handle the windows
 *   correctly. In GTK the X window ID can be checked using
 *   GDK_WINDOW_XID(GTK_WIDGET(parent)->window).
 * @description: the message to confirm.
 *
 * Create a new confirmation note. Confirmation note has a text (description)
 * that you specify and two buttons.
 *
 * Returns: a new #HildonNote.
 */
GtkWidget*
hildon_note_new_confirmation                    (GtkWindow *parent,
                                                 const gchar *description)
{
    GtkWidget *dialog = NULL;

    g_return_val_if_fail (description != NULL, NULL);

    dialog = g_object_new (HILDON_TYPE_NOTE,
            "note-type",
            HILDON_NOTE_TYPE_CONFIRMATION,
            "description", description, NULL);

    if (parent != NULL)
        gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

    return dialog;
}

/**
 * hildon_note_new_confirmation_with_icon_name:
 * @parent: the parent window. The X window ID of the parent window
 *   has to be the same as the X window ID of the application. This is
 *   important so that the window manager could handle the windows
 *   correctly. In GTK the X window ID can be checked using
 *   GDK_WINDOW_XID(GTK_WIDGET(parent)->window).
 * @description: the message to confirm
 * @icon_name: icon to be displayed. If NULL, default icon is used.
 * 
 * Create a new confirmation note. Confirmation note has a text (description) 
 * that you specify and two buttons.
 *
 * Deprecated: Since 2.2, icons are not shown in confirmation notes. Icons set
 * with this function will be ignored. Use hildon_note_new_confirmation() instead.
 *
 * Returns: a new #HildonNote.
 */
GtkWidget*
hildon_note_new_confirmation_with_icon_name     (GtkWindow *parent,
                                                 const gchar *description,
                                                 const gchar *icon_name)
{
    GtkWidget *dialog;

    dialog = hildon_note_new_confirmation (parent, description);
    g_object_set (dialog, "icon", icon_name, NULL);

    return dialog;
}

/**
 * hildon_note_new_information:
 * @parent: the parent window. The X window ID of the parent window
 *   has to be the same as the X window ID of the application. This is
 *   important so that the window manager could handle the windows
 *   correctly. In GTK the X window ID can be checked using
 *   GDK_WINDOW_XID(GTK_WIDGET(parent)->window).
 * @description: the message to confirm.
 * 
 * Create a new information note. Information note has text (a description)
 * that you specify and an OK button.
 * 
 * Returns: a new #HildonNote.
 */
GtkWidget*
hildon_note_new_information                     (GtkWindow *parent,
                                                 const gchar *description)
{
    GtkWidget *dialog = NULL;

    g_return_val_if_fail (description != NULL, NULL);

    dialog = g_object_new (HILDON_TYPE_NOTE,
            "note-type",
            HILDON_NOTE_TYPE_INFORMATION_THEME,
            "description", description, NULL);

    if (parent != NULL)
        gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

    return dialog;
}

/**
 * hildon_note_new_information_with_icon_name:
 * @parent: the parent window. The X window ID of the parent window
 *   has to be the same as the X window ID of the application. This is
 *   important so that the window manager could handle the windows
 *   correctly. In GTK the X window ID can be checked using
 *   GDK_WINDOW_XID(GTK_WIDGET(parent)->window).
 * @description: the message to confirm.
 * @icon_name: icon to be displayed. If %NULL, the default icon is used.
 * 
 * Create a new information note. An information note has text (a description)
 * that you specify, an OK button and an icon.
 * 
 * Deprecated: Since 2.2, icons are not shown in confirmation notes. Icons set
 * with this function will be ignored. Use hildon_note_new_information()
 * instead.
 *
 * Returns: a new #HildonNote.
 */
GtkWidget*
hildon_note_new_information_with_icon_name      (GtkWindow * parent,
                                                 const gchar *description,
                                                 const gchar *icon_name)
{
    GtkWidget *dialog;

    dialog = hildon_note_new_information (parent, description);
    g_object_set (dialog, "icon", icon_name, NULL);

    return dialog;
}

/* FIXME This documentation string LIES! */

/**
 * hildon_note_new_cancel_with_progress_bar:
 * @parent: the parent window. The X window ID of the parent window
 *   has to be the same as the X window ID of the application. This is
 *   important so that the window manager could handle the windows
 *   correctly. In GTK the X window ID can be checked using
 *   GDK_WINDOW_XID(GTK_WIDGET(parent)->window).
 * @description: the action to cancel.
 * @progressbar: a pointer to #GtkProgressBar to be filled with the
 *   progressbar assigned to this note. Use this to set the fraction of
 *   progressbar done. This parameter can be %NULL as well, in which
 *   case plain text cancel note appears.
 *
 * Create a new cancel note with a progress bar. Cancel note has 
 * text(description) that you specify, a Cancel button and a progress bar.
 *
 * Returns: a #GtkDialog. Use this to get rid of this note when you
 *   no longer need it.
 */
GtkWidget*
hildon_note_new_cancel_with_progress_bar        (GtkWindow *parent,
                                                 const gchar *description,
                                                 GtkProgressBar *progressbar)
{
    GtkWidget *dialog = NULL;

    g_return_val_if_fail (description != NULL, NULL);

    dialog = g_object_new (HILDON_TYPE_NOTE,
            "note-type",
            HILDON_NOTE_TYPE_PROGRESSBAR,
            "description", description,
            "progressbar",
            progressbar, NULL);

    if (parent != NULL)
        gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

    return dialog;
}


/**
 * hildon_note_set_button_text:
 * @note: a #HildonNote.
 * @text: sets the button text. If there are two buttons in dialog,
 *   the button texts will be &lt;text&gt;, "Cancel".  
 *
 * Sets the text of the button in @note.
 */
void 
hildon_note_set_button_text                     (HildonNote *note, 
                                                 const gchar *text)
{
    HildonNotePrivate *priv;

    g_return_if_fail (HILDON_IS_NOTE (note));

    priv = HILDON_NOTE_GET_PRIVATE (HILDON_NOTE (note));
    g_assert (priv);

    if (priv->okButton) {
        gtk_button_set_label (GTK_BUTTON (priv->okButton), text);
        gtk_button_set_label (GTK_BUTTON (priv->cancelButton),
                _("wdgt_bd_no"));
    } else {
        gtk_button_set_label (GTK_BUTTON (priv->cancelButton), text);
    }
}

/**
 * hildon_note_set_button_texts:
 * @note: a #HildonNote.
 * @text_ok: the new text of the default OK button.
 * @text_cancel: the new text of the default cancel button.
 *
 * Sets the text for the buttons in @note.
 */
void 
hildon_note_set_button_texts                    (HildonNote *note,
                                                 const gchar *text_ok,
                                                 const gchar *text_cancel)
{
    HildonNotePrivate *priv;

    g_return_if_fail (HILDON_IS_NOTE (note));

    priv = HILDON_NOTE_GET_PRIVATE (HILDON_NOTE (note));
    g_assert (priv);

    if (priv->okButton) {
        gtk_button_set_label (GTK_BUTTON (priv->okButton), text_ok);
        gtk_button_set_label (GTK_BUTTON (priv->cancelButton), text_cancel);
    } else {
        gtk_button_set_label (GTK_BUTTON (priv->cancelButton), text_cancel);
    }
}

static void
on_show_cb                                      (GtkWidget *widget,
                                                 gpointer data)
{
    HildonNotePrivate *priv;

    priv = HILDON_NOTE_GET_PRIVATE (widget);
    priv->idle_handler = gdk_threads_add_idle (sound_handling, widget);
}

/* We play a system sound when the note comes visible */
static gboolean
sound_handling                                  (gpointer data)
{
    HildonNotePrivate *priv = HILDON_NOTE_GET_PRIVATE (data);
    g_assert (priv);

    switch (priv->note_n)
    {
        case HILDON_NOTE_TYPE_INFORMATION:
        case HILDON_NOTE_TYPE_INFORMATION_THEME:
            hildon_play_system_sound (INFORMATION_SOUND_PATH);
            break;

        case HILDON_NOTE_TYPE_CONFIRMATION:
        case HILDON_NOTE_TYPE_CONFIRMATION_BUTTON:
            hildon_play_system_sound (CONFIRMATION_SOUND_PATH);
            break;

        default:
            break;
    };

    priv->idle_handler = 0;

    return FALSE;
}
