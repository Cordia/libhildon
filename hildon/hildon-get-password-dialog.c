/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006, 2009 Nokia Corporation, all rights reserved.
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
 * SECTION:hildon-get-password-dialog
 * @short_description: A widget used to get a password.
 * @see_also: #HildonSetPasswordDialog
 * 
 * HildonGetPasswordDialog prompts the user for a password.  It allows
 * inputting password, with an optional configurable label eg. for
 * showing a custom message. The maximum length of the password can be set.
 *
 * <example>
 * <title>HildonGetPassword example</title>
 * <programlisting>
 * get_dialog =  HILDON_GET_PASSWORD_DIALOG (hildon_get_password_dialog_new (parent, FALSE));
 * <!-- -->
 * gtk_widget_show (GTK_WIDGET (get_dialog));
 * <!-- -->
 * i = gtk_dialog_run (GTK_DIALOG (get_dialog));
 * <!-- -->
 * pass = hildon_get_password_dialog_get_password (get_dialog);
 * <!-- -->
 * if (i == GTK_RESPONSE_OK &amp;&amp; (strcmp (pass, dialog.current_password) != 0))
 * {
 *      gtk_infoprint (GTK_WINDOW (parent), STR_PASSWORD_INCORRECT);
 *      gtk_widget_set_sensitive (GTK_WIDGET (dialog.button2), FALSE);
 *      gtk_widget_set_sensitive (GTK_WIDGET (dialog.button3), FALSE);
 *      gtk_widget_set_sensitive (GTK_WIDGET (dialog.button4), FALSE);
 * }
 * <!-- -->
 * else if (i == GTK_RESPONSE_OK)
 * {   
 *      gtk_widget_set_sensitive( GTK_WIDGET( dialog.button2 ), TRUE);
 * }
 * <!-- -->
 * else
 * {
 *      gtk_widget_set_sensitive (GTK_WIDGET (dialog.button2), FALSE);
 *      gtk_widget_set_sensitive (GTK_WIDGET (dialog.button3), FALSE);
 *      gtk_widget_set_sensitive (GTK_WIDGET (dialog.button4), FALSE);
 * }
 * gtk_widget_destroy (GTK_WIDGET (get_dialog));
 * }
 * </programlisting>
 * </example>
 */

#undef                                          HILDON_DISABLE_DEPRECATED

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <errno.h>
#include                                        <string.h>
#include                                        <strings.h>
#include                                        <unistd.h>
#include                                        <stdio.h>
#include                                        <libintl.h>

#include                                        "hildon-get-password-dialog.h"
#include                                        "hildon-caption.h"
#include                                        "hildon-banner.h"
#include                                        "hildon-get-password-dialog-private.h"
#include                                        "hildon-entry.h"
#include                                        "hildon-stock.h"

#define                                         _(String) dgettext("hildon-libs", String)

#define                                         HILDON_GET_PASSWORD_DIALOG_TITLE "ecdg_ti_get_old_password"

#define                                         HILDON_GET_PASSWORD_DIALOG_PASSWORD "ecdg_fi_get_old_pwd_enter_pwd"

#define                                         HILDON_GET_PASSWORD_DIALOG_OK HILDON_STOCK_DONE

#define                                         HILDON_GET_PASSWORD_DIALOG_CANCEL "ecdg_bd_get_old_password_dialog_cancel"

#define                                         HILDON_GET_PASSWORD_VERIFY_DIALOG_TITLE "ecdg_ti_verify_password"

#define                                         HILDON_GET_PASSWORD_VERIFY_DIALOG_PASSWORD "ecdg_fi_verify_pwd_enter_pwd"

#define                                         HILDON_GET_PASSWORD_VERIFY_DIALOG_OK HILDON_STOCK_DONE

#define                                         HILDON_GET_PASSWORD_VERIFY_DIALOG_CANCEL "ecdg_bd_verify_password_dialog_cancel"

#define                                         HILDON_GET_PASSWORD_DIALOG_MAX_CHARS "ckdg_ib_maximum_characters_reached"

