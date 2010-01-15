/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2008 Nokia Corporation.
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
 * SECTION:hildon-touch-selector
 * @short_description: A selector widget with several columns.
 *
 * #HildonTouchSelector is a selector widget, that allows users to
 * select items from one to many predefined lists. It is very similar
 * to #GtkComboBox, but with several individual pannable
 * columns. #HildonTouchSelector does not need to be placed in a
 * #HildonPannableArea (in fact, doing so will prevent panning).
 *
 * Normally, you would use #HildonTouchSelector together with a
 * #HildonPickerDialog activated from a button. For the most common
 * cases, you should use #HildonPickerButton.
 *
 * The contents of each #HildonTouchSelector column are stored in a
 * #GtkTreeModel. To add a new column to a #HildonTouchSelector, use
 * hildon_touch_selector_append_column(). If you want to add a
 * text-only column, without special attributes, use
 * hildon_touch_selector_append_text_column().
 *
 * It is highly recommended that you use only one column
 * #HildonTouchSelector<!-- -->s.
 * If you only need a text only, one column selector, you can create it with
 * hildon_touch_selector_new_text() and populate with
 * hildon_touch_selector_append_text(), hildon_touch_selector_prepend_text(),
 * and hildon_touch_selector_insert_text().
 *
 * If you need a selector widget that also accepts user inputs, you
 * can use #HildonTouchSelectorEntry.
 *
 * The current selection has a string representation. In the most common cases,
 * each column model will contain a text column. You can configure
 * which column in particular using the #HildonTouchSelectorColumn property
 * #HildonTouchSelectorColumn:text-column
 *
 * You can get this string representation using
 * hildon_touch_selector_get_current_text().
 * You can configure how the selection is printed with
 * hildon_touch_selector_set_print_func(), that sets the current hildon touch
 * selector print function. The widget has a default print function, that
 * uses the #HildonTouchSelectorColumn:text-column property on each
 * #HildonTouchSelectorColumn to compose the final representation.
 *
 * If you create the selector using hildon_touch_selector_new_text() you
 * don't need to take care of this property, as the model is created internally.
 * If you create the selector using hildon_touch_selector_new(), you
 * need to specify properly the property for your custom model in order to get a
 * non-empty string representation, or define your custom print function.
 *
 * <example>
 * <title>Creating a HildonTouchSelector</title>
 * <programlisting>
 * void
 * selection_changed (HildonTouchSelector *selector,
 *                    gint                 column,
 *                    gpointer             user_data)
 * {
 *   gchar *current_selection = NULL;
 * <!-- -->
 *   current_selection = hildon_touch_selector_get_current_text (selector);
 *   g_debug ("Current selection : &percnt;s", current_selection);
 * }
 * <!-- -->
 * static GtkWidget *
 * create_customized_selector ()
 * {
 *   GtkWidget *selector = NULL;
 *   GSList *icon_list = NULL;
 *   GtkListStore *store_icons = NULL;
 *   GSList *item = NULL;
 *   GtkCellRenderer *renderer = NULL;
 *   HildonTouchSelectorColumn *column = NULL;
 * <!-- -->
 *   selector = hildon_touch_selector_new ();
 * <!-- -->
 *   icon_list = gtk_stock_list_ids ();
 * <!-- -->
 *   store_icons = gtk_list_store_new (1, G_TYPE_STRING);
 *   for (item = icon_list; item; item = g_slist_next (item)) {
 *     GtkTreeIter iter;
 *     gchar *label = item->data;
 * <!-- -->
 *     gtk_list_store_append (store_icons, &amp;iter);
 *     gtk_list_store_set (store_icons, &amp;iter, 0, label, -1);
 *     g_free (label);
 *   }
 *   g_slist_free (icon_list);
 * <!-- -->
 *   renderer = gtk_cell_renderer_pixbuf_new ();
 *   gtk_cell_renderer_set_fixed_size (renderer, -1, 100);
 * <!-- -->
 *   column = hildon_touch_selector_append_column (HILDON_TOUCH_SELECTOR (selector),
 *                                                 GTK_TREE_MODEL (store_icons),
 *                                                 renderer, "stock-id", 0, NULL);
 * <!-- -->
 *   hildon_touch_selector_column_set_text_column (column, 0);
 * <!-- -->
 *   g_object_unref (store_icons);
 * <!-- -->
 *   hildon_touch_selector_set_column_selection_mode (HILDON_TOUCH_SELECTOR (selector),
 *                                                    HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE);
 * <!-- -->
 *   g_signal_connect (G_OBJECT (selector), "changed",
 *                     G_CALLBACK (selection_changed), NULL);
 * <!-- -->
 *   return selector;
 * }
 * <!-- -->
 * static GtkWidget *
 * create_simple_selector ()
 * {
 *   GtkWidget *selector = NULL;
 *   gint i;
 * <!-- -->
 *   selector = hildon_touch_selector_new_text ();
 *   hildon_touch_selector_set_column_selection_mode (HILDON_TOUCH_SELECTOR (selector),
 *                                                    HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE);
 * <!-- -->
 *   g_signal_connect (G_OBJECT (selector), "changed",
 *                     G_CALLBACK (selection_changed), NULL);
 * <!-- -->
 *   for (i = 1; i <= 10 ; i++) {
 *     gchar *label = g_strdup_printf ("Item &amp;percnt;d", i);
 * <!-- -->
 *     hildon_touch_selector_append_text (HILDON_TOUCH_SELECTOR (selector),
 *                                        label);
 * <!-- -->
 *     g_free (label);
 *   }
 * <!-- -->
 *   return selector;
 * }
 * </programlisting>
 * </example>
 */

/**
 * SECTION:hildon-touch-selector-column
 * @short_description: A visible column in a #HildonTouchSelector
 * @see_also: #HildonTouchSelector
 *
 * A #HildonTouchSelectorColumn is a column in a
 * #HildonTouchSelector. This class implements the #GtkCellLayout interface, allowing
 * a flexible management of the cellrenderers in each #HildonTouchSelector column.
 */

#undef HILDON_DISABLE_DEPRECATED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include "hildon-gtk.h"

#include "hildon-pannable-area.h"
#include "hildon-touch-selector.h"
#include "hildon-touch-selector-private.h"
#include "hildon-live-search.h"

#define HILDON_TOUCH_SELECTOR_GET_PRIVATE(obj)                          \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HILDON_TYPE_TOUCH_SELECTOR, HildonTouchSelectorPrivate))

G_DEFINE_TYPE (HildonTouchSelector, hildon_touch_selector, GTK_TYPE_VBOX)

/*
 * IMPLEMENTATION NOTES:
 * Struct to maintain the data of each column. The columns are the elements
 * of the widget that belongs properly to the selection behaviour. Although
 * internally the columns are arranged in a private #GtkHBox, as the selector
 * itself is a #GtkVBox, you can add more widgets, like buttons etc., so
 * you finally could have a widget with more elements that the columns, but
 * this doesn't belongs to the selection logic
 */
struct _HildonTouchSelectorColumnPrivate
{
  HildonTouchSelector *parent;    /* the selector that contains this column */
  GtkTreeModel *model;
  gint text_column;
  GtkTreeView *tree_view;
  gulong realize_handler;
  GtkTreePath *initial_path;
  GtkTreeModel *filter;
  GtkWidget *livesearch;

  GtkWidget *panarea;           /* the pannable widget */
  GtkWidget *vbox;
  GtkTreeRowReference *last_activated;
};

struct _HildonTouchSelectorPrivate
{
  GSList *columns;              /* the selection columns */
  GtkWidget *hbox;              /* the container for the selector's columns */
  gboolean initial_scroll;      /* whether initial fancy scrolling to selection */
  gboolean has_live_search;
  GSList *norm_tokens;

  gboolean changed_blocked;

  HildonTouchSelectorPrintFunc print_func;
  gpointer print_user_data;
  GDestroyNotify print_destroy_func;

  HildonUIMode hildon_ui_mode;
};

enum
{
  PROP_HAS_MULTIPLE_SELECTION = 1,
  PROP_INITIAL_SCROLL,
  PROP_HILDON_UI_MODE,
  PROP_LIVE_SEARCH
};

enum
{
  CHANGED,
  COLUMNS_CHANGED,
  LAST_SIGNAL
};

static gint hildon_touch_selector_signals[LAST_SIGNAL] = { 0 };

static void
hildon_touch_selector_dispose                   (GObject * object);

static void
hildon_touch_selector_get_property              (GObject * object,
                                                 guint prop_id,
                                                 GValue * value,
                                                 GParamSpec * pspec);
static void
hildon_touch_selector_set_property              (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec);
/* gtkwidget */

/* gtkcontainer */
static void hildon_touch_selector_remove        (GtkContainer * container,
                                                 GtkWidget * widget);
/* private functions */
static void _row_tapped_cb                      (GtkTreeView * tree_view,
                                                 GtkTreePath * path,
                                                 gpointer user_data);
static void
hildon_touch_selector_row_activated_cb          (GtkTreeView       *tree_view,
                                                 GtkTreePath       *path,
                                                 GtkTreeViewColumn *column,
                                                 gpointer           user_data);

static gchar *_default_print_func               (HildonTouchSelector * selector,
                                                 gpointer user_data);

static HildonTouchSelectorColumn *_create_new_column (HildonTouchSelector * selector,
                                                 GtkTreeModel * model,
                                                 gboolean *emit_changed,
                                                 GtkCellRenderer * renderer,
                                                 va_list args);
static gboolean
on_realize_cb                                  (GtkWidget *widget,
                                                gpointer data);
static void
on_row_changed                                 (GtkTreeModel *model,
                                                GtkTreePath *path,
                                                GtkTreeIter *iter,
                                                gpointer userdata);
static void
on_row_deleted                                 (GtkTreeModel *model,
                                                GtkTreePath *path,
                                                gpointer userdata);

static void
hildon_touch_selector_scroll_to (HildonTouchSelectorColumn *column,
                                 GtkTreeView *tv,
                                 GtkTreePath *path);
static gboolean
_hildon_touch_selector_center_on_selected_items (HildonTouchSelector *selector,
                                                 HildonTouchSelectorColumn *column);
static void
_hildon_touch_selector_set_model                (HildonTouchSelector * selector,
                                                 gint num_column,
                                                 GtkTreeModel * model);
static gboolean
_hildon_touch_selector_has_multiple_selection   (HildonTouchSelector * selector);

static void
hildon_touch_selector_emit_value_changed        (HildonTouchSelector *selector,
                                                 gint column);

/* GtkCellLayout implementation (HildonTouchSelectorColumn)*/
static void hildon_touch_selector_column_cell_layout_init         (GtkCellLayoutIface      *iface);

static void hildon_touch_selector_column_cell_layout_pack_start   (GtkCellLayout         *cell_layout,
                                                                   GtkCellRenderer       *cell,
                                                                   gboolean               expand);
static void hildon_touch_selector_column_cell_layout_pack_end     (GtkCellLayout         *cell_layout,
                                                                   GtkCellRenderer       *cell,
                                                                   gboolean               expand);
static void hildon_touch_selector_column_cell_layout_clear        (GtkCellLayout         *cell_layout);
static void hildon_touch_selector_column_cell_layout_add_attribute(GtkCellLayout         *cell_layout,
                                                                   GtkCellRenderer       *cell,
                                                                   const gchar           *attribute,
                                                                   gint                   column);
static void hildon_touch_selector_column_cell_layout_set_cell_data_func (GtkCellLayout         *cell_layout,
                                                                         GtkCellRenderer       *cell,
                                                                         GtkCellLayoutDataFunc  func,
                                                                         gpointer               func_data,
                                                                         GDestroyNotify         destroy);
static void hildon_touch_selector_column_cell_layout_clear_attributes   (GtkCellLayout         *cell_layout,
                                                                         GtkCellRenderer       *cell);
static void hildon_touch_selector_column_cell_layout_reorder       (GtkCellLayout         *cell_layout,
                                                                    GtkCellRenderer       *cell,
                                                                    gint                   position);
static GList *hildon_touch_selector_column_cell_layout_get_cells   (GtkCellLayout         *cell_layout);

static void
hildon_touch_selector_check_ui_mode_coherence   (HildonTouchSelector *selector);

