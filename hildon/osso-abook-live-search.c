/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 * Copyright (C) 2007-2009 Nokia Corporation. All rights reserved.
 *
 * Contact: Joergen Scheibengruber <jorgen.scheibengruber@nokia.com>
 */

/**
 * SECTION:osso-abook-live-search
 * @short_description: A widget for manipulating contact filters.
 *
 * This widget provides a user interface for manipulating
 * #OssoABookFilterModel instances.
 */

#include "config.h"
#include "osso-abook-live-search.h"

#include <hildon/hildon.h>
#include <string.h>

G_DEFINE_TYPE (OssoABookLiveSearch, osso_abook_live_search,
               GTK_TYPE_TOOLBAR);

#define GET_PRIVATE(o) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((o), OSSO_ABOOK_TYPE_LIVE_SEARCH, \
                                      OssoABookLiveSearchPrivate))

typedef struct _OssoABookLiveSearchPrivate OssoABookLiveSearchPrivate;

struct _OssoABookLiveSearchPrivate
{
        OssoABookFilterModel *filter;
        
        GtkTreeView *treeview;
        
        GtkWidget *entry;
        GtkWidget *event_widget;
        GHashTable *selection_map;
        
        GtkIMContext *im_context;

        gulong key_press_id;
        gulong key_release_id;
        gulong realize_id;
        gulong unrealize_id;
        gulong focus_in_event_id;
        gulong focus_out_event_id;
        gulong on_entry_changed_id;
};

enum
{
  PROP_0,

  PROP_FILTER,
  PROP_WIDGET
};

static void
osso_abook_live_search_real_show (GtkWidget *widget);

static void
osso_abook_live_search_real_hide (GtkWidget *widget);

/* Private implementation */

static void
grab_treeview_focus (OssoABookLiveSearchPrivate *priv)
{
        if (GTK_WIDGET_HAS_FOCUS (GTK_WIDGET (priv->treeview)))
                gtk_im_context_focus_in (priv->im_context);

        gtk_widget_grab_focus (GTK_WIDGET (priv->treeview));
}

/**
 * selection_map_create:
 * @priv: The private pimpl
 *
 * Adds a selection map which is useful when merging selected rows in
 * the treeview, when the live search widget is used.
 **/
static void
selection_map_create (OssoABookLiveSearchPrivate *priv)
{
        gboolean working;
        GtkTreeModel *base_model;
        GtkTreeIter iter;

        g_assert (priv->selection_map == NULL);

        base_model = gtk_tree_model_filter_get_model (priv->filter);

        priv->selection_map = g_hash_table_new
                (g_direct_hash, g_direct_equal);

        for (working = gtk_tree_model_get_iter_first (base_model, &iter);
             working;
             working = gtk_tree_model_iter_next (base_model, &iter)) {
                OssoABookContact *contact;

                gtk_tree_model_get (base_model, &iter,
                                    OSSO_ABOOK_LIST_STORE_COLUMN_CONTACT,
                                    &contact,
                                    -1);
                g_hash_table_insert (priv->selection_map,
                                     contact, GINT_TO_POINTER (FALSE));
                g_object_unref (contact);
        }
}

/**
 * selection_map_destroy:
 * @priv: The private pimpl
 *
 * Destroys resources associated with the selection map.
 **/
static void
selection_map_destroy (OssoABookLiveSearchPrivate *priv)
{
        if (priv->selection_map != NULL) {
                g_hash_table_destroy (priv->selection_map);
                priv->selection_map = NULL;
        }
}

/**
 * selection_map_update_map_from_selection:
 * @priv: The private pimpl
 *
 * Find out which rows are visible in filter, and mark them as selected
 * or unselected from treeview to selection map.
 **/
