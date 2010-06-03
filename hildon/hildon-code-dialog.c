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
 * SECTION:hildon-code-dialog
 * @short_description: A keypad-like widget used to enter pincodes.
 *
 * #HildonCodeDialog displays a keypad that can be used to enter 
 * numerical pin codes or lock codes. It emits a 'input' signal each time 
 * an input action is performed on the dialog.
 *
 */

/* FIXME We need property access in this widget */

#undef                                          HILDON_DISABLE_DEPRECATED

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <libintl.h>
#include                                        <gdk/gdkkeysyms.h>

#include                                        "hildon-code-dialog.h"
#include                                        "hildon-defines.h"
#include                                        "hildon-banner.h"
#include                                        "hildon-stock.h"
#include                                        "hildon-code-dialog-private.h"

#define                                         HEIGHT (48-HILDON_MARGIN_DEFAULT)

#define                                         WIDTH  (100-HILDON_MARGIN_DEFAULT)

#define                                         BACKSPACE_ICON "general_backspace"

#define                                         _(String) \
                                                dgettext("hildon-libs", String)

#define                                         c_(String) \
                                                dgettext("hildon-common-strings", String)

#define                                         DEVICELOCK_OK HILDON_STOCK_DONE

#define                                         DEVICELOCK_TITLE dgettext("osso-system-lock", "secu_application_title")

#define                                         DEVICELOCK_MAX_CHAR_REACHED c_("ckdg_ib_maximum_characters_reached")
        
#define                                         MAX_PINCODE_LEN (10)

static void 
hildon_code_dialog_class_init                   (HildonCodeDialogClass *cd_class);

static void 
hildon_code_dialog_init                         (HildonCodeDialog *self);

static void
hildon_code_dialog_realize                      (GtkWidget *widget);

static void
hildon_code_dialog_unrealize                    (GtkWidget *widget);

static void
hildon_code_dialog_finalize                     (GObject *object);

static void 
hildon_code_dialog_button_clicked               (GtkButton *buttonm,
                                                 gpointer user_data);

static void
hildon_code_dialog_backspace                    (HildonCodeDialog *dialog);

static void
hildon_code_dialog_im_commit                    (GtkIMContext *im_context,
                                                 gchar *utf8,
                                                 gpointer user_data);

static void 
hildon_code_dialog_insert_text                  (GtkEditable *editable,
                                                 gchar *new_text,
                                                 gint new_text_length,
                                                 gint *position,
                                                 gpointer user_data);

static gboolean 
hildon_code_dialog_key_press_event              (GtkWidget *widget,
                                                 GdkEventKey *event,
                                                 gpointer user_data);

static void 
hildon_code_dialog_real_input                   (HildonCodeDialog *dialog);

static void
hildon_code_dialog_input                        (HildonCodeDialog *dialog);

static GtkDialogClass*                          parent_class = NULL;

static guint                                    input_signal;

/**
 * hildon_code_dialog_get_type:
 *
 * Initializes and returns the type of a hildon code dialog.
 *
 * Returns: GType of #HildonCodeDialog
 */
GType G_GNUC_CONST
hildon_code_dialog_get_type                     (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo info = 
        {
            sizeof (HildonCodeDialogClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) hildon_code_dialog_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof (HildonCodeDialog),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_code_dialog_init
        };
        type = g_type_register_static (GTK_TYPE_DIALOG,
                "HildonCodeDialog", &info, 0);
    }
    return type;
}

