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
 * SECTION:hildon-set-password-dialog
 * @short_description: A dialog used to set, change or remove a password.
 * @see_also: #HildonGetPasswordDialog
 *
 * HildonSetPasswordDialog allows setting and changing a password. 
 * 
 * In Change mode: Dialog is used to change or remove an existing
 * password. Unselecting the check box dims the password fields below
 * it. If the dialog is accepted with 'OK' while the check box is
 * unselected, a Confirmation Note is shown.  If the Confirmation Note
 * Dialog is accepted with 'Remove', the password protection is removed.  
 * 
 * In Set mode: Set Password Dialog is used to define a password, or
 * change a password that cannot be removed.
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
#include                                        <glib.h>
#include                                        <gdk/gdkkeysyms.h>

#include                                        "hildon-stock.h"
#include                                        "hildon-set-password-dialog.h"
#include                                        "hildon-caption.h"
#include                                        "hildon-note.h"
#include                                        "hildon-defines.h"
#include                                        "hildon-banner.h"
#include                                        "hildon-set-password-dialog-private.h"
#include                                        "hildon-entry.h"
#include                                        "hildon-check-button.h"

#define                                         HILDON_SET_PASSWORD_DIALOG_TITLE "ecdg_ti_set_password"

#define                                         HILDON_SET_PASSWORD_DIALOG_PASSWORD "ecdg_fi_set_passwd_enter_pwd"

#define                                         HILDON_SET_PASSWORD_DIALOG_VERIFY_PASSWORD "ecdg_fi_set_passwd_confirm"

#define                                         HILDON_SET_PASSWORD_DIALOG_OK HILDON_STOCK_DONE

#define                                         HILDON_SET_PASSWORD_DIALOG_CANCEL "ecdg_bd_set_password_dialog_cancel"

#define                                         HILDON_SET_MODIFY_PASSWORD_DIALOG_TITLE "ckdg_ti_dialog_c_passwd_change_password"

#define                                         HILDON_SET_MODIFY_PASSWORD_DIALOG_LABEL "ckdg_fi_dialog_c_passwd_pwd_protect"

#define                                         HILDON_SET_MODIFY_PASSWORD_DIALOG_PASSWORD "ckdg_fi_dialog_c_passwd_new_pwd"

#define                                         HILDON_SET_MODIFY_PASSWORD_DIALOG_VERIFY_PASSWORD "ckdg_fi_dialog_c_passwd_ver_pwd"

#define                                         HILDON_SET_MODIFY_PASSWORD_DIALOG_OK HILDON_STOCK_DONE

#define                                         HILDON_SET_MODIFY_PASSWORD_DIALOG_CANCEL "ckdg_bd_change_password_dialog_cancel"

#define                                         HILDON_SET_PASSWORD_DIALOG_MISMATCH "ecdg_ib_passwords_do_not_match"

#define                                         HILDON_SET_PASSWORD_DIALOG_EMPTY "ecdg_ib_password_is_empty"

#define                                         HILDON_SET_PASSWORD_DIALOG_REMOVE_PROTECTION "ckdg_nc_dialog_c_passwd_remove_pwd"

#define                                         HILDON_REMOVE_PROTECTION_CONFIRMATION_REMOVE "ckdg_bd_dialog_c_passwd_remove_button"

#define                                         HILDON_REMOVE_PROTECTION_CONFIRMATION_CANCEL "ckdg_bd_dialog_c_passwd_cancel_button"

#define                                         _(String) dgettext("hildon-libs", String)

#define                                         c_(String) dgettext("hildon-common-strings", String)

static GtkDialogClass*                          parent_class;

static void
hildon_set_password_dialog_class_init           (HildonSetPasswordDialogClass *class);

static void 
hildon_set_password_dialog_init                 (HildonSetPasswordDialog *dialog);

static void 
hildon_checkbox_toggled                         (GtkWidget *widget, 
                                                 gpointer dialog);

static void
hildon_set_password_response_change             (GtkDialog *d,
                                                 gint arg1, 
                                                 gpointer unused);

static void
hildon_set_password_response_set                (GtkDialog * d, 
                                                 gint arg1, 
                                                 gpointer unused);

static void 
create_contents                                 (HildonSetPasswordDialog *dialog);