static GtkDialogClass*                          parent_class;

static void
hildon_get_password_dialog_class_init           (HildonGetPasswordDialogClass *class);

static void
hildon_get_password_dialog_init                 (HildonGetPasswordDialog *widget);

static void
hildon_get_password_set_property                (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec);

static void
hildon_get_password_get_property                (GObject *object,
                                                 guint prop_id, 
                                                 GValue *value,
                                                 GParamSpec *pspec);

static void 
create_contents                                 (HildonGetPasswordDialog *dialog);

#ifdef MAEMO_GTK 
static void 
invalid_input                                   (GtkWidget *widget, 
                                                 GtkInvalidInputType reason, 
                                                 gpointer unused);
#endif

enum
{
    PROP_0,
    PROP_MESSAGE,
    PROP_PASSWORD,
    PROP_NUMBERS_ONLY,
    PROP_CAPTION_LABEL,
    PROP_MAX_CHARS,
    PROP_GET_OLD
};

/* Private functions */
static void
hildon_get_password_set_property                (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonGetPasswordDialog *dialog = HILDON_GET_PASSWORD_DIALOG (object);
    HildonGetPasswordDialogPrivate *priv;

    priv = HILDON_GET_PASSWORD_DIALOG_GET_PRIVATE (object);
    g_assert (priv);

    switch (prop_id) {

        case PROP_MESSAGE:
            /* Set label text representing password domain */
            gtk_label_set_text (priv->message_label, g_value_get_string (value));
            break;

        case PROP_PASSWORD:
            gtk_entry_set_text (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->password_entry))),
                    g_value_get_string(value));
            break;

        case PROP_NUMBERS_ONLY:
#ifdef MAEMO_GTK 
            /* Set input mode for the password entry */
            g_object_set(G_OBJECT (gtk_bin_get_child GTK_BIN ((priv->password_entry))),
                    "hildon-input-mode",
                    (g_value_get_boolean (value)
                     ? HILDON_GTK_INPUT_MODE_NUMERIC
                     : HILDON_GTK_INPUT_MODE_FULL),
                    NULL);
#endif
            break;

        case PROP_CAPTION_LABEL:
            hildon_get_password_dialog_set_caption (dialog, g_value_get_string (value));
            break;

        case PROP_MAX_CHARS:
            hildon_get_password_dialog_set_max_characters (dialog, g_value_get_int (value));
            break;

        case PROP_GET_OLD:
            priv->get_old = g_value_get_boolean (value);
            create_contents (dialog);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_get_password_get_property                (GObject *object,
                                                 guint prop_id,
                                                 GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonGetPasswordDialog *dialog = HILDON_GET_PASSWORD_DIALOG (object);
    HildonGetPasswordDialogPrivate *priv;
    const gchar *string;
    gint max_length;
#ifdef MAEMO_GTK 
    gint input_mode;
#endif

    priv = HILDON_GET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    switch (prop_id) {

        case PROP_MESSAGE:
            string = gtk_label_get_text (priv->message_label);
            g_value_set_string (value, string);
            break;

        case PROP_PASSWORD:
            string = gtk_entry_get_text (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->password_entry))));
            g_value_set_string (value, string);
            break;

        case PROP_NUMBERS_ONLY:
#ifdef MAEMO_GTK 
            /* This property is set if and only if the input mode
               of the password entry has been set to numeric only */
            g_object_get (G_OBJECT (gtk_bin_get_child (GTK_BIN (priv->password_entry))),
                    "hildon-input-mode", &input_mode, NULL);
            g_value_set_boolean (value,
                    (input_mode == HILDON_GTK_INPUT_MODE_NUMERIC));
#else
            g_value_set_boolean (value, FALSE);
