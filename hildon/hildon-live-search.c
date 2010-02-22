/*
 * This file is a part of hildon
 *
 * Copyright (C) 2007-2009 Nokia Corporation.
 *
 * Based in OssoABookLiveSearch, OSSO Address Book.
 * Author: Joergen Scheibengruber <jorgen.scheibengruber@nokia.com>
 * Hildon version: Claudio Saavedra <csaavedra@igalia.com>
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
 * SECTION:hildon-live-search
 * @short_description: A widget for manipulating #GtkTreeModelFilter
 * instances.
 *
 * This widget provides a user interface for manipulating
 * #GtkTreeModelFilter instances.
 *
 * To set a #GtkTreeFilterModel to filter with, use
 * hildon_live_search_set_filter(). By default, #HildonLiveSearch
 * filters on the child model of the filter model set using a case
 * sensitive prefix comparison on the model's column specified by
 * #HildonLiveSearch:text-column. If a more refined filtering is
 * necessary, you can use hildon_live_search_set_visible_func() to
 * specify a #HildonLiveSearchVisibleFunc to use.
 *
 */

#include                                        "hildon-live-search.h"

#include                                        <hildon/hildon.h>
#include                                        <string.h>

G_DEFINE_TYPE (HildonLiveSearch, hildon_live_search, GTK_TYPE_TOOLBAR);

#define                                         GET_PRIVATE(o)                     \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                                HILDON_TYPE_LIVE_SEARCH,           \
                                                HildonLiveSearchPrivate))

struct _HildonLiveSearchPrivate
{
    GtkTreeModelFilter *filter;

    GtkWidget *kb_focus_widget;

    GtkWidget *entry;
    GtkWidget *event_widget;
    GHashTable *selection_map;

    gulong key_press_id;
    gulong event_widget_destroy_id;
    gulong kb_focus_widget_destroy_id;

    gchar *prefix;
    gint text_column;

    HildonLiveSearchVisibleFunc visible_func;
    gpointer visible_data;
    GDestroyNotify visible_destroy;
    gboolean visible_func_set;
};

enum
{
    PROP_0,

    PROP_FILTER,
    PROP_WIDGET,
    PROP_TEXT_COLUMN
};