static void
hildon_set_password_set_property                (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec);

static void
hildon_set_password_get_property                (GObject *object,
                                                 guint prop_id, 
                                                 GValue *value,
                                                 GParamSpec *pspec);

enum 
{
    PROP_0,
    PROP_MESSAGE,
    PROP_PASSWORD,
    PROP_HILDON_PASSWORD_DIALOG
};

static void
hildon_set_password_set_property                (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonSetPasswordDialog *dialog = HILDON_SET_PASSWORD_DIALOG (object);
    HildonSetPasswordDialogPrivate *priv;

    priv = HILDON_SET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    switch (prop_id) {

        case PROP_MESSAGE:
            /* Update domain label to display new value */
            gtk_label_set_text (priv->message_label, g_value_get_string (value));
            break;

        case PROP_PASSWORD:
            /* Update password entry to display new value */
            gtk_entry_set_text (GTK_ENTRY (priv->pwd1st_entry), g_value_get_string (value));
            break;

        case PROP_HILDON_PASSWORD_DIALOG:
            /* Note this is a G_PARAM_CONSTRUCT_ONLY type property */
            priv->protection = g_value_get_boolean (value);

            /* We now have the necessary information to populate the dialog */
            /* FIXME It looks kinda extremally bad that this is here... 
             * what about situations where this prop is NOT set during 
             * constructing? */
            create_contents (dialog);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_set_password_get_property                (GObject *object,
                                                 guint prop_id,
                                                 GValue *value,
                                                 GParamSpec *pspec)
{
    HildonSetPasswordDialogPrivate *priv = NULL;

    priv = HILDON_SET_PASSWORD_DIALOG_GET_PRIVATE (object);
    g_assert (priv);

    switch (prop_id) {

        case PROP_MESSAGE:
            g_value_set_string (value, gtk_label_get_text (priv->message_label));
            break;

        case PROP_PASSWORD:
            g_value_set_string (value,
                    gtk_entry_get_text (GTK_ENTRY (priv->pwd1st_entry)));
            break;

        case PROP_HILDON_PASSWORD_DIALOG:
            g_value_set_boolean (value, priv->protection);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
create_contents                                 (HildonSetPasswordDialog *dialog)
{
    HildonSetPasswordDialogPrivate *priv = NULL;
    AtkObject *atk_aux = NULL;

    GtkSizeGroup *group;

    priv = HILDON_SET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);
    priv->checkbox = NULL;

    /* Size group for labels */
    group = GTK_SIZE_GROUP (gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL));

    gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

    /* Setup and pack domain label */
    priv->message_label = GTK_LABEL (gtk_label_new (NULL));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
            GTK_WIDGET(priv->message_label), FALSE, FALSE, 0);
    gtk_widget_show (GTK_WIDGET (priv->message_label));

    if (priv->protection == TRUE) {
        /* Use Change Password Dialog strings */
        priv->pwd1st_caption_string = _(HILDON_SET_MODIFY_PASSWORD_DIALOG_PASSWORD);
        priv->pwd2nd_caption_string = _(HILDON_SET_MODIFY_PASSWORD_DIALOG_VERIFY_PASSWORD);

        /* Setup checkbox to enable/disable password protection */
	priv->checkbox = hildon_check_button_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);
	gtk_button_set_label (GTK_BUTTON (priv->checkbox), _(HILDON_SET_MODIFY_PASSWORD_DIALOG_LABEL));
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                priv->checkbox, TRUE, TRUE, 0);
        gtk_widget_show (priv->checkbox);
        hildon_check_button_set_active (HILDON_CHECK_BUTTON (priv->checkbox),
                TRUE);
        gtk_signal_connect (GTK_OBJECT (priv->checkbox), "toggled",
                G_CALLBACK (hildon_checkbox_toggled), dialog);

        /* Setup appropriate response handler */
        g_signal_connect (G_OBJECT (dialog), "response",
                G_CALLBACK (hildon_set_password_response_change),
                NULL);
    } else {
        /* Use Set Password Dialog strings */
        priv->pwd1st_caption_string = _(HILDON_SET_PASSWORD_DIALOG_PASSWORD);
        priv->pwd2nd_caption_string = _(HILDON_SET_PASSWORD_DIALOG_VERIFY_PASSWORD);

        /* Setup appropriate response handler */
        g_signal_connect (G_OBJECT (dialog), "response",
                G_CALLBACK(hildon_set_password_response_set),
                NULL);
    }

    /* Create the password field */
    priv->pwd1st_entry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
    if ((atk_aux = gtk_widget_get_accessible(priv->pwd1st_entry)))
      {
	atk_object_set_name(atk_aux, "Old Passwd");
      }
    