static void
selection_map_update_map_from_selection (OssoABookLiveSearchPrivate *priv)
{
        gboolean working;
        GtkTreeModel *base_model;
        GtkTreeSelection *selection;
        GtkTreeIter iter;

        base_model = gtk_tree_model_filter_get_model (priv->filter);
        selection = gtk_tree_view_get_selection (priv->treeview);

        for (working = gtk_tree_model_get_iter_first (base_model, &iter);
             working;
             working = gtk_tree_model_iter_next (base_model, &iter)) {
                if (osso_abook_filter_model_is_row_visible (priv->filter,
                                                            &iter)) {
                                OssoABookContact *contact;
                                GtkTreeIter filter_iter;

                                gtk_tree_model_get
                                        (base_model, &iter,
                                         OSSO_ABOOK_LIST_STORE_COLUMN_CONTACT,
                                         &contact,
                                         -1);

                                gtk_tree_model_filter_convert_child_iter_to_iter
                                        (GTK_TREE_MODEL_FILTER (priv->filter),
                                         &filter_iter, &iter);

                                if (gtk_tree_selection_iter_is_selected
                                    (selection, &filter_iter)) {
                                        g_hash_table_replace
                                                (priv->selection_map,
                                                 contact,
                                                 GINT_TO_POINTER (TRUE));
                                } else {
                                        g_hash_table_replace
                                                (priv->selection_map,
                                                 contact,
                                                 GINT_TO_POINTER (FALSE));
                                }

                                g_object_unref (contact);
                        }
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
selection_map_update_selection_from_map (OssoABookLiveSearchPrivate *priv)
{
        gboolean working;
        GtkTreeModel *base_model;
        GtkTreeSelection *selection;
        GtkTreeIter iter;

        base_model = gtk_tree_model_filter_get_model (priv->filter);
        selection = gtk_tree_view_get_selection (priv->treeview);

        for (working = gtk_tree_model_get_iter_first (base_model, &iter);
             working;
             working = gtk_tree_model_iter_next (base_model, &iter)) {
                if (osso_abook_filter_model_is_row_visible (priv->filter,
                                                            &iter)) {
                                OssoABookContact *contact;
                                GtkTreeIter filter_iter;
                                gboolean selected;

                                gtk_tree_model_get
                                        (base_model, &iter,
                                         OSSO_ABOOK_LIST_STORE_COLUMN_CONTACT,
                                         &contact,
                                         -1);

                                selected = GPOINTER_TO_INT
                                        (g_hash_table_lookup
                                         (priv->selection_map, contact));

                                gtk_tree_model_filter_convert_child_iter_to_iter
                                        (GTK_TREE_MODEL_FILTER (priv->filter),
                                         &filter_iter, &iter);

                                if (selected) {
                                        gtk_tree_selection_select_iter
                                                (selection, &filter_iter);
                                } else {
                                        gtk_tree_selection_unselect_iter
                                                (selection, &filter_iter);
                                }

                                g_object_unref (contact);
                        }
        }
}

/*
 * Entry changed handler, called when the text entry is changed directly
 * (if it is focused) or indirectly (via #on_key_press_event which
 * captures key presses on the source widget). This will update the
 * filter as required.
 */
static void
on_entry_changed (GtkEntry *entry,
                  gpointer user_data)
{
        OssoABookLiveSearchPrivate *priv;
        const char *text;
        glong len;
        char *old_prefix;

        g_return_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (user_data));
        priv = GET_PRIVATE (user_data);
        
        text = gtk_entry_get_text (GTK_ENTRY (entry));
        len = g_utf8_strlen (text, -1);

        old_prefix = g_strdup
                (osso_abook_filter_model_get_text (priv->filter));

        if (len < 1) {
                text = NULL;
        } else {
                if ((old_prefix == NULL) || (strlen (old_prefix) == 0)) {
                        selection_map_create (priv);
                }
        }

        selection_map_update_map_from_selection (priv);
        osso_abook_filter_model_set_prefix (priv->filter, text);
        selection_map_update_selection_from_map (priv);

        if (len < 1) {
                selection_map_destroy (priv);
        }

        g_free (old_prefix);
}

static void
adj_changed_cb (GtkAdjustment *adj, gpointer user_data)
{
        GtkTreePath *path = NULL;
        GtkTreeView *view = user_data;

        gtk_tree_view_get_cursor (view, &path, NULL);
        if (path) {
                gtk_tree_view_scroll_to_cell (view, path, NULL, FALSE, 0.0f, 0.0f);
        }
        gtk_tree_path_free (path);
        
        g_object_disconnect (adj, "any_signal::changed", G_CALLBACK (adj_changed_cb), user_data, NULL);
}

static void
scroll_to_focus (GtkTreeView *treeview)
{
        GtkAdjustment *adj;
        
        adj = gtk_tree_view_get_vadjustment (treeview);
        g_signal_connect (G_OBJECT (adj), "changed", G_CALLBACK (adj_changed_cb), treeview);
}

/**
 * osso_abook_live_search_append_text:
 * @livesearch: An #OssoABookLiveSearch widget
 * @utf8: The text to append. This text is copied internally, and @utf8 can be freed later by the caller.
 *
 * Appends a string to the entry text in the live search widget.
 **/
void
osso_abook_live_search_append_text (OssoABookLiveSearch *livesearch,
                                     const char *utf8)
{
        OssoABookLiveSearchPrivate *priv;
        GtkEditable *editable;
        int pos, start, end;

        g_return_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (livesearch));
        g_return_if_fail (NULL != utf8);

        priv = GET_PRIVATE (livesearch);

        if (FALSE == GTK_WIDGET_MAPPED (livesearch)) {
                gunichar c;
        
                c = g_utf8_get_char_validated (utf8, strlen (utf8));
                if (g_unichar_isgraph (c))
                        gtk_widget_show (GTK_WIDGET (livesearch));
                else
                        return;
        }

        editable = GTK_EDITABLE (priv->entry);
        
        if (gtk_editable_get_selection_bounds (editable,
                                               &start,
                                               &end)) {
                gtk_editable_delete_text (editable, start, end);
        }
        pos = gtk_editable_get_position (editable);
        gtk_editable_insert_text (editable, utf8, strlen (utf8), &pos);
        gtk_editable_set_position (editable, pos + 1);
}