#endif
            break;

        case PROP_CAPTION_LABEL:
            string = hildon_caption_get_label (priv->password_entry);
            g_value_set_string (value, string);
            break;

        case PROP_MAX_CHARS:
            max_length = gtk_entry_get_max_length 
                    (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->password_entry))));
            g_value_set_int (value, max_length);
            break;

        case PROP_GET_OLD:
            g_value_set_boolean (value, priv->get_old);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_get_password_dialog_class_init           (HildonGetPasswordDialogClass *class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (class);

    parent_class = g_type_class_peek_parent (class);

    /* Override virtual functions */
    object_class->set_property = hildon_get_password_set_property;
    object_class->get_property = hildon_get_password_get_property;

    /* Install new properties */
    
    /**
     * HildonGetPasswordDialog:message:
     *
     * Optional message displayed to the user.
     */
    g_object_class_install_property 
        (object_class, 
         PROP_MESSAGE, 
         g_param_spec_string ("message",
             "Message",
             "Set optional message",
             NULL,
             G_PARAM_READWRITE));

    /**
     * HildonGetPasswordDialog:password:
     *
     * Password field contents.
     */
    g_object_class_install_property
        (object_class, 
         PROP_PASSWORD,
         g_param_spec_string ("password",
             "Password",
             "Password field",
             "DEFAULT",
             G_PARAM_READWRITE));

    /**
     * HildonGetPasswordDialog:numbers-only:
     *
     * If the password entry field is operating in numbers-only mode.
     */
    g_object_class_install_property
        (object_class, 
         PROP_NUMBERS_ONLY,
         g_param_spec_boolean ("numbers-only",
             "NumbersOnly",
             "Set entry to accept only numeric values",
             FALSE,
             G_PARAM_READWRITE));

    /**
     * HildonGetPasswordDialog:caption-label:
     *
     * Caption label.
     */
    g_object_class_install_property
        (object_class, 
         PROP_CAPTION_LABEL,
         g_param_spec_string ("caption-label",
             "Caption Label",
             "The text to be set as the caption label",
             NULL,
             G_PARAM_READWRITE));
    
    /**
     * HildonGetPasswordDialog:max-characters:
     *
     * Maximum characters than can be entered.
     */
    g_object_class_install_property
        (object_class, 
         PROP_MAX_CHARS,
         g_param_spec_int ("max-characters",
             "Maximum Characters",
             "The maximum number of characters the password"
             " dialog accepts",
             G_MININT,
             G_MAXINT,
             0,
             G_PARAM_READWRITE));

    /**
     * HildonGetPasswordDialog:get-old:
     *
     * If the dialog is used to retrieve an old password or set a new one.
     */
    g_object_class_install_property
        (object_class,
         PROP_GET_OLD,
         g_param_spec_boolean ("get-old",
             "Get Old Password",
             "TRUE if dialog is a get old password dialog, "
             "FALSE if dialog is a get password dialog",
             FALSE,
             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    /* Install private structure */
    g_type_class_add_private (class, sizeof (HildonGetPasswordDialogPrivate));
}

static void
hildon_get_password_dialog_init                 (HildonGetPasswordDialog *dialog)
{
    /* Set initial properties for the dialog; the actual contents are
       created once the 'get-old' property is set with g_object_new */

    gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
}

static void
create_contents                                 (HildonGetPasswordDialog *dialog)
{
    HildonGetPasswordDialogPrivate *priv;
    GtkSizeGroup * group;
    GtkWidget *control;
    AtkObject *atk_aux = NULL;

    /* Cache private pointer for faster member access */
    priv = HILDON_GET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    /* Sizegroup for captions */
    group = GTK_SIZE_GROUP (gtk_size_group_new
            (GTK_SIZE_GROUP_HORIZONTAL));

    /* Dialog title */
    gtk_window_set_title (GTK_WINDOW (dialog),
            priv->get_old
            ? _(HILDON_GET_PASSWORD_DIALOG_TITLE)
            : _(HILDON_GET_PASSWORD_VERIFY_DIALOG_TITLE));

    /* Optional password domain label */
    priv->message_label = GTK_LABEL (gtk_label_new (NULL));

    /* Create buttons */
    gtk_dialog_add_button (GTK_DIALOG (dialog),
                    (priv->get_old
                     ? HILDON_GET_PASSWORD_DIALOG_OK
                     : HILDON_GET_PASSWORD_VERIFY_DIALOG_OK),
                    GTK_RESPONSE_OK);

    gtk_dialog_add_button (GTK_DIALOG (dialog),
                    (priv->get_old
                     ? _(HILDON_GET_PASSWORD_DIALOG_CANCEL)
                     : _(HILDON_GET_PASSWORD_VERIFY_DIALOG_CANCEL)),
                    GTK_RESPONSE_CANCEL);

    /* Create password text entry */
    control = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
    if ((atk_aux = gtk_widget_get_accessible(control)))
      {
	atk_object_set_name(atk_aux, "Passwd");
      }

    gtk_entry_set_width_chars (GTK_ENTRY (control), 20);

#ifdef MAEMO_GTK 
    g_object_set (control, "hildon-input-mode", HILDON_GTK_INPUT_MODE_FULL, NULL);
#endif

    gtk_entry_set_visibility (GTK_ENTRY(control), FALSE);
    priv->password_entry = HILDON_CAPTION
        (hildon_caption_new(group,
                            (priv->get_old
                             ? _(HILDON_GET_PASSWORD_DIALOG_PASSWORD)
                             : _(HILDON_GET_PASSWORD_VERIFY_DIALOG_PASSWORD)),
                            control, NULL,
                            HILDON_CAPTION_OPTIONAL));
    hildon_caption_set_separator (HILDON_CAPTION (priv->password_entry), "");

    /* Do the basic layout */
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
            GTK_WIDGET (priv->message_label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
            GTK_WIDGET (priv->password_entry), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);

    /* Ensure group is freed when all its contents have been removed */
    g_object_unref (group);
}

/**
 * hildon_get_password_dialog_get_type:
 *
 * Returns GType for HildonGetPasswordDialog as produced by
 * g_type_register_static().
 *
 * Returns: HildonGetPasswordDialog type
 */
GType G_GNUC_CONST
hildon_get_password_dialog_get_type             (void)
{
    static GType dialog_type = 0;

    if (! dialog_type) {
        static const GTypeInfo dialog_info = {
            sizeof (HildonGetPasswordDialogClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) hildon_get_password_dialog_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof (HildonGetPasswordDialog),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_get_password_dialog_init
        };

        dialog_type = g_type_register_static (GTK_TYPE_DIALOG,
                "HildonGetPasswordDialog",
                &dialog_info, 0);
    }
    return dialog_type;
}

/**
 * hildon_get_password_dialog_new:
 * @parent: parent window; can be NULL
 * @get_old: FALSE creates a new get password dialog and
 *           TRUE creates a new get old password dialog. That is,
 *           if the password to be obtained is the old password, 
 *           this parameter is specified TRUE.  
 * 
 * Construct a new HildonGetPasswordDialog.
 *
 * Returns: a new #GtkWidget of type HildonGetPasswordDialog
 */
GtkWidget*
hildon_get_password_dialog_new                  (GtkWindow *parent,
                                                 gboolean get_old)
{
    HildonGetPasswordDialog *dialog = g_object_new
        (HILDON_TYPE_GET_PASSWORD_DIALOG,
         "get-old", get_old, NULL);

    if (parent != NULL) {
        gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
    }

    return (GtkWidget *) dialog;
}

/**
 * hildon_get_password_dialog_new_with_default:
 * @parent: parent window; can be NULL
 * @password: a default password to be shown in password field
 * @get_old: FALSE creates a new get password dialog and
 *           TRUE creates a new get old password dialog. That is,
 *           if the password to be obtained is the old password,
 *           this parameter is specified TRUE.
 *                        
 * 
 * Same as #hildon_get_password_dialog_new but with a default password
 * in password field.
 *
 * Returns: a new #GtkWidget of type HildonGetPasswordDialog
 */
GtkWidget*
hildon_get_password_dialog_new_with_default     (GtkWindow *parent,
                                                 const gchar *password,
                                                 gboolean get_old)
{
    GtkWidget *dialog;

    dialog = hildon_get_password_dialog_new (parent, get_old);

    if (password != NULL)
        g_object_set (G_OBJECT (dialog), "password", password, NULL);

    return GTK_WIDGET (dialog);
}

/**
 * hildon_get_password_dialog_get_password:
 * @dialog: pointer to HildonSetPasswordDialog
 * 
 * Gets the currently entered password. The string should not be freed.
 *
 * Returns: current password entered by the user.
 */
const gchar*
hildon_get_password_dialog_get_password         (HildonGetPasswordDialog *dialog)
{
    GtkEntry *entry1;
    gchar *text1;

    HildonGetPasswordDialogPrivate *priv;

    g_return_val_if_fail (HILDON_IS_GET_PASSWORD_DIALOG (dialog), NULL);
    priv = HILDON_GET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    /* Retrieve the password entry widget */
    entry1 = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->password_entry)));
    text1 = GTK_ENTRY (entry1)->text;

    return text1;
}