enum
{
  REFILTER,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static gboolean
visible_func                                    (GtkTreeModel *model,
                                                 GtkTreeIter  *iter,
                                                 gpointer      data);

/* Private implementation */

static guint
hash_func                                       (gconstpointer key)
{
    GtkTreePath *path;
    gchar *path_str;
    guint val;

    path = (GtkTreePath *) key;
    path_str = gtk_tree_path_to_string (path);
    val = g_str_hash (path_str);
    g_free (path_str);

    return val;
}

static gboolean
key_equal_func                                  (gconstpointer v1,
                                                 gconstpointer v2)
{
    return gtk_tree_path_compare ((GtkTreePath *)v1,
                                  (GtkTreePath *)v2) == 0;
}


static void
refilter (HildonLiveSearch *livesearch)
{
    HildonLiveSearchPrivate *priv = livesearch->priv;
    gboolean handled = FALSE;

    g_signal_emit (livesearch, signals[REFILTER], 0, &handled);
    if (!handled)
        gtk_tree_model_filter_refilter (priv->filter);
}

/**
 * selection_map_create:
 * @priv: The private pimpl
 *
 * Adds a selection map which is useful when merging selected rows in
 * a treeview, when the live search widget is used.
 **/
static void
selection_map_create                            (HildonLiveSearchPrivate *priv)
{
    gboolean walking;
    GtkTreeModel *base_model;
    GtkTreeIter iter_child;
    GtkTreePath *path;
    GtkTreeIter virtual_root;
    GtkTreePath *virtual_root_path;

    g_assert (priv->selection_map == NULL);

    base_model = gtk_tree_model_filter_get_model (priv->filter);

    priv->selection_map = g_hash_table_new_full
        (hash_func, key_equal_func, (GDestroyNotify) gtk_tree_path_free, NULL);

    g_object_get (priv->filter, "virtual-root", &virtual_root_path, NULL);
    gtk_tree_model_get_iter (base_model, &virtual_root, virtual_root_path);
    walking = gtk_tree_model_iter_children (base_model, &iter_child,
        &virtual_root);

    while (walking) {
        path = gtk_tree_model_get_path (base_model, &iter_child);
        g_hash_table_insert (priv->selection_map,
                             path, GINT_TO_POINTER (FALSE));

        walking = gtk_tree_model_iter_next (base_model, &iter_child);
    }
}

/**
 * selection_map_destroy:
 * @priv: The private pimpl
 *
 * Destroys resources associated with the selection map.
 **/
static void
selection_map_destroy                           (HildonLiveSearchPrivate *priv)
{
    if (priv->selection_map != NULL) {
        g_hash_table_destroy (priv->selection_map);
        priv->selection_map = NULL;
    }
}

static GtkTreePath *
convert_child_path_to_path (GtkTreeModel *model,
                            GtkTreeModel *base_model,
                            GtkTreePath *path)
{
  g_return_val_if_fail (model != NULL, NULL);
  g_return_val_if_fail (base_model != NULL, NULL);
  g_return_val_if_fail (path != NULL, NULL);

  if (model == base_model)
    return gtk_tree_path_copy (path);

  g_assert (gtk_tree_model_sort_get_model (model) == base_model);
  g_assert (GTK_IS_TREE_MODEL_SORT (model));

  return gtk_tree_model_sort_convert_child_path_to_path
      (GTK_TREE_MODEL_SORT (model), path);
}

/**
 * selection_map_update_map_from_selection:
 * @priv: The private pimpl
 *
 * Find out which rows are visible in filter, and mark them as selected
 * or unselected from treeview to selection map.
 **/
static void
selection_map_update_map_from_selection         (HildonLiveSearchPrivate *priv)
{
    gboolean walking;
    GtkTreeModel *base_model;
    GtkTreeSelection *selection;
    GtkTreeIter iter_child;
    GtkTreeIter virtual_root;
    GtkTreePath *virtual_root_path;

    base_model = gtk_tree_model_filter_get_model (priv->filter);
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->kb_focus_widget));

    g_object_get (priv->filter, "virtual-root", &virtual_root_path, NULL);
    gtk_tree_model_get_iter (base_model, &virtual_root, virtual_root_path);
    walking = gtk_tree_model_iter_children (base_model, &iter_child,
        &virtual_root);

    while (walking) {
        GtkTreePath *base_path, *filter_path;
        base_path = gtk_tree_model_get_path (base_model, &iter_child);
        filter_path = gtk_tree_model_filter_convert_child_path_to_path
          (priv->filter, base_path);

        if (filter_path) {
            GtkTreePath *view_path;
            view_path = convert_child_path_to_path (
                gtk_tree_view_get_model (GTK_TREE_VIEW (priv->kb_focus_widget)),
                GTK_TREE_MODEL (priv->filter), filter_path);
            if (gtk_tree_selection_path_is_selected
                (selection, view_path)) {
                g_hash_table_replace
                    (priv->selection_map,
                     base_path,
                     GINT_TO_POINTER (TRUE));
            } else {
                g_hash_table_replace
                    (priv->selection_map,
                     base_path,
                     GINT_TO_POINTER (FALSE));
            }
            gtk_tree_path_free (view_path);
            gtk_tree_path_free (filter_path);
        } else {
            gtk_tree_path_free (base_path);
        }
        walking = gtk_tree_model_iter_next (base_model, &iter_child);
    }
}

/**
 * selection_map_update_selection_from_map:
 * @priv: The private pimpl
 *
 * For currently visible rows in filter, set selection from selection
 * map to treeview.
 **/