static void
hildon_touch_selector_class_init (HildonTouchSelectorClass * class)
{
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkContainerClass *container_class;

  gobject_class = G_OBJECT_CLASS (class);
  object_class = GTK_OBJECT_CLASS (class);
  container_class = GTK_CONTAINER_CLASS (class);

  /* GObject */
  gobject_class->dispose = hildon_touch_selector_dispose;
  gobject_class->get_property = hildon_touch_selector_get_property;
  gobject_class->set_property = hildon_touch_selector_set_property;

  /* GtkWidget */

  /* GtkContainer */
  container_class->remove = hildon_touch_selector_remove;

  /* HildonTouchSelector */
  class->changed = NULL;
  class->set_model = _hildon_touch_selector_set_model;

  class->has_multiple_selection = _hildon_touch_selector_has_multiple_selection;

  /* signals */
  /**
   * HildonTouchSelector::changed:
   * @widget: the object which received the signal
   * @column: the number of the column that has changed
   *
   * The "changed" signal is emitted when the active item on any column is changed.
   * This can be due to the user selecting a different item from the list, or
   * due to a call to hildon_touch_selector_select_iter() on one of the columns.
   *
   * Since: 2.2
   */
  hildon_touch_selector_signals[CHANGED] =
    g_signal_new ("changed",
                  G_OBJECT_CLASS_TYPE (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (HildonTouchSelectorClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

  /**
   * HildonTouchSelector::columns-changed:
   * @selector: the object which received the signal
   *
   * The "columns-changed" signal is emitted when the number
   * of columns in the #HildonTouchSelector change.
   *
   * Since: 2.2
   */
  hildon_touch_selector_signals[COLUMNS_CHANGED] =
    g_signal_new ("columns-changed",
                  G_OBJECT_CLASS_TYPE (class),
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /* properties */

  g_object_class_install_property (gobject_class, PROP_HAS_MULTIPLE_SELECTION,
                                   g_param_spec_boolean ("has-multiple-selection",
                                                         "has multiple selection",
                                                         "Whether the widget has multiple "
                                                         "selection (like multiple columns, "
                                                         "multiselection mode, or multiple "
                                                         "internal widgets) and therefore "
                                                         "it may need a confirmation button, "
                                                         "for instance.",
                                                         FALSE,
                                                         G_PARAM_READABLE));

  g_object_class_install_property (G_OBJECT_CLASS (gobject_class),
                                   PROP_INITIAL_SCROLL,
                                   g_param_spec_boolean ("initial-scroll",
                                                         "Initial scroll",
                                                         "Whether to scroll to the"
                                                         "current selection when"
                                                         "the selector is first"
                                                         "shown",
                                                         TRUE,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonTouchSelector:hildon-ui-mode:
     *
     * Specifies which UI mode to use in the internal treeviews.  A setting
     * of %HILDON_UI_MODE_NORMAL will cause these tree view to disable selections
     * and emit row-activated as soon as a row is pressed (unless it is pressed
     * to drag the pannable area where the treeview is). You can use the
     * method hildon_touch_selector_get_last_activated_row() to get it. When
     * %HILDON_UI_MODE_EDIT is set, selections can be made according to the
     * setting of the mode on GtkTreeSelection.
     *
     * Toggling this property will cause the tree view to select an
     * appropriate selection mode if not already done.
     *
     * Since: Hildon 2.2
     */
  g_object_class_install_property (gobject_class,
                                   PROP_HILDON_UI_MODE,
                                   g_param_spec_enum ("hildon-ui-mode",
                                                      "Hildon UI Mode",
                                                      "The Hildon UI mode according "
                                                      "to which the touch selector "
                                                      "should behave",
                                                      HILDON_TYPE_UI_MODE,
                                                      HILDON_UI_MODE_EDIT,
                                                      G_PARAM_READWRITE));

  g_object_class_install_property (G_OBJECT_CLASS (gobject_class),
                                   PROP_LIVE_SEARCH,
                                   g_param_spec_boolean ("live-search",
                                                         "Live search",
                                                         "Whether the widget should have built-in"
                                                         "live search capabilities",
                                                         TRUE,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  /* style properties */
  /* We need to ensure fremantle mode for the treeview in order to work
     properly. This is not about the appearance, this is about behaviour */
  gtk_rc_parse_string ("style \"fremantle-htst\" {\n"
                       "  GtkWidget::hildon-mode = 1\n"
                       "} widget \"*.fremantle-htst\" style \"fremantle-htst\""
                       "widget_class \"*<HildonPannableArea>.GtkTreeView\" style :highest \"fremantle-htst\"");

  g_type_class_add_private (object_class, sizeof (HildonTouchSelectorPrivate));
}

static void
hildon_touch_selector_get_property (GObject * object,
                                    guint prop_id,
                                    GValue * value, GParamSpec * pspec)
{
  HildonTouchSelectorPrivate *priv = HILDON_TOUCH_SELECTOR (object)->priv;

  switch (prop_id) {
  case PROP_HAS_MULTIPLE_SELECTION:
    g_value_set_boolean (value,
                         hildon_touch_selector_has_multiple_selection (HILDON_TOUCH_SELECTOR (object)));
    break;
  case PROP_INITIAL_SCROLL:
    g_value_set_boolean (value, priv->initial_scroll);
    break;
  case PROP_HILDON_UI_MODE:
    g_value_set_enum (value, priv->hildon_ui_mode);
    break;
  case PROP_LIVE_SEARCH:
    g_value_set_boolean (value,
                         hildon_touch_selector_get_live_search (HILDON_TOUCH_SELECTOR (object)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hildon_touch_selector_set_property (GObject *object, guint prop_id,
                                    const GValue *value, GParamSpec *pspec)
{
  HildonTouchSelectorPrivate *priv = HILDON_TOUCH_SELECTOR (object)->priv;

  switch (prop_id) {
  case PROP_INITIAL_SCROLL:
    priv->initial_scroll = g_value_get_boolean (value);
    break;
  case PROP_HILDON_UI_MODE:
    hildon_touch_selector_set_hildon_ui_mode (HILDON_TOUCH_SELECTOR (object),
                                              g_value_get_enum (value));
    break;
  case PROP_LIVE_SEARCH:
    hildon_touch_selector_set_live_search (HILDON_TOUCH_SELECTOR (object),
                                           g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}


static void
hildon_touch_selector_init (HildonTouchSelector * selector)
{
  selector->priv = HILDON_TOUCH_SELECTOR_GET_PRIVATE (selector);

  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (selector), GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (selector), FALSE);

  selector->priv->columns = NULL;

  selector->priv->norm_tokens = NULL;
  selector->priv->print_func = NULL;
  selector->priv->print_user_data = NULL;
  selector->priv->print_destroy_func = NULL;
  selector->priv->initial_scroll = TRUE;
  selector->priv->hbox = gtk_hbox_new (FALSE, 0);

  selector->priv->changed_blocked = FALSE;

  selector->priv->hildon_ui_mode = HILDON_UI_MODE_EDIT;

  gtk_box_pack_end (GTK_BOX (selector), selector->priv->hbox,
                    TRUE, TRUE, 0);
  gtk_widget_show (selector->priv->hbox);
}

static void
hildon_touch_selector_dispose (GObject * object)
{
  GObjectClass *gobject_class;
  HildonTouchSelector *selector = HILDON_TOUCH_SELECTOR (object);

  hildon_touch_selector_set_print_func_full (selector,
                                             NULL, NULL, NULL);

  if (selector->priv->norm_tokens != NULL) {
      g_slist_foreach (selector->priv->norm_tokens, (GFunc) g_free, NULL);
      g_slist_free (selector->priv->norm_tokens);
      selector->priv->norm_tokens = NULL;
  }

  gobject_class = G_OBJECT_CLASS (hildon_touch_selector_parent_class);

  if (gobject_class->dispose)
    (* gobject_class->dispose) (object);
}

static void
clean_column                                    (HildonTouchSelectorColumn *col,
                                                 HildonTouchSelector *selector)
{
  g_signal_handlers_disconnect_by_func (col->priv->model,
                                        on_row_changed, selector);
  g_signal_handlers_disconnect_by_func (col->priv->model,
                                        on_row_deleted, selector);

  if (col->priv->last_activated != NULL) {
    gtk_tree_row_reference_free (col->priv->last_activated);
    col->priv->last_activated = NULL;
  }
}

/*
 * IMPLEMENTATION NOTES:
 * Some people sent questions regarding the fact that the dispose/finalize functions
 * doesn't clean the internal widgets that could lead to leak memory, so we will
 * clarify this topic.
 *
 * This is not required as #HildonTouchSelector extends #GtkContainer. When the
 * widget is freed, the #GtkContainer freeing memory functions are called. This
 * process includes remove each widget individually, so all the widgets are
 * properly freed.
 *
 * In the same way, this widget redefines gtk_container->remove function, in
 * order to free the column related information if it is required.
 *
 * Please take a look to hildon_touch_selector_remove for more information.
 */

/*------------------------------ GtkContainer ------------------------------ */

/*
 * Required in order to free the column at the columns list
 */
static void
hildon_touch_selector_remove (GtkContainer * container, GtkWidget * widget)
{
  HildonTouchSelector *selector = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (container));
  selector = HILDON_TOUCH_SELECTOR (container);

  /* Remove the extra data related to the columns, if required. */
  if (widget == selector->priv->hbox) {
    g_slist_foreach (selector->priv->columns, (GFunc) clean_column, selector);
    g_slist_foreach (selector->priv->columns, (GFunc) g_object_unref, NULL);

    g_slist_free (selector->priv->columns);

    selector->priv->columns = NULL;
  }

  /* Now remove the widget itself from the container */
  GTK_CONTAINER_CLASS (hildon_touch_selector_parent_class)->remove (container, widget);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */
void
hildon_touch_selector_block_changed             (HildonTouchSelector *selector)
{
  selector->priv->changed_blocked = TRUE;
}

void
hildon_touch_selector_unblock_changed           (HildonTouchSelector *selector)
{
  selector->priv->changed_blocked = FALSE;
}

static void
hildon_touch_selector_emit_value_changed        (HildonTouchSelector *selector,
                                                 gint column)
{
  /* FIXME: it could be good to emit too the GtkTreePath of the element
     selected, as now it is required to connect to the signal and then ask
     for the element selected. We can't do this API change, in order to avoid
     and ABI break */
  if (!selector->priv->changed_blocked) {
    g_signal_emit (selector, hildon_touch_selector_signals[CHANGED], 0, column);
    if (hildon_touch_selector_get_column_selection_mode (selector) == HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE &&
        selector->priv->columns) {
      HildonTouchSelectorColumn *col;
      col = (HildonTouchSelectorColumn *) selector->priv->columns->data;
      if (col->priv->livesearch) {
        hildon_live_search_clean_selection_map (HILDON_LIVE_SEARCH (col->priv->livesearch));
      }
    }
  }
}

static void
hildon_touch_selector_check_ui_mode_coherence   (HildonTouchSelector *selector)
{
  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));

  if (hildon_touch_selector_get_num_columns (selector) > 1) {
    hildon_touch_selector_set_hildon_ui_mode (selector, HILDON_UI_MODE_EDIT);
  }
}

/**
 * default_print_func:
 * @selector: a #HildonTouchSelector
 *
 * Default print function
 *
 * Returns: a new string that represents the selected items
 *
 * Since: 2.2
 **/
static gchar *
_default_print_func (HildonTouchSelector * selector, gpointer user_data)
{
  gchar *result = NULL;
  gchar *aux = NULL;
  gint num_columns = 0;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  gchar *current_string = NULL;
  gint i;
  HildonTouchSelectorSelectionMode mode;
  GList *item = NULL;
  GtkTreePath *current_path = NULL;
  GList *selected_rows = NULL;
  gint initial_value = 0;
  gint text_column = -1;
  HildonTouchSelectorColumn *column = NULL;

  num_columns = hildon_touch_selector_get_num_columns (selector);

  mode = hildon_touch_selector_get_column_selection_mode (selector);

  if ((mode == HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE)
      && (num_columns > 0)) {
    /* In this case we get the first column first */
    selected_rows = hildon_touch_selector_get_selected_rows (selector, 0);
    model = hildon_touch_selector_get_model (selector, 0);
    column = hildon_touch_selector_get_column (selector, 0);
    text_column = hildon_touch_selector_column_get_text_column (column);

    result = g_strdup_printf ("(");
    i = 0;
    for (item = selected_rows; item; item = g_list_next (item)) {
      current_path = item->data;
      gtk_tree_model_get_iter (model, &iter, current_path);

      if (text_column != -1) {
        gtk_tree_model_get (model, &iter, text_column, &current_string, -1);
      }

      if (i < g_list_length (selected_rows) - 1) {
        aux = g_strconcat (result, current_string, ",", NULL);
        g_free (result);
        result = aux;
      } else {
        aux = g_strconcat (result, current_string, NULL);
        g_free (result);
        result = aux;
      }

      if (current_string) {
        g_free (current_string);
        current_string = NULL;
      }

      i++;
    }

    aux = g_strconcat (result, ")", NULL);
    g_free (result);
    result = aux;

    g_list_foreach (selected_rows, (GFunc) (gtk_tree_path_free), NULL);
    g_list_free (selected_rows);
    initial_value = 1;
  } else {
    initial_value = 0;
  }

  for (i = initial_value; i < num_columns; i++) {
    model = hildon_touch_selector_get_model (selector, i);
    column = hildon_touch_selector_get_column (selector, i);
    text_column = hildon_touch_selector_column_get_text_column (column);

    if (hildon_touch_selector_get_selected (selector, i, &iter)) {
      if (text_column == -1 ) {
        g_warning ("Trying to use the default print function in HildonTouchSelector, but "
                   "\"text-column\" property is not set for HildonTouchSelectorColumn %p.", column);
        current_string = NULL;
      } else {
        gtk_tree_model_get (model, &iter, text_column, &current_string, -1);
      }

      if (i == 0) {
        result = current_string;
      } else {
        aux = g_strconcat (result, ":", current_string, NULL);
        g_free (result);
        g_free (current_string);
        current_string = NULL;
        result = aux;
      }
    }
  }

  return result;
}

static void
hildon_touch_selector_row_activated_cb          (GtkTreeView       *tree_view,
                                                 GtkTreePath       *path,
                                                 GtkTreeViewColumn *column,
                                                 gpointer           user_data)
{
  HildonTouchSelectorColumn *selector_column = NULL;
  GtkTreePath *child_path;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (user_data));
  selector_column = HILDON_TOUCH_SELECTOR_COLUMN (user_data);

  if (selector_column->priv->last_activated != NULL) {
    gtk_tree_row_reference_free (selector_column->priv->last_activated);
  }

  child_path = gtk_tree_model_filter_convert_path_to_child_path (GTK_TREE_MODEL_FILTER (selector_column->priv->filter),
                                                                 path);
  selector_column->priv->last_activated = gtk_tree_row_reference_new (selector_column->priv->model,
                                                                      child_path);
  gtk_tree_path_free (child_path);
}

static void
_row_tapped_cb (GtkTreeView * tree_view, GtkTreePath * path, gpointer user_data)
{
  HildonTouchSelector *selector = NULL;
  HildonTouchSelectorColumn *column = NULL;
  gint num_column = -1;

  column = HILDON_TOUCH_SELECTOR_COLUMN (user_data);
  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (column->priv->parent));

  selector = column->priv->parent;

  num_column = g_slist_index (selector->priv->columns, column);

  hildon_touch_selector_emit_value_changed (selector, num_column);
}


static HildonTouchSelectorColumn *
_create_new_column (HildonTouchSelector * selector,
                    GtkTreeModel * model,
                    gboolean *emit_changed,
                    GtkCellRenderer * renderer, va_list args)
{
  HildonTouchSelectorColumn *new_column = NULL;
  GtkTreeViewColumn *tree_column = NULL;
  GtkTreeView *tv = NULL;
  GtkTreeModel *filter;
  GtkWidget *panarea = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;
  gchar *attribute;
  gint value;

  tree_column = gtk_tree_view_column_new ();

  if (renderer != NULL) {
    gtk_tree_view_column_pack_start (tree_column, renderer, TRUE);

    attribute = va_arg (args, gchar *);
    while (attribute != NULL) {
      value = va_arg (args, gint);
      gtk_tree_view_column_add_attribute (tree_column, renderer, attribute,
                                          value);
      attribute = va_arg (args, gchar *);
    }
  }

#ifdef MAEMO_GTK
  tv = GTK_TREE_VIEW (hildon_gtk_tree_view_new (selector->priv->hildon_ui_mode));
#else
  tv = GTK_TREE_VIEW (gtk_tree_view_new ());
#endif /* MAEMO_GTK */

  gtk_tree_view_set_enable_search (tv, FALSE);
  if (!selector->priv->has_live_search) {
    GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (tv), GTK_CAN_FOCUS);
  }

  filter = gtk_tree_model_filter_new (model, NULL);
  gtk_tree_view_set_model (tv, filter);
  g_signal_connect (model, "row-changed",
                    G_CALLBACK (on_row_changed), selector);
  g_signal_connect_after (model, "row-deleted",
                          G_CALLBACK (on_row_deleted), selector);
  gtk_tree_view_set_rules_hint (tv, TRUE);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), tree_column);

  new_column = g_object_new (HILDON_TYPE_TOUCH_SELECTOR_COLUMN, NULL);
  new_column->priv->parent = selector;

  panarea = hildon_pannable_area_new ();

  gtk_container_add (GTK_CONTAINER (panarea), GTK_WIDGET (tv));

  new_column->priv->model = g_object_ref (model);
  new_column->priv->tree_view = tv;
  new_column->priv->panarea = panarea;
  new_column->priv->filter = filter;
  new_column->priv->livesearch = NULL;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

  /* select the first item */
  *emit_changed = FALSE;
  if ((gtk_tree_model_get_iter_first (filter, &iter))&&
      (selector->priv->hildon_ui_mode == HILDON_UI_MODE_EDIT)) {
    gtk_tree_selection_select_iter (selection, &iter);
    *emit_changed = TRUE;
  }

  /* connect to the hildon-row-tapped signal connection */
  g_signal_connect (G_OBJECT (tv), "hildon-row-tapped",
                    G_CALLBACK (_row_tapped_cb), new_column);

  g_signal_connect (G_OBJECT (tv), "row-activated",
                    G_CALLBACK (hildon_touch_selector_row_activated_cb), new_column);

  return new_column;
}


/* ------------------------ HildonTouchSelectorColumn implementation ---------------------- */
G_DEFINE_TYPE_WITH_CODE (HildonTouchSelectorColumn, hildon_touch_selector_column, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_CELL_LAYOUT,
                                                hildon_touch_selector_column_cell_layout_init))

enum
{
  PROP_TEXT_COLUMN = 1
};

static void
hildon_touch_selector_column_class_init (HildonTouchSelectorColumnClass *klass);

static void
hildon_touch_selector_column_get_property (GObject *object, guint property_id,
                                           GValue *value, GParamSpec *pspec);

static void
hildon_touch_selector_column_set_property  (GObject *object, guint property_id,
                                            const GValue *value, GParamSpec *pspec);

static void
hildon_touch_selector_column_dispose       (GObject *object);

static void
hildon_touch_selector_column_finalize      (GObject *object);

static void
hildon_touch_selector_column_class_init (HildonTouchSelectorColumnClass *klass)
{
  GObjectClass *gobject_class = NULL;

  gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (gobject_class, sizeof (HildonTouchSelectorColumnPrivate));

  /* GObject */
  gobject_class->get_property = hildon_touch_selector_column_get_property;
  gobject_class->set_property = hildon_touch_selector_column_set_property;
  gobject_class->dispose      = hildon_touch_selector_column_dispose;
  gobject_class->finalize     = hildon_touch_selector_column_finalize;

  /**
   * HildonTouchSelectorColumn:text-column:
   *
   * A column in the data source model to get the strings from.
   *
   * Since: maemo 2.2
   **/
  g_object_class_install_property (G_OBJECT_CLASS(klass),
                                   PROP_TEXT_COLUMN,
                                   g_param_spec_int ("text-column",
                                                     "Text Column",
                                                     "A column in the data source model to get the strings from.",
                                                     -1,
                                                     G_MAXINT,
                                                     -1,
                                                     G_PARAM_READWRITE));
}

static void
hildon_touch_selector_column_init (HildonTouchSelectorColumn *column)
{
  column->priv = G_TYPE_INSTANCE_GET_PRIVATE (column, HILDON_TYPE_TOUCH_SELECTOR_COLUMN,
                                              HildonTouchSelectorColumnPrivate);
  column->priv->text_column = -1;
  column->priv->last_activated = NULL;
  column->priv->realize_handler = 0;
  column->priv->initial_path = NULL;
}

/**
 * stripped_char:
 *
 * Returns a stripped version of @ch, removing any case, accentuation
 * mark, or any special mark on it.
 **/
static gunichar
stripped_char (gunichar ch)
{
  gunichar *decomp, retval;
  GUnicodeType utype;
  gsize dlen;

  utype = g_unichar_type (ch);

  switch (utype) {
  case G_UNICODE_CONTROL:
  case G_UNICODE_FORMAT:
  case G_UNICODE_UNASSIGNED:
  case G_UNICODE_COMBINING_MARK:
    /* Ignore those */
    return 0;
    break;
  default:
    /* Convert to lowercase, fall through */
    ch = g_unichar_tolower (ch);
  case G_UNICODE_LOWERCASE_LETTER:
    if ((decomp = g_unicode_canonical_decomposition (ch, &dlen))) {
      retval = decomp[0];
      g_free (decomp);
      return retval;
    }
    break;
  }

  return 0;
}

static gchar *
e_util_unicode_get_utf8 (const gchar *text, gunichar *out)
{
  *out = g_utf8_get_char (text);
  return (*out == (gunichar)-1) ? NULL : g_utf8_next_char (text);
}

/**
 * get_next:
 * @p: a pointer to the string to search.
 * @out: a place to store the next valid
 * @separators: whether to search only for alphanumeric strings
 * and skip any word separator.
 *
 * Gets the next character that is valid in our search scope, and
 * store it into @out. The next char, after @out is returned.
 *
 * Returns: the next point in the string @p where to continue the
 * string iteration.
 **/
static const gchar *
get_next (const gchar *p, gunichar *out, gboolean separators)
{
  const gchar *q;
  gunichar utf8;

  if (separators) {
    do {
       q = p;
       p = e_util_unicode_get_utf8 (q, &utf8);
       *out = stripped_char (utf8);
    } while (p && *out && !g_unichar_isalnum (*out));
  } else {
    p = e_util_unicode_get_utf8 (p, &utf8);
    *out = stripped_char (utf8);
  }

  return p;
}

/**
 * e_util_utf8_strstrcasedecomp_needle_stripped:
 * @haystack: a haystack where to search
 * @nuni: a needle to search for, already stripped with strip_string()
 *
 * Heavily modified version of e_util_utf8_strstrcasedecomp(). As its
 * original version, it finds the first occurrence of @nuni in
 * @haystack.  However, instead of stripping @nuni, it expect it to be
 * already stripped.
 *
 * This is done for performance reasons, since this search is done
 * several times for the same string @nuni, it is undesired to strip
 * it more than once.
 *
 * Also, the search is done as a prefix search, starting in the first
 * alphanumeric character after any non-alphanumeric one. Searching
 * for "aba" in "Abasto" will match, searching in "Moraba" will not,
 * and searching in "A tool (abacus)" will do.
 *
 * Returns: the first instance of @nuni in @haystack
 **/
static const gchar *
e_util_utf8_strstrcasedecomp_needle_stripped (const gchar *haystack, const gunichar *nuni)
{
  gunichar unival;
  gint nlen = 0;
  const gchar *o, *p;
  gunichar sc;

  if (haystack == NULL) return NULL;
  if (nuni == NULL) return NULL;
  if (strlen (haystack) == 0) return NULL;
  while (*(nuni + nlen) != 0) nlen++;

  if (nlen < 1) return haystack;

  o = haystack;

  for (p = get_next (o, &sc, g_unichar_isalnum (nuni[0]));
       p && sc;
       p = get_next (p, &sc, g_unichar_isalnum (nuni[0]))) {
    if (sc) {
      /* We have valid stripped gchar */
      if (sc == nuni[0]) {
        const gchar *q = p;
        gint npos = 1;
        while (npos < nlen) {
          q = e_util_unicode_get_utf8 (q, &unival);
          if (!q || !unival) return NULL;
          sc = stripped_char (unival);
          if ((!sc) || (sc != nuni[npos])) break;
          npos++;
        }
        if (npos == nlen) {
          return o;
        }
      }
    }
    while (p) {
      sc = g_utf8_get_char (p);
      if (!g_unichar_isalnum (sc))
        break;
      p = g_utf8_next_char (p);
    }

    o = p;
  }

  return NULL;
}

/**
 * strip_string:
 * @string: a string to be stripped off.
 *
 * Strips all capitalization and accentuation marks from a string.
 * The returned unicode string is 0 terminated.
 *
 * Returns: an unicode, lowercase, and without accentuation marks
 * version of @string, or %NULL if @string is an empty string.
 **/
static gunichar *
strip_string (const gchar *string)
{
  gunichar *nuni;
  gint nlen;
  gunichar unival;
  const gchar *p;

  if (strlen (string) == 0) return NULL;

  nuni = g_malloc (sizeof (gunichar) * (strlen (string) + 1));

  nlen = 0;
  for (p = e_util_unicode_get_utf8 (string, &unival);
       p && unival;
       p = e_util_unicode_get_utf8 (p, &unival)) {
      gunichar sc;
      sc = stripped_char (unival);
      if (sc) {
          nuni[nlen++] = sc;
      }
  }

  /* NULL means there was illegal utf-8 sequence */
  if (!p) nlen = 0;

  nuni[nlen] = 0;

  return nuni;
}

static gboolean
hildon_live_search_visible_func (GtkTreeModel *model,
                                 GtkTreeIter *iter,
                                 gchar *prefix,
                                 gpointer userdata)
{
  gboolean visible = TRUE;
  gchar *string;
  GSList *list_iter;
  HildonTouchSelectorColumn *col;
  HildonTouchSelector *selector;

  col = HILDON_TOUCH_SELECTOR_COLUMN (userdata);
  selector = col->priv->parent;
  gint text_column = GPOINTER_TO_INT (col->priv->text_column);

  gtk_tree_model_get (model, iter, text_column, &string, -1);
  list_iter = selector->priv->norm_tokens;
  while (visible && list_iter) {
    visible = (string != NULL &&
               e_util_utf8_strstrcasedecomp_needle_stripped (string,
                                                             (gunichar *)list_iter->data) != NULL);
    list_iter = list_iter->next;
  }

  g_free (string);

  return visible;
}

static gboolean
on_live_search_refilter (HildonLiveSearch *livesearch,
                         gpointer userdata)
{
    HildonTouchSelector *selector = HILDON_TOUCH_SELECTOR (userdata);

    gchar **tokens = g_strsplit (hildon_live_search_get_text (livesearch), " ", -1);
    gunichar *token;
    gint i;

    if (selector->priv->norm_tokens != NULL) {
        g_slist_foreach (selector->priv->norm_tokens, (GFunc) g_free, NULL);
        g_slist_free (selector->priv->norm_tokens);
        selector->priv->norm_tokens = NULL;
    }

    for (i = 0; tokens [i] != NULL; i++) {
        token = strip_string (tokens[i]);
        if (token != NULL)
            selector->priv->norm_tokens = g_slist_prepend (selector->priv->norm_tokens,
                                                           token);
    }

    g_strfreev (tokens);

    return FALSE;
}

/**
 * hildon_touch_selector_column_set_text_column:
 * @column: A #HildonTouchSelectorColumn
 * @text_column: the index of a model column in the model for @column.
 *
 * Sets the model column to be displayed in @column. @text_column must point to a
 * column in the model used with type %G_TYPE_STRING. Initially, this property
 * is unset. You should set it before using the #HildonTouchSelector that uses
 * @column.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_column_set_text_column (HildonTouchSelectorColumn *column,
                                              gint text_column)
{
  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (column));
  g_return_if_fail (text_column >= -1);

  column->priv->text_column = text_column;

  if (column->priv->livesearch) {
    hildon_live_search_set_visible_func (HILDON_LIVE_SEARCH (column->priv->livesearch),
                                         hildon_live_search_visible_func,
                                         column,
                                         NULL);
  }

  g_object_notify (G_OBJECT (column), "text-column");
}

/**
 * hildon_touch_selector_column_get_text_column:
 * @column: a #HildonTouchSelectorColumn
 *
 * Gets the model column set as the text source for @column.
 *
 * Returns: the index of the text column for @column, or -1 if unset.
 *
 * Since: 2.2
 **/
gint
hildon_touch_selector_column_get_text_column (HildonTouchSelectorColumn *column)
{
  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (column), -1);

  return column->priv->text_column;
}

static void
hildon_touch_selector_column_get_property (GObject *object, guint property_id,
                                           GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_TEXT_COLUMN:
    g_value_set_int (value,
                     hildon_touch_selector_column_get_text_column (HILDON_TOUCH_SELECTOR_COLUMN (object)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
hildon_touch_selector_column_set_property (GObject *object, guint property_id,
                                           const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_TEXT_COLUMN:
    hildon_touch_selector_column_set_text_column (HILDON_TOUCH_SELECTOR_COLUMN (object),
                                                  g_value_get_int (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
hildon_touch_selector_column_dispose      (GObject *object)
{
  HildonTouchSelectorColumnPrivate *priv =
      HILDON_TOUCH_SELECTOR_COLUMN (object)->priv;

  if (priv->model != NULL) {
      g_object_unref (priv->model);
      priv->model = NULL;
  }

  if (priv->filter != NULL) {
      g_object_unref (priv->filter);
      priv->filter = NULL;
  }

  G_OBJECT_CLASS (hildon_touch_selector_column_parent_class)->dispose (object);
}

static void
hildon_touch_selector_column_finalize     (GObject *object)
{
  HildonTouchSelectorColumnPrivate *priv = HILDON_TOUCH_SELECTOR_COLUMN (object)->priv;

  if (priv->initial_path) {
    gtk_tree_path_free (priv->initial_path);
  }

  G_OBJECT_CLASS (hildon_touch_selector_column_parent_class)->finalize (object);
}

/* ------------------------ GtkCellLayout implementation -------------------- */
static void
hildon_touch_selector_column_cell_layout_init (GtkCellLayoutIface      *iface)
{
  iface->pack_start         = hildon_touch_selector_column_cell_layout_pack_start;
  iface->pack_end           = hildon_touch_selector_column_cell_layout_pack_end;
  iface->clear              = hildon_touch_selector_column_cell_layout_clear;
  iface->add_attribute      = hildon_touch_selector_column_cell_layout_add_attribute;
  iface->set_cell_data_func = hildon_touch_selector_column_cell_layout_set_cell_data_func;
  iface->clear_attributes   = hildon_touch_selector_column_cell_layout_clear_attributes;
  iface->reorder            = hildon_touch_selector_column_cell_layout_reorder;
  iface->get_cells          = hildon_touch_selector_column_cell_layout_get_cells;
}

static void
hildon_touch_selector_column_cell_layout_pack_start (GtkCellLayout         *cell_layout,
                                               GtkCellRenderer       *cell,
                                               gboolean               expand)
{
  HildonTouchSelectorColumn *sel_column = NULL;
  GtkTreeViewColumn *view_column = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (cell_layout));
  sel_column = HILDON_TOUCH_SELECTOR_COLUMN (cell_layout);

  view_column = gtk_tree_view_get_column (sel_column->priv->tree_view, 0);

  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(view_column), cell, expand);

}

static void
hildon_touch_selector_column_cell_layout_pack_end (GtkCellLayout         *cell_layout,
                                             GtkCellRenderer       *cell,
                                             gboolean               expand)
{
  HildonTouchSelectorColumn *sel_column = NULL;
  GtkTreeViewColumn *view_column = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (cell_layout));
  sel_column = HILDON_TOUCH_SELECTOR_COLUMN (cell_layout);

  view_column = gtk_tree_view_get_column (sel_column->priv->tree_view, 0);

  gtk_cell_layout_pack_end (GTK_CELL_LAYOUT(view_column), cell, expand);
}

static void
hildon_touch_selector_column_cell_layout_clear (GtkCellLayout         *cell_layout)
{
  HildonTouchSelectorColumn *sel_column = NULL;
  GtkTreeViewColumn *view_column = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (cell_layout));
  sel_column = HILDON_TOUCH_SELECTOR_COLUMN (cell_layout);

  view_column = gtk_tree_view_get_column (sel_column->priv->tree_view, 0);

  gtk_cell_layout_clear (GTK_CELL_LAYOUT(view_column));
}

static void
hildon_touch_selector_column_cell_layout_add_attribute (GtkCellLayout         *cell_layout,
                                                  GtkCellRenderer       *cell,
                                                  const gchar           *attribute,
                                                  gint                   column)
{
  HildonTouchSelectorColumn *sel_column = NULL;
  GtkTreeViewColumn *view_column = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (cell_layout));
  sel_column = HILDON_TOUCH_SELECTOR_COLUMN (cell_layout);

  view_column = gtk_tree_view_get_column (sel_column->priv->tree_view, 0);

  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(view_column), cell, attribute, column);
}


static void
hildon_touch_selector_column_cell_layout_set_cell_data_func (GtkCellLayout         *cell_layout,
                                                       GtkCellRenderer       *cell,
                                                       GtkCellLayoutDataFunc  func,
                                                       gpointer               func_data,
                                                       GDestroyNotify         destroy)
{
  HildonTouchSelectorColumn *sel_column = NULL;
  GtkTreeViewColumn *view_column = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (cell_layout));
  sel_column = HILDON_TOUCH_SELECTOR_COLUMN (cell_layout);

  view_column = gtk_tree_view_get_column (sel_column->priv->tree_view, 0);

  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT(view_column), cell, func,
                                      func_data, destroy);
}

static void
hildon_touch_selector_column_cell_layout_clear_attributes (GtkCellLayout         *cell_layout,
                                                     GtkCellRenderer       *cell)
{
  HildonTouchSelectorColumn *sel_column = NULL;
  GtkTreeViewColumn *view_column = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (cell_layout));
  sel_column = HILDON_TOUCH_SELECTOR_COLUMN (cell_layout);

  view_column = gtk_tree_view_get_column (sel_column->priv->tree_view, 0);

  gtk_cell_layout_clear_attributes (GTK_CELL_LAYOUT (view_column), cell);
}

static void
hildon_touch_selector_column_cell_layout_reorder (GtkCellLayout         *cell_layout,
                                            GtkCellRenderer       *cell,
                                            gint                   position)
{
  HildonTouchSelectorColumn *sel_column = NULL;
  GtkTreeViewColumn *view_column = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (cell_layout));
  sel_column = HILDON_TOUCH_SELECTOR_COLUMN (cell_layout);

  view_column = gtk_tree_view_get_column (sel_column->priv->tree_view, 0);

  gtk_cell_layout_reorder (GTK_CELL_LAYOUT(view_column), cell, position);
}

