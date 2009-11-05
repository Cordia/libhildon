/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008, 2009 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version. or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * SECTION:hildon-touch-selector-entry
 * @short_description: A selector widget with one column and a text entry
 * @see_also: #HildonTouchSelector, #HildonPickerButton
 *
 * #HildonTouchSelectorEntry is a selector widget with a text entry, similar in
 * behaviour to #GtkComboBoxEntry, that allows user to select an item from a
 * predefined list or to enter a different one in a #HildonEntry. Items can also
 * be searched and selected by typing in the entry. For more specific use cases,
 * the #HildonEntry can be accessed directly with hildon_touch_selector_get_entry().
 *
 * The main difference between the #GtkTreeModel used by #HildonTouchSelector
 * and #HildonTouchSelectorEntry, is that the latter must always include a text
 * column. You should set it with hildon_touch_selector_entry_set_text_column().
 *
 * Normally, you would use #HildonTouchSelectorEntry together with a
 * #HildonPickerDialog activated from a button. For the most common
 * cases, you should use #HildonPickerButton.
 *
 * If you only need a text only, one column selector, you can create it with
 * hildon_touch_selector_entry_new_text() and populate it with
 * hildon_touch_selector_append_text(), hildon_touch_selector_prepend_text(),
 * and hildon_touch_selector_insert_text().
 *
 */

#include "hildon-touch-selector.h"
#include "hildon-touch-selector-entry.h"
#include "hildon-entry.h"

G_DEFINE_TYPE (HildonTouchSelectorEntry, hildon_touch_selector_entry, HILDON_TYPE_TOUCH_SELECTOR)

#define HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), HILDON_TYPE_TOUCH_SELECTOR_ENTRY, HildonTouchSelectorEntryPrivate))

typedef struct _HildonTouchSelectorEntryPrivate HildonTouchSelectorEntryPrivate;

static void entry_on_text_changed (GtkEditable * editable, gpointer userdata);
static void hildon_touch_selector_entry_changed (HildonTouchSelector * selector,
                                                 gint column,
                                                 gpointer user_data);
static void hildon_touch_selector_entry_set_model (HildonTouchSelector * selector,
                                                   gint column, GtkTreeModel *model);
static gboolean hildon_touch_selector_entry_has_multiple_selection (HildonTouchSelector * selector);

static void
_text_column_modified (GObject *pspec, GParamSpec *gobject, gpointer data);


struct _HildonTouchSelectorEntryPrivate {
  gulong signal_id;
  GtkWidget *entry;
};

enum {
  PROP_TEXT_COLUMN = 1
};