static void
selection_map_update_selection_from_map         (HildonLiveSearchPrivate *priv)
{
    gboolean walking;
    GtkTreeModel *base_model;
    GtkTreeSelection *selection;
    GtkTreeIter iter_child;
    GtkTreeIter virtual_root;
    GtkTreePath *virtual_root_path;

    base_model = gtk_tree_model_filter_get_model (priv->filter);
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->kb_focus_widget));

    g_object_get (priv->filter, "virtual-root", &virtual_root_path, NULL);
    gtk_tree_model_get_iter (base_model, &virtual_root, virtual_root_path);
    walking = gtk_tree_model_iter_children (base_model, &iter_child,
        &virtual_root);

    while (walking) {
        GtkTreePath *base_path, *filter_path;
        base_path = gtk_tree_model_get_path (base_model, &iter_child);
        filter_path = gtk_tree_model_filter_convert_child_path_to_path
          (priv->filter, base_path);

        if (filter_path) {
            gboolean selected;
            GtkTreePath *view_path;
            view_path = convert_child_path_to_path (
                gtk_tree_view_get_model (GTK_TREE_VIEW (priv->kb_focus_widget)),
                GTK_TREE_MODEL (priv->filter), filter_path);

            selected = GPOINTER_TO_INT
                (g_hash_table_lookup
                 (priv->selection_map, base_path));

            if (selected) {
                gtk_tree_selection_select_path
                    (selection, view_path);
            } else {
                gtk_tree_selection_unselect_path
                    (selection, view_path);
            }
            gtk_tree_path_free (view_path);
            gtk_tree_path_free (filter_path);
        }
        gtk_tree_path_free (base_path);
        walking = gtk_tree_model_iter_next (base_model, &iter_child);
    }
}

static void
on_entry_changed                                (GtkEntry *entry,
                                                 gpointer  user_data)
{
    HildonLiveSearch *livesearch = user_data;
    HildonLiveSearchPrivate *priv;
    const char *text;
    glong len;
    char *old_prefix;

    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));
    priv = livesearch->priv;

    text = gtk_entry_get_text (GTK_ENTRY (entry));
    len = g_utf8_strlen (text, -1);

    old_prefix = priv->prefix;

    if (len < 1) {
        text = NULL;
    } else {
        if (((old_prefix == NULL) || (strlen (old_prefix) == 0)) &&
            GTK_IS_TREE_VIEW (priv->kb_focus_widget)) {
            selection_map_create (priv);
        }
    }

    if (GTK_IS_TREE_VIEW (priv->kb_focus_widget))
        selection_map_update_map_from_selection (priv);

    priv->prefix = g_strdup (text);
    refilter (livesearch);

    if (GTK_IS_TREE_VIEW (priv->kb_focus_widget))
        selection_map_update_selection_from_map (priv);

    if (len < 1) {
        if (GTK_IS_TREE_VIEW (priv->kb_focus_widget))
            selection_map_destroy (priv);
        gtk_widget_hide (GTK_WIDGET (livesearch));
    } else {
        gtk_widget_show (GTK_WIDGET (livesearch));
    }

    g_free (old_prefix);
}

/**
 * hildon_live_search_append_text:
 * @livesearch: An #HildonLiveSearch widget
 * @text: Text to append. @text is copied internally
 * can be freed later by the caller.
 *
 * Appends a string to the entry text in the live search widget.
 *
 * Since: 2.2.4
 **/
void
hildon_live_search_append_text                  (HildonLiveSearch *livesearch,
                                                 const char       *text)
{
    GtkEditable *editable;
    int pos, start, end;

    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));
    g_return_if_fail (NULL != text);

    editable = GTK_EDITABLE (livesearch->priv->entry);

    if (gtk_editable_get_selection_bounds (editable, &start, &end)) {
        gtk_editable_delete_text (editable, start, end);
    }
    pos = gtk_editable_get_position (editable);
    gtk_editable_insert_text (editable, text, strlen (text), &pos);
    gtk_editable_set_position (editable, pos + 1);
}

/**
 * hildon_live_search_get_text:
 * @livesearch: An #HildonLiveSearch widget
 *
 * Retrieves the text contents of the #HildonLiveSearch widget.
 *
 * Returns: a pointer to the text contents of the widget as a
 * string. This string should not be freed, modified, or stored.
 *
 * Since: 2.2.4
 **/
const char *
hildon_live_search_get_text                     (HildonLiveSearch *livesearch)
{
    g_return_val_if_fail (HILDON_IS_LIVE_SEARCH (livesearch), NULL);

    return gtk_entry_get_text (GTK_ENTRY (livesearch->priv->entry));
}

