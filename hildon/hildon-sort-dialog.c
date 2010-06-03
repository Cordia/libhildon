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
 * SECTION:hildon-sort-dialog
 * @short_description: A widget for defining the sorting order of items.
 * 
 * HildonSortDialog is used to define an order (ascending/descending)
 * and a field by which items are sorted in a list. The combo boxes
 * display the current value when the dialog is opened.
 *
 * <note>
 *   <para>
 * #HildonSortDialog has been deprecated since Hildon 2.2
 * See <link linkend="hildon-migrating-sort-dialogs">Migrating Sort Dialogs</link>
 * section to know how to migrate this deprecated widget.
 *   </para>
 * </note>
 *
 * <example>
 * <title>An example for using HildonSortDialog</title>
 * <programlisting>
 * HildonSortDialog *sort_dialog = HILDON_SORT_DIALOG (hildon_sort_dialog_new (parent));
 * <!-- -->
 * gint response_id, add_sort_index;
 * <!-- -->
 * sort_by[0] = STR_SORT_BY_DATE;
 * sort_by[1] = STR_SORT_BY_NAME;
 * sort_by[2] = STR_SORT_BY_SIZE;
 * sort_by[3] = NULL;
 * <!-- -->
 * sorting_order[0] = STR_SORTING_ORDER_ASCENDING;
 * sorting_order[1] = STR_SORTING_ORDER_DESCENDING;
 * sorting_order[2] = NULL;
 * <!-- -->
 * add_sort_index = hildon_sort_dialog_add_sort_key (sort_dialog, STR_SORT_BY_DATE);
 * <!-- -->
 * hildon_sort_dialog_add_sort_key (sort_dialog, STR_SORT_BY_NAME);
 * <!-- -->
 * hildon_sort_dialog_add_sort_key (sort_dialog, STR_SORT_BY_SIZE);
 * <!-- -->
 * if (dialog.first_time_clicked == TRUE)
 * {
 * 	hildon_sort_dialog_set_sort_key (sort_dialog, add_sort_index);
 * }
 * <!-- -->
 * if (dialog.first_time_clicked == FALSE)
 * {
 * 	hildon_sort_dialog_set_sort_key (sort_dialog, dialog.sort_key);
 * 	hildon_sort_dialog_set_sort_order (sort_dialog, dialog.sort_order);
 * }
 * <!-- -->
 * gtk_widget_show (GTK_WIDGET (sort_dialog));
 * <!-- -->
 * response_id = gtk_dialog_run (GTK_DIALOG (sort_dialog));
 * <!-- -->
 * if (response_id == GTK_RESPONSE_OK)
 * {
 * 	dialog.sort_key = hildon_sort_dialog_get_sort_key (sort_dialog);
 * <!-- -->
 * 	gtk_label_set_text (GTK_LABEL (dialog.label1), sort_by [dialog.sort_key]);
 * <!-- -->
 * 	dialog.sort_order = hildon_sort_dialog_get_sort_order (sort_dialog);
 * <!-- -->
 * 	gtk_label_set_text (GTK_LABEL (dialog.label2), sorting_order [dialog.sort_order]);
 * <!-- -->
 * 	dialog.first_time_clicked = FALSE;
 * }
 * </programlisting>
 * </example>
 */

#undef                                          HILDON_DISABLE_DEPRECATED

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <stdio.h>
#include                                        <libintl.h>

#include                                        "hildon-sort-dialog.h"
#include                                        "hildon-caption.h"
#include                                        "hildon-sort-dialog-private.h"
#include                                        "hildon-stock.h"

#define                                         _(String) \
                                                dgettext("hildon-libs", String)

static GtkDialogClass*                          parent_class;

static void 
hildon_sort_dialog_class_init                   (HildonSortDialogClass *class);

static void
hildon_sort_dialog_init                         (HildonSortDialog *widget);

static void 
hildon_sort_dialog_set_property                 (GObject * object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec * pspec);

static void 
hildon_sort_dialog_get_property                 (GObject *object,
                                                 guint prop_id,
                                                 GValue * value, 
                                                 GParamSpec * pspec);

static void 
reconstruct_combo                               (HildonSortDialog *dialog, 
                                                 gboolean remove, 
                                                 gboolean reversed);

static void
sort_key_changed                                (GtkWidget *widget, 
                                                 HildonSortDialog *dialog);

static void 
hildon_sort_dialog_finalize                     (GObject *object);