static void
hildon_touch_selector_entry_get_property (GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_TEXT_COLUMN:
    g_value_set_int (value,
                     hildon_touch_selector_entry_get_text_column (HILDON_TOUCH_SELECTOR_ENTRY (object)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
hildon_touch_selector_entry_set_property (GObject *object, guint property_id,
                                          const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_TEXT_COLUMN:
    hildon_touch_selector_entry_set_text_column (HILDON_TOUCH_SELECTOR_ENTRY (object),
                                                 g_value_get_int (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static GObject *
hildon_touch_selector_entry_constructor (GType type,
					 guint n_construct_properties,
					 GObjectConstructParam *construct_properties)
{
  GObject *object;

  object = G_OBJECT_CLASS (hildon_touch_selector_entry_parent_class)->constructor
    (type, n_construct_properties, construct_properties);

  g_object_set (object, "live-search", FALSE, NULL);

  return object;
}

static void
hildon_touch_selector_entry_class_init (HildonTouchSelectorEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  HildonTouchSelectorClass *selector_class = HILDON_TOUCH_SELECTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (HildonTouchSelectorEntryPrivate));

  selector_class->set_model = hildon_touch_selector_entry_set_model;
  selector_class->has_multiple_selection = hildon_touch_selector_entry_has_multiple_selection;

  object_class->constructor  = hildon_touch_selector_entry_constructor;
  object_class->get_property = hildon_touch_selector_entry_get_property;
  object_class->set_property = hildon_touch_selector_entry_set_property;

  /**
   * HildonTouchSelectorEntry:text-column:
   *
   * A column in the data source model to get the strings from.
   *
   * This property is deprecated. Use HildonTouchSelectorColumn:text-column
   * instead. Use hildon_touch_selector_entry_set_text_column() and
   * hildon_touch_selector_entry_get_text_column() to manage it.
   *
   * Deprecated: use HildonTouchSelectorColumn:text-column instead
   *
   * Since: maemo 2.2
   **/
  g_object_class_install_property (G_OBJECT_CLASS (klass),
                                   PROP_TEXT_COLUMN,
                                   g_param_spec_int ("text-column",
                                                     "Text Column",
                                                     "A column in the data source model to get the strings from.",
                                                     -1,
                                                     G_MAXINT,
                                                     -1,
                                                     G_PARAM_READWRITE));
}

static gchar *
hildon_touch_selector_entry_print_func (HildonTouchSelector * selector, gpointer user_data)
{
  HildonTouchSelectorEntryPrivate *priv;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint column;
  gchar *text = NULL;

  priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (selector);

  if (*(gtk_entry_get_text (GTK_ENTRY (priv->entry))) != '\0') {
    text = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry)));
  } else {
    model = hildon_touch_selector_get_model (selector, 0);
    if (hildon_touch_selector_get_selected (selector, 0, &iter)) {
      column = hildon_touch_selector_entry_get_text_column (HILDON_TOUCH_SELECTOR_ENTRY (selector));
      gtk_tree_model_get (model, &iter, column, &text, -1);
    }
  }

  return text;
}

static void
hildon_touch_selector_entry_init (HildonTouchSelectorEntry *self)
{
  HildonTouchSelectorEntryPrivate *priv;
  GtkEntryCompletion *completion;
  HildonGtkInputMode input_mode;

  priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (self);

  priv->entry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT);
  gtk_entry_set_activates_default (GTK_ENTRY (priv->entry), TRUE);
  input_mode = hildon_gtk_entry_get_input_mode (GTK_ENTRY (priv->entry));

  /* Disable unsupported input modes. */
  input_mode &= ~HILDON_GTK_INPUT_MODE_MULTILINE;
  input_mode &= ~HILDON_GTK_INPUT_MODE_INVISIBLE;
  input_mode &= ~HILDON_GTK_INPUT_MODE_DICTIONARY;

  hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry), input_mode);

  completion = gtk_entry_completion_new ();
  gtk_entry_completion_set_inline_completion (completion, TRUE);
  gtk_entry_completion_set_popup_completion (completion, FALSE);
  gtk_entry_set_completion (GTK_ENTRY (priv->entry), completion);
  g_object_unref (completion);

  gtk_widget_show (priv->entry);
  g_signal_connect (G_OBJECT (priv->entry), "changed",
                    G_CALLBACK (entry_on_text_changed), self);
  priv->signal_id = g_signal_connect (G_OBJECT (self), "changed",
                                      G_CALLBACK (hildon_touch_selector_entry_changed), NULL);

  hildon_touch_selector_set_print_func (HILDON_TOUCH_SELECTOR (self), hildon_touch_selector_entry_print_func);
  gtk_box_pack_start (GTK_BOX (self), priv->entry, FALSE, FALSE, 0);
}