static GList*
hildon_touch_selector_column_cell_layout_get_cells (GtkCellLayout         *cell_layout)
{
  HildonTouchSelectorColumn *sel_column = NULL;
  GtkTreeViewColumn *view_column = NULL;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR_COLUMN (cell_layout), NULL);
  sel_column = HILDON_TOUCH_SELECTOR_COLUMN (cell_layout);

  view_column = gtk_tree_view_get_column (sel_column->priv->tree_view, 0);

  return gtk_cell_layout_get_cells (GTK_CELL_LAYOUT(view_column));
}

/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * hildon_touch_selector_new:
 *
 * Creates a new empty #HildonTouchSelector.
 *
 * Returns: a new #HildonTouchSelector.
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_touch_selector_new (void)
{
  return g_object_new (HILDON_TYPE_TOUCH_SELECTOR, NULL);
}

/**
 * hildon_touch_selector_new_text:
 *
 * Creates a #HildonTouchSelector with a single text column that
 * can be populated conveniently through hildon_touch_selector_append_text(),
 * hildon_touch_selector_prepend_text(), hildon_touch_selector_insert_text().
 *
 * Returns: A new #HildonTouchSelector
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_touch_selector_new_text (void)
{
  GtkWidget *selector;
  GtkListStore *store;

  selector = hildon_touch_selector_new ();
  store = gtk_list_store_new (1, G_TYPE_STRING);

  hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector),
                                            GTK_TREE_MODEL (store), TRUE);

  g_object_unref (store);

  return selector;
}

/**
 * hildon_touch_selector_append_text:
 * @selector: A #HildonTouchSelector.
 * @text: a non %NULL text string.
 *
 * Appends a new entry in a #HildonTouchSelector created with
 * hildon_touch_selector_new_text().
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_append_text (HildonTouchSelector * selector,
                                   const gchar * text)
{
  GtkTreeModel *model;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail (text != NULL);

  model = hildon_touch_selector_get_model (HILDON_TOUCH_SELECTOR (selector), 0);

  g_return_if_fail (GTK_IS_LIST_STORE (model));

  gtk_list_store_insert_with_values (GTK_LIST_STORE (model),
                                     NULL, G_MAXINT, 0, text, -1);
}

/**
 * hildon_touch_selector_prepend_text:
 * @selector: A #HildonTouchSelector.
 * @text: a non %NULL text string.
 *
 * Prepends a new entry in a #HildonTouchSelector created with
 * hildon_touch_selector_new_text().
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_prepend_text (HildonTouchSelector * selector,
                                    const gchar * text)
{
  GtkTreeModel *model;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail (text != NULL);

  model = hildon_touch_selector_get_model (HILDON_TOUCH_SELECTOR (selector), 0);

  g_return_if_fail (GTK_IS_LIST_STORE (model));

  gtk_list_store_insert_with_values (GTK_LIST_STORE (model),
                                     NULL, 0, 0, text, -1);
}

/**
 * hildon_touch_selector_insert_text:
 * @selector: a #HildonTouchSelector.
 * @position: the position to insert @text.
 * @text: A non %NULL text string.
 *
 * Inserts a new entry in a particular position of a
 * #HildonTouchSelector created with hildon_touch_selector_new_text().
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_insert_text (HildonTouchSelector * selector,
                                   gint position, const gchar * text)
{
  GtkTreeModel *model;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail (text != NULL);
  g_return_if_fail (position >= 0);

  model = hildon_touch_selector_get_model (HILDON_TOUCH_SELECTOR (selector), 0);

  g_return_if_fail (GTK_IS_LIST_STORE (model));

  gtk_list_store_insert_with_values (GTK_LIST_STORE (model),
                                     NULL, position, 0, text, -1);
}

static void
hildon_touch_selector_add_live_search (HildonTouchSelector *selector,
                                       HildonTouchSelectorColumn *column)
{
  if (column->priv->livesearch == NULL) {
    gint text_column;

    column->priv->livesearch = hildon_live_search_new ();
    hildon_live_search_set_filter (HILDON_LIVE_SEARCH (column->priv->livesearch),
                                   GTK_TREE_MODEL_FILTER (column->priv->filter));
    g_signal_connect (column->priv->livesearch, "refilter",
                      G_CALLBACK (on_live_search_refilter), selector);
    gtk_box_pack_start (GTK_BOX (column->priv->vbox),
                        column->priv->livesearch,
                        FALSE, FALSE, 0);
    hildon_live_search_widget_hook (HILDON_LIVE_SEARCH (column->priv->livesearch),
                                    GTK_WIDGET (column->priv->vbox),
                                    GTK_WIDGET (column->priv->tree_view));
    text_column = hildon_touch_selector_column_get_text_column (column);
    if (text_column > -1) {
      hildon_live_search_set_visible_func (HILDON_LIVE_SEARCH (column->priv->livesearch),
                                           hildon_live_search_visible_func,
                                           column,
                                           NULL);
    }
  }
}

static void
hildon_touch_selector_remove_live_search (HildonTouchSelector *selector)
{
  HildonTouchSelectorColumn *col;

  if (selector->priv->has_live_search == FALSE ||
      selector->priv->columns == NULL)
    return;

  col = (HildonTouchSelectorColumn *) selector->priv->columns->data;

  if (col->priv->livesearch != NULL) {
    hildon_live_search_widget_unhook (HILDON_LIVE_SEARCH (col->priv->livesearch));
    gtk_widget_destroy (col->priv->livesearch);
    col->priv->livesearch = NULL;
  }

  selector->priv->has_live_search = FALSE;
}

/**
 * hildon_touch_selector_append_column
 * @selector: a #HildonTouchSelector
 * @model: the #GtkTreeModel with the data of the column
 * @cell_renderer: The #GtkCellRenderer where to draw each row contents.
 * @Varargs: a %NULL-terminated pair of attributes and column numbers.
 *
 * This functions adds a new column to the widget, whose data will
 * be obtained from the model. Only widgets added this way should used on
 * the selection logic, the print function, the #HildonTouchSelector::changed
 * signal, etc. Internally, a #GtkTreeView will be added to the widget, using
 * @model as the data source. This will add a reference to @model, so you
 * must unref it after calling this function if you are not going to
 * use it anymore.
 *
 * You can optionally pass a #GtkCellRenderer in @cell_renderer,
 * together with a %NULL-terminated list of pairs property/value, in
 * the same way you would use gtk_tree_view_column_set_attributes().
 * This will pack @cell_renderer at the start of the column, expanded
 * by default.  If you prefer not to add it this way, you can simply
 * pass %NULL to @cell_renderer and use the #GtkCellLayout interface
 * on the returned #HildonTouchSelectorColumn to set your
 * renderers. Please note that the returned #HildonTouchSelectorColumn
 * is owned by @selector, you shouldn't unref it after setting it
 * up.
 *
 * Initially, the returned #HildonTouchSelectorColumn will have its
 * #HildonTouchSelectorColumn:text-column property unset. You should set
 * it to a column in @model with type %G_TYPE_STRING. See
 * hildon_touch_selector_column_set_text_column().
 *
 * This method could change the current #HildonTouchSelector:hildon-ui-mode.
 * %HILDON_UI_MODE_NORMAL is only allowed with one column, so if the selector
 * is in this mode, and a additional column is added,
 * #HildonTouchSelector:hildon-ui-mode will change to %HILDON_UI_MODE_EDIT.
 *
 * Returns: the new column added added, %NULL otherwise.
 *
 * Since: 2.2
 **/

