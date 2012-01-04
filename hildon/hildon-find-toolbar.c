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
 * SECTION:hildon-find-toolbar
 * @short_description: A toolbar with a search field.
 * @see_also: #HildonWindow
 *
 * HildonFindToolbar is a toolbar that contains a search entry and a dropdown
 * list with previously searched strings. The list is represented using a 
 * #GtkListStore and can be accesed using a property 'list'. Entries are added
 * automatically to the list when the search button is pressed.
 *
 */

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <string.h>
#include                                        <libintl.h>
#include                                        <gdk/gdkkeysyms.h>

#include                                        "hildon-find-toolbar.h"
#include                                        "hildon-defines.h"
#include                                        "hildon-find-toolbar-private.h"
#include                                        "hildon-marshalers.h"

#define                                         _(String) \
                                                dgettext("hildon-libs", String)

/* Same define as gtkentry.c as entry will further handle this */

#define                                         MAX_SIZE G_MAXUSHORT

#define                                         FIND_LABEL_XPADDING 6

#define                                         FIND_LABEL_YPADDING 0

static GtkTreeModel*
hildon_find_toolbar_get_list_model              (HildonFindToolbarPrivate *priv);

static GtkEntry*
hildon_find_toolbar_get_entry                   (HildonFindToolbarPrivate *priv);

static gboolean
hildon_find_toolbar_filter                      (GtkTreeModel *model,
                                                 GtkTreeIter *iter,
                                                 gpointer self);

static void
hildon_find_toolbar_apply_filter                (HildonFindToolbar *self,  
                                                 GtkTreeModel *model);

static void
hildon_find_toolbar_get_property                (GObject *object,
                                                 guint prop_id,
                                                 GValue *value,
                                                 GParamSpec *pspec);

static void
hildon_find_toolbar_set_property                (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec);

static gboolean
hildon_find_toolbar_find_string                 (HildonFindToolbar *self,
                                                 GtkTreeIter *iter,
                                                 gint column,
                                                 const gchar *string);

static gboolean
hildon_find_toolbar_history_append              (HildonFindToolbar *self,
                                                 gpointer data);

static void
hildon_find_toolbar_emit_close                  (GtkButton *button, 
                                                 gpointer self);

static void
hildon_find_toolbar_entry_activate              (GtkWidget *widget,
                                                 gpointer user_data);

static void
hildon_find_toolbar_class_init                  (HildonFindToolbarClass *klass);

static void
hildon_find_toolbar_init                        (HildonFindToolbar *self);

enum
{
    SEARCH = 0,
    CLOSE,
    INVALID_INPUT,
    HISTORY_APPEND,

    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_LABEL = 1,
    PROP_PREFIX,
    PROP_LIST,
    PROP_COLUMN,
    PROP_MAX,
    PROP_HISTORY_LIMIT
};

static guint                                    HildonFindToolbar_signal [LAST_SIGNAL] = {0};

/**
 * hildon_find_toolbar_get_type:
 *
 * Initializes and returns the type of a #HildonFindToolbar.
 *
 * Returns: GType of #HildonFindToolbar
 */
GType G_GNUC_CONST
hildon_find_toolbar_get_type                    (void)
{
    static GType find_toolbar_type = 0;

    if (! find_toolbar_type) {
        static const GTypeInfo find_toolbar_info = {
            sizeof(HildonFindToolbarClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) hildon_find_toolbar_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof(HildonFindToolbar),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_find_toolbar_init,
        };
        find_toolbar_type = g_type_register_static (GTK_TYPE_TOOLBAR,
                "HildonFindToolbar",
                &find_toolbar_info, 0);
    }

    return find_toolbar_type;
}

static GtkTreeModel*
hildon_find_toolbar_get_list_model              (HildonFindToolbarPrivate *priv)
{
    GtkTreeModel *filter_model =
        gtk_combo_box_get_model (GTK_COMBO_BOX (priv->entry_combo_box));

    return filter_model == NULL ? NULL :
        gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (filter_model));
}

static GtkEntry*
hildon_find_toolbar_get_entry                   (HildonFindToolbarPrivate *priv)
{
    return GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->entry_combo_box)));
}