static gint 
hildon_sort_dialog_add_sort_key_with_sorting    (HildonSortDialog *dialog, 
                                                 const gchar *sort_key, 
                                                 gboolean sorting);

enum 
{
    PROP_0,
    PROP_SORT_KEY,
    PROP_SORT_ORDER
};

static void 
sort_key_changed                                (GtkWidget *widget, 
                                                 HildonSortDialog *dialog)
{
    g_return_if_fail (HILDON_IS_SORT_DIALOG (dialog));

    HildonSortDialogPrivate *priv = HILDON_SORT_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    gint index = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    g_return_if_fail (index < priv->index_counter && index >= 0);

    if (priv->key_reversed [index] != priv->reversed) {
        reconstruct_combo (dialog, TRUE, priv->key_reversed [index]);
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_order), 0);
    }

    priv->reversed = priv->key_reversed [index];
}

/* Initialises the sort dialog class. */
static void
hildon_sort_dialog_class_init                   (HildonSortDialogClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    parent_class = g_type_class_peek_parent (class);
    g_type_class_add_private (class, sizeof (HildonSortDialogPrivate));
    
    gobject_class->set_property = hildon_sort_dialog_set_property;
    gobject_class->get_property = hildon_sort_dialog_get_property;
    gobject_class->finalize     = (gpointer) hildon_sort_dialog_finalize;

    /**
     * HildonSortDialog:sort-key:
     *
     * The currently active sort key.
     */
    g_object_class_install_property (gobject_class, PROP_SORT_KEY,
        g_param_spec_int ("sort-key",
                          "Sort Key",
                          "The currently active sort key",
                          G_MININT,
                          G_MAXINT,
                          0, G_PARAM_READWRITE));
   
    /**
     * HildonSortDialog:sort-order:
     *
     * The sort order for the currently active sort key.
     */
    g_object_class_install_property (gobject_class, PROP_SORT_ORDER,
        g_param_spec_enum ("sort-order",
                          "Sort Order",
                          "The current sorting order",
                          GTK_TYPE_SORT_TYPE,
                          GTK_SORT_ASCENDING,
                          G_PARAM_READWRITE));
}

static gint 
hildon_sort_dialog_add_sort_key_with_sorting    (HildonSortDialog *dialog, 
                                                 const gchar *sort_key, 
                                                 gboolean sorting)
{
    HildonSortDialogPrivate *priv;

    g_return_val_if_fail (HILDON_IS_SORT_DIALOG (dialog), -1);
    g_return_val_if_fail (sort_key != NULL, -1);

    priv = HILDON_SORT_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    gboolean *new_array = g_malloc (sizeof (gboolean) * (priv->index_counter + 1));

    /* Rewrite the old values */
    int i = 0;
    for (i = 0; i < priv->index_counter; i++) 
        new_array [i] = priv->key_reversed [i];

    new_array [priv->index_counter] = sorting;
    gtk_combo_box_append_text (GTK_COMBO_BOX (priv->combo_key), sort_key);

    /* Free the old one and reassign */
    if (priv->key_reversed != NULL)
        g_free (priv->key_reversed);
    priv->key_reversed = new_array;

    return priv->index_counter++;
}

static void 
reconstruct_combo                               (HildonSortDialog *dialog, 
                                                 gboolean remove, 
                                                 gboolean reversed)
{
    HildonSortDialogPrivate *priv;
    priv = HILDON_SORT_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    if (remove) {
        gtk_combo_box_remove_text (GTK_COMBO_BOX (priv->combo_order), 1);
        gtk_combo_box_remove_text (GTK_COMBO_BOX (priv->combo_order), 0);
    }

    if (reversed) {
        gtk_combo_box_append_text (GTK_COMBO_BOX (priv->combo_order), _("ckdg_va_sort_descending"));
        gtk_combo_box_append_text (GTK_COMBO_BOX (priv->combo_order), _("ckdg_va_sort_ascending"));
    } else  {
        gtk_combo_box_append_text (GTK_COMBO_BOX (priv->combo_order), _("ckdg_va_sort_ascending"));
        gtk_combo_box_append_text (GTK_COMBO_BOX (priv->combo_order), _("ckdg_va_sort_descending"));
    }
}