/**
 * hildon_touch_selector_entry_new:
 *
 * Creates a #HildonTouchSelectorEntry
 *
 * Returns: A new #HildonTouchSelectorEntry
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_touch_selector_entry_new (void)
{
  return g_object_new (HILDON_TYPE_TOUCH_SELECTOR_ENTRY, NULL);
}

/**
 * hildon_touch_selector_entry_new_text:
 *
 * Creates a #HildonTouchSelectorEntry with a single text column that
 * can be populated conveniently through hildon_touch_selector_append_text(),
 * hildon_touch_selector_prepend_text(), hildon_touch_selector_insert_text().
 *
 * Returns: A new #HildonTouchSelectorEntry
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_touch_selector_entry_new_text (void)
{
  GtkListStore *model;
  GtkWidget *selector;
  GtkEntryCompletion *completion;
  HildonTouchSelectorEntryPrivate *priv;
  HildonTouchSelectorColumn *column = NULL;

  selector = hildon_touch_selector_entry_new ();

  priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (selector);

  model = gtk_list_store_new (1, G_TYPE_STRING);
  completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
  gtk_entry_completion_set_model (completion, GTK_TREE_MODEL (model));
  column = hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector),
                                                     GTK_TREE_MODEL (model), FALSE);
  g_object_unref (model);

  g_signal_connect (column, "notify::text-column", G_CALLBACK (_text_column_modified),
                    selector);
  hildon_touch_selector_entry_set_text_column (HILDON_TOUCH_SELECTOR_ENTRY (selector), 0);

  return selector;
}

static void
_text_column_modified (GObject *pspec, GParamSpec *gobject, gpointer data)
{
  HildonTouchSelectorEntry *selector;
  HildonTouchSelectorEntryPrivate *priv;
  GtkEntryCompletion *completion;
  gint text_column = -1;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_ENTRY (data));
  selector = HILDON_TOUCH_SELECTOR_ENTRY (data);

  priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (HILDON_TOUCH_SELECTOR_ENTRY(selector));
  completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));

  text_column = hildon_touch_selector_entry_get_text_column (selector);

  gtk_entry_completion_set_text_column (completion, text_column);
}

/**
 * hildon_touch_selector_entry_set_text_column:
 * @selector: A #HildonTouchSelectorEntry
 * @text_column: A column in model to get the strings from
 *
 * Sets the model column which touch selector box should use to get strings
 * from to be @text_column.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_entry_set_text_column (HildonTouchSelectorEntry *selector,
                                             gint text_column)
{
  HildonTouchSelectorColumn *column = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_ENTRY (selector));
  g_return_if_fail (text_column >= -1);

  column = hildon_touch_selector_get_column (HILDON_TOUCH_SELECTOR (selector), 0);

  hildon_touch_selector_column_set_text_column (column, text_column);
}

/**
 * hildon_touch_selector_entry_get_text_column:
 * @selector: A #HildonTouchSelectorEntry
 *
 * Gets the text column that @selector is using as a text column.
 *
 * Returns: the number of the column used as a text column.
 *
 * Since: 2.2
 **/
gint
hildon_touch_selector_entry_get_text_column (HildonTouchSelectorEntry *selector)
{
  HildonTouchSelectorColumn *column = NULL;
  gint text_column = -1;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR_ENTRY (selector), -1);

  column = hildon_touch_selector_get_column (HILDON_TOUCH_SELECTOR (selector),
                                             0);

  text_column = hildon_touch_selector_column_get_text_column (column);

  return text_column;
}

/**
 * hildon_touch_selector_entry_set_input_mode:
 * @selector: a #HildonTouchSelectorEntry
 * @input_mode: #HildonGtkInputMode mask
 *
 * Sets the input mode to be used in the #GtkEntry in @selector. See hildon_gtk_entry_set_input_mode()
 * for details.
 *
 * It must be noted that not all input modes are available for the
 * entry in @selector. In particular,
 * %HILDON_GTK_INPUT_MODE_MULTILINE, %HILDON_GTK_INPUT_MODE_INVISIBLE,
 * %HILDON_GTK_INPUT_MODE_DICTIONARY are disabled, since these are irrelevant
 * for #HildonTouchSelectorEntry.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_entry_set_input_mode (HildonTouchSelectorEntry * selector,
                                            HildonGtkInputMode input_mode)
{
  HildonTouchSelectorEntryPrivate *priv;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_ENTRY (selector));
  g_return_if_fail (!(input_mode & (HILDON_GTK_INPUT_MODE_MULTILINE |
				    HILDON_GTK_INPUT_MODE_INVISIBLE |
				    HILDON_GTK_INPUT_MODE_DICTIONARY)));

  priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (selector);

  hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry), input_mode);
}

/**
 * hildon_touch_selector_entry_get_input_mode:
 * @selector: a #HildonTouchSelectorEntry
 *
 * Gets the input mode used in the #GtkEntry in @selector. See hildon_gtk_entry_get_input_mode()
 * for details.
 *
 * Returns: a mask of #HildonGtkInputMode
 *
 * Since: 2.2
 **/
HildonGtkInputMode
hildon_touch_selector_entry_get_input_mode (HildonTouchSelectorEntry * selector)
{
  HildonTouchSelectorEntryPrivate *priv;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR_ENTRY (selector), HILDON_GTK_INPUT_MODE_ALPHA);

  priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (selector);

  return hildon_gtk_entry_get_input_mode (GTK_ENTRY (priv->entry));
}