/**
 * hildon_get_password_dialog_set_message
 * @dialog: the dialog
 * @message: a custom message or some other descriptive text to be set
 * 
 * Sets the optional descriptive text displayed at the top of the dialog.
 */
void 
hildon_get_password_dialog_set_message          (HildonGetPasswordDialog *dialog, 
                                                 const gchar *message)
{
    HildonGetPasswordDialogPrivate *priv;

    g_return_if_fail (HILDON_IS_GET_PASSWORD_DIALOG (dialog));

    priv = HILDON_GET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    gtk_label_set_text (priv->message_label, message);

}

/**
 * hildon_get_password_dialog_set_caption:
 * @dialog: the dialog
 * @new_caption: the text to be set as the caption label
 * 
 * Sets the password entry field's neigbouring label.
 */
void 
hildon_get_password_dialog_set_caption          (HildonGetPasswordDialog *dialog,
                                                 const gchar *new_caption)
{
    HildonGetPasswordDialogPrivate *priv;

    g_return_if_fail (HILDON_IS_GET_PASSWORD_DIALOG (dialog));
    g_return_if_fail (new_caption != NULL);

    priv = HILDON_GET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    hildon_caption_set_label (priv->password_entry, new_caption);
}

/**
 * hildon_get_password_dialog_set_max_characters:
 * @dialog: the dialog
 * @max_characters: the maximum number of characters the password dialog
 * accepts
 * 
 * sets the maximum number of characters allowed as the password
 */