HildonTouchSelectorColumn*
hildon_touch_selector_append_column (HildonTouchSelector * selector,
                                     GtkTreeModel * model,
                                     GtkCellRenderer * cell_renderer, ...)
{
  va_list args;
  HildonTouchSelectorColumn *new_column = NULL;
  gboolean emit_changed = FALSE;
  gint colnum;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), NULL);
  g_return_val_if_fail (GTK_IS_TREE_MODEL (model), NULL);

  if (model != NULL) {

    va_start (args, cell_renderer);
    new_column = _create_new_column (selector, model, &emit_changed, cell_renderer, args);
    va_end (args);

    /* If we already have one column, disable live search */
    if (selector->priv->has_live_search &&
        selector->priv->columns != NULL &&
        selector->priv->columns->next == NULL) {
	    hildon_touch_selector_remove_live_search (selector);
    }

    selector->priv->columns = g_slist_append (selector->priv->columns,
                                              new_column);

    new_column->priv->vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (new_column->priv->vbox),
                        new_column->priv->panarea,
                        TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (selector->priv->hbox),
                        new_column->priv->vbox,
                        TRUE, TRUE, 6);

    if (selector->priv->has_live_search) {
      hildon_touch_selector_add_live_search (selector, new_column);
    }

    gtk_widget_show_all (new_column->priv->vbox);

    gtk_widget_child_focus (GTK_WIDGET (selector),
                            GTK_DIR_TAB_FORWARD);

    if (selector->priv->initial_scroll) {
      _hildon_touch_selector_center_on_selected_items (selector, new_column);
    }

  } else {
    return NULL;
  }

  g_signal_emit (selector, hildon_touch_selector_signals[COLUMNS_CHANGED], 0);
  if (emit_changed) {
    colnum = g_slist_length (selector->priv->columns);
    hildon_touch_selector_emit_value_changed (selector, colnum);
  }

  hildon_touch_selector_check_ui_mode_coherence (selector);

  return new_column;
}