/*
 * Key press handler. This takes key presses from the source widget and
 * manipulate the entry. This manipulation then calls #on_entry_changed.
 */
static gboolean
on_key_press_event                              (GtkWidget        *widget,
                                                 GdkEventKey      *event,
                                                 HildonLiveSearch *live_search)
{
    HildonLiveSearchPrivate *priv;
    gboolean handled = FALSE;

    g_return_val_if_fail (HILDON_IS_LIVE_SEARCH (live_search), FALSE);
    priv = live_search->priv;

    if (GTK_WIDGET_VISIBLE (priv->kb_focus_widget)) {
        /* If the live search is hidden, Ctrl+whatever is always
         * passed to the focus widget */
        if (GTK_WIDGET_VISIBLE (live_search) ||
            !(event->state & GDK_CONTROL_MASK ||
              event->keyval == GDK_Control_L ||
              event->keyval == GDK_Control_R)) {
            GdkEvent *new_event;

            /* If the entry is realized and has focus, it is enough to catch events.
             * This assumes that the toolbar is a child of the hook widget. */
            gtk_widget_realize (priv->entry);
            if (!GTK_WIDGET_HAS_FOCUS (priv->entry))
                gtk_widget_grab_focus (priv->entry);

            new_event = gdk_event_copy ((GdkEvent *)event);
            handled = gtk_widget_event (priv->entry, new_event);
            gdk_event_free (new_event);
        } else if (!GTK_WIDGET_HAS_FOCUS (priv->kb_focus_widget)) {
            gtk_widget_grab_focus (GTK_WIDGET (priv->kb_focus_widget));
        }
    }

    return handled;
}

static void
on_hide_cb                                      (GtkWidget        *widget,
                                                 HildonLiveSearch *live_search)
{
    gtk_entry_set_text (GTK_ENTRY (live_search->priv->entry), "");
    gtk_widget_grab_focus (live_search->priv->kb_focus_widget);
}

/* GObject methods */