/**
 * osso_abook_live_search_get_text:
 * @livesearch: An #OssoABookLiveSearch widget
 *
 * Retrieves the text contents of the #OssoABookLiveSearch widget.
 *
 * Returns: a pointer to the text contents of the widget as a
 * string. This string points to an internal widget buffer and must not
 * be freed, modified or stored.
 **/
const char *
osso_abook_live_search_get_text (OssoABookLiveSearch *livesearch)
{
        OssoABookLiveSearchPrivate *priv;

        g_return_val_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (livesearch), NULL);

        priv = GET_PRIVATE (livesearch);

        return gtk_entry_get_text (GTK_ENTRY (priv->entry));
}

/*
 * IM commit handler. This receives the finalized utf8 input based on the
 * previous source widget key event(s) and manipules the entry. This
 * manipulation then calls #on_entry_changed.
 */
static void
on_im_context_commit (GtkIMContext *imcontext,
                      gchar *utf8,
                      gpointer user_data)
{
        OssoABookLiveSearch *live_search;

        g_return_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (user_data));

        live_search = OSSO_ABOOK_LIVE_SEARCH (user_data);
        osso_abook_live_search_append_text (live_search, utf8);
}

/*
 * Key press handler. This takes key presses from the source widget and
 * manipulate the entry. This manipulation then calls #on_entry_changed.
 */
static gboolean
on_key_press_event (GtkWidget *widget,
                    GdkEventKey *key,
                    OssoABookLiveSearch *live_search)
{
        OssoABookLiveSearchPrivate *priv;
        
        g_return_val_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (live_search), FALSE);
        priv = GET_PRIVATE (live_search);
        
        /* Don't intercept control- presses.
         * Make an exception for ctr-space because it is the keyboard layout
         * switch */
        if (key->state & GDK_CONTROL_MASK &&
            key->keyval != GDK_space)
                return FALSE;

        /* Are we already visible, yet ? */
        if (GTK_WIDGET_MAPPED (live_search)) {
                
                /* Eat escape */
                if (key->keyval == GDK_Escape)
                        return TRUE;

                /* Don't eat returns */
                if (key->keyval == GDK_Return)
                        return FALSE;
                if (key->keyval == GDK_BackSpace) {
                        GtkEditable *editable;
                        int start, end;
                        
                        editable = GTK_EDITABLE (priv->entry);
                        if (gtk_editable_get_selection_bounds (editable,
                                                               &start,
                                                               &end)) {
                                gtk_editable_delete_text (editable, start, end);
                        } else {
                                int pos;
                                pos = gtk_editable_get_position (editable);
                                if (pos < 1)
                                        return TRUE;
                                gtk_editable_delete_text (editable, pos - 1, pos);
                        }
                        scroll_to_focus (priv->treeview);

                        return TRUE;
                }
                /* Does the entry have focus? */
                if (GTK_WIDGET_HAS_FOCUS (priv->entry))
                        return FALSE;

                /* Run the key through the IM context filter to get the value */
                return gtk_im_context_filter_keypress (priv->im_context, key);
        }
        
        /* If the treeview is not visible, then don't bother */
        if (FALSE == GTK_WIDGET_MAPPED (priv->treeview))
                return FALSE;

        /* Return and space do not start searching. But ctr-space must still be
         * sent to switch the keyboard layout */
        if (key->keyval == GDK_Return ||
            (key->keyval == GDK_space &&
             !(key->state & GDK_CONTROL_MASK)))
                return FALSE;

        /* Run the key through the IM context filter to get the value */
        gtk_im_context_filter_keypress (priv->im_context, key);

        return FALSE;
}