/**
 * hildon_touch_selector_append_text_column
 * @selector: a #HildonTouchSelector
 * @model: a #GtkTreeModel with data for the column
 * @center: whether to center the text on the column
 *
 * Equivalent to hildon_touch_selector_append_column(), but using a
 * default text cell renderer. This is the most common use case of the
 * widget.
 *
 * Note that this call adds a reference to @model, so you must unref
 * it afterwards if you are not going to use it anymore.
 *
 * Returns: the new column added, NULL otherwise.
 *
 * Since: 2.2
 **/
HildonTouchSelectorColumn*
hildon_touch_selector_append_text_column (HildonTouchSelector * selector,
                                          GtkTreeModel * model, gboolean center)
{
  gfloat xalign = center ? 0.5 : 0.0;
  GtkCellRenderer *renderer;
  HildonTouchSelectorColumn *column = NULL;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), NULL);
  g_return_val_if_fail (GTK_IS_TREE_MODEL (model), NULL);

  renderer = gtk_cell_renderer_text_new ();

  g_object_set (renderer,
                "width", 1,
                "xalign", xalign,
                NULL);

  column = hildon_touch_selector_append_column (selector, model, renderer,
                                                "text", 0, NULL);
  hildon_touch_selector_column_set_text_column (column, 0);

  return column;
}

/**
 * hildon_touch_selector_remove_column:
 * @selector: a #HildonTouchSelector
 * @column: the position of the column to be removed
 *
 * Removes a column from @selector.
 *
 * Returns: %TRUE if the column was removed, %FALSE otherwise
 *
 * Since: 2.2
 **/
gboolean
hildon_touch_selector_remove_column (HildonTouchSelector * selector, gint column)
{
  HildonTouchSelectorColumn *current_column = NULL;
  HildonTouchSelectorPrivate *priv;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), FALSE);
  g_return_val_if_fail (column <
                        hildon_touch_selector_get_num_columns (selector), FALSE);

  priv = HILDON_TOUCH_SELECTOR_GET_PRIVATE (selector);
  current_column = g_slist_nth_data (priv->columns, column);

  gtk_container_remove (GTK_CONTAINER (priv->hbox), current_column->priv->vbox);
  priv->columns = g_slist_remove (priv->columns, current_column);
  g_object_unref (current_column);

  g_signal_emit (selector, hildon_touch_selector_signals[COLUMNS_CHANGED], 0);

  return TRUE;
}

/**
 * hildon_touch_selector_set_column_attributes:
 * @selector: a #HildonTouchSelector
 * @num_column: the number of the column whose attributes we're setting
 * @cell_renderer: the #GtkCellRendere we're setting the attributes of
 * @Varargs: A %NULL-terminated list of attributes.
 *
 * Sets the attributes for the given column. The attributes must be given
 * in attribute/column pairs, just like in gtk_tree_view_column_set_attributes().
 * All existing attributes are removed and replaced with the new ones.
 *
 * Deprecated: #HildonTouchSelectorColumn implements #GtkCellLayout, use this
 *             interface instead. See
 *             hildon_touch_selector_get_column().
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_set_column_attributes (HildonTouchSelector * selector,
                                             gint num_column,
                                             GtkCellRenderer * cell_renderer,
                                             ...)
{
  va_list args;
  GtkTreeViewColumn *tree_column = NULL;
  HildonTouchSelectorColumn *current_column = NULL;
  gchar *attribute = NULL;
  gint value = 0;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail (num_column <
                    hildon_touch_selector_get_num_columns (selector));

  current_column = g_slist_nth_data (selector->priv->columns, num_column);

  tree_column = gtk_tree_view_get_column (current_column->priv->tree_view, 0);
  gtk_tree_view_remove_column (current_column->priv->tree_view, tree_column);

  tree_column = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (tree_column, cell_renderer, TRUE);

  va_start (args, cell_renderer);
  attribute = va_arg (args, gchar *);

  gtk_tree_view_column_clear_attributes (tree_column, cell_renderer);

  while (attribute != NULL) {
    value = va_arg (args, gint);
    gtk_tree_view_column_add_attribute (tree_column, cell_renderer,
                                        attribute, value);
    attribute = va_arg (args, gchar *);
  }

  va_end (args);

  gtk_tree_view_append_column (current_column->priv->tree_view, tree_column);
}

/**
 * hildon_touch_selector_get_num_columns:
 * @selector: a #HildonTouchSelector
 *
 * Gets the number of columns in the #HildonTouchSelector.
 *
 * Returns: the number of columns in @selector.
 *
 * Since: 2.2
 **/
gint
hildon_touch_selector_get_num_columns (HildonTouchSelector * selector)
{
  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), -1);

  return g_slist_length (selector->priv->columns);
}

/**
 * hildon_touch_selector_get_column_selection_mode:
 * @selector: a #HildonTouchSelector
 *
 * Gets the selection mode of @selector.
 *
 * Returns: one of #HildonTouchSelectorSelectionMode
 *
 * Since: 2.2
 **/
HildonTouchSelectorSelectionMode
hildon_touch_selector_get_column_selection_mode (HildonTouchSelector * selector)
{
  HildonTouchSelectorSelectionMode result =
    HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE;
  GtkSelectionMode treeview_mode = GTK_SELECTION_BROWSE;
  HildonTouchSelectorColumn *column = NULL;
  GtkTreeSelection *selection = NULL;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), result);
  g_return_val_if_fail (hildon_touch_selector_get_num_columns (selector) > 0,
                        result);

  column = HILDON_TOUCH_SELECTOR_COLUMN (selector->priv->columns->data);

  selection = gtk_tree_view_get_selection (column->priv->tree_view);
  treeview_mode = gtk_tree_selection_get_mode (selection);


  if (treeview_mode == GTK_SELECTION_MULTIPLE) {
    result = HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE;
  } else {
    result = HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE;
  }

  return result;
}