static void
hildon_sort_dialog_init                         (HildonSortDialog * dialog)
{
    HildonSortDialogPrivate *priv;
    GtkSizeGroup *group;

    g_assert(HILDON_IS_SORT_DIALOG (dialog));

    priv = HILDON_SORT_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    priv->index_counter = 0;
    priv->reversed = FALSE;
    priv->key_reversed = NULL;

    group = GTK_SIZE_GROUP (gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL));

    gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_window_set_title (GTK_WINDOW (dialog), _("ckdg_ti_sort"));

    /* Tab one */
    priv->combo_key = gtk_combo_box_new_text ();
    priv->caption_key = hildon_caption_new(group, _("ckdg_fi_sort_field"), priv->combo_key,
            NULL, HILDON_CAPTION_OPTIONAL);
    hildon_caption_set_separator(HILDON_CAPTION (priv->caption_key), "");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
            priv->caption_key, FALSE, FALSE, 0);

    /* Tab two */
    priv->combo_order = gtk_combo_box_new_text ();
    reconstruct_combo (dialog, FALSE, FALSE);

    priv->caption_order = hildon_caption_new (group, _("ckdg_fi_sort_order"),
            priv->combo_order,
            NULL, HILDON_CAPTION_OPTIONAL);
    hildon_caption_set_separator(HILDON_CAPTION(priv->caption_order), "");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
            priv->caption_order, FALSE, FALSE, 0);

    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_key), 0);
    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_order), 0);
    g_signal_connect (G_OBJECT (priv->combo_key), "changed", (gpointer) sort_key_changed, dialog);

    /* Create the OK/CANCEL buttons */
    (void) gtk_dialog_add_button (GTK_DIALOG(dialog),
            HILDON_STOCK_SORT, GTK_RESPONSE_OK);
    /* FIXME: Hardcoded sizes are bad */
    gtk_window_resize (GTK_WINDOW (dialog), 370, 100);
    gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);

    g_object_unref (group); /* Captions now own their references to sizegroup */
}

/**
 * hildon_sort_dialog_get_type:
 *
 * Returns GType for HildonSortDialog as produced by 
 * g_type_register_static().
 *
 * Returns: HildonSortDialog type
 */
GType G_GNUC_CONST
hildon_sort_dialog_get_type                     (void)
{
    static GType dialog_type = 0;

    if (!dialog_type) {
        static const GTypeInfo dialog_info = {
            sizeof (HildonSortDialogClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) hildon_sort_dialog_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof (HildonSortDialog),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_sort_dialog_init
        };

        dialog_type = g_type_register_static (GTK_TYPE_DIALOG,
                "HildonSortDialog",
                &dialog_info, 0);
    }
    return dialog_type;
}

/**
 * hildon_sort_dialog_new:
 * @parent: widget to be transient for, or NULL if none
 *
 * HildonSortDialog contains two HildonCaptions with combo boxes. 
 *
 * Returns: pointer to a new @HildonSortDialog widget
 */
GtkWidget*
hildon_sort_dialog_new                          (GtkWindow * parent)
{
    GtkWidget *sort_dialog = g_object_new (HILDON_TYPE_SORT_DIALOG, NULL);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (sort_dialog), parent);

    return sort_dialog;
}

/**
 * hildon_sort_dialog_get_sort_key:
 * @dialog: the #HildonSortDialog widget
 *
 * Gets index to currently active sort key.
 * 
 * Returns: an integer which is the index value of the "Sort by" 
 * field 
 */
gint
hildon_sort_dialog_get_sort_key                 (HildonSortDialog *dialog)
{
    GtkWidget *combo_key;
    HildonSortDialogPrivate *priv;

    g_return_val_if_fail (HILDON_IS_SORT_DIALOG (dialog), -1);

    priv = HILDON_SORT_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    combo_key = gtk_bin_get_child (GTK_BIN (priv->caption_key));

    return gtk_combo_box_get_active (GTK_COMBO_BOX (combo_key));
}

/**
 * hildon_sort_dialog_get_sort_order:
 * @dialog: the #HildonSortDialog widget
 *
 * Gets current sorting order from "Sort order" field.
 *
 * Returns: current sorting order as #GtkSortType
 */
GtkSortType 
hildon_sort_dialog_get_sort_order               (HildonSortDialog *dialog)
{
    GtkWidget *combo_order;
    HildonSortDialogPrivate *priv;

    g_return_val_if_fail (HILDON_IS_SORT_DIALOG (dialog), 0);

    priv = HILDON_SORT_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    combo_order = gtk_bin_get_child (GTK_BIN (priv->caption_order));

    gint sort_order = gtk_combo_box_get_active (GTK_COMBO_BOX (combo_order));

    if (priv->reversed)
        return (sort_order == 0) ? 1 : 0;
    else
        return sort_order;
}