static gboolean
hildon_find_toolbar_filter                      (GtkTreeModel *model,
                                                 GtkTreeIter *iter,
                                                 gpointer self)
{
    GtkTreePath *path;
    const gint *indices;
    gint n;
    gint limit;
    gint total;

    total = gtk_tree_model_iter_n_children (model, NULL);
    g_object_get (self, "history_limit", &limit, NULL);
    path = gtk_tree_model_get_path (model, iter);
    indices = gtk_tree_path_get_indices (path);

    /* set the row's index, list store has only one level */
    n = indices [0];
    gtk_tree_path_free (path);

    /*if the row is among the latest "history_limit" additions of the 
     * model, then we show it */
    if( (total - limit <= n) && (n < total) )
        return TRUE;
    else
        return FALSE;
}

static void
hildon_find_toolbar_apply_filter                (HildonFindToolbar *self,  
                                                 GtkTreeModel *model)
{
    GtkTreeModel *filter;
    HildonFindToolbarPrivate *priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (self);
    g_assert (priv);

    /* Create a filter for the given model. Its only purpose is to hide
       the oldest entries so only "history_limit" entries are visible. */
    filter = gtk_tree_model_filter_new (model, NULL);

    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER(filter), 
            hildon_find_toolbar_filter,
            self, NULL);

    gtk_combo_box_set_model (GTK_COMBO_BOX (priv->entry_combo_box), filter);

    /* ComboBox keeps the only needed reference to the filter */
    g_object_unref (filter);
}