void 
hildon_get_password_dialog_set_max_characters   (HildonGetPasswordDialog *dialog, 
                                                 gint max_characters)
{
    HildonGetPasswordDialogPrivate *priv;

    g_return_if_fail (max_characters > 0);
    g_return_if_fail (HILDON_IS_GET_PASSWORD_DIALOG (dialog));

    priv = HILDON_GET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    /* Apply the given length to password entry */
    gtk_entry_set_max_length (GTK_ENTRY
            (gtk_bin_get_child
             GTK_BIN ((priv->password_entry))),
             max_characters);

    /* FIXME There is a bug here -- the prev. signal needs to be
     * disconnected before connecting the new signal. Besides, this 
     * should go into the constructor */

#ifdef MAEMO_GTK 
    /* Connect callback to show error banner if the limit is exceeded */
    g_signal_connect (GTK_ENTRY
            (gtk_bin_get_child
             GTK_BIN ((priv->password_entry))),
            "invalid_input",
            G_CALLBACK (invalid_input),
            NULL
            );
#endif
}

#ifdef MAEMO_GTK 
/* Callback that gets called when maximum chars is reached in the entry */
static void 
invalid_input                                   (GtkWidget *widget, 
                                                 GtkInvalidInputType reason, 
                                                 gpointer unused) 
{
    if (reason == GTK_INVALID_INPUT_MAX_CHARS_REACHED) 
    {
        hildon_banner_show_information (widget, NULL, _(HILDON_GET_PASSWORD_DIALOG_MAX_CHARS));
    }
}
#endif