static void
hildon_code_dialog_class_init                   (HildonCodeDialogClass *cd_class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (cd_class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (cd_class);

    parent_class = GTK_DIALOG_CLASS (g_type_class_peek_parent (cd_class));
    g_type_class_add_private (cd_class, sizeof (HildonCodeDialogPrivate));

    gobject_class->finalize = hildon_code_dialog_finalize;

    widget_class->realize = hildon_code_dialog_realize;
    widget_class->unrealize = hildon_code_dialog_unrealize;

    cd_class->input = hildon_code_dialog_real_input;

    /* FIXME Document this signal! */
    input_signal = g_signal_new("input",
                                HILDON_TYPE_CODE_DIALOG,
                                G_SIGNAL_RUN_LAST,
                                G_STRUCT_OFFSET (HildonCodeDialogClass, input),
                                NULL, NULL,
                                g_cclosure_marshal_VOID__VOID,
                                G_TYPE_NONE,
                                0);
}

static void 
hildon_code_dialog_init                         (HildonCodeDialog *dialog)
{
    HildonCodeDialogPrivate *priv;
    gint i, x, y;
    GtkWidget *dialog_vbox1 = NULL;
    GtkWidget *table = NULL;
    GtkWidget *alignment = NULL;
    GtkWidget *vbox1 = NULL;
    GtkWidget *image1 = NULL;
    GtkWidget *dialog_action_area1 = NULL;
    GdkGeometry hints;
    GtkWidget *okButton;

    priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);
    
    const gchar* numstrs[10] = {
        "0","1","2","3","4","5","6","7","8","9"
    };

    GdkPixbuf* pixbuf = NULL;
    GtkIconTheme* icon_theme = NULL;
    GtkIconInfo *icon_info = NULL;
    gint base_size = 0;

    /* Set default title */
    gtk_window_set_title (GTK_WINDOW (dialog), DEVICELOCK_TITLE);

    gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_dialog_set_has_separator ((GtkDialog *) dialog, FALSE);

    hints.min_width  = -1;
    hints.min_height = -1;
    hints.max_width  = -1;
    hints.max_height = -1;

    gtk_window_set_geometry_hints (GTK_WINDOW (dialog), GTK_WIDGET (dialog), &hints,
            GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);

    table = gtk_table_new (4, 3, FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), HILDON_MARGIN_DEFAULT );
    gtk_table_set_col_spacings (GTK_TABLE (table), HILDON_MARGIN_DEFAULT );

    dialog_vbox1 = GTK_DIALOG (dialog)->vbox;
    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_box_set_spacing (GTK_BOX (vbox1), HILDON_MARGIN_DOUBLE);

    priv->help_text = gtk_label_new ("");
    alignment = gtk_alignment_new (0.5,0,1,1);
    gtk_container_add (GTK_CONTAINER (alignment), priv->help_text);

    priv->entry = gtk_entry_new ();
    
    GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (priv->entry), GTK_CAN_FOCUS);
    gtk_entry_set_invisible_char (GTK_ENTRY (priv->entry), g_utf8_get_char ("*"));
    gtk_entry_set_alignment (GTK_ENTRY (priv->entry), 1.0);

    gtk_editable_set_editable (GTK_EDITABLE (priv->entry),FALSE);
    gtk_entry_set_visibility (GTK_ENTRY (priv->entry), FALSE);

    gtk_box_pack_start (GTK_BOX (vbox1), alignment, TRUE,FALSE,0);
    gtk_box_pack_start (GTK_BOX (vbox1), priv->entry, TRUE,FALSE,0);
    gtk_box_pack_start (GTK_BOX (vbox1), table, FALSE,TRUE,0);

    gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, FALSE,TRUE,0);

    for(i = 1;i <= 3; i++) {
        priv->buttons[0][i-1] = gtk_button_new_with_mnemonic (numstrs[i]);
        gtk_widget_set_size_request (priv->buttons[0][i-1], WIDTH, HEIGHT);
        gtk_table_attach_defaults (GTK_TABLE (table), priv->buttons[0][i-1],
                i-1, i, 0, 1);
    }

    for(i = 4;i <= 6;i++) {
        priv->buttons[1][i-4] = gtk_button_new_with_mnemonic (numstrs[i]);
        gtk_widget_set_size_request (priv->buttons[1][i-4], WIDTH, HEIGHT);
        gtk_table_attach_defaults (GTK_TABLE (table), priv->buttons[1][i-4],
                i-4, i-3, 1, 2);
    }

    for(i=7;i<=9;i++) {
        priv->buttons[2][i-7] = gtk_button_new_with_mnemonic (numstrs[i]);
        gtk_widget_set_size_request (priv->buttons[2][i-7], WIDTH, HEIGHT);
        gtk_table_attach_defaults (GTK_TABLE (table), priv->buttons[2][i-7],
                i-7, i-6, 2, 3);
    }

    priv->buttons[3][0] = priv->buttons[3][1] = 
        gtk_button_new_with_mnemonic (numstrs[0]);
    gtk_widget_set_size_request (priv->buttons[3][0], WIDTH, HEIGHT);
    gtk_table_attach (GTK_TABLE (table), priv->buttons[3][0],
            0,2,3,4, (GtkAttachOptions) (GTK_FILL),
            (GtkAttachOptions) (0), 0, 0);

    priv->buttons[3][2] = gtk_button_new ();
    gtk_widget_set_size_request (priv->buttons[3][2], WIDTH, HEIGHT);
    gtk_table_attach_defaults (GTK_TABLE (table), priv->buttons[3][2],
            2, 3, 3, 4);

    icon_theme = gtk_icon_theme_get_default ();

    icon_info = gtk_icon_theme_lookup_icon (icon_theme, BACKSPACE_ICON, 1,
            GTK_ICON_LOOKUP_NO_SVG);
    base_size = gtk_icon_info_get_base_size (icon_info);
    gtk_icon_info_free (icon_info);
    icon_info = NULL;
    pixbuf = gtk_icon_theme_load_icon (icon_theme,
            BACKSPACE_ICON, base_size,
            GTK_ICON_LOOKUP_NO_SVG,
            NULL);

    image1 = gtk_image_new_from_pixbuf (pixbuf);
    g_object_unref (G_OBJECT(pixbuf));
    gtk_container_add (GTK_CONTAINER (priv->buttons[3][2]), image1);
    dialog_action_area1 = GTK_DIALOG (dialog)->action_area;
    gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1),