#ifdef MAEMO_GTK
    g_object_set (priv->pwd1st_entry, "hildon-input-mode", HILDON_GTK_INPUT_MODE_FULL, NULL);
#endif

    gtk_entry_set_visibility (GTK_ENTRY(priv->pwd1st_entry), FALSE);
    gtk_widget_show (priv->pwd1st_entry);
    priv->pwd1st_caption = hildon_caption_new (group,
            priv->pwd1st_caption_string,
            priv->pwd1st_entry,
            NULL, HILDON_CAPTION_OPTIONAL);

    hildon_caption_set_separator (HILDON_CAPTION(priv->pwd1st_caption), "");
    gtk_entry_set_visibility (GTK_ENTRY (priv->pwd1st_entry), FALSE);
    gtk_box_pack_start (GTK_BOX(GTK_DIALOG (dialog)->vbox),
            priv->pwd1st_caption, TRUE, TRUE, 0);
    gtk_widget_show (priv->pwd1st_caption);

    /* Create the password verify field */
    priv->pwd2nd_entry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
    if ((atk_aux = gtk_widget_get_accessible(priv->pwd2nd_entry)))
      {
	atk_object_set_name(atk_aux, "New Passwd");
      }


#ifdef MAEMO_GTK
    g_object_set (priv->pwd2nd_entry, "hildon-input-mode", HILDON_GTK_INPUT_MODE_FULL, NULL);
#endif

    gtk_widget_show (priv->pwd2nd_entry);
    priv->pwd2nd_caption = hildon_caption_new (group,
            priv->pwd2nd_caption_string,
            priv->pwd2nd_entry,
            NULL, HILDON_CAPTION_OPTIONAL);
    hildon_caption_set_separator (HILDON_CAPTION (priv->pwd2nd_caption), "");
    gtk_entry_set_visibility (GTK_ENTRY (priv->pwd2nd_entry), FALSE);
    gtk_box_pack_start (GTK_BOX(GTK_DIALOG (dialog)->vbox),
            priv->pwd2nd_caption, TRUE, TRUE, 0);
    gtk_widget_show (priv->pwd2nd_caption);

    /* Set dialog title */
    gtk_window_set_title (GTK_WINDOW (dialog),
            _(priv->protection
                ? HILDON_SET_MODIFY_PASSWORD_DIALOG_TITLE
                : HILDON_SET_PASSWORD_DIALOG_TITLE));

    /* Create the OK/CANCEL buttons */
    gtk_dialog_add_button (GTK_DIALOG (dialog),  _(priv->protection
                                ? HILDON_SET_MODIFY_PASSWORD_DIALOG_OK
                                : HILDON_SET_PASSWORD_DIALOG_OK), GTK_RESPONSE_OK);

    gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);
    gtk_widget_show_all (GTK_DIALOG (dialog)->action_area);
 
    /* Ensure group is freed when all its contents have been removed */
    g_object_unref (group);
}

static void
hildon_set_password_dialog_class_init           (HildonSetPasswordDialogClass *class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (class);

    parent_class = g_type_class_peek_parent (class);

    /* Override virtual methods */
    object_class->set_property = hildon_set_password_set_property;
    object_class->get_property = hildon_set_password_get_property;

    /* Install new properties */
    g_object_class_install_property (object_class, 
            PROP_MESSAGE, 
            g_param_spec_string ("message",
                "Message",
                "A message to display to the user",
                NULL,
                G_PARAM_READWRITE));

    /**
     * HildonSetPasswordDialog:modify-protection:
     *
     * Password type.
     */
    g_object_class_install_property (object_class, 
            PROP_HILDON_PASSWORD_DIALOG, 
            g_param_spec_boolean ("modify-protection",
                "Password type",
                "Set type to dialog",
                TRUE, 
                G_PARAM_CONSTRUCT_ONLY |
                G_PARAM_READWRITE));

    /**
     * HildonSetPasswordDialog:password:
     *
     * Content of the password field.
     */
    g_object_class_install_property (object_class, 
            PROP_PASSWORD, 
            g_param_spec_string ("password",
                "Password content",
                "Set content to dialog",
                "DEFAULT",
                G_PARAM_READWRITE));

    /* Install private structure */
    g_type_class_add_private (class,
            sizeof (HildonSetPasswordDialogPrivate));
}