static void
hildon_live_search_get_property                 (GObject    *object,
                                                 guint       property_id,
                                                 GValue     *value,
                                                 GParamSpec *pspec)
{
    HildonLiveSearch *livesearch = HILDON_LIVE_SEARCH (object);

    switch (property_id) {
    case PROP_FILTER:
        g_value_set_object (value, livesearch->priv->filter);
        break;
    case PROP_TEXT_COLUMN:
        g_value_set_int (value, livesearch->priv->text_column);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
hildon_live_search_set_property                 (GObject      *object,
                                                 guint         property_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
    HildonLiveSearch *livesearch = HILDON_LIVE_SEARCH (object);

    switch (property_id) {
    case PROP_FILTER:
        hildon_live_search_set_filter (livesearch,
                                       g_value_get_object (value));
        break;
    case PROP_TEXT_COLUMN:
        hildon_live_search_set_text_column (livesearch,
                                            g_value_get_int (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
hildon_live_search_dispose                      (GObject *object)
{
    HildonLiveSearchPrivate *priv = HILDON_LIVE_SEARCH (object)->priv;

    hildon_live_search_widget_unhook (HILDON_LIVE_SEARCH (object));

    if (priv->filter) {
        selection_map_destroy (priv);
        g_object_unref (priv->filter);
        priv->filter = NULL;
    }

    if (priv->prefix) {
        g_free (priv->prefix);
        priv->prefix = NULL;
    }

    if (priv->visible_destroy) {
        priv->visible_destroy (priv->visible_data);
        priv->visible_destroy = NULL;
    }

    G_OBJECT_CLASS (hildon_live_search_parent_class)->dispose (object);
}

static void
hildon_live_search_class_init                   (HildonLiveSearchClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (HildonLiveSearchPrivate));

    object_class->get_property = hildon_live_search_get_property;
    object_class->set_property = hildon_live_search_set_property;
    object_class->dispose = hildon_live_search_dispose;

    /**
     * HildonLiveSearch:filter:
     *
     * The #GtkTreeModelFilter to filter.
     *
     * Since: 2.2.4
     */
    g_object_class_install_property (object_class,
                                     PROP_FILTER,
                                     g_param_spec_object ("filter",
                                                          "Filter",
                                                          "Model filter",
                                                          GTK_TYPE_TREE_MODEL_FILTER,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_STATIC_NICK |
                                                          G_PARAM_STATIC_NAME |
                                                          G_PARAM_STATIC_BLURB));

    /**
     * HildonLiveSearch:text-column:
     *
     * A %G_TYPE_STRING column in the child model of #HildonLiveSearch:filter,
     * to be used by the default filtering function of #HildonLiveSearch.
     *
     * Since: 2.2.4
     */
    g_object_class_install_property (object_class,
                                     PROP_TEXT_COLUMN,
                                     g_param_spec_int ("text-column",
                                                       "Text column",
                                                       "Column to use to filter "
                                                       "elements from the #GtkTreeModelFilter",
                                                       -1, G_MAXINT, -1,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_STRINGS));

  /**
   * HildonLiveSearch::refilter:
   * @livesearch: the object which received the signal
   *
   * The "refilter" signal is emitted when the text in the entry changed and a
   * refilter is needed.
   *
   * If this signal is not stopped, gtk_tree_model_filter_refilter() will be
   * called on the filter model. Otherwise the handler is responsible to
   * refilter it.
   *
   * Returns: %TRUE to stop other handlers from being invoked for the event.
   * %FALSE to propagate the event further.
   *
   * Since: 2.2.4
   */
  signals[REFILTER] =
    g_signal_new ("refilter",
                  HILDON_TYPE_LIVE_SEARCH,
                  G_SIGNAL_RUN_LAST, 0,
                  g_signal_accumulator_true_handled, NULL,
                  _hildon_marshal_BOOLEAN__VOID, G_TYPE_BOOLEAN, 0);
}

static void
close_button_clicked_cb                         (GtkWidget *button,
                                                 gpointer   user_data)
{
    gtk_widget_hide (GTK_WIDGET (user_data));
}

static HildonGtkInputMode
filter_input_mode                               (HildonGtkInputMode imode)
{
    return imode & ~(HILDON_GTK_INPUT_MODE_AUTOCAP |
                     HILDON_GTK_INPUT_MODE_DICTIONARY);
}

static void
hildon_live_search_init                         (HildonLiveSearch *self)
{
    HildonLiveSearchPrivate *priv;
    GtkWidget *close_button_alignment;
    GtkToolItem *close_button_container;
    GtkWidget *close;
    GtkToolItem *close_button;
    GtkToolItem *entry_container;
    GtkWidget *entry_hbox;
    HildonGtkInputMode imode;

    self->priv = priv = GET_PRIVATE (self);

    gtk_toolbar_set_style (GTK_TOOLBAR (self), GTK_TOOLBAR_ICONS);
    gtk_container_set_border_width (GTK_CONTAINER (self), 0);

    priv->kb_focus_widget = NULL;
    priv->prefix = NULL;

    priv->visible_func = NULL;
    priv->visible_data = NULL;
    priv->visible_destroy = NULL;
    priv->visible_func_set = FALSE;

    priv->text_column = -1;

    entry_container = gtk_tool_item_new ();
    gtk_tool_item_set_expand (entry_container, TRUE);

    entry_hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (entry_container), entry_hbox);

    priv->entry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT);

    /* Unset the autocap and dictionary input flags from the
       HildonEntry. */
    imode = hildon_gtk_entry_get_input_mode (GTK_ENTRY (priv->entry));
    hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry),
                                     filter_input_mode (imode));

    gtk_widget_set_name (GTK_WIDGET (priv->entry),
                         "HildonLiveSearchEntry");

    gtk_box_pack_start (GTK_BOX (entry_hbox), priv->entry, TRUE, TRUE,
                        HILDON_MARGIN_DEFAULT);

    gtk_toolbar_insert (GTK_TOOLBAR (self), entry_container, 0);
    gtk_widget_show_all (GTK_WIDGET (entry_container));

    close = gtk_image_new_from_icon_name ("general_close",
                                          HILDON_ICON_SIZE_FINGER);
    gtk_misc_set_padding (GTK_MISC (close), 0, 0);
    close_button = gtk_tool_button_new (close, NULL);
    GTK_WIDGET_UNSET_FLAGS (close_button, GTK_CAN_FOCUS);

    close_button_alignment = gtk_alignment_new (0.0f, 0.0f, 1.0f, 1.0f);
    gtk_alignment_set_padding (GTK_ALIGNMENT (close_button_alignment),
                               0, 0,
                               0, HILDON_MARGIN_DEFAULT);
    gtk_container_add (GTK_CONTAINER (close_button_alignment),
                       GTK_WIDGET (close_button));

    close_button_container = gtk_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (close_button_container),
                       close_button_alignment);

    gtk_toolbar_insert (GTK_TOOLBAR (self), close_button_container, -1);
    gtk_widget_show_all (GTK_WIDGET (close_button_container));

    g_signal_connect (G_OBJECT (close_button), "clicked",
                      G_CALLBACK (close_button_clicked_cb), self);

    g_signal_connect (G_OBJECT (priv->entry), "changed",
                      G_CALLBACK (on_entry_changed), self);

    g_signal_connect (self, "hide",
                      G_CALLBACK (on_hide_cb), self);

    gtk_widget_set_no_show_all (GTK_WIDGET (self), TRUE);
}