/**
 * hildon_touch_selector_set_column_selection_mode:
 * @selector: a #HildonTouchSelector
 * @mode: the #HildonTouchSelectorMode for @selector
 *
 * Sets the selection mode for @selector. See #HildonTouchSelectorSelectionMode.
 *
 * The new @mode will be set, but take into into account that the
 * #HildonTouchSelectorSelectionMode is ignored if the @selector
 * #HildonTouchSelector:hildon-ui-mode property is set to %HILDON_UI_MODE_NORMAL
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_set_column_selection_mode (HildonTouchSelector * selector,
                                                 HildonTouchSelectorSelectionMode mode)
{
  GtkTreeView *tv = NULL;
  HildonTouchSelectorColumn *column = NULL;
  GtkTreeSelection *selection = NULL;
  GtkSelectionMode treeview_mode = GTK_SELECTION_MULTIPLE;
  GtkTreeIter iter;
  HildonTouchSelectorSelectionMode current_mode;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail (hildon_touch_selector_get_num_columns (selector) > 0);

  current_mode = hildon_touch_selector_get_column_selection_mode (selector);

  if (current_mode == mode) {
    return;
  }

  column = HILDON_TOUCH_SELECTOR_COLUMN ((g_slist_nth (selector->priv->columns, 0))->data);
  tv = column->priv->tree_view;

  if (tv) {
    switch (mode) {
    case HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE:
      treeview_mode = GTK_SELECTION_BROWSE;
      break;
    case HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE:
      treeview_mode = GTK_SELECTION_MULTIPLE;
      break;
    }

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
    gtk_tree_selection_set_mode (selection, treeview_mode);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tv));
    gtk_tree_selection_unselect_all (selection);
    if (gtk_tree_model_get_iter_first (column->priv->filter, &iter)) {
        gtk_tree_selection_select_iter (selection, &iter);
    }

    /* the column changed was the first one */
    hildon_touch_selector_emit_value_changed (selector, 0);
  }

}

/**
 * hildon_touch_selector_set_print_func:
 * @selector: a #HildonTouchSelector
 * @func: a #HildonTouchSelectorPrintFunc function
 *
 * Sets the function to be used by hildon_touch_selector_get_current_text().
 * See hildon_touch_selector_set_print_func_full().
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_set_print_func (HildonTouchSelector * selector,
                                      HildonTouchSelectorPrintFunc func)
{
  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));

  hildon_touch_selector_set_print_func_full (selector, func, NULL, NULL);
}

/**
 * hildon_touch_selector_set_print_func_full:
 * @selector: a #HildonTouchSelector
 * @func: a #HildonTouchSelectorPrintFunc function
 * @user_data: a pointer to user data or %NULL
 * @destroy_func: a callback for freeing the user data or %NULL
 *
 * Sets the function to be used by hildon_touch_selector_get_current_text()
 * to produce a text representation of the currently selected items in @selector.
 * The default function will return a concatenation of comma separated items
 * selected in each column in @selector. Use this to override this method if you
 * need a particular representation for your application.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_set_print_func_full (HildonTouchSelector          *selector,
                                           HildonTouchSelectorPrintFunc  func,
                                           gpointer                      user_data,
                                           GDestroyNotify                destroy_func)
{
  gpointer       old_user_data;
  GDestroyNotify old_destroy_func;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));

  old_user_data = selector->priv->print_user_data;
  old_destroy_func = selector->priv->print_destroy_func;

  selector->priv->print_func = func;
  selector->priv->print_user_data = user_data;
  selector->priv->print_destroy_func = destroy_func;

  if (old_destroy_func && old_user_data != user_data)
    (*old_destroy_func) (old_user_data);
}

/**
 * hildon_touch_selector_get_print_func:
 * @selector: a #HildonTouchSelector
 *
 * Gets the #HildonTouchSelectorPrintFunc currently used. See
 * hildon_touch_selector_set_print_func().
 *
 * Returns: a #HildonTouchSelectorPrintFunc or %NULL if the default
 * one is currently used.
 **/
HildonTouchSelectorPrintFunc
hildon_touch_selector_get_print_func (HildonTouchSelector * selector)
{
  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), NULL);

  return selector->priv->print_func;
}

/**
 * hildon_touch_selector_set_active:
 * @selector: a #HildonTouchSelector
 * @column: column number
 * @index: the index of the item to select, or -1 to have no active item
 *
 * Sets the active item of the #HildonTouchSelector to @index. The
 * column number is taken from @column.
 *
 * @selector must be in %HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_set_active                (HildonTouchSelector *selector,
                                                 gint                 column,
                                                 gint                 index)
{
  GtkTreeSelection *selection = NULL;
  HildonTouchSelectorColumn *current_column = NULL;
  HildonTouchSelectorSelectionMode mode;
  GtkTreePath *path, *filter_path;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail (column < hildon_touch_selector_get_num_columns (selector));
  mode = hildon_touch_selector_get_column_selection_mode (selector);
  g_return_if_fail (mode == HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE);

  current_column = g_slist_nth_data (selector->priv->columns, column);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (current_column->priv->tree_view));

  if (index == -1) {
      gtk_tree_selection_unselect_all (selection);
      hildon_touch_selector_emit_value_changed (selector, column);
      return;
  }

  path = gtk_tree_path_new_from_indices (index, -1);
  filter_path = gtk_tree_model_filter_convert_child_path_to_path (
      GTK_TREE_MODEL_FILTER (current_column->priv->filter), path);

  if (filter_path != NULL) {
      gtk_tree_selection_unselect_all (selection);
      gtk_tree_selection_select_path (selection, filter_path);

      hildon_touch_selector_emit_value_changed (selector, column);
      gtk_tree_path_free (filter_path);
  }
  gtk_tree_path_free (path);
}

/**
 * hildon_touch_selector_get_active:
 * @selector: a #HildonTouchSelector
 * @column: column number
 *
 * Returns the index of the currently active item in column number
 * @column, or -1 if there's no active item.
 *
 * @selector must be in %HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE
 *
 * Returns: an integer which is the index of the currently active
 * item, or -1 if there's no active item.
 *
 * Since: 2.2
 **/
gint
hildon_touch_selector_get_active                (HildonTouchSelector *selector,
                                                 gint                 column)
{
  GtkTreeSelection *selection = NULL;
  HildonTouchSelectorColumn *current_column = NULL;
  HildonTouchSelectorSelectionMode mode;
  GtkTreeIter filter_iter;
  gint index = -1;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), -1);
  g_return_val_if_fail (column < hildon_touch_selector_get_num_columns (selector), -1);

  mode = hildon_touch_selector_get_column_selection_mode (selector);
  g_return_val_if_fail (mode == HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE, -1);

  current_column = g_slist_nth_data (selector->priv->columns, column);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (current_column->priv->tree_view));

  if (gtk_tree_selection_get_selected (selection, NULL, &filter_iter)) {
    GtkTreePath *path;
    GtkTreeIter iter;
    gtk_tree_model_filter_convert_iter_to_child_iter (GTK_TREE_MODEL_FILTER (current_column->priv->filter),
                                                      &iter, &filter_iter);
    path = gtk_tree_model_get_path (current_column->priv->model, &iter);
    index = (gtk_tree_path_get_indices (path))[0];
    gtk_tree_path_free (path);
  }

  return index;
}

/**
 * hildon_touch_selector_get_selected:
 * @selector: a #HildonTouchSelector
 * @column: the column number we want to get the element
 * @iter: #GtkTreeIter currently selected
 *
 * Sets @iter to the currently selected node on the nth-column, if selection is
 * set to %HILDON_TOUCH_SELECTOR_SINGLE or %HILDON_TOUCH_SELECTOR_MULTIPLE with
 * a column different that the first one. @iter may be %NULL if you just want to
 * test if selection has any selected items.
 *
 * This function will not work if selection is in
 * %HILDON_TOUCH_SELECTOR_MULTIPLE mode and the column is the first one.
 *
 * See gtk_tree_selection_get_selected() for more information.
 *
 * Returns: %TRUE if @iter was correctly set, %FALSE otherwise
 *
 * Since: 2.2
 **/
gboolean
hildon_touch_selector_get_selected (HildonTouchSelector * selector,
                                    gint column, GtkTreeIter * iter)
{
  GtkTreeSelection *selection = NULL;
  HildonTouchSelectorColumn *current_column = NULL;
  HildonTouchSelectorSelectionMode mode;
  GtkTreeIter filter_iter;
  gboolean result;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), FALSE);
  g_return_val_if_fail (column < hildon_touch_selector_get_num_columns (selector),
                        FALSE);
  mode = hildon_touch_selector_get_column_selection_mode (selector);
  g_return_val_if_fail
    ((mode == HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE) ||
     ((mode == HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE)&&(column>0)),
     FALSE);

  current_column = g_slist_nth_data (selector->priv->columns, column);

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (current_column->priv->tree_view));

  result = gtk_tree_selection_get_selected (selection, NULL, &filter_iter);

  if (iter) {
    if (result == TRUE) {
      gtk_tree_model_filter_convert_iter_to_child_iter
        (GTK_TREE_MODEL_FILTER (current_column->priv->filter),
         iter, &filter_iter);
    } else {
      memset (iter, 0, sizeof (GtkTreeIter));
    }
  }

  return result;
}

/**
 * hildon_touch_selector_select_iter
 * @selector: a #HildonTouchSelector
 * @column:   the column to selects
 * @iter:     the #GtkTreeIter to be selected
 * @scroll_to: whether to smoothly scroll to the item
 *
 * Sets the currently selected item in the column @column to the one pointed by @iter,
 * optionally smoothly scrolling to it.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_select_iter (HildonTouchSelector * selector,
                                   gint column, GtkTreeIter * iter,
                                   gboolean scroll_to)
{
  GtkTreePath *filter_path;
  GtkTreeIter filter_iter;
  HildonTouchSelectorColumn *current_column = NULL;
  GtkTreeView *tv = NULL;
  GtkTreeSelection *selection = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail (column < hildon_touch_selector_get_num_columns (selector));

  current_column = g_slist_nth_data (selector->priv->columns, column);

  tv = current_column->priv->tree_view;
  selection = gtk_tree_view_get_selection (tv);

  /* The given iter might be not visible, due to the
     GtkTreeModelFilter we use. If so, don't change the selection. */
  if (gtk_tree_model_filter_convert_child_iter_to_iter (GTK_TREE_MODEL_FILTER (current_column->priv->filter),
                                                        &filter_iter, iter) == FALSE)
          return;

  filter_path = gtk_tree_model_get_path (current_column->priv->filter, &filter_iter);

  gtk_tree_selection_select_iter (selection, &filter_iter);

  if (scroll_to) {
    hildon_touch_selector_scroll_to (current_column, tv, filter_path);
  }

  hildon_touch_selector_emit_value_changed (selector, column);

  gtk_tree_path_free (filter_path);
}

/**
 * hildon_touch_selector_unselect_iter
 * @selector: a #HildonTouchSelector
 * @column:   the column to unselects from
 * @iter:     the #GtkTreeIter to be unselected
 *
 * Unselect the item pointed by @iter in the column @column
 *
 * Since: 2.2
 **/

void hildon_touch_selector_unselect_iter (HildonTouchSelector * selector,
                                          gint column,
                                          GtkTreeIter * iter)
{
  HildonTouchSelectorColumn *current_column = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter filter_iter;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail (column < hildon_touch_selector_get_num_columns (selector));

  current_column = g_slist_nth_data (selector->priv->columns, column);
  selection = gtk_tree_view_get_selection (current_column->priv->tree_view);
  if (gtk_tree_model_filter_convert_child_iter_to_iter (GTK_TREE_MODEL_FILTER (current_column->priv->filter),
                                                        &filter_iter, iter) == FALSE)
    return;

  gtk_tree_selection_unselect_iter (selection, &filter_iter);
  hildon_touch_selector_emit_value_changed (selector, column);
}

/**
 * hildon_touch_selector_unselect_all:
 * @selector: a #HildonTouchSelector
 * @column: the position of the column to get the selected rows from
 *
 * Unselects all the selected items in the column @column.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_unselect_all (HildonTouchSelector * selector,
                                    gint column)
{
  HildonTouchSelectorColumn *current_column = NULL;
  GtkTreeSelection *selection = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail (column < hildon_touch_selector_get_num_columns (selector));

  current_column = g_slist_nth_data (selector->priv->columns, column);
  selection = gtk_tree_view_get_selection (current_column->priv->tree_view);
  gtk_tree_selection_unselect_all (selection);

  hildon_touch_selector_emit_value_changed (selector, column);
}

/**
 * hildon_touch_selector_filter_selected_to_child_selected:
 * @filter: 
 * @filter_selected: 
 *
 * Converts a list of #GtkTreePath<!-- -->s from the internal
 * #GtkTreeModelFilter to the child #GtkTreeModel.
 *
 * Returns: a newly created list of #GtkTreePath<!-- -->s. The list
 * and paths should be freed when not needed anymore.
 **/