static gboolean
on_key_release_event (GtkWidget *widget,
                      GdkEventKey *key,
                      OssoABookLiveSearch *live_search)
{
        OssoABookLiveSearchPrivate *priv;

        g_return_val_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (live_search), FALSE);
        priv = GET_PRIVATE (live_search);

        /* Don't intercept control- presses */
        if (key->state & GDK_CONTROL_MASK)
                return FALSE;
        /* Are we already visible, yet ? */
        if (GTK_WIDGET_MAPPED (live_search)) {
                if (key->keyval == GDK_Escape) {
                        gtk_widget_hide (GTK_WIDGET (live_search));
                        return TRUE;
                }

                if (key->keyval == GDK_BackSpace) {
                        const gchar *text = gtk_entry_get_text
                                (GTK_ENTRY (priv->entry));
                        glong len = g_utf8_strlen (text, -1);

                        if (len <= 0) {
                                gtk_widget_hide (GTK_WIDGET (live_search));
                                return TRUE;
                        }
                }

                /* Does the entry have focus? */
                if (GTK_WIDGET_HAS_FOCUS (priv->entry))
                        return FALSE;
                
                /* Run the key through the IM context filter to get the
                   value */
                return gtk_im_context_filter_keypress (priv->im_context, key);
        }

        /* If the treeview is not visible, then don't bother */
        if (FALSE == GTK_WIDGET_MAPPED (priv->treeview))
                return FALSE;
        
        /* Don't pop up fkb or the preview bar */
        if (key->keyval == GDK_Return ||
            key->keyval == GDK_space)
                return FALSE;
        /* Run the key through the IM context filter to get the value */
        gtk_im_context_filter_keypress (priv->im_context, key);

        return FALSE;
}

static void
on_unmap (GtkWidget *widget,
          gpointer   user_data)
{
        OssoABookLiveSearchPrivate *priv;

        priv = GET_PRIVATE (user_data);
        
        hildon_gtk_im_context_hide (priv->im_context);
}

static void
on_hook_widget_realize (GtkWidget *hook_widget,
                        gpointer   user_data)
{
        OssoABookLiveSearchPrivate *priv;

        priv = GET_PRIVATE (user_data);
        gtk_im_context_set_client_window (priv->im_context,
                                          hook_widget->window);
}

static void
on_hook_widget_unrealize (GtkWidget *hook_widget,
                          gpointer   user_data)
{
        OssoABookLiveSearchPrivate *priv;

        priv = GET_PRIVATE (user_data);
        gtk_im_context_set_client_window (priv->im_context,
                                          NULL);
}

/* GObject methods */

static void
osso_abook_live_search_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
        OssoABookLiveSearch *livesearch = OSSO_ABOOK_LIVE_SEARCH (object);
        OssoABookLiveSearchPrivate *priv = GET_PRIVATE (livesearch);

        switch (property_id) {
        case PROP_FILTER:
                g_value_set_object (value, priv->filter);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        }
}

static void
osso_abook_live_search_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
        OssoABookLiveSearch *livesearch = OSSO_ABOOK_LIVE_SEARCH (object);

        switch (property_id) {
        case PROP_FILTER:
                osso_abook_live_search_set_filter (livesearch,
                                                    g_value_get_object (value));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        }
}