/**
 * hildon_sort_dialog_set_sort_key:
 * @dialog: the #HildonSortDialog widget
 * @key: combo box's index value
 *
 * Sets the index value of the #HildonSortDialog widget.
 */
void
hildon_sort_dialog_set_sort_key                 (HildonSortDialog * dialog, 
                                                 gint key)
{
    GtkWidget *combo_key;
    HildonSortDialogPrivate *priv;

    g_return_if_fail (HILDON_IS_SORT_DIALOG (dialog));

    priv = HILDON_SORT_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    combo_key = gtk_bin_get_child (GTK_BIN (priv->caption_key));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_key), key);

    g_object_notify (G_OBJECT (dialog), "sort-key");
}

/**
 * hildon_sort_dialog_set_sort_order:
 * @dialog: the #HildonSortDialog widget
 * @order: combo box's index value
 *
 * Sets the index value of the #HildonSortDialog widget.
 */
void
hildon_sort_dialog_set_sort_order               (HildonSortDialog *dialog,
                                                 GtkSortType order)
{
    GtkWidget *combo_order;
    HildonSortDialogPrivate *priv;

    g_return_if_fail (HILDON_IS_SORT_DIALOG (dialog));

    priv = HILDON_SORT_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    combo_order = gtk_bin_get_child (GTK_BIN (priv->caption_order));

    if (priv->reversed) 
        order = (order == 0) ? 1 : 0;

    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_order), order);

    g_object_notify (G_OBJECT (dialog), "sort-order");
}

/**
 * hildon_sort_dialog_add_sort_key:
 * @dialog: the #HildonSortDialog widget
 * @sort_key: combo box's index value
 *
 * Adds a new sort key and returns the respective index in
 * sort key combobox.
 *
 * Returns: an integer which is the index of the added combo box's
 * item
 */
gint
hildon_sort_dialog_add_sort_key                 (HildonSortDialog *dialog,
                                                 const gchar *sort_key)
{
    return hildon_sort_dialog_add_sort_key_with_sorting (dialog, sort_key, FALSE);
}

/**
 * hildon_sort_dialog_add_sort_key_reversed:
 * @dialog: the #HildonSortDialog widget
 * @sort_key: combo box's index value
 *
 * Adds a new sort key and returns the respective index in
 * sort key combobox. The default sort order for this key is reversed (Descending first).
 *
 * Returns: an integer which is the index of the added combo box's
 * item
 *
 */
gint
hildon_sort_dialog_add_sort_key_reversed        (HildonSortDialog *dialog,
                                                 const gchar *sort_key)
{
    return hildon_sort_dialog_add_sort_key_with_sorting (dialog, sort_key, TRUE);
}

static void
hildon_sort_dialog_set_property                 (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonSortDialog *dialog;

    dialog = HILDON_SORT_DIALOG(object);

    switch (prop_id) {

        case PROP_SORT_KEY:
            hildon_sort_dialog_set_sort_key (dialog, g_value_get_int (value));
            break;

        case PROP_SORT_ORDER:
            hildon_sort_dialog_set_sort_order (dialog, g_value_get_enum (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_sort_dialog_get_property                 (GObject *object,
                                                 guint prop_id, 
                                                 GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonSortDialog *dialog;

    dialog = HILDON_SORT_DIALOG (object);

    switch (prop_id) {

        case PROP_SORT_KEY:
            g_value_set_int (value, hildon_sort_dialog_get_sort_key (dialog));
            break;

        case PROP_SORT_ORDER:
            g_value_set_enum (value, hildon_sort_dialog_get_sort_order (dialog));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;

    }
}

static void 
hildon_sort_dialog_finalize                     (GObject *object)
{
    HildonSortDialogPrivate *priv;
    HildonSortDialog *dialog;

    dialog = HILDON_SORT_DIALOG (object);
    priv = HILDON_SORT_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    if (priv != NULL && priv->key_reversed != NULL)
        g_free (priv->key_reversed);

    if (G_OBJECT_CLASS (parent_class)->finalize)
        G_OBJECT_CLASS (parent_class)->finalize(object);
}