/* Public interface */

/**
 * hildon_live_search_new:
 *
 * Creates and returns a new #HildonLiveSearch widget.
 *
 * Returns: The newly created live search widget.
 *
 * Since: 2.2.4
 **/
GtkWidget *
hildon_live_search_new                          (void)
{
    return g_object_new (HILDON_TYPE_LIVE_SEARCH, NULL);
}

static gboolean
visible_func                                    (GtkTreeModel *model,
                                                 GtkTreeIter  *iter,
                                                 gpointer      data)
{
    HildonLiveSearchPrivate *priv;
    gchar *string;
    gboolean visible = FALSE;

    priv = (HildonLiveSearchPrivate *) data;

    if (priv->prefix == NULL)
        return TRUE;

    if (priv->visible_func == NULL && priv->text_column == -1)
        return TRUE;

    if (priv->visible_func) {
        visible = (priv->visible_func) (model, iter,
                                        priv->prefix,
                                        priv->visible_data);
    } else {
        gtk_tree_model_get (model, iter, priv->text_column, &string, -1);
        visible = (string != NULL && g_str_has_prefix (string, priv->prefix));
        g_free (string);
    }

    return visible;
}

/**
 * hildon_live_search_set_filter:
 * @livesearch: An #HildonLiveSearch widget
 * @filter: a #GtkTreeModelFilter, or %NULL
 *
 * Sets a filter for @livesearch.
 *
 * Since: 2.2.4
 */
void
hildon_live_search_set_filter                   (HildonLiveSearch   *livesearch,
                                                 GtkTreeModelFilter *filter)
{
    HildonLiveSearchPrivate *priv;

    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));
    g_return_if_fail (filter == NULL || GTK_IS_TREE_MODEL_FILTER (filter));

    priv = livesearch->priv;

    if (filter == priv->filter)
        return;

    if (filter)
        g_object_ref (filter);

    if (priv->filter)
        g_object_unref (priv->filter);

    priv->filter = filter;

    if (priv->visible_func_set == FALSE &&
        (priv->text_column != -1 || priv->visible_func)) {
        gtk_tree_model_filter_set_visible_func (filter,
                                                visible_func,
                                                priv,
                                                NULL);
        priv->visible_func_set = TRUE;
    }

    g_object_notify (G_OBJECT (livesearch), "filter");
}

/**
 * hildon_live_search_get_filter:
 * @livesearch: An #HildonLiveSearch widget
 *
 * Returns: The #GtkTreeModelFilter set with hildon_live_search_set_filter()
 *
 * Since: 2.2.4
 */
GtkTreeModelFilter *
hildon_live_search_get_filter (HildonLiveSearch *livesearch)
{
    HildonLiveSearchPrivate *priv;

    g_return_val_if_fail (HILDON_IS_LIVE_SEARCH (livesearch), NULL);

    priv = livesearch->priv;

    return priv->filter;
}