static void
osso_abook_live_search_dispose (GObject *object)
{
        OssoABookLiveSearchPrivate *priv = GET_PRIVATE (object);

        if (priv->on_entry_changed_id) {
                g_signal_handler_disconnect (priv->entry,
                                             priv->on_entry_changed_id);
                priv->on_entry_changed_id = 0;
        }

        osso_abook_live_search_widget_unhook (OSSO_ABOOK_LIVE_SEARCH (object));
        
        if (priv->filter) {
                selection_map_destroy (priv);
                osso_abook_filter_model_set_prefix (priv->filter, NULL);
                g_object_unref (priv->filter);
                priv->filter = NULL;
        }

        if (priv->im_context) {
                g_object_unref (priv->im_context);
                priv->im_context = NULL;
        }
        
        G_OBJECT_CLASS (osso_abook_live_search_parent_class)->dispose (object);
}

static void
osso_abook_live_search_class_init (OssoABookLiveSearchClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        g_type_class_add_private (klass, sizeof (OssoABookLiveSearchPrivate));

        object_class->get_property = osso_abook_live_search_get_property;
        object_class->set_property = osso_abook_live_search_set_property;
        object_class->dispose = osso_abook_live_search_dispose;

        widget_class->show = osso_abook_live_search_real_show;
        widget_class->hide = osso_abook_live_search_real_hide;

        g_object_class_install_property (object_class,
                                         PROP_FILTER,
                                         g_param_spec_object ("filter",
                                                              "Filter",
                                                              "Model filter",
                                                              OSSO_ABOOK_TYPE_FILTER_MODEL,
                                                              G_PARAM_READWRITE |
                                                              G_PARAM_STATIC_NICK |
                                                              G_PARAM_STATIC_NAME |
                                                              G_PARAM_STATIC_BLURB));
}

static void
close_button_clicked_cb (GtkWidget *button,
                         gpointer user_data)
{
        gtk_widget_hide (GTK_WIDGET (user_data));
}

static HildonGtkInputMode
filter_input_mode (HildonGtkInputMode imode)
{
        return imode & ~(HILDON_GTK_INPUT_MODE_AUTOCAP |
                         HILDON_GTK_INPUT_MODE_DICTIONARY);
}

static void
osso_abook_live_search_init (OssoABookLiveSearch *self)
{
        OssoABookLiveSearchPrivate *priv;
        GtkWidget *close_button_alignment;
        GtkToolItem *close_button_container;
        GtkWidget *close;
        GtkToolItem *close_button;
        GtkToolItem *entry_container;
        GtkWidget *entry_hbox;
        HildonGtkInputMode imode;

        priv = GET_PRIVATE (self);

        gtk_toolbar_set_style (GTK_TOOLBAR (self), GTK_TOOLBAR_ICONS);
        gtk_container_set_border_width (GTK_CONTAINER (self), 0);
        
        priv->treeview = NULL;

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
                             "OssoABookLiveSearchEntry");

        gtk_box_pack_start (GTK_BOX (entry_hbox), priv->entry, TRUE, TRUE,
                            HILDON_MARGIN_DEFAULT);

        gtk_toolbar_insert (GTK_TOOLBAR (self), entry_container, 0);

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

        priv->on_entry_changed_id =
                g_signal_connect (G_OBJECT (priv->entry), "changed",
                                  G_CALLBACK (on_entry_changed), self);

        priv->im_context = gtk_im_multicontext_new();

        g_object_get (priv->im_context,
                      "hildon-input-mode", &imode,
                      NULL);
        g_object_set (priv->im_context,
                      "hildon-input-mode", filter_input_mode (imode),
                      NULL);

        g_signal_connect (G_OBJECT (priv->im_context), "commit",
                          G_CALLBACK (on_im_context_commit), self);

        g_signal_connect (G_OBJECT (self), "unmap",
                          G_CALLBACK (on_unmap), self);

        g_signal_connect (G_OBJECT (close_button), "clicked",
                          G_CALLBACK (close_button_clicked_cb),
                          self);

        gtk_widget_show_all (GTK_WIDGET (self));
        gtk_widget_set_no_show_all (GTK_WIDGET (self), TRUE);
}

/* Public interface */

