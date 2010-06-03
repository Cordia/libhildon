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
 * SECTION:hildon-login-dialog
 * @short_description: A widget which allows a user to enter an username
 * and a password.
 * @see_also: #HildonGetPasswordDialog, #HildonSetPasswordDialog
 *
 * #HildonLoginDialog is used to enter a username and password
 * when accessing a password protected area. The widget performs no
 * input checking and is used only for retrieving the username and a
 * password. 
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

#include                                        "hildon-login-dialog.h"
#include                                        "hildon-caption.h"
#include                                        "hildon-login-dialog-private.h"
#include                                        "hildon-entry.h"
#include                                        "hildon-stock.h"

enum
{
    PROP_0,
    PROP_MESSAGE,
    PROP_USERNAME,
    PROP_PASSWORD
};

static void
hildon_login_dialog_class_init                  (HildonLoginDialogClass *class);

static void
hildon_login_dialog_init                        (HildonLoginDialog *widget);

static void
hildon_login_dialog_set_property                (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value, 
                                                 GParamSpec *pspec);

static void
hildon_login_dialog_get_property                (GObject *object,
                                                 guint prop_id,
                                                 GValue *value, 
                                                 GParamSpec *pspec);

#define                                         HILDON_LOGIN_DIALOG_TITLE "frw_ti_get_user_name_and_pwd"

#define                                         HILDON_LOGIN_DIALOG_USERNAME "frw_ti_get_user_name_and_pwd_enter_user_name"

#define                                         HILDON_LOGIN_DIALOG_PASSWORD "frw_ti_get_user_name_and_pwd_enter_pwd"

#define                                         HILDON_LOGIN_DIALOG_OK HILDON_STOCK_DONE

#define                                         _(String) dgettext("hildon-libs", String)

static GtkDialogClass*                          parent_class;

/**
 * hildon_login_dialog_get_type:
 *
 * Returns GType for HildonLoginDialog.
 *
 * Returns: HildonLoginDialog type
 */
GType G_GNUC_CONST
hildon_login_dialog_get_type                    (void)
{
    static GType dialog_type = 0;

    if (! dialog_type) {
        static const GTypeInfo dialog_info = {
            sizeof (HildonLoginDialogClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) hildon_login_dialog_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof(HildonLoginDialog),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_login_dialog_init
        };
        dialog_type = g_type_register_static (GTK_TYPE_DIALOG,
                "HildonLoginDialog",
                &dialog_info, 0);
    }

    return dialog_type;
}

static void
hildon_login_dialog_set_property                (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonLoginDialog *dialog = NULL;
    HildonLoginDialogPrivate *priv   = NULL;

    dialog = HILDON_LOGIN_DIALOG (object);
    priv   = HILDON_LOGIN_DIALOG_GET_PRIVATE(dialog);
    g_assert (priv);

    switch (prop_id) {

        case PROP_MESSAGE:
            /* Set the password message text */
            hildon_login_dialog_set_message (dialog, g_value_get_string (value));
            break;

        case PROP_USERNAME:
            /* Set the current username displayed in the dialog */
            gtk_entry_set_text (GTK_ENTRY (priv->username_entry), g_value_get_string (value));
            break;

        case PROP_PASSWORD:
            /* Set the currently entered password */
            gtk_entry_set_text (GTK_ENTRY (priv->password_entry), g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_login_dialog_get_property                (GObject *object,
                                                 guint prop_id,
                                                 GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonLoginDialog *dialog = NULL;
    HildonLoginDialogPrivate *priv = NULL;

    dialog = HILDON_LOGIN_DIALOG (object);
    priv = HILDON_LOGIN_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    switch (prop_id) {

        case PROP_MESSAGE:
            g_value_set_string (value, gtk_label_get_text (priv->message_label));
            break;

        case PROP_USERNAME:
            g_value_set_string (value, hildon_login_dialog_get_username (dialog));
            break;

        case PROP_PASSWORD:
            g_value_set_string (value, hildon_login_dialog_get_password (dialog));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_login_dialog_class_init                  (HildonLoginDialogClass *class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (class);

    parent_class = g_type_class_peek_parent (class);

    /* Override virtual functions */
    object_class->set_property = hildon_login_dialog_set_property;
    object_class->get_property = hildon_login_dialog_get_property;

    /* Install new properties */

    /**
     * HildonLoginDialog:message:
     *
     * Optional message displayed to the user.
     */
    g_object_class_install_property (object_class, 
            PROP_MESSAGE, 
            g_param_spec_string ("message",
                "Message",
                "Message displayed by the dialog",
                NULL,
                G_PARAM_READWRITE));

    /**
     * HildonLoginDialog:username:
     *
     * Contents of the username field.
     */
    g_object_class_install_property (object_class,
            PROP_USERNAME, 
            g_param_spec_string ("username",
                "Username",
                "Username field",
                "DEFAULT",
                G_PARAM_READWRITE));
    
    /**
     * HildonLoginDialog:password:
     *
     * Contents of the password field.
     */
    g_object_class_install_property (object_class, 
            PROP_PASSWORD,
            g_param_spec_string ("password",
                "Password",
                "Password field",
                "DEFAULT",
                G_PARAM_READWRITE));

    /* Install private data structure */
    g_type_class_add_private (class, sizeof (HildonLoginDialogPrivate));
}

static void
hildon_login_dialog_init                        (HildonLoginDialog *dialog)
{
    /* Access private structure */
    HildonLoginDialogPrivate *priv = HILDON_LOGIN_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    /* Size group for captions */
    GtkSizeGroup *group = GTK_SIZE_GROUP (gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL));
    HildonCaption *caption;

    /* Initialize dialog */
    gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
    gtk_window_set_title (GTK_WINDOW(dialog), _(HILDON_LOGIN_DIALOG_TITLE));

    /* Optional message label */    
    /* FIXME Set the warpping for the message label */
    priv->message_label = GTK_LABEL (gtk_label_new (NULL));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), 
            GTK_WIDGET (priv->message_label), FALSE, FALSE, 0);

    /* Create buttons */    
    gtk_dialog_add_button (GTK_DIALOG (dialog), HILDON_STOCK_DONE, GTK_RESPONSE_OK);

    /* Setup username entry */
    priv->username_entry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);

#ifdef MAEMO_GTK 
    g_object_set (priv->username_entry, "hildon-input-mode", HILDON_GTK_INPUT_MODE_FULL, NULL);
#endif

    caption = HILDON_CAPTION (hildon_caption_new
            (group,
             _(HILDON_LOGIN_DIALOG_USERNAME),
             GTK_WIDGET (priv->username_entry), NULL,
             HILDON_CAPTION_OPTIONAL));

    hildon_caption_set_separator (caption, "");
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), 
            GTK_WIDGET (caption), FALSE, FALSE, 0);

    /* Setup password entry */
    priv->password_entry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);