#if GTK_CHECK_VERSION(2,11,0) || defined(MAEMO_GTK)
                               GTK_BUTTONBOX_CENTER);
#else
                               GTK_BUTTONBOX_DEFAULT_STYLE);
#endif

    okButton = gtk_dialog_add_button (GTK_DIALOG (dialog) ,DEVICELOCK_OK,
            GTK_RESPONSE_OK);

    gtk_widget_set_sensitive (okButton, FALSE);

    priv->buttons[4][0] = priv->buttons[4][1] = okButton;

    priv->im_context = gtk_im_multicontext_new();
#ifdef MAEMO_GTK
    g_object_set (G_OBJECT (priv->im_context), "hildon-input-mode",
                  HILDON_GTK_INPUT_MODE_NUMERIC, NULL);
#endif

    /*
       Connect signals.
    */
    g_signal_connect (G_OBJECT (priv->im_context), "commit",
                      G_CALLBACK (hildon_code_dialog_im_commit), dialog);

    g_signal_connect (G_OBJECT (priv->entry), "insert_text",
            G_CALLBACK (hildon_code_dialog_insert_text), dialog);
    
    gtk_entry_set_max_length (GTK_ENTRY (priv->entry), MAX_PINCODE_LEN);

    for (x = 0; x < 3; x++)
    {
        for (y = 0; y < 3; y++)
        {
            g_signal_connect (G_OBJECT (priv->buttons[x][y]), "clicked",
                G_CALLBACK (hildon_code_dialog_button_clicked), dialog);
            g_signal_connect (G_OBJECT (priv->buttons[x][y]), "key-press-event",
                G_CALLBACK (hildon_code_dialog_key_press_event), dialog);
        }
    }
            
    g_signal_connect (G_OBJECT (priv->buttons[3][0]), "clicked",
                G_CALLBACK (hildon_code_dialog_button_clicked), dialog);
    g_signal_connect (G_OBJECT (priv->buttons[3][0]), "key-press-event",
                G_CALLBACK (hildon_code_dialog_key_press_event), dialog);
    
    g_signal_connect (G_OBJECT (priv->buttons[3][2]), "clicked",
                G_CALLBACK (hildon_code_dialog_button_clicked), dialog);
    g_signal_connect (G_OBJECT (priv->buttons[3][2]), "key-press-event",
                G_CALLBACK (hildon_code_dialog_key_press_event), dialog);
        
    g_signal_connect (G_OBJECT (okButton), "key-press-event",
                G_CALLBACK(hildon_code_dialog_key_press_event), dialog);
    
    gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (dialog)->vbox));
}