/**
 * osso_abook_live_search_new:
 *
 * Creates and returns a new #OssoABookLiveSearch widget.
 *
 * Returns: The newly created live search widget.
 **/
GtkWidget *
osso_abook_live_search_new (void)
{
        return g_object_new (OSSO_ABOOK_TYPE_LIVE_SEARCH, NULL);
}

static void
osso_abook_live_search_real_show (GtkWidget *widget)
{
        OssoABookLiveSearch *livesearch;
        OssoABookLiveSearchPrivate *priv;

        livesearch = OSSO_ABOOK_LIVE_SEARCH (widget);
        priv = GET_PRIVATE (livesearch);
        
        GTK_WIDGET_CLASS (osso_abook_live_search_parent_class)->show (widget);

        if (priv->treeview != NULL)
                grab_treeview_focus (priv);
}

static void
osso_abook_live_search_real_hide (GtkWidget *widget)
{
        OssoABookLiveSearch *livesearch;
        OssoABookLiveSearchPrivate *priv;
        
        livesearch = OSSO_ABOOK_LIVE_SEARCH (widget);
        priv = GET_PRIVATE (livesearch);
        
        gtk_editable_delete_text (GTK_EDITABLE (priv->entry), 0, -1);

        GTK_WIDGET_CLASS (osso_abook_live_search_parent_class)->hide (widget);

        if (priv->treeview != NULL) {
                if (GTK_WIDGET_MAPPED (priv->treeview))
                        scroll_to_focus (priv->treeview);

                grab_treeview_focus (priv);
        }
}

/**
 * osso_abook_live_search_set_filter:
 * @livesearch: An #OssoABookLiveSearch widget
 * @filter: a #OssoABookFilterModel, or %NULL
 *
 * Sets the filter for @livesearch.
 */
void
osso_abook_live_search_set_filter (OssoABookLiveSearch  *livesearch,
                                   OssoABookFilterModel *filter)
{
        OssoABookLiveSearchPrivate *priv;
        
        g_return_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (livesearch));
        g_return_if_fail (filter == NULL || OSSO_ABOOK_IS_FILTER_MODEL (filter));
        
        priv = GET_PRIVATE (livesearch);
        
        if (priv->filter) {
                g_object_unref (priv->filter);
                priv->filter = NULL;
        }

        if (filter) {
                priv->filter = g_object_ref (filter);
        }

        g_object_notify (G_OBJECT (livesearch), "filter");
}

static gboolean
on_hook_widget_focus_in_out_event (GtkWidget     *widget,
                                   GdkEventFocus *focus,
                                   gpointer       user_data)
{
        OssoABookLiveSearchPrivate *priv;
        
        priv = GET_PRIVATE (user_data);
        
        if (focus->in) {
                gtk_im_context_focus_in (priv->im_context);
        } else {
                gtk_im_context_focus_out (priv->im_context);
        }
        return FALSE;
}

/**
 * osso_abook_live_search_widget_hook:
 * @livesearch: An #OssoABookLiveSearch widget
 * @hook_widget: A widget on which we listen for key events
 * @kb_focus: The widget which we grab focus on
 *
 * This function must be called after an #OssoABookLiveSearch widget is
 * constructed to set the hook widget and the focus widget for
 * @livesearch. After that, the #OssoABookLiveSearch widget can be
 * packed into a container and used.
 **/