#ifdef MAEMO_GTK
    g_object_set (priv->password_entry, "hildon-input-mode", HILDON_GTK_INPUT_MODE_FULL, NULL);
#endif

    gtk_entry_set_visibility (GTK_ENTRY (priv->password_entry), FALSE);

    caption = HILDON_CAPTION (hildon_caption_new (group,
                    _(HILDON_LOGIN_DIALOG_PASSWORD),
                    GTK_WIDGET (priv->password_entry),
                    NULL,
                    HILDON_CAPTION_OPTIONAL));

    hildon_caption_set_separator (caption, "");
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
            GTK_WIDGET (caption), FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);
    gtk_widget_show_all (GTK_DIALOG (dialog)->action_area);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    /* Ensure group is freed when all its contents have been removed */
    g_object_unref (group);
}

/**
 * hildon_login_dialog_new:
 * @parent: the parent window of the dialog
 *
 * Creates a new #HildonLoginDialog widget with Ok and Close
 * buttons.
 *
 * Returns: the newly created #HildonLoginDialog
 */
GtkWidget*
hildon_login_dialog_new                         (GtkWindow *parent)
{
    GtkWidget *self = g_object_new (HILDON_TYPE_LOGIN_DIALOG, NULL);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (self), parent);

    return self;
}

/**
 * hildon_login_dialog_new_with_default:
 * @parent: the parent window of the dialog
 * @name: default username, NULL if unset
 * @password: default password, NULL if unset
 * 
 * Same as #hildon_login_dialog_new but with a 
 * default username and password.
 *
 * Returns: the newly created #HildonLoginDialog
 */
GtkWidget*
hildon_login_dialog_new_with_default            (GtkWindow *parent,
                                                 const gchar *name,
                                                 const gchar *password)
{
    GtkWidget *self = hildon_login_dialog_new(parent);

    if (name != NULL)
        g_object_set (G_OBJECT (self), "username", name, NULL);

    if (password != NULL)
        g_object_set (G_OBJECT (self), "password", password, NULL);

    return self;
}

/**
 * hildon_login_dialog_get_username:
 * @dialog: the dialog
 *
 * Gets the text that's in the username entry.
 *
 * Returns: a pointer to the name string. You should not modify it.
 */
const gchar*
hildon_login_dialog_get_username                (HildonLoginDialog *dialog)
{
    HildonLoginDialogPrivate *priv;

    g_return_val_if_fail (HILDON_IS_LOGIN_DIALOG (dialog), NULL);

    priv = HILDON_LOGIN_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    return gtk_entry_get_text (GTK_ENTRY (priv->username_entry));
}

/**
 * hildon_login_dialog_get_password:
 * @dialog: the dialog
 * 
 * Gets the text that's in the password entry.
 * 
 * Returns: a pointer to the password string. You should not modify it.
 */
const gchar*
hildon_login_dialog_get_password                (HildonLoginDialog *dialog)
{
    HildonLoginDialogPrivate *priv;

    g_return_val_if_fail (HILDON_IS_LOGIN_DIALOG (dialog), NULL);

    priv = HILDON_LOGIN_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    return gtk_entry_get_text (GTK_ENTRY (priv->password_entry));
}

/**
 * hildon_login_dialog_set_message:
 * @dialog: the dialog
 * @msg: the message or some other descriptive text to be set
 * 
 * Sets the optional descriptive text that is displayed on the top 
 * of the dialog. 
 */
void 
hildon_login_dialog_set_message                 (HildonLoginDialog *dialog, 
                                                 const gchar *msg)
{
    HildonLoginDialogPrivate *priv;

    g_return_if_fail (HILDON_IS_LOGIN_DIALOG (dialog));

    priv = HILDON_LOGIN_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    gtk_label_set_text (priv->message_label, msg);
}