static GList *
hildon_touch_selector_filter_selected_to_child_selected (GtkTreeModelFilter *filter,
                                                         GList *filter_selected)
{
    GList *iter;
    GList *child_selected = NULL;

    for (iter = filter_selected; iter; iter = iter->next)
        child_selected = g_list_append (child_selected,
                                        gtk_tree_model_filter_convert_path_to_child_path (
                                            filter, (GtkTreePath *)iter->data));

    return child_selected;
}

/**
 * hildon_touch_selector_get_selected_rows:
 * @selector: a #HildonTouchSelector
 * @column: the position of the column to get the selected rows from
 *
 * Creates a list of #GtkTreePath<!-- -->s of all selected rows in a column. Additionally,
 * if you to plan to modify the model after calling this function, you may
 * want to convert the returned list into a list of GtkTreeRowReferences. To do this,
 * you can use gtk_tree_row_reference_new().
 *
 * See gtk_tree_selection_get_selected_rows() for more information.
 *
 * Returns: A new #GList containing a #GtkTreePath for each selected row in the column @column.
 *
 * Since: 2.2
 **/
GList *
hildon_touch_selector_get_selected_rows (HildonTouchSelector * selector,
                                         gint column)
{
  GList *result;
  GList *filter_selected;
  HildonTouchSelectorColumn *current_column = NULL;
  GtkTreeSelection *selection = NULL;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), NULL);
  g_return_val_if_fail (column < hildon_touch_selector_get_num_columns (selector),
                        NULL);

  current_column = g_slist_nth_data (selector->priv->columns, column);
  selection = gtk_tree_view_get_selection (current_column->priv->tree_view);

  filter_selected = gtk_tree_selection_get_selected_rows (selection, NULL);
  result = hildon_touch_selector_filter_selected_to_child_selected
      (GTK_TREE_MODEL_FILTER (current_column->priv->filter),
       filter_selected);
  g_list_foreach (filter_selected, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (filter_selected);

  return result;
}

/**
 * hildon_touch_selector_get_model:
 * @selector: a #HildonTouchSelector
 * @column: the position of the column in @selector
 *
 * Gets the model of a column of @selector.
 *
 * Returns: the #GtkTreeModel for the column @column of @selector.
 *
 * Since: 2.2
 **/
GtkTreeModel *
hildon_touch_selector_get_model (HildonTouchSelector * selector, gint column)
{
  HildonTouchSelectorColumn *current_column = NULL;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), NULL);
  g_return_val_if_fail (column < hildon_touch_selector_get_num_columns (selector),
                        NULL);

  current_column = g_slist_nth_data (selector->priv->columns, column);

  return current_column->priv->model;
}

static void
on_row_changed (GtkTreeModel *model,
                GtkTreePath *path,
                GtkTreeIter *iter,
                gpointer userdata)
{
  HildonTouchSelector *selector;
  HildonTouchSelectorColumn *current_column;
  GtkTreePath *filter_path;

  gint column = 0;
  GSList *col;

  selector = HILDON_TOUCH_SELECTOR (userdata);

  for (col = selector->priv->columns; col != NULL; col = col->next) {
    current_column = HILDON_TOUCH_SELECTOR_COLUMN (col->data);
    if (current_column->priv->model == model) {
        filter_path =
            gtk_tree_model_filter_convert_child_path_to_path (GTK_TREE_MODEL_FILTER (current_column->priv->filter),
                                                              path);
        if (filter_path &&
            gtk_tree_selection_path_is_selected (gtk_tree_view_get_selection (current_column->priv->tree_view),
                                                 filter_path)) {
            hildon_touch_selector_emit_value_changed (selector, column);
        }
        gtk_tree_path_free (filter_path);
    }
    column ++;
  }
}

static void
on_row_deleted (GtkTreeModel *model,
                GtkTreePath *path,
                gpointer userdata)
{
  HildonTouchSelector *selector = HILDON_TOUCH_SELECTOR (userdata);
  gint column = 0;
  GSList *col = selector->priv->columns;

  while (col != NULL) {
    HildonTouchSelectorColumn *current_column;
    current_column = HILDON_TOUCH_SELECTOR_COLUMN (col->data);
    if (current_column->priv->model == model) {
      GtkTreeSelection *sel = gtk_tree_view_get_selection (current_column->priv->tree_view);
      if (gtk_tree_selection_get_mode (sel) == GTK_SELECTION_BROWSE &&
          gtk_tree_model_iter_n_children (model, NULL) > 0 &&
          gtk_tree_selection_count_selected_rows (sel) == 0) {
        GtkTreeIter iter;
        GtkTreePath *first;

        gtk_tree_model_get_iter_first (current_column->priv->filter, &iter);
        first = gtk_tree_model_get_path (current_column->priv->filter, &iter);
        gtk_tree_selection_select_path (sel, first);
        gtk_tree_path_free (first);
      }
      hildon_touch_selector_emit_value_changed (selector, column);
    }
    col = col->next;
    column ++;
  }
}

static void
_hildon_touch_selector_set_model (HildonTouchSelector * selector,
                                  gint column, GtkTreeModel * model)
{
  HildonTouchSelectorColumn *current_column = NULL;

  current_column =
    HILDON_TOUCH_SELECTOR_COLUMN (g_slist_nth_data (selector->priv->columns, column));

  if (current_column->priv->model) {
    g_signal_handlers_disconnect_by_func (current_column->priv->model,
                                          on_row_changed, selector);
    g_signal_handlers_disconnect_by_func (current_column->priv->model,
                                          on_row_deleted, selector);
    g_object_unref (current_column->priv->model);
  }

  current_column->priv->model = g_object_ref (model);

  if (current_column->priv->filter) {
    g_object_unref (current_column->priv->filter);
  }

  current_column->priv->filter = gtk_tree_model_filter_new (model, NULL);
  gtk_tree_view_set_model (current_column->priv->tree_view,
                           current_column->priv->filter);

  g_signal_connect (model, "row-changed",
                    G_CALLBACK (on_row_changed), selector);
  g_signal_connect_after (model, "row-deleted",
                          G_CALLBACK (on_row_deleted), selector);
}

/**
 * hildon_touch_selector_set_model:
 * @selector: a #HildonTouchSelector
 * @column: the position of the column to set the model to
 * @model: a #GtkTreeModel. It increases a reference so you are not
 * going to use the model anymore, unref it.
 *
 * Sets the #GtkTreeModel for a particular column in @model.
 *
 * Note that this call adds a reference to @model, so you must unref
 * it afterwards if you are not going to use it anymore.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_set_model (HildonTouchSelector * selector,
                                 gint column, GtkTreeModel * model)
{
  g_return_if_fail (HILDON_TOUCH_SELECTOR (selector));
  g_return_if_fail (column < hildon_touch_selector_get_num_columns (selector));

  HILDON_TOUCH_SELECTOR_GET_CLASS (selector)->set_model (selector, column, model);
}

/**
 * hildon_touch_selector_get_current_text:
 * @selector: a #HildonTouchSelector
 *
 * Returns a string representing the currently selected items for
 * each column of @selector. See hildon_touch_selector_set_print_func().
 *
 * Returns: a newly allocated string.
 *
 * Since: 2.2
 **/
gchar *
hildon_touch_selector_get_current_text (HildonTouchSelector * selector)
{
  gchar *result = NULL;
  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), NULL);

  if (selector->priv->print_func) {
    result = (*selector->priv->print_func) (selector, selector->priv->print_user_data);
  } else {
    result = _default_print_func (selector, NULL);
  }

  return result;
}

static void
search_nearest_element (HildonPannableArea *panarea,
                        GtkTreeView *tv,
                        GList *selected_rows,
                        GtkTreePath **nearest_path)
{
  GtkAdjustment *adj = NULL;
  gdouble target_value = 0;
  GdkRectangle rect;
  GList *iter = NULL;
  GtkTreePath *path = NULL;
  gint y = -1;
  gdouble nearest_distance = -1;
  gdouble current_distance = -1;
  GtkTreePath *result_path = NULL;

  g_assert (nearest_path != NULL);

  if (selected_rows == NULL) {
    *nearest_path = NULL;
    return;
  }

  adj = hildon_pannable_area_get_vadjustment (panarea);
  g_return_if_fail (adj != NULL);

  /* we add this in order to check the nearest to the center of
     the visible area */
  target_value = gtk_adjustment_get_value (adj) + adj->page_size/2;

  path = result_path = selected_rows->data;
  gtk_tree_view_get_background_area (tv, path, NULL, &rect);
  gtk_tree_view_convert_bin_window_to_tree_coords (tv, 0, rect.y, NULL, &y);
  nearest_distance = abs (target_value - y);

  for (iter = selected_rows->next; iter; iter = g_list_next (iter)) {
    gtk_tree_view_get_background_area (tv, path, NULL, &rect);
    gtk_tree_view_convert_bin_window_to_tree_coords (tv, 0, rect.y, NULL, &y);
    current_distance = abs (target_value - y);
    if (current_distance < nearest_distance) {
      nearest_distance = current_distance;
      result_path = path;
    }
  }

  *nearest_path = result_path;
}

static gboolean
on_realize_cb                                  (GtkWidget *widget,
                                                gpointer data)
{
  HildonTouchSelectorColumn *column = NULL;
  GdkRectangle rect;
  gint y;

  column = HILDON_TOUCH_SELECTOR_COLUMN (data);

  if (column->priv->initial_path) {
    gtk_tree_view_get_background_area (GTK_TREE_VIEW (column->priv->tree_view),
                                       column->priv->initial_path, NULL, &rect);
    gtk_tree_view_convert_bin_window_to_tree_coords
      (GTK_TREE_VIEW (column->priv->tree_view),
       0, rect.y, NULL, &y);

    hildon_pannable_area_scroll_to (HILDON_PANNABLE_AREA (column->priv->panarea),
                                    -1, y);

    gtk_tree_path_free (column->priv->initial_path);

    column->priv->initial_path = NULL;

  }

  g_signal_handler_disconnect (column->priv->panarea,
                               column->priv->realize_handler);

  return FALSE;
}

static void
hildon_touch_selector_scroll_to (HildonTouchSelectorColumn *column,
                                 GtkTreeView *tv,
                                 GtkTreePath *path)
{
  if (GTK_WIDGET_REALIZED (column->priv->panarea)) {
    GdkRectangle rect;
    gint y;

    gtk_tree_view_get_background_area (tv,
                                       path, NULL, &rect);
    gtk_tree_view_convert_bin_window_to_tree_coords (tv,
                                                     0, rect.y, NULL, &y);

    hildon_pannable_area_scroll_to (HILDON_PANNABLE_AREA
                                    (column->priv->panarea), -1, y);
  } else {
    if (column->priv->realize_handler != 0) {

      if (column->priv->initial_path) {
        gtk_tree_path_free (column->priv->initial_path);
        column->priv->initial_path = NULL;
      }

      g_signal_handler_disconnect (column->priv->panarea,
                                   column->priv->realize_handler);
      column->priv->realize_handler = 0;
    }

    column->priv->initial_path = gtk_tree_path_copy (path);
    column->priv->realize_handler =
      g_signal_connect_after (G_OBJECT (column->priv->panarea), "realize",
                              G_CALLBACK (on_realize_cb),
                              column);
  }
}

/**
 *
 * Center on the selected item of a concrete column
 *
 * Returns: TRUE if was able to do that
 *          FALSE otherwise
 */
static gboolean
_hildon_touch_selector_center_on_selected_items (HildonTouchSelector *selector,
                                                 HildonTouchSelectorColumn *column)
{
  GtkTreePath *path = NULL;
  GList *selected_rows = NULL;
  gint num_column = -1;

  num_column = g_slist_index (selector->priv->columns, column);

  selected_rows = hildon_touch_selector_get_selected_rows (selector, num_column);
  if (selected_rows) {
    search_nearest_element (HILDON_PANNABLE_AREA (column->priv->panarea),
                             GTK_TREE_VIEW (column->priv->tree_view),
                             selected_rows,
                             &path);

    if (path != NULL) {
      hildon_touch_selector_scroll_to (column,
                                       GTK_TREE_VIEW (column->priv->tree_view),
                                       path);
    } else {
      return FALSE;
    }

    g_list_foreach (selected_rows, (GFunc) (gtk_tree_path_free), NULL);
    g_list_free (selected_rows);
  }

  return TRUE;
}

static gboolean
_hildon_touch_selector_has_multiple_selection (HildonTouchSelector * selector)
{
  HildonTouchSelectorSelectionMode mode;
  gint n_columns;

  n_columns = hildon_touch_selector_get_num_columns (selector);
  mode = hildon_touch_selector_get_column_selection_mode (selector);

  return ((n_columns > 1) || (mode == HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE));
}