static void
hildon_set_password_dialog_init                 (HildonSetPasswordDialog *dialog)
{
    /* Most of the initializations are done in create_contents()
       after the 'modify_protection' property has been set */

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
}


/* We come here when response button is clicked and dialog 
   is used to change existing password. */
static void
hildon_set_password_response_change             (GtkDialog *dialog, 
                                                 gint arg1,
                                                 gpointer unused)
{
    GtkEntry *pwd1st_entry;
    GtkEntry *pwd2nd_entry;
    gchar *text1;
    gchar *text2;
    HildonNote *note;
    gint i;
    HildonSetPasswordDialogPrivate *priv;

    priv = HILDON_SET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    /* Password and verification */
    pwd1st_entry = GTK_ENTRY (gtk_bin_get_child
            (GTK_BIN (priv->pwd1st_caption)));
    pwd2nd_entry = GTK_ENTRY (gtk_bin_get_child
            (GTK_BIN (priv->pwd2nd_caption)));
    text1 = GTK_ENTRY (pwd1st_entry)->text;
    text2 = GTK_ENTRY (pwd2nd_entry)->text;

    /* User accepted the dialog */
    if (arg1 == GTK_RESPONSE_OK) {
        /* Is the checkbox marked, so password protection is still in use? */  
        if (hildon_check_button_get_active (HILDON_CHECK_BUTTON (priv->checkbox))){
            /* Yes, Something is given as password as well? */
            if (text1[0] != '\0') {
                if (strcmp (text1, text2) == 0) {
                    /* Passwords match, so accept change password */
                    priv->protection = TRUE;

                } else if (text2[0] == '\0') {
                    /* Second field is empty, so show error, but don't clear fields */
                    g_signal_stop_emission_by_name (G_OBJECT(dialog),
                            "response");

                    hildon_banner_show_information (GTK_WIDGET (dialog), NULL,
                            c_(HILDON_SET_PASSWORD_DIALOG_MISMATCH));

                    gtk_widget_grab_focus (GTK_WIDGET (pwd2nd_entry));

                } else {
                    /* Error: Passwords don't match, so start over */
                    g_signal_stop_emission_by_name (G_OBJECT(dialog),
                            "response");

                    gtk_entry_set_text (GTK_ENTRY (pwd1st_entry), "");
                    gtk_entry_set_text (GTK_ENTRY (pwd2nd_entry), "");

                    hildon_banner_show_information (GTK_WIDGET (dialog), NULL,
                            c_(HILDON_SET_PASSWORD_DIALOG_MISMATCH));

                    gtk_widget_grab_focus (GTK_WIDGET (pwd1st_entry));
                }
            } else {
                /* No, the password is empty */
                g_signal_stop_emission_by_name (G_OBJECT (dialog), "response");

                if (text2[0] == '\0') {
                    /* Error: Both fields are empty */
                    hildon_banner_show_information (GTK_WIDGET (dialog), NULL, c_(HILDON_SET_PASSWORD_DIALOG_EMPTY));
                } else {
                    /* Error: Second field doesn't match
                       the empty first field, so start over */
                    hildon_banner_show_information (GTK_WIDGET (dialog), NULL, c_(HILDON_SET_PASSWORD_DIALOG_MISMATCH));
                    gtk_entry_set_text (GTK_ENTRY (pwd2nd_entry), "");
                }

                gtk_widget_grab_focus (GTK_WIDGET (pwd1st_entry));
            }
        } else {
            /* No, user wants to remove password protection. 
               Confirm remove password protection */
            note = HILDON_NOTE (hildon_note_new_confirmation
                    (GTK_WINDOW (dialog),
                     c_(HILDON_SET_PASSWORD_DIALOG_REMOVE_PROTECTION)));

            hildon_note_set_button_texts
                (HILDON_NOTE (note),
                 c_(HILDON_REMOVE_PROTECTION_CONFIRMATION_REMOVE), 
                 c_(HILDON_REMOVE_PROTECTION_CONFIRMATION_CANCEL));

            /* Display confirmation note */
            i = gtk_dialog_run (GTK_DIALOG (note));

            gtk_widget_destroy (GTK_WIDGET (note));

            if (i == GTK_RESPONSE_OK)
                /* Remove password protection */
                priv->protection = FALSE;
            else {
                /* Remove password protection cancelled */
                priv->protection = TRUE;
                g_signal_stop_emission_by_name (G_OBJECT(dialog), "response");
            }
        }

    } else {
        /* Watch out for fading boolean values */
        priv->protection = TRUE;
    }
}