void
osso_abook_live_search_widget_hook (OssoABookLiveSearch *livesearch,
                                    GtkWidget           *hook_widget,
                                    GtkTreeView         *kb_focus)
{
        OssoABookLiveSearchPrivate *priv;
        
        g_return_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (livesearch));
        priv = GET_PRIVATE (livesearch);

        g_return_if_fail (priv->event_widget == NULL);
        
        priv->event_widget = hook_widget;

        priv->treeview = kb_focus;
        if (priv->treeview != NULL)
                grab_treeview_focus (priv);

        priv->key_press_id =
                g_signal_connect (hook_widget, "key-press-event",
                                  G_CALLBACK (on_key_press_event), livesearch);
        priv->key_release_id =
                g_signal_connect (hook_widget, "key-release-event",
                                  G_CALLBACK (on_key_release_event), livesearch);
        if (GTK_WIDGET_REALIZED (hook_widget)) {
                gtk_im_context_set_client_window (priv->im_context,
                                          hook_widget->window);
                priv->realize_id = 0;
        } else {
                priv->realize_id =
                        g_signal_connect (G_OBJECT (hook_widget), "realize",
                                          G_CALLBACK (on_hook_widget_realize),
                                          livesearch);
        }
        priv->unrealize_id = g_signal_connect (G_OBJECT (hook_widget), "unrealize",
                                               G_CALLBACK (on_hook_widget_unrealize),
                                               livesearch);

        priv->focus_in_event_id =
                g_signal_connect (G_OBJECT (hook_widget), "focus-in-event",
                                  G_CALLBACK (on_hook_widget_focus_in_out_event),
                                   livesearch);

        priv->focus_out_event_id =
                g_signal_connect (G_OBJECT (hook_widget), "focus-out-event",
                                  G_CALLBACK (on_hook_widget_focus_in_out_event),
                                  livesearch);
}

/**
 * osso_abook_live_search_widget_unhook:
 * @livesearch: An #OssoABookLiveSearch widget
 *
 * This function unsets the hook and focus widgets which were set
 * earlier using osso_abook_live_search_widget_hook().
 **/
void
osso_abook_live_search_widget_unhook (OssoABookLiveSearch *livesearch)
{
        OssoABookLiveSearchPrivate *priv;
        
        g_return_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (livesearch));
        priv = GET_PRIVATE (livesearch);
        
        if (priv->event_widget == NULL)
                return;
        
        if (priv->key_press_id) {
                g_signal_handler_disconnect (priv->event_widget, priv->key_press_id);
                priv->key_press_id = 0;
        }
        if (priv->key_release_id) {
                g_signal_handler_disconnect (priv->event_widget, priv->key_release_id);
                priv->key_release_id = 0;
        }
        if (priv->realize_id) {
                g_signal_handler_disconnect (priv->event_widget, priv->realize_id);
                priv->realize_id = 0;
        }
        if (priv->unrealize_id) {
                g_signal_handler_disconnect (priv->event_widget, priv->unrealize_id);
                priv->unrealize_id = 0;
        }
        if (priv->focus_in_event_id) {
                g_signal_handler_disconnect (priv->event_widget, priv->focus_in_event_id);
                priv->focus_in_event_id = 0;
        }
        if (priv->focus_out_event_id) {
                g_signal_handler_disconnect (priv->event_widget, priv->focus_out_event_id);
                priv->focus_out_event_id = 0;
        }

        gtk_im_context_focus_out (priv->im_context);
        gtk_im_context_set_client_window (priv->im_context, NULL);

        priv->event_widget = NULL;
        priv->treeview = NULL;
}

/**
 * osso_abook_live_search_save_state:
 * @livesearch: An #OssoABookLiveSearch widget
 * @key_file: The key file to save to
 *
 * Saves the live search text to a #GKeyFile.
 **/
void
osso_abook_live_search_save_state (OssoABookLiveSearch *livesearch,
                                   GKeyFile *key_file)
{
        OssoABookLiveSearchPrivate *priv;
        const char *text;
        
        g_return_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (livesearch));
        priv = GET_PRIVATE (livesearch);
        
        text = gtk_entry_get_text (GTK_ENTRY (priv->entry));
        if (text) {
                g_key_file_set_string (key_file,
                                       "LiveSearch",
                                       "Text",
                                       text);
        }
}

/**
 * osso_abook_live_search_restore_state:
 * @livesearch: An #OssoABookLiveSearch widget
 * @key_file: The key file to read from
 *
 * Restores a live search widget's text from a #GKeyFile.
 **/
void
osso_abook_live_search_restore_state (OssoABookLiveSearch *livesearch,
                                      GKeyFile *key_file)
{
        OssoABookLiveSearchPrivate *priv;
        char *text;
        
        g_return_if_fail (OSSO_ABOOK_IS_LIVE_SEARCH (livesearch));
        priv = GET_PRIVATE (livesearch);

        text = g_key_file_get_string (key_file,
                                      "LiveSearch",
                                      "Text",
                                      NULL);
        if (text) {
                gtk_entry_set_text (GTK_ENTRY (priv->entry), text);
        }
}