static void
hildon_find_toolbar_get_property                (GObject *object,
                                                 guint prop_id,
                                                 GValue *value,
                                                 GParamSpec *pspec)
{
    HildonFindToolbarPrivate *priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (object);
    g_assert (priv);

    const gchar *string;
    gint c_n, max_len;

    switch (prop_id)
    {
        case PROP_LABEL:
            string = gtk_label_get_text (GTK_LABEL (priv->label));
            g_value_set_string (value, string);
            break;

        case PROP_PREFIX:
            string = gtk_entry_get_text (hildon_find_toolbar_get_entry(priv));
            g_value_set_string (value, string);
            break;

        case PROP_LIST:
            g_value_set_object (value, hildon_find_toolbar_get_list_model(priv));
            break;

        case PROP_COLUMN:
            c_n = gtk_combo_box_entry_get_text_column (priv->entry_combo_box);
            g_value_set_int (value, c_n);
            break;

        case PROP_MAX:
            max_len = gtk_entry_get_max_length (hildon_find_toolbar_get_entry(priv));
            g_value_set_int (value, max_len);
            break;

        case PROP_HISTORY_LIMIT:
            g_value_set_int (value, priv->history_limit);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_find_toolbar_set_property                (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec)
{
    HildonFindToolbar *self = HILDON_FIND_TOOLBAR(object);
    HildonFindToolbarPrivate *priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (object);
    g_assert (priv);

    GtkTreeModel *model;
    const gchar *string;

    switch (prop_id)
    {
        case PROP_LABEL:
            string = g_value_get_string (value);   
            gtk_label_set_text (GTK_LABEL (priv->label), string);
            break;

        case PROP_PREFIX:
            string = g_value_get_string (value);
            gtk_entry_set_text (hildon_find_toolbar_get_entry(priv), string);
            break;

        case PROP_LIST:
            model = GTK_TREE_MODEL (g_value_get_object(value));
            hildon_find_toolbar_apply_filter (self, model);
            break;

        case PROP_COLUMN:
            gtk_combo_box_entry_set_text_column (priv->entry_combo_box,
                    g_value_get_int (value));
            break;

        case PROP_MAX:
            gtk_entry_set_max_length (hildon_find_toolbar_get_entry(priv),
                    g_value_get_int (value));
            break;

        case PROP_HISTORY_LIMIT:
            priv->history_limit = g_value_get_int (value);

            /* Re-apply the history limit to the model. */
            model = hildon_find_toolbar_get_list_model (priv);
            if (model != NULL)
            {
                /* Note that refilter function doesn't update the status of the
                   combobox popup arrow, so we'll just recreate the filter. */
                hildon_find_toolbar_apply_filter (self, model);

                if (gtk_combo_box_entry_get_text_column (priv->entry_combo_box) == -1)
                {
                    /* FIXME: This is only for backwards compatibility, although
                       probably nothing actually relies on it. The behavior was only
                       an accidental side effect of original code */
                    gtk_combo_box_entry_set_text_column (priv->entry_combo_box, 0);
                }
            }
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static gboolean
hildon_find_toolbar_find_string                 (HildonFindToolbar *self,
                                                 GtkTreeIter *iter,
                                                 gint column,
                                                 const gchar *string)
{
    GtkTreeModel *model = NULL;
    gchar *old_string;
    HildonFindToolbarPrivate *priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (self);
    g_assert (priv);

    model = hildon_find_toolbar_get_list_model (priv);

    if (gtk_tree_model_get_iter_first (model, iter))
    {
        do {
            gtk_tree_model_get (model, iter, column, &old_string, -1);
            if (old_string != NULL && strcmp (string, old_string) == 0)
            {
                /* Found it */
                return TRUE;
            }
        } while (gtk_tree_model_iter_next (model, iter));
    }

    return FALSE;
}

static gboolean
hildon_find_toolbar_history_append              (HildonFindToolbar *self,
                                                 gpointer data) 
{
    HildonFindToolbarPrivate *priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (self);
    g_assert (priv);

    gchar *string;
    gint column = 0;
    GtkTreeModel *model = NULL;
    GtkListStore *list = NULL;
    GtkTreeIter iter;
    gboolean self_create = FALSE;

    g_object_get (self, "prefix", &string, NULL);

    if (*string == '\0')
    {
        /* empty prefix, ignore */
        g_free (string);
        return TRUE;
    }


    /* If list store is set, get it */
    model = hildon_find_toolbar_get_list_model(priv);
    if(model != NULL)
    {
        list = GTK_LIST_STORE (model);
        g_object_get(self, "column", &column, NULL);

        if (column < 0)
        {
            /* Column number is -1 if "column" property hasn't been set but
               "list" property is. */
            g_free (string);
            return TRUE;
        }

        /* Latest string is always the first one in list. If the string
           already exists, remove it so there are no duplicates in list. */
        if (hildon_find_toolbar_find_string (self, &iter, column, string))
            gtk_list_store_remove (list, &iter);
    }
    else
    {
        /* No list store set. Create our own. */
        list = gtk_list_store_new (1, G_TYPE_STRING);
        model = GTK_TREE_MODEL (list);
        self_create = TRUE;
    }

    /* Add the string to first in list */
    gtk_list_store_append (list, &iter);
    gtk_list_store_set (list, &iter, column, string, -1);

    if(self_create)
    {
        /* Add the created list to ComboBoxEntry */
        hildon_find_toolbar_apply_filter (self, model);
        /* ComboBoxEntry keeps the only needed reference to this list */
        g_object_unref (list);

        /* Set the column only after ComboBoxEntry's model is set
           in hildon_find_toolbar_apply_filter() */
        g_object_set (self, "column", 0, NULL);
    }
    else
    {
        /* Refilter to get the oldest entry hidden from history */
        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER(
                    gtk_combo_box_get_model (GTK_COMBO_BOX(priv->entry_combo_box))));
    }

    g_free (string);

    return FALSE;
}

static void
hildon_find_toolbar_emit_close                  (GtkButton *button, 
                                                 gpointer self)
{
    /* Clicked close button */
    g_signal_emit (self, HildonFindToolbar_signal [CLOSE], 0);
}

static void
hildon_find_toolbar_entry_activate              (GtkWidget *widget,
                                                 gpointer user_data)
{
    GtkWidget *find_toolbar = GTK_WIDGET (user_data);
    gboolean rb;  

    /* NB#40936 stop focus from moving to next widget */
    g_signal_stop_emission_by_name (widget, "activate");

    g_signal_emit (find_toolbar, HildonFindToolbar_signal [SEARCH], 0);
    g_signal_emit (find_toolbar, HildonFindToolbar_signal [HISTORY_APPEND], 0, &rb);
}

static void
hildon_find_toolbar_class_init                  (HildonFindToolbarClass *klass)
{
    GObjectClass *object_class;

    g_type_class_add_private (klass, sizeof (HildonFindToolbarPrivate));

    object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = hildon_find_toolbar_get_property;
    object_class->set_property = hildon_find_toolbar_set_property;

    klass->history_append = (gpointer) hildon_find_toolbar_history_append;

    /**
     * HildonFindToolbar:label:
     *
     * The label to display before the search box.
     *                      
     */
    g_object_class_install_property (object_class, PROP_LABEL, 
            g_param_spec_string ("label", 
                "Label", "Displayed name for"
                " find-toolbar",
                _("ecdg_ti_find_toolbar_label"),
                G_PARAM_READWRITE |
                G_PARAM_CONSTRUCT));

    /**
     * HildonFindToolbar:prefix:
     *
     * The search string.
     *                      
     */
    g_object_class_install_property (object_class, PROP_PREFIX, 
            g_param_spec_string ("prefix", 
                "Prefix", "Search string", NULL,
                G_PARAM_READWRITE));

    /**
     * HildonFindToolbar:list:
     *
     * A #GtkListStore where the search history is kept.
     *                      
     */
    g_object_class_install_property (object_class, PROP_LIST,
            g_param_spec_object ("list",
                "List"," GtkListStore model where "
                "history list is kept",
                GTK_TYPE_LIST_STORE,
                G_PARAM_READWRITE));

    /**
     * HildonFindToolbar:column:
     *
     * The column number in GtkListStore where strings of
     * search history are kept.
     *                      
     */
    g_object_class_install_property(object_class, PROP_COLUMN,
            g_param_spec_int ("column",
                "Column", "Column number in GtkListStore "
                "where history list strings are kept",
                0, G_MAXINT,
                0, G_PARAM_READWRITE));

    /**
     * HildonFindToolbar:max-characters:
     *
     * Maximum number of characters in search string.
     *                      
     */
    g_object_class_install_property (object_class, PROP_MAX,
            g_param_spec_int ("max_characters",
                "Maximum number of characters",
                "Maximum number of characters "
                "in search string",
                0, MAX_SIZE,
                0, G_PARAM_READWRITE |
                G_PARAM_CONSTRUCT));

    /**
     * HildonFindToolbar:history-limit:
     *
     * Maximum number of history items in the combobox.
     *                      
     */
    g_object_class_install_property (object_class, PROP_HISTORY_LIMIT,
            g_param_spec_int ("history-limit",
                "Maximum number of history items",
                "Maximum number of history items "
                "in search combobox",
                0, G_MAXINT,
                5, G_PARAM_READWRITE |
                G_PARAM_CONSTRUCT));

    /**
     * HildonFindToolbar::search:
     * @toolbar: the toolbar which received the signal
     * 
     * Gets emitted when the find button is pressed.
     */ 
    HildonFindToolbar_signal[SEARCH] = 
        g_signal_new(
                "search", HILDON_TYPE_FIND_TOOLBAR,
                G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET 
                (HildonFindToolbarClass, search),
                NULL, NULL, g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE, 0);

    /**
     * HildonFindToolbar::close:
     * @toolbar: the toolbar which received the signal
     * 
     * Gets emitted when the close button is pressed.
     */ 
    HildonFindToolbar_signal[CLOSE] = 
        g_signal_new(
                "close", HILDON_TYPE_FIND_TOOLBAR,
                G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET 
                (HildonFindToolbarClass, close),
                NULL, NULL, g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE, 0);

    /**
     * HildonFindToolbar::invalid-input:
     * @toolbar: the toolbar which received the signal
     * 
     * Gets emitted when the maximum search prefix length is reached and
     * user tries to type more.
     */
    HildonFindToolbar_signal[INVALID_INPUT] = 
        g_signal_new(
                "invalid_input", HILDON_TYPE_FIND_TOOLBAR,
                G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET 
                (HildonFindToolbarClass, invalid_input),
                NULL, NULL, g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE, 0);

    /**
     * HildonFindToolbar::history-append:
     * @toolbar: the toolbar which received the signal
     * 
     * Gets emitted when the current search prefix should be added to history.
     */ 
    HildonFindToolbar_signal[HISTORY_APPEND] = 
        g_signal_new(
                "history_append", HILDON_TYPE_FIND_TOOLBAR,
                G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET 
                (HildonFindToolbarClass, history_append),
                g_signal_accumulator_true_handled, NULL, 
                _hildon_marshal_BOOLEAN__VOID,
                G_TYPE_BOOLEAN, 0);
}

static void
hildon_find_toolbar_init                        (HildonFindToolbar *self)
{
    GtkToolItem *label_container;
    GtkToolItem *entry_combo_box_container;
    GtkAlignment *alignment;

    HildonFindToolbarPrivate *priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (self);
    g_assert (priv);

    /* Create the label */
    priv->label = gtk_label_new (_("ecdg_ti_find_toolbar_label"));

    gtk_misc_set_padding (GTK_MISC (priv->label), FIND_LABEL_XPADDING,
            FIND_LABEL_YPADDING);

    label_container = gtk_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (label_container), 
            priv->label);

    gtk_widget_show_all (GTK_WIDGET (label_container));
    gtk_toolbar_insert (GTK_TOOLBAR (self), label_container, -1);

    /* ComboBoxEntry for search prefix string / history list */
    priv->entry_combo_box = GTK_COMBO_BOX_ENTRY (gtk_combo_box_entry_new ());

    entry_combo_box_container = gtk_tool_item_new ();
    alignment = GTK_ALIGNMENT (gtk_alignment_new (0, 0.5, 1, 0));

    gtk_tool_item_set_expand (entry_combo_box_container, TRUE);
    gtk_container_add (GTK_CONTAINER (alignment),
            GTK_WIDGET (priv->entry_combo_box));
    gtk_container_add (GTK_CONTAINER (entry_combo_box_container),
            GTK_WIDGET (alignment));
    gtk_widget_show_all(GTK_WIDGET (entry_combo_box_container));
    gtk_toolbar_insert (GTK_TOOLBAR (self), entry_combo_box_container, -1);
    g_signal_connect (hildon_find_toolbar_get_entry (priv),
            "activate",
            G_CALLBACK(hildon_find_toolbar_entry_activate), self);

    /* Separator */
    priv->separator = gtk_separator_tool_item_new();
    gtk_widget_set_size_request (GTK_WIDGET (priv->separator), 72, -1);
    gtk_widget_show(GTK_WIDGET(priv->separator));
    gtk_toolbar_insert (GTK_TOOLBAR(self), priv->separator, -1);

    /* Close button */
    priv->close_button = gtk_tool_button_new (
            gtk_image_new_from_icon_name ("general_close",
                HILDON_ICON_PIXEL_SIZE_FINGER),
            "Close");
    g_signal_connect(priv->close_button, "clicked",
            G_CALLBACK(hildon_find_toolbar_emit_close), self);
    gtk_widget_show_all(GTK_WIDGET(priv->close_button));
    gtk_toolbar_insert (GTK_TOOLBAR(self), priv->close_button, -1);
    if ( GTK_WIDGET_CAN_FOCUS( GTK_BIN(priv->close_button)->child) )
        GTK_WIDGET_UNSET_FLAGS(
                GTK_BIN(priv->close_button)->child, GTK_CAN_FOCUS);
}

/**
 * hildon_find_toolbar_new:
 * @label: label for the #HildonFindToolbar, %NULL to set the label to
 *         default "Find".
 * 
 * Creates a new #HildonFindToolbar.
 *
 * Returns: a new #HildonFindToolbar.
 */
GtkWidget*
hildon_find_toolbar_new                         (const gchar *label)
{
    GtkWidget *findtoolbar;

    findtoolbar = GTK_WIDGET (g_object_new (HILDON_TYPE_FIND_TOOLBAR, NULL));

    if (label != NULL)
        g_object_set(findtoolbar, "label", label, NULL);

    return findtoolbar;
}

/**
 * hildon_find_toolbar_new_with_model:
 * @label: label for the #HildonFindToolbar, %NULL to set the label to
 *         default "Find".
 * @model: a #GtkListStore.
 * @column: indicates which column the search history list will
 *          retrieve the string from.
 * 
 * Creates a new #HildonFindToolbar with an initial #GtkTreeModel.
 *
 * Returns: a new #HildonFindToolbar.
 */
GtkWidget*
hildon_find_toolbar_new_with_model              (const gchar *label,
                                                 GtkListStore *model,
                                                 gint column)
{
    GtkWidget *findtoolbar;

    findtoolbar = hildon_find_toolbar_new (label);

    g_object_set (findtoolbar, "list", model, "column", column, NULL);

    return findtoolbar;
}

/**
 * hildon_find_toolbar_highlight_entry:
 * @ftb: #HildonFindToolbar whose entry is to be highlighted.
 * @get_focus: if user passes %TRUE to this value, then the text in
 * the entry will not only get highlighted, but also get focused.
 *
 * Highlights the current entry in the #HildonFindToolbar.
 * 
 */
void
hildon_find_toolbar_highlight_entry             (HildonFindToolbar *self,
                                                 gboolean get_focus)
{
    GtkEntry *entry = NULL;
    HildonFindToolbarPrivate *priv;

    g_return_if_fail (HILDON_IS_FIND_TOOLBAR (self));
    priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (self);
    g_assert (priv);

    entry = hildon_find_toolbar_get_entry (priv);

    gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);

    if (get_focus)
        gtk_widget_grab_focus (GTK_WIDGET (entry));
}

/**
 * hildon_find_toolbar_set_active:
 * @toolbar: A #HildonFindToolbar to operate on.
 * @index: An index in the model, or -1 to have no active item.
 *
 * Sets the active item on the #HildonFindToolbar's #GtkComboBox. Simply calls
 * gtk_combo_box_set_active() on the #HildonFindToolbar's #GtkComboBox.
 * 
 */
void
hildon_find_toolbar_set_active                  (HildonFindToolbar *toolbar,
                                                 gint index)
{
    HildonFindToolbarPrivate *priv;

    g_return_if_fail (HILDON_IS_FIND_TOOLBAR (toolbar));
    priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (toolbar);

    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->entry_combo_box), index);
}