static void
hildon_code_dialog_realize                      (GtkWidget *widget)
{
    HildonCodeDialog *dialog = HILDON_CODE_DIALOG (widget);
    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);

    if (GTK_WIDGET_CLASS (parent_class)->realize)
      (* GTK_WIDGET_CLASS (parent_class)->realize) (widget);

    gtk_im_context_set_client_window (GTK_IM_CONTEXT (priv->im_context),
                                     GTK_WIDGET (dialog)->window);
    gtk_im_context_focus_in (priv->im_context);
}

static void
hildon_code_dialog_unrealize                    (GtkWidget *widget)
{
    HildonCodeDialog *dialog = HILDON_CODE_DIALOG (widget);
    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);

    gtk_im_context_set_client_window (GTK_IM_CONTEXT (priv->im_context), NULL);

    if (GTK_WIDGET_CLASS (parent_class)->unrealize)
      (* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static void
hildon_code_dialog_finalize                     (GObject *object)
{
    HildonCodeDialog *dialog = HILDON_CODE_DIALOG (object);
    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);

    g_object_unref (priv->im_context);

    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void
hildon_code_dialog_backspace                    (HildonCodeDialog *dialog)
{
    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    gchar *text, *pos;

    g_assert (priv);

    text = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry)));
        
    pos = text;

    while (*pos != '\0')
    {
      pos ++;
    }

    pos = g_utf8_find_prev_char (text, pos);

    if (pos)
    {
      *pos=0;
    }

    gtk_entry_set_text (GTK_ENTRY (priv->entry), text);

    if (*text == 0)
    {
      gtk_widget_set_sensitive (priv->buttons[4][0], FALSE);
    }

    gtk_editable_set_position (GTK_EDITABLE (priv->entry), -1);

    g_free (text);
}

static void 
hildon_code_dialog_button_clicked               (GtkButton *button, 
                                                 gpointer user_data)
{
    HildonCodeDialog *dialog = HILDON_CODE_DIALOG (user_data);
    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    const char *number = gtk_button_get_label (button);

    if (number && *number )
    {
        gtk_editable_set_editable (GTK_EDITABLE (priv->entry), TRUE);

        g_signal_emit_by_name (GTK_ENTRY (priv->entry)->im_context, "commit",
                               number);

        gtk_editable_set_editable (GTK_EDITABLE (priv->entry), FALSE);

        gtk_editable_set_position (GTK_EDITABLE (priv->entry), -1);
    }
    else
    {
        hildon_code_dialog_backspace (dialog);
    }
}

static void
hildon_code_dialog_im_commit                    (GtkIMContext *im_context,
                                                 gchar *utf8,
                                                 gpointer user_data)
{
    HildonCodeDialog *dialog = HILDON_CODE_DIALOG (user_data);
    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    gint digit;

    g_assert (priv);

    digit = g_ascii_strtod(utf8, NULL);

    if (g_ascii_isdigit(*utf8))
    {
        gtk_editable_set_editable (GTK_EDITABLE (priv->entry), TRUE);

        g_signal_emit_by_name (GTK_ENTRY (priv->entry)->im_context, "commit",
                               utf8);

        gtk_editable_set_editable (GTK_EDITABLE (priv->entry), FALSE);

        gtk_editable_set_position (GTK_EDITABLE (priv->entry), -1);
    }
}

static void 
hildon_code_dialog_insert_text                  (GtkEditable *editable,
                                                 gchar *new_text,
                                                 gint new_text_length,
                                                 gint *position,
                                                 gpointer user_data)
{
    HildonCodeDialog *dialog = HILDON_CODE_DIALOG (user_data);
    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    gchar *text = g_strdup(gtk_entry_get_text (GTK_ENTRY (priv->entry)));
    glong length = g_utf8_strlen (text, -1);
    g_free (text);
    g_assert (priv);

    if (length == MAX_PINCODE_LEN)
    {
        hildon_banner_show_information (GTK_WIDGET (dialog),
                                        NULL,
                                        DEVICELOCK_MAX_CHAR_REACHED);
    }

    else if (! length)
    { 
        /* make the Ok button sensitive */
        gtk_widget_set_sensitive (priv->buttons[4][0], TRUE);
    }

    hildon_code_dialog_input (dialog);
}