/* We come here when response button is clicked and dialog 
   is used to set new password. */
static void
hildon_set_password_response_set                (GtkDialog *dialog, 
                                                 gint arg1,
                                                 gpointer unused)
{
    GtkEntry *pwd1st_entry;
    GtkEntry *pwd2nd_entry;
    gchar *text1;
    gchar *text2;

    HildonSetPasswordDialogPrivate *priv;

    priv = HILDON_SET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    /* Password and confirmation */
    pwd1st_entry = GTK_ENTRY (gtk_bin_get_child
            (GTK_BIN (priv->pwd1st_caption)));

    pwd2nd_entry = GTK_ENTRY (gtk_bin_get_child
            (GTK_BIN (priv->pwd2nd_caption)));

    text1 = GTK_ENTRY (pwd1st_entry)->text;
    text2 = GTK_ENTRY (pwd2nd_entry)->text;

    if (arg1 == GTK_RESPONSE_OK) {
        /* User provided something for password? */
        if (text1[0] != '\0') {
            if (strcmp (text1, text2) == 0) {
                /* Passwords match, so accept set password */
                priv->protection = TRUE;

            } else if (text2[0] == '\0') {
                /* Second field is empty, so show error,
                   but don't clear the fields */
                g_signal_stop_emission_by_name (G_OBJECT(dialog), "response");
                hildon_banner_show_information (GTK_WIDGET (dialog), NULL, c_(HILDON_SET_PASSWORD_DIALOG_MISMATCH));

                gtk_widget_grab_focus (GTK_WIDGET (priv->pwd2nd_entry));

            } else {
                /* Error: Passwords don't match, so start over */
                g_signal_stop_emission_by_name (G_OBJECT(dialog), "response");
                gtk_entry_set_text (GTK_ENTRY (pwd1st_entry), "");
                gtk_entry_set_text (GTK_ENTRY (pwd2nd_entry), "");
                hildon_banner_show_information (GTK_WIDGET (dialog), NULL, c_(HILDON_SET_PASSWORD_DIALOG_MISMATCH));

                gtk_widget_grab_focus (GTK_WIDGET (priv->pwd1st_entry));
            }
        } else {
            /* First field is empty */
            g_signal_stop_emission_by_name (G_OBJECT (dialog), "response");
            if (text2[0] == '\0') {
                /* Error: Both fields are empty */
                hildon_banner_show_information (GTK_WIDGET (dialog), NULL, c_(HILDON_SET_PASSWORD_DIALOG_EMPTY));
            } else {
                /* Error: Second field doesn't match
                   the empty first field, so start over */
                hildon_banner_show_information (GTK_WIDGET (dialog), NULL, c_(HILDON_SET_PASSWORD_DIALOG_MISMATCH));
                gtk_entry_set_text (GTK_ENTRY (pwd2nd_entry), "");
            }

            gtk_widget_grab_focus (GTK_WIDGET (pwd1st_entry));
        }
    } else { 
        /* Watch out for fading boolean values */
        priv->protection = FALSE;
    }         
}

static void
hildon_checkbox_toggled                         (GtkWidget *widget, 
                                                 gpointer dialog)
{
    HildonSetPasswordDialogPrivate *priv = HILDON_SET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    gboolean active;

    g_assert (priv);

    /* If the user enabled/disabled the password protection feature
       we enable/disable password entries accordingly */
    active = hildon_check_button_get_active (HILDON_CHECK_BUTTON (widget));
    gtk_widget_set_sensitive (GTK_WIDGET (priv->pwd1st_entry), active);
    gtk_widget_set_sensitive (GTK_WIDGET (priv->pwd2nd_entry), active);
}