/**
 * hildon_find_toolbar_get_active:
 * @toolbar: A #HildonFindToolbar to query.
 *
 * Gets the index of the currently active item, or -1 if there's no active item.
 * Simply calls gtk_combo_box_get_active() on the #HildonFindToolbar's
 * #GtkComboBox.
 *
 * Returns: the index of the currently active item, or -1 if there is no active
 * item.
 * 
 */
gint
hildon_find_toolbar_get_active                  (HildonFindToolbar *toolbar)
{
    HildonFindToolbarPrivate *priv;

    g_return_val_if_fail (HILDON_IS_FIND_TOOLBAR (toolbar), -1);
    priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (toolbar);

    return gtk_combo_box_get_active (GTK_COMBO_BOX (priv->entry_combo_box));
}

/**
 * hildon_find_toolbar_set_active_iter:
 * @toolbar: A #HildonFindToolbar to operate on.
 * @iter: A #GtkTreeIter to activate.
 *
 * Sets the current active item to be the one referenced by @iter. Simply calls
 * gtk_combo_box_set_active_iter() on the #HildonFindToolbar's #GtkComboBox.
 * 
 */
void
hildon_find_toolbar_set_active_iter             (HildonFindToolbar *toolbar, 
                                                 GtkTreeIter *iter)
{
    HildonFindToolbarPrivate *priv;

    g_return_if_fail (HILDON_IS_FIND_TOOLBAR (toolbar));
    priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (toolbar);

    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->entry_combo_box), iter);
}