static void
entry_on_text_changed (GtkEditable * editable,
                       gpointer userdata)
{
  HildonTouchSelector *selector;
  HildonTouchSelectorEntryPrivate *priv;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkEntry *entry;
  const gchar *prefix;
  gchar *text;
  gboolean found = FALSE;
  gint text_column = -1;

  entry = GTK_ENTRY (editable);
  selector = HILDON_TOUCH_SELECTOR (userdata);
  priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (selector);

  text_column =
    hildon_touch_selector_entry_get_text_column (HILDON_TOUCH_SELECTOR_ENTRY (selector));

  prefix = gtk_entry_get_text (entry);

  if (prefix[0] == '\0') {
	  return;
  }

  model = hildon_touch_selector_get_model (selector, 0);

  if (!gtk_tree_model_get_iter_first (model, &iter)) {
    return;
  }

  do {
    gtk_tree_model_get (model, &iter, text_column, &text, -1);
    found = g_str_has_prefix (text, prefix);
    g_free (text);
  } while (found != TRUE && gtk_tree_model_iter_next (model, &iter));

  g_signal_handler_block (selector, priv->signal_id);
  {
    /* We emit the HildonTouchSelector::changed signal because a change in the
       GtkEntry represents a change in current selection, and therefore, users
       should be notified. */
    if (found) {
      hildon_touch_selector_select_iter (selector, 0, &iter, TRUE);
    }
    g_signal_emit_by_name (selector, "changed", 0);
  }
  g_signal_handler_unblock (selector, priv->signal_id);

}

/* FIXME: This is actually a very ugly way to retrieve the text. Ideally,
   we would have API to retrieve it from the base clase (HildonTouchSelector).
   In the meantime, leaving it here.
 */
static gchar *
hildon_touch_selector_get_text_from_model (HildonTouchSelectorEntry * selector)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreePath *path;
  GList *selected_rows;
  gchar *text;
  gint text_column = -1;

  model = hildon_touch_selector_get_model (HILDON_TOUCH_SELECTOR (selector), 0);
  text_column = hildon_touch_selector_entry_get_text_column (selector);
  selected_rows = hildon_touch_selector_get_selected_rows (HILDON_TOUCH_SELECTOR (selector), 0);

  if (selected_rows == NULL) {
    return NULL;
  }

  /* We are in single selection mode */
  g_assert (selected_rows->next == NULL);

  path = (GtkTreePath *)selected_rows->data;
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, text_column, &text, -1);

  gtk_tree_path_free (path);
  g_list_free (selected_rows);

  return text;
}

static void
hildon_touch_selector_entry_changed (HildonTouchSelector * selector,
                                     gint column, gpointer user_data)
{
  HildonTouchSelectorEntryPrivate *priv;
  gchar *text;

  priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (selector);

  text = hildon_touch_selector_get_text_from_model (HILDON_TOUCH_SELECTOR_ENTRY (selector));
  if (text != NULL) {
    gtk_entry_set_text (GTK_ENTRY (priv->entry), text);
    gtk_editable_select_region (GTK_EDITABLE (priv->entry), 0, -1);
    g_free (text);
  }
}

static void
hildon_touch_selector_entry_set_model (HildonTouchSelector * selector,
                                       gint column, GtkTreeModel *model)
{
  GtkEntryCompletion *completion;
  HildonTouchSelectorEntryPrivate *priv;
  gint text_column = -1;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_ENTRY (selector));
  g_return_if_fail (column == 0);
  g_return_if_fail (GTK_IS_TREE_MODEL (model));

  HILDON_TOUCH_SELECTOR_CLASS (hildon_touch_selector_entry_parent_class)->set_model (selector, column, model);

  priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (selector);

  completion = gtk_entry_get_completion (GTK_ENTRY (priv->entry));
  gtk_entry_completion_set_model (completion, model);

  text_column = hildon_touch_selector_entry_get_text_column (HILDON_TOUCH_SELECTOR_ENTRY (selector));

  gtk_entry_completion_set_text_column (completion, text_column);
}

static gboolean
hildon_touch_selector_entry_has_multiple_selection (HildonTouchSelector * selector)
{
  /* Always TRUE, given the GtkEntry. */
  return TRUE;
}

/**
 * hildon_touch_selector_entry_get_entry:
 * @selector: a #HildonTouchSelectorEntry.
 *
 * Provides access to the #HildonEntry in @selector. Use to programmatically
 * change the contents in entry or modify its behavior.
 *
 * Returns: a #HildonEntry.
 *
 * Since: 2.2
 **/
HildonEntry *
hildon_touch_selector_entry_get_entry (HildonTouchSelectorEntry * selector)
{
	HildonTouchSelectorEntryPrivate *priv;

	g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR_ENTRY (selector), NULL);

	priv = HILDON_TOUCH_SELECTOR_ENTRY_GET_PRIVATE (selector);

	return HILDON_ENTRY (priv->entry);
}