/**
 * hildon_set_password_dialog_get_type:
 *
 * Returns GType for HildonPasswordDialog as produced by
 * g_type_register_static().
 *
 * Returns: HildonSetPasswordDialog type
 */
GType G_GNUC_CONST
hildon_set_password_dialog_get_type             (void)
{
    static GType dialog_type = 0;

    if (! dialog_type) {
        static const GTypeInfo dialog_info = {
            sizeof (HildonSetPasswordDialogClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) hildon_set_password_dialog_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof (HildonSetPasswordDialog),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_set_password_dialog_init
        };

        dialog_type = g_type_register_static (GTK_TYPE_DIALOG,
                "HildonSetPasswordDialog",
                &dialog_info, 0);
    }

    return dialog_type;
}

/**
 * hildon_set_password_dialog_new:
 * @parent: parent window; can be NULL
 * @modify_protection: TRUE creates a new change password dialog and FALSE
 *                     creates a new set password dialog 
 * 
 * Constructs a new HildonSetPasswordDialog.
 *
 * Returns: a new #GtkWidget of type HildonSetPasswordDialog
 */
GtkWidget*
hildon_set_password_dialog_new                  (GtkWindow *parent,
                                                 gboolean modify_protection)
{
    return hildon_set_password_dialog_new_with_default (parent, "", modify_protection);
}

/**
 * hildon_set_password_dialog_new_with_default:
 * @parent: parent window; can be NULL
 * @password: a default password to be shown in password field
 * @modify_protection: TRUE creates a new change password dialog and FALSE
 *                     creates a new set password dialog 
 * 
 * Same as #hildon_set_password_dialog_new, but with a default password
 * in password field.
 *
 * Returns: a new #GtkWidget of type HildonSetPasswordDialog
 */

GtkWidget*
hildon_set_password_dialog_new_with_default     (GtkWindow *parent,
                                                 const gchar *password,
                                                 gboolean modify_protection)
{
    GtkWidget *dialog = g_object_new (HILDON_TYPE_SET_PASSWORD_DIALOG,
            "modify_protection", modify_protection,
            "password", password, NULL);

    if (parent != NULL) {
        gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
    }

    return dialog;
}

/**
 * hildon_set_password_dialog_get_password:
 * @dialog: pointer to HildonSetPasswordDialog
 * 
 * Returns current password.
 *
 * Returns: changed password ( if the dialog is successfully 
 * accepted with 'OK' ( and when the check box is 'ON' ( in Change Password
 * Dialog ))
 */
const gchar*
hildon_set_password_dialog_get_password         (HildonSetPasswordDialog *dialog)
{
    HildonSetPasswordDialogPrivate *priv;

    g_return_val_if_fail (HILDON_IS_SET_PASSWORD_DIALOG (dialog), NULL);

    priv = HILDON_SET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    return GTK_ENTRY (priv->pwd1st_entry)->text;
}

/**
 * hildon_set_password_dialog_get_protected:
 * @dialog: pointer to HildonSetPasswordDialog
 * 
 * Returns the protection mode.
 *
 * Returns: password protection mode ( TRUE when the protection is
 *               'ON' and FALSE when the protection is 'OFF' )
 */
gboolean
hildon_set_password_dialog_get_protected        (HildonSetPasswordDialog *dialog)
{
    HildonSetPasswordDialogPrivate *priv;

    g_return_val_if_fail (HILDON_IS_SET_PASSWORD_DIALOG (dialog), FALSE);

    priv = HILDON_SET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    return priv->protection;
}

/**
 * hildon_set_password_dialog_set_message:
 * @dialog: the dialog
 * @message: the message or some other descriptive text to be set
 * 
 * Sets the optional descriptive text.
 */
void 
hildon_set_password_dialog_set_message          (HildonSetPasswordDialog *dialog, 
                                                 const gchar *message)
{
    HildonSetPasswordDialogPrivate *priv = NULL;

    g_return_if_fail (HILDON_IS_SET_PASSWORD_DIALOG (dialog));

    priv = HILDON_SET_PASSWORD_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    gtk_label_set_text (priv->message_label, message);  
}