static gboolean 
hildon_code_dialog_key_press_event              (GtkWidget *widget,
                                                 GdkEventKey *event,
                                                 gpointer user_data)
{
    HildonCodeDialog *dialog = HILDON_CODE_DIALOG (user_data);
    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    GtkWidget *new_widget = widget;
    gint x, y;
    
    g_assert (priv);

    if (gtk_im_context_filter_keypress (priv->im_context, event))
        return TRUE;

    if (event->keyval == GDK_BackSpace)
    {
        hildon_code_dialog_backspace (dialog);
        return TRUE;
    }

    for (x = 0; x < 5; x++)
    {
        for (y = 0; y < 3; y++)
        {
            if (priv->buttons[x][y] == widget)
                goto found;
        }
    }
    return FALSE;

found:

    while (new_widget == widget)
    {
        switch (event->keyval)
        {
            case GDK_Up:
                x = (x+4)%5;
                break;

            case GDK_Down:
                x = (x+1)%5;
                break;

            case GDK_Left:
                y = (y+2)%3;
                break;

            case GDK_Right:
                y = (y+1)%3;
                break;

            default:
                return FALSE;
        }
                
        new_widget = priv->buttons[x][y];
    }

    gtk_widget_grab_focus (new_widget);

    return TRUE;
}

/**
 * hildon_code_dialog_new:
 *
 * Use this function to create a new HildonCodeDialog.
 *
 * Return value: A @HildonCodeDialog.
 **/
GtkWidget*
hildon_code_dialog_new                          (void)
{
    HildonCodeDialog *dialog = g_object_new (HILDON_TYPE_CODE_DIALOG, NULL);

    return GTK_WIDGET (dialog);
}

/**
 * hildon_code_dialog_get_code:
 * @dialog: The #HildonCodeDialog from which to get the entered code
 *
 * Use this function to access the code entered by the user.
 *
 * Return value: The entered code.
 **/
const gchar*
hildon_code_dialog_get_code                     (HildonCodeDialog *dialog)
{
    g_return_val_if_fail (HILDON_IS_CODE_DIALOG (dialog), NULL);
    
    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    return gtk_entry_get_text (GTK_ENTRY (priv->entry));
}

/**
 * hildon_code_dialog_clear_code:
 * @dialog: The #HildonCodeDialog whose entry should be cleared:
 *
 * Use this function to clear the user entered code.
 **/
void 
hildon_code_dialog_clear_code                   (HildonCodeDialog *dialog)
{
    g_return_if_fail (HILDON_IS_CODE_DIALOG (dialog));

    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    gtk_entry_set_text (GTK_ENTRY (priv->entry), "");
    gtk_widget_set_sensitive (priv->buttons[4][0], FALSE);
}

/**
 * hildon_code_dialog_set_help_text:
 * @dialog: The #HildonCodeDialog whose entry should be cleared:
 * @text: The text to use in the help label.
 *
 * Use this function to set the text that will be displayd in the
 * help label
 **/
void 
hildon_code_dialog_set_help_text                (HildonCodeDialog *dialog, 
                                                 const gchar *text)
{
    g_return_if_fail (HILDON_IS_CODE_DIALOG (dialog));

    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    gtk_label_set_text (GTK_LABEL (priv->help_text), text);
}

static void 
hildon_code_dialog_real_input                   (HildonCodeDialog *dialog)
{
}

static void 
hildon_code_dialog_input                        (HildonCodeDialog *dialog)
{
    /* Emit the signal */
    g_signal_emit (dialog, input_signal, 0);
}

/**
 * hildon_code_dialog_set_input_sensitive
 * @dialog: The #HildonCodeDialog whose state is to be changed
 * @sensitive: The new state 
 *
 * This function will block or enable the input on the code dialog by
 * making the input button sensitive (or not).
 **/
void
hildon_code_dialog_set_input_sensitive          (HildonCodeDialog *dialog, 
                                                 gboolean sensitive)
{
    int i;
    int k;

    g_return_if_fail (HILDON_IS_CODE_DIALOG (dialog));

    HildonCodeDialogPrivate *priv = HILDON_CODE_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    for (i = 0; i < 5; i++)
        for (k = 0; k < 3; k++) 
            if (i != 4 && (k != 0 || k != 2))
                gtk_widget_set_sensitive (priv->buttons [i][k], sensitive);

    gtk_widget_set_sensitive (priv->help_text, sensitive);
    gtk_widget_set_sensitive (priv->entry, sensitive);
}