/**
 * hildon_live_search_set_text_column:
 * @livesearch: a #HildonLiveSearch
 * @text_column: the column in the model of @livesearch to get the strings
 * to filter from
 *
 * Sets the column to be used by the default filtering method.
 * This column must be of type %G_TYPE_STRING.
 *
 * Calling this method will trigger filtering of the model, so use
 * with moderation. Note that you can only use either
 * #HildonLiveSearch:text-column or
 * hildon_live_search_set_visible_func().
 *
 * Since: 2.2.4
 **/
void
hildon_live_search_set_text_column              (HildonLiveSearch *livesearch,
                                                 gint              text_column)
{
    HildonLiveSearchPrivate *priv;

    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));
    g_return_if_fail (-1 <= text_column);

    priv = livesearch->priv;

    g_return_if_fail (text_column < gtk_tree_model_get_n_columns (gtk_tree_model_filter_get_model (priv->filter)));
    g_return_if_fail (priv->visible_func == NULL);

    if (priv->text_column == text_column)
        return;

    priv->text_column = text_column;

    if (priv->visible_func_set == FALSE) {
        gtk_tree_model_filter_set_visible_func (priv->filter,
                                                visible_func,
                                                priv,
                                                NULL);
        priv->visible_func_set = TRUE;
    }

    refilter (livesearch);
}

static void
on_widget_destroy                               (GtkObject *object,
                                                 gpointer   user_data)
{
    hildon_live_search_widget_unhook (HILDON_LIVE_SEARCH (user_data));
}

/**
 * hildon_live_search_widget_hook:
 * @livesearch: An #HildonLiveSearch widget
 * @hook_widget: A widget on which we listen for key events
 * @kb_focus: The widget which we grab focus on
 *
 * This function must be called after a #HildonLiveSearch widget is
 * constructed to set the hook widget and the focus widget for
 * @livesearch. After that, the #HildonLiveSearch widget can be
 * packed into a container and used.
 *
 * Since: 2.2.4
 **/
void
hildon_live_search_widget_hook                  (HildonLiveSearch *livesearch,
                                                 GtkWidget        *hook_widget,
                                                 GtkWidget        *kb_focus)
{
    HildonLiveSearchPrivate *priv;

    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));
    g_return_if_fail (GTK_IS_WIDGET (hook_widget));
    g_return_if_fail (GTK_IS_WIDGET (kb_focus));

    priv = livesearch->priv;

    g_return_if_fail (priv->event_widget == NULL);

    priv->event_widget = g_object_ref (hook_widget);
    priv->kb_focus_widget = g_object_ref (kb_focus);

    priv->key_press_id =
        g_signal_connect (hook_widget, "key-press-event",
                          G_CALLBACK (on_key_press_event), livesearch);

    priv->event_widget_destroy_id =
        g_signal_connect (hook_widget, "destroy",
                          G_CALLBACK (on_widget_destroy), livesearch);

    priv->kb_focus_widget_destroy_id =
        g_signal_connect (kb_focus, "destroy",
                          G_CALLBACK (on_widget_destroy), livesearch);
}

/**
 * hildon_live_search_widget_unhook:
 * @livesearch: An #HildonLiveSearch widget
 *
 * This function unsets the hook and focus widgets which were set
 * earlier using hildon_live_search_widget_hook().
 *
 * Since: 2.2.4
 **/
void
hildon_live_search_widget_unhook                (HildonLiveSearch *livesearch)
{
    HildonLiveSearchPrivate *priv;

    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));
    priv = livesearch->priv;

    if (priv->event_widget == NULL)
        return;

    /* All these variables are set together on hildon_live_search_widget_hook(),
     * so event_widget != NULL implies that all these are non-NULL too */
    g_return_if_fail (priv->kb_focus_widget != NULL);
    g_return_if_fail (priv->key_press_id != 0);
    g_return_if_fail (priv->event_widget_destroy_id != 0);
    g_return_if_fail (priv->kb_focus_widget_destroy_id != 0);

    g_signal_handler_disconnect (priv->event_widget, priv->key_press_id);
    g_signal_handler_disconnect (priv->event_widget, priv->event_widget_destroy_id);
    g_signal_handler_disconnect (priv->kb_focus_widget, priv->kb_focus_widget_destroy_id);

    g_object_unref (priv->kb_focus_widget);
    g_object_unref (priv->event_widget);

    priv->key_press_id = 0;
    priv->event_widget_destroy_id = 0;
    priv->kb_focus_widget_destroy_id = 0;

    priv->kb_focus_widget = NULL;
    priv->event_widget = NULL;
}