/**
 * hildon_find_toolbar_get_active_iter:
 * @toolbar: A #HildonFindToolbar to query.
 * @iter: The uninitialized #GtkTreeIter.
 *
 * Sets @iter to point to the current active item, if it exists. Simply calls
 * gtk_combo_box_get_active_iter() on the #HildonFindToolbar's #GtkComboBox.
 * 
 * Returns: %TRUE, if @iter was set.
 *
 */
gboolean
hildon_find_toolbar_get_active_iter             (HildonFindToolbar *toolbar, 
                                                 GtkTreeIter *iter)
{
    HildonFindToolbarPrivate *priv;

    g_return_val_if_fail (HILDON_IS_FIND_TOOLBAR (toolbar), FALSE);
    priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (toolbar);

    return gtk_combo_box_get_active_iter (GTK_COMBO_BOX (priv->entry_combo_box), iter);
}

/**
 * hildon_find_toolbar_get_last_index
 * @toolbar: A #HildonFindToolbar to query.
 *
 * Returns the index of the last (most recently added) item in the
 * #HildonFindToolbar. Can be used to set this item active in the
 * #HildonFindToolbar::history-append signal.
 *
 * 
 * Returns: Index of the most recent entry.
 *
 */
gint32
hildon_find_toolbar_get_last_index              (HildonFindToolbar *toolbar)
{
    HildonFindToolbarPrivate *priv;
    GtkTreeModel *filter_model;
    
    g_return_val_if_fail (HILDON_IS_FIND_TOOLBAR (toolbar), FALSE);
    priv = HILDON_FIND_TOOLBAR_GET_PRIVATE (toolbar);

    filter_model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->entry_combo_box));

    if (filter_model == NULL)
        return 0;

    gint i = 0;
    GtkTreeIter iter;

    gtk_tree_model_get_iter_first (filter_model, &iter);

    while (gtk_tree_model_iter_next (filter_model, &iter))
        i++;

    return i;
}