/**
 * hildon_touch_selector_has_multiple_selection:
 * @selector: A #HildonTouchSelector
 *
 * Determines whether @selector is complex enough to actually require an
 * extra selection step than only picking an item. This is normally %TRUE
 * if @selector has multiple columns, multiple selection, or when it is a
 * more complex widget, like #HildonTouchSelectorEntry.
 *
 * This information is useful for widgets containing a #HildonTouchSelector,
 * like #HildonPickerDialog, that could need a "Done" button, in case that
 * its internal #HildonTouchSelector has multiple columns, for instance.
 *
 * Returns: %TRUE if @selector requires multiple selection steps.
 *
 * Since: 2.2
 **/
gboolean
hildon_touch_selector_has_multiple_selection (HildonTouchSelector * selector)
{
  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), FALSE);

  return HILDON_TOUCH_SELECTOR_GET_CLASS (selector)->has_multiple_selection (selector);
}


/**
 * hildon_touch_selector_get_column:
 * @selector: A #HildonTouchSelector
 * @column: a column number
 *
 * Use this method to retrieve a #HildonTouchSelectorColumn. Then, you can use
 * the #GtkCellLayout interface to set up the layout of the column.
 *
 * Returns: the @column<!-- -->-th #HildonTouchSelectorColumn in @selector
 *
 * Since: 2.2
 **/
HildonTouchSelectorColumn *
hildon_touch_selector_get_column (HildonTouchSelector * selector,
                                  gint column)
{
  gint num_columns = -1;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), NULL);
  num_columns = hildon_touch_selector_get_num_columns (selector);
  g_return_val_if_fail (column < num_columns && column >= 0, NULL);

  return g_slist_nth_data (selector->priv->columns, column);
}


/**
 * hildon_touch_selector_center_on_selected:
 * @selector: a #HildonTouchSelector
 *
 * Ensures all the columns in a #HildonTouchSelector show a selected
 * item. If one of the columns is in
 * %HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE mode, that column
 * will be scrolled to ensure the selected item that is closest to the
 * currently visible area is shown.
 *
 * The #HildonTouchSelector:initial-scroll property configure the widget
 * in order to use this function at the first show.
 *
 * Take into account that the element is not centered until the widget is
 * realized. If the widget is not realized when the function is called, it
 * will be postponed. If you call this functions several times before the
 * widgets is realized, only the last one will be used.
 *
 * This behaviour includes any call to hildon_touch_selector_center_on_index(),
 * so take care calling this functions, or with the
 * #HildonTouchSelector:initial-scroll property in order to get centered on the
 * proper element.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_center_on_selected         (HildonTouchSelector *selector)
{
  GSList *iter = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));

  for (iter = selector->priv->columns; iter; iter = g_slist_next (iter)) {
    _hildon_touch_selector_center_on_selected_items (selector,
                                                    HILDON_TOUCH_SELECTOR_COLUMN (iter->data));
  }
}

/**
 * hildon_touch_selector_optimal_size_request
 * @selector: a #HildonTouchSelector
 * @requisition: a #GtkRequisition
 *
 * Gets the optimal size request of the touch selector. This function is mostly
 * intended for dialog implementations that include a #HildonTouchSelector and
 * want to optimize the screen real state, for example, when you want a dialog
 * to show as much of the selector, avoiding any extra empty space below the
 * selector.
 *
 * See #HildonPickerDialog implementation for an example.
 *
 * This function is oriented to be used in the size_request of a dialog or window,
 * if you are not sure do not use it.
 *
 * There is a precondition to this function: Since this function does not
 * call the "size_request" method, it can only be used when you know that
 * gtk_widget_size_request() has been called since the last time a resize was
 * queued.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_optimal_size_request      (HildonTouchSelector *selector,
                                                 GtkRequisition *requisition)
{
  GSList *iter = NULL;
  gint height = 0;
  gint base_height = 0;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));

  iter = selector->priv->columns;

  /* Default optimal values are the current ones */
  gtk_widget_get_child_requisition (GTK_WIDGET (selector),
                                    requisition);

  if (iter == NULL) {
    height = requisition->height;
  } else {
    /* we use the normal requisition as base, as the touch selector can has
       extra widgets, not only the columns (ie: HildonTouchSelectorEntry) */
    base_height = requisition->height;
  }

  /* Compute optimal height for the columns */
  while (iter) {
    HildonTouchSelectorColumn *column;
    GtkWidget *child;
    GtkRequisition child_requisition = {0};

    column = HILDON_TOUCH_SELECTOR_COLUMN (iter->data);
    child = GTK_WIDGET (column->priv->tree_view);

    gtk_widget_get_child_requisition (child, &child_requisition);

    height = MAX (height, child_requisition.height);

    iter = g_slist_next (iter);
  }

  requisition->height = base_height + height;
}



/**
 * hildon_touch_selector_get_hildon_ui_mode
 * @selector: a #HildonTouchSelector
 *
 * Gets the current hildon-ui-mode, see #HildonUIMode for more information
 *
 * Returns: the current hildon-ui-mode
 *
 * Since: 2.2
 **/
HildonUIMode
hildon_touch_selector_get_hildon_ui_mode        (HildonTouchSelector *selector)
{
  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), HILDON_UI_MODE_EDIT);

  return selector->priv->hildon_ui_mode;
}

/**
 * hildon_touch_selector_set_hildon_ui_mode
 * @selector: a #HildonTouchSelector
 * @mode: a #HildonUIMode
 *
 * Sets the value of the property #HildonTouchSelector:hildon-ui-mode to be @mode,
 * see #HildonUIMode for more information
 *
 * Note that the %HILDON_UI_MODE_NORMAL can be only used when the selector has
 * one column, use the return value to check if the change was effective.
 *
 * Returns: %TRUE if #HildonTouchSelector:hildon-ui-mode was changed
 *          %FALSE otherwise
 *
 * Since: 2.2
 **/
gboolean
hildon_touch_selector_set_hildon_ui_mode        (HildonTouchSelector *selector,
                                                 HildonUIMode         mode)
{
  gint num = 0;
  GSList *iter = NULL;
  HildonTouchSelectorColumn *column = NULL;
  GtkTreeView *tree_view = NULL;

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), FALSE);
  num = hildon_touch_selector_get_num_columns (selector);
  g_return_val_if_fail ((mode == HILDON_UI_MODE_EDIT) || (num == 1), FALSE);

  if (mode == selector->priv->hildon_ui_mode) {
    return FALSE;
  }

  for (iter = selector->priv->columns; iter; iter = g_slist_next (iter)) {
    column = HILDON_TOUCH_SELECTOR_COLUMN (iter->data);
    tree_view = column->priv->tree_view;

    hildon_tree_view_set_hildon_ui_mode (tree_view, mode);

    /* looking at the code of hildon_tree_view_set_hildon_ui_mode, it seems
       that it call the unselect_all, but it is required anyway */
    if (mode == HILDON_UI_MODE_NORMAL) {
      GtkTreeSelection *selection = gtk_tree_view_get_selection (tree_view);

      gtk_tree_selection_unselect_all (selection);
    }
  }

  selector->priv->hildon_ui_mode = mode;

  return TRUE;
}

/**
 * hildon_touch_selector_get_last_activated_row
 * @selector: a #HildonTouchSelector
 * @column: column number
 *
 * Gets a #GtkTreePath of the last row activated in a column (the last row that
 * emitted a #GtkTreeView::row-activated signal). This is mainly useful if the
 * @selector #HildonTouchSelector:hildon-ui-mode in set to %HILDON_UI_MODE_NORMAL,
 * as using this state there is no real selection, so a method like
 * hildon_touch_selector_get_selected_rows() will return always a empty
 * selection.
 *
 * Anyway, this method works as well on %HILDON_UI_MODE_EDIT, but in this case
 * is better, and more useful, to get the current selection.
 *
 * Returns: a newly allocated #GtkTreePath pointing to the last activated row
 *          NULL if no row were activated.
 *
 * Since: 2.2
 **/
GtkTreePath*
hildon_touch_selector_get_last_activated_row    (HildonTouchSelector *selector,
                                                 gint                 column)
{
  HildonTouchSelectorColumn *selector_column = NULL;

  /* this method with check selector and that the column number is correct*/
  selector_column = hildon_touch_selector_get_column (selector, column);

  if (selector_column == NULL) {
    return NULL;
  }

  if (selector_column->priv->last_activated != NULL) {
    return gtk_tree_row_reference_get_path (selector_column->priv->last_activated);
  } else {
    return NULL;
  }
}


/**
 * hildon_touch_selector_center_on_index:
 * @selector: a #HildonTouchSelector
 * @column: column number
 * @index: the index of the item to center on
 *
 * Ensures that the column number @column shows the element @index
 *
 * This is similar to hildon_touch_selector_center_on_selected() but with the
 * difference that allows to center on a column item not selected.
 *
 * Take into account that the element is not centered until the widget is
 * realized. If the widget is not realized when the function is called, it will
 * be postponed. If you call this function several times before the widget is
 * realized, only the last one will be used.
 *
 * This behaviour includes any call to hildon_touch_selector_center_on_selected().
 * Check this function for more tips.
 *
 * Since: 2.2
 **/
void
hildon_touch_selector_center_on_index (HildonTouchSelector *selector,
                                       gint column,
                                       gint index)
{
  HildonTouchSelectorColumn *current_column = NULL;
  GtkTreePath *path = NULL;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));
  g_return_if_fail ((column >= 0) && (column < hildon_touch_selector_get_num_columns (selector)));
  g_return_if_fail (index >= 0);

  current_column = g_slist_nth_data (selector->priv->columns, column);

  path = gtk_tree_path_new_from_indices (index, -1);

  hildon_touch_selector_scroll_to (current_column,
                                   current_column->priv->tree_view,
                                   path);
  gtk_tree_path_free (path);
}

/**
 * hildon_touch_selector_set_live_search:
 * @selector: a #HildonTouchSelector
 * @live_search: whether @selector should have live search capabilities
 *
 * Toggles a #HildonLiveSearch widget in @selector. This is enabled by
 * default in #HildonTouchSelector but disabled for some of its
 * subclasses like #HildonTouchSelectorEntry, #HildonDateSelector, and
 * #HildonTimeSelector.  For the former, #HildonLiveSearch makes no
 * sense. In a multi-column #HildonTouchSelector, #HildonLiveSearch
 * is not supported, so don't try to enable it in one.
 *
 * If more columns are added to @selector, the #HildonLiveSearch
 * will be removed and the property set to %FALSE.
 *
 * Since: 2.2.10
 **/
void
hildon_touch_selector_set_live_search (HildonTouchSelector *selector,
                                       gboolean live_search)
{
  HildonTouchSelectorColumn *col;

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));

  if (selector->priv->has_live_search == live_search)
    return;

  if (live_search) {
    if (selector->priv->columns &&
        selector->priv->columns->next == NULL) {
      /* There is one and only one column already.  */
      col = (HildonTouchSelectorColumn *) selector->priv->columns->data;
      /* There is already a livesearch widget. Let's hook it up.  */
      if (col->priv->livesearch) {
        hildon_live_search_widget_hook (HILDON_LIVE_SEARCH (col->priv->livesearch),
                                        GTK_WIDGET (col->priv->vbox),
                                        GTK_WIDGET (col->priv->tree_view));
      } else {
        /* There is no livesearch widget yet. Create one.  */
        hildon_touch_selector_add_live_search (selector, col);
      }
    } else if (selector->priv->columns &&
               selector->priv->columns->next != NULL) {
      g_critical ("Trying to set HildonTouchSelector::live-search to TRUE "
                  "in a HildonTouchSelector instance with more than one column.");
      return;
    }
  } else {
    if (selector->priv->columns &&
        selector->priv->columns->next == NULL) {
        col = (HildonTouchSelectorColumn *) selector->priv->columns->data;
        gtk_widget_hide (col->priv->livesearch);
        hildon_live_search_widget_unhook (HILDON_LIVE_SEARCH (col->priv->livesearch));
    }
  }

  selector->priv->has_live_search = live_search;
}

/**
 * hildon_touch_selector_get_live_search:
 * @selector: a #HildonTouchSelector
 *
 * Whether @selector has an embedded #HildonLiveSearch widget
 *
 * Returns: %TRUE if there is a #HildonLiveSearch in @selector, %FALSE
 * otherwise
 *
 * Since: 2.2.10
 **/
gboolean
hildon_touch_selector_get_live_search (HildonTouchSelector *selector)
{
  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), FALSE);

  return selector->priv->has_live_search;
}