/**
 * hildon_live_search_save_state:
 * @livesearch: An #HildonLiveSearch widget
 * @key_file: The key file to save to
 *
 * Saves the live search text to a #GKeyFile.
 *
 * Since: 2.2.4
 **/
void
hildon_live_search_save_state                   (HildonLiveSearch *livesearch,
                                                 GKeyFile         *key_file)
{
    const char *text;

    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));

    text = gtk_entry_get_text (GTK_ENTRY (livesearch->priv->entry));
    if (text) {
        g_key_file_set_string (key_file,
                               "LiveSearch",
                               "Text",
                               text);
    }
}

/**
 * hildon_live_search_restore_state:
 * @livesearch: An #HildonLiveSearch widget
 * @key_file: The key file to read from
 *
 * Restores a live search widget's text from a #GKeyFile.
 *
 * Since: 2.2.4
 **/
void
hildon_live_search_restore_state                (HildonLiveSearch *livesearch,
                                                 GKeyFile         *key_file)
{
    char *text;

    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));

    text = g_key_file_get_string (key_file,
                                  "LiveSearch",
                                  "Text",
                                  NULL);
    if (text) {
        gtk_entry_set_text (GTK_ENTRY (livesearch->priv->entry), text);
        g_free (text);
    }
}

/**
 * hildon_live_search_set_visible_func:
 * @livesearch: a HildonLiveSearch
 * @func: a #HildonLiveSearchVisibleFunc
 * @data: user data to pass to @func or %NULL
 * @destroy: Destroy notifier of @data, or %NULL.
 *
 * Sets the function to use to determine whether a row should be
 * visible when the text in the entry changes. Internally,
 * gtk_tree_model_filter_set_visible_func() is used.
 *
 * This is convenience API to replace #GtkTreeModelFilter's visible function
 * by one that gives the current text in the entry.
 *
 * If this function is unset, #HildonLiveSearch:text-column is used.
 *
 * Since: 2.2.4
 **/
void
hildon_live_search_set_visible_func             (HildonLiveSearch           *livesearch,
                                                 HildonLiveSearchVisibleFunc func,
                                                 gpointer                    data,
                                                 GDestroyNotify              destroy)
{
    HildonLiveSearchPrivate *priv;

    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));
    g_return_if_fail (func != NULL);

    priv = livesearch->priv;

    g_return_if_fail (priv->text_column == -1);

    if (priv->visible_destroy) {
        priv->visible_destroy (priv->visible_data);
    }

    priv->visible_func = func;
    priv->visible_data = data;
    priv->visible_destroy = destroy;

    if (priv->visible_func_set == FALSE) {
        gtk_tree_model_filter_set_visible_func (priv->filter,
                                                visible_func,
                                                priv,
                                                NULL);
        priv->visible_func_set = TRUE;
    }
}

/**
 * hildon_live_search_clean_selection_map:
 * @livesearch: a #HildonLiveSearch
 *
 * Cleans the selection map maintained by
 * @livesearch. When used together with a #GtkTreeView,
 * #HildonLiveSearch maintains internally a selection
 * map, to make sure that selection is invariant to filtering.
 *
 * In some cases, you might want to clean this selection mapping, to
 * ensure that after removing the entered text from @livesearch, the
 * selection is not restored. This is useful in particular when you
 * are using @livesearch with a #GtkTreeModel in a #GtkTreeView with
 * #GtkSelectionMode set to %GTK_SELECTION_SINGLE.
 *
 * Since: 2.2.10
 **/
void
hildon_live_search_clean_selection_map (HildonLiveSearch * livesearch)
{
    g_return_if_fail (HILDON_IS_LIVE_SEARCH (livesearch));

    if (livesearch->priv->selection_map) {
        selection_map_destroy (livesearch->priv);
        selection_map_create (livesearch->priv);
        selection_map_update_map_from_selection (livesearch->priv);
    }
}
