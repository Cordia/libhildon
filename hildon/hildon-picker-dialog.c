/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2008, 2009 Nokia Corporation.
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
 * SECTION:hildon-picker-dialog
 * @short_description: A utility widget that shows a #HildonTouchSelector widget
 *
 * #HildonPickerDialog is a dialog that is used to show a
 * #HildonTouchSelector widget and a 'Done' button to allow users to
 * finish their selections.
 *
 * The #HildonPickerDialog will show a 'Done' button in case the
 * #HildonTouchSelector allows multiple selection. The label of the
 * button can be set using hildon_picker_dialog_set_done_label() and
 * retrieved using hildon_picker_dialog_get_done_label()
 *
 * #HildonPickerDialog will emit #GtkDialog::response when a selection
 * is completed, with the response id set to #GTK_RESPONSE_OK (or
 * #GTK_RESPONSE_DELETE_EVENT if the selection was cancelled).
 *
 * Note that in most cases developers don't need to deal directly with
 * this widget. #HildonPickerButton is designed to pop up a
 * #HildonPickerDialog and manage the interaction with it.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <libintl.h>

#include "hildon-touch-selector.h"
#include "hildon-touch-selector-entry.h"
#include "hildon-picker-dialog.h"
#include "hildon-stock.h"

#define HILDON_PICKER_DIALOG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HILDON_TYPE_PICKER_DIALOG, HildonPickerDialogPrivate))

G_DEFINE_TYPE (HildonPickerDialog, hildon_picker_dialog, HILDON_TYPE_DIALOG)

struct _HildonPickerDialogPrivate
{
  GtkWidget *selector;
  GtkWidget *button;

  gulong signal_changed_id;
  gulong signal_columns_changed_id;

  gboolean center_on_show;
  GSList *current_selection;
  gchar *current_text;
};

/* properties */
enum
{
  PROP_0,
  PROP_DONE_BUTTON_TEXT,
  PROP_CENTER_ON_SHOW,
  PROP_LAST
};

enum
{
  RESPONSE,
  LAST_SIGNAL
};

#define DEFAULT_DONE_BUTTON_TEXT        HILDON_STOCK_DONE

static void
hildon_picker_dialog_set_property               (GObject * object,
                                                 guint prop_id,
                                                 const GValue * value,
                                                 GParamSpec * pspec);

static void
hildon_picker_dialog_get_property               (GObject * object,
                                                 guint prop_id,
                                                 GValue * value, GParamSpec * pspec);

static void
hildon_picker_dialog_finalize                   (GObject *object);

/* gtkwidget */
static void
hildon_picker_dialog_show                       (GtkWidget *widget);

static void
hildon_picker_dialog_realize                    (GtkWidget *widget);

static void
hildon_picker_dialog_size_request               (GtkWidget *widget,
                                                 GtkRequisition *requisition);

/* private functions */
static gboolean
requires_done_button                            (HildonPickerDialog * dialog);

static void
prepare_action_area                             (HildonPickerDialog *dialog);

static void
setup_interaction_mode                          (HildonPickerDialog * dialog);

static void
_select_on_selector_changed_cb                  (HildonTouchSelector * dialog,
                                                 gint column,
                                                 gpointer data);

static gboolean
_hildon_picker_dialog_set_selector              (HildonPickerDialog * dialog,
                                                 HildonTouchSelector * selector);

static void
_on_dialog_response                             (GtkDialog *dialog,
                                                 gint response_id,
                                                 gpointer data);

static void
_save_current_selection                         (HildonPickerDialog *dialog);

static void
_restore_current_selection                      (HildonPickerDialog *dialog);

static void
_clean_current_selection                        (HildonPickerDialog *dialog);

static guint
hildon_picker_dialog_get_max_height             (HildonPickerDialog *dialog);

static void
on_done_button_clicked_cb                       (GtkButton *button,
                                                 gpointer data);

/**********************************************************************************/

static void
hildon_picker_dialog_class_init (HildonPickerDialogClass * class)
{
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  gobject_class = (GObjectClass *) class;
  object_class = (GtkObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;
  container_class = (GtkContainerClass *) class;

  /* GObject */
  gobject_class->set_property = hildon_picker_dialog_set_property;
  gobject_class->get_property = hildon_picker_dialog_get_property;
  gobject_class->finalize = hildon_picker_dialog_finalize;

  /* GtkWidget */
  widget_class->show = hildon_picker_dialog_show;
  widget_class->realize = hildon_picker_dialog_realize;
  widget_class->size_request = hildon_picker_dialog_size_request,

  /* HildonPickerDialog */
  class->set_selector = _hildon_picker_dialog_set_selector;

  /* signals */

  /* properties */
  /**
   * HildonPickerDialog
   *
   * Button label
   *
   * Since: 2.2
   */
  g_object_class_install_property (gobject_class,
                                   PROP_DONE_BUTTON_TEXT,
                                   g_param_spec_string ("done-button-text",
                                                        "Done Button Label",
                                                        "Done Button Label",
                                                        DEFAULT_DONE_BUTTON_TEXT,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class,
                                   PROP_CENTER_ON_SHOW,
                                   g_param_spec_boolean ("center-on-show",
                                                         "Center on show",
                                                         "If the dialog should center"
                                                         " on the current selection"
                                                         " when it is showed",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  /* Using the default height, we get 5 full rows. With the header it sums 404 pixels */
  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_uint
                                           ("max-height-landscape",
                                            "Max dialog height on landscape mode",
                                            "Maximum dialog height on landscape mode",
                                            0,
                                            G_MAXUINT,
                                            358,
                                            G_PARAM_READWRITE));

  /* Using the default height, we get 9 full rows. With the header it sums 684 pixels */
  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_uint
                                           ("max-height-portrait",
                                            "Max dialog height on portrait mode",
                                            "Maximum dialog height on portrait mode",
                                            0,
                                            G_MAXUINT,
                                            638,
                                            G_PARAM_READWRITE));

  g_type_class_add_private (object_class, sizeof (HildonPickerDialogPrivate));
}


static void
hildon_picker_dialog_init (HildonPickerDialog * dialog)
{
  dialog->priv = HILDON_PICKER_DIALOG_GET_PRIVATE (dialog);

  dialog->priv->selector = NULL;
  dialog->priv->button =
    gtk_dialog_add_button (GTK_DIALOG (dialog), "", GTK_RESPONSE_OK);
  gtk_widget_grab_default (dialog->priv->button);
  gtk_button_set_focus_on_click (GTK_BUTTON (dialog->priv->button), FALSE);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

  dialog->priv->signal_changed_id = 0;
  dialog->priv->signal_columns_changed_id = 0;
  dialog->priv->center_on_show = TRUE;
  dialog->priv->current_selection = NULL;
  dialog->priv->current_text = NULL;

  g_signal_connect (G_OBJECT (dialog),
                    "response", G_CALLBACK (_on_dialog_response),
                    NULL);

  g_signal_connect (G_OBJECT (dialog->priv->button),
                    "clicked", G_CALLBACK (on_done_button_clicked_cb),
                    dialog);
}


static void
hildon_picker_dialog_set_property (GObject * object,
                                   guint param_id,
                                   const GValue * value, GParamSpec * pspec)
{
  HildonPickerDialog *dialog;

  dialog = HILDON_PICKER_DIALOG (object);

  switch (param_id) {
  case PROP_DONE_BUTTON_TEXT:
    hildon_picker_dialog_set_done_label (HILDON_PICKER_DIALOG (object),
                                         g_value_get_string (value));
    break;
  case PROP_CENTER_ON_SHOW:
    dialog->priv->center_on_show = g_value_get_boolean (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
    break;
  }
}

static void
hildon_picker_dialog_get_property (GObject * object,
                                   guint param_id,
                                   GValue * value, GParamSpec * pspec)
{
  HildonPickerDialog *dialog;

  dialog = HILDON_PICKER_DIALOG (object);

  switch (param_id) {
  case PROP_DONE_BUTTON_TEXT:
    g_value_set_string (value, hildon_picker_dialog_get_done_label (dialog));
    break;
  case PROP_CENTER_ON_SHOW:
    g_value_set_boolean (value, dialog->priv->center_on_show);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
    break;
  }
}

static void
hildon_picker_dialog_finalize (GObject *object)
{
  _clean_current_selection (HILDON_PICKER_DIALOG (object));

  G_OBJECT_CLASS (hildon_picker_dialog_parent_class)->finalize (object);
}

static void
hildon_picker_dialog_show                       (GtkWidget *widget)
{
  HildonPickerDialog *dialog = HILDON_PICKER_DIALOG (widget);
  HildonTouchSelector *selector;

  GTK_WIDGET_CLASS (hildon_picker_dialog_parent_class)->show (widget);

  if (dialog->priv->center_on_show) {
    selector = hildon_picker_dialog_get_selector (dialog);
    hildon_touch_selector_center_on_selected (selector);
  }

  _save_current_selection (dialog);
  prepare_action_area (dialog);

}

static void
hildon_picker_dialog_size_request               (GtkWidget *widget,
                                                 GtkRequisition *requisition)
{
  HildonTouchSelector *selector;

  selector = hildon_picker_dialog_get_selector (HILDON_PICKER_DIALOG (widget));

  if (selector) {
    GtkRequisition child_requisition;
    GtkRequisition optimal_requisition;
    GtkRequisition selector_requisition;
    GtkBin *bin;
    guint max_height;

    bin = GTK_BIN (widget);

    requisition->width = GTK_CONTAINER (widget)->border_width * 2;
    /* Adding pannable container border using 4 instead of 2 */
    requisition->height = GTK_CONTAINER (widget)->border_width * 4;

    /* assure the requisition is done */
    gtk_widget_size_request (bin->child, &child_requisition);
    gtk_widget_get_child_requisition (GTK_WIDGET (selector), &selector_requisition);

    hildon_touch_selector_optimal_size_request (selector,
                                                &optimal_requisition);

    requisition->width += child_requisition.width;
    requisition->height += child_requisition.height;

    max_height = hildon_picker_dialog_get_max_height (HILDON_PICKER_DIALOG (widget));

    requisition->height = MIN (max_height,
                               requisition->height +
                               optimal_requisition.height -
                               selector_requisition.height);
  } else
    GTK_WIDGET_CLASS (hildon_picker_dialog_parent_class)->size_request
      (widget, requisition);
}

static void
hildon_picker_dialog_realize (GtkWidget *widget)
{
  setup_interaction_mode (HILDON_PICKER_DIALOG (widget));

  GTK_WIDGET_CLASS (hildon_picker_dialog_parent_class)->realize (widget);

  hildon_gtk_window_set_portrait_flags (GTK_WINDOW (widget), HILDON_PORTRAIT_MODE_SUPPORT);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */

static guint
hildon_picker_dialog_get_max_height             (HildonPickerDialog *dialog)
{
  gboolean landscape = TRUE;
  guint max_value = 0;
  GdkScreen *screen = NULL;

  screen = gtk_widget_get_screen (GTK_WIDGET (dialog));
  if (screen != NULL) {
    if (gdk_screen_get_width (screen) > gdk_screen_get_height (screen)) {
      landscape = TRUE;
    } else {
      landscape = FALSE;
    }
  }

  if (landscape) {
    gtk_widget_style_get (GTK_WIDGET (dialog), "max-height-landscape",
                          &max_value, NULL);
  } else {
    gtk_widget_style_get (GTK_WIDGET (dialog), "max-height-portrait",
                          &max_value, NULL);
  }

  return max_value;
}


static void
_select_on_selector_changed_cb (HildonTouchSelector * selector,
                                gint column, gpointer data)
{
  g_return_if_fail (HILDON_IS_PICKER_DIALOG (data));

  gtk_dialog_response (GTK_DIALOG (data), GTK_RESPONSE_OK);
}

static void
on_done_button_clicked_cb                       (GtkButton *button,
                                                 gpointer data)
{
  HildonPickerDialog *dialog = NULL;
  HildonTouchSelector *selector = NULL;
  HildonEntry *entry = NULL;

  g_return_if_fail (HILDON_IS_PICKER_DIALOG (data));

  dialog = HILDON_PICKER_DIALOG (data);
  selector = hildon_picker_dialog_get_selector (dialog);

  if (HILDON_IS_TOUCH_SELECTOR_ENTRY (selector)) {
    entry = hildon_touch_selector_entry_get_entry (HILDON_TOUCH_SELECTOR_ENTRY (selector));

    gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);
    gtk_widget_grab_focus (GTK_WIDGET (entry));
  }
}


static gboolean
selection_completed (HildonPickerDialog *dialog)
{
  HildonPickerDialogPrivate *priv;
  GList *list;
  gint i, n_cols;
  gboolean all_selected = TRUE;
  HildonUIMode mode = HILDON_UI_MODE_NORMAL;

  priv = HILDON_PICKER_DIALOG_GET_PRIVATE (dialog);

  if (HILDON_IS_TOUCH_SELECTOR_ENTRY (priv->selector)) {
    HildonTouchSelector *sel = HILDON_TOUCH_SELECTOR (priv->selector);
    gchar *text = hildon_touch_selector_get_current_text (sel);
    gboolean is_empty = (text == NULL || *text == '\0');
    g_free (text);
    if (!is_empty) {
      return TRUE;
    }
  }

  mode = hildon_touch_selector_get_hildon_ui_mode (HILDON_TOUCH_SELECTOR (priv->selector));
  if (mode == HILDON_UI_MODE_NORMAL) {
    return TRUE;
  }

  n_cols = hildon_touch_selector_get_num_columns (HILDON_TOUCH_SELECTOR (priv->selector));
  for (i = 0; i < n_cols; i++) {
    list = hildon_touch_selector_get_selected_rows (HILDON_TOUCH_SELECTOR (priv->selector), i);
    if (list == NULL) {
      all_selected = FALSE;
      break;
    }
    g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
    g_list_free (list);
  }

  return all_selected;
}

static void
_on_dialog_response                             (GtkDialog *dialog,
                                                 gint response_id,
                                                 gpointer data)
{
  HildonPickerDialog *picker = HILDON_PICKER_DIALOG (dialog);
  if (response_id == GTK_RESPONSE_OK) {
    if (selection_completed (picker) == FALSE) {
      g_signal_stop_emission_by_name (dialog, "response");
    }
  } else if (response_id == GTK_RESPONSE_DELETE_EVENT) {
    if (requires_done_button (picker)) {
      _restore_current_selection (picker);
    }
  }
}

static void
on_selector_columns_changed (HildonTouchSelector * selector, gpointer userdata)
{
  HildonPickerDialog * dialog;

  dialog = HILDON_PICKER_DIALOG (userdata);

  prepare_action_area (dialog);
  if (GTK_WIDGET_REALIZED (dialog)) {
    setup_interaction_mode (dialog);
  }
}

/**
 * hildon_picker_dialog_set_done_label:
 * @dialog: a #HildonPickerDialog
 * @label: a string
 *
 * Sets a custom string to be used as the 'Done' button label in @dialog.
 *
 * Since: 2.2
 **/
void
hildon_picker_dialog_set_done_label (HildonPickerDialog * dialog,
                                     const gchar * label)
{
  HildonPickerDialogPrivate *priv;

  g_return_if_fail (HILDON_IS_PICKER_DIALOG (dialog));
  g_return_if_fail (label != NULL);

  priv = HILDON_PICKER_DIALOG_GET_PRIVATE (dialog);

  gtk_button_set_label (GTK_BUTTON (priv->button), label);
}

/**
 * hildon_picker_dialog_get_done_label:
 * @dialog: a #HildonPickerDialog
 *
 * Retrieves current 'Done' button label.
 *
 * Returns: the custom string to be used.
 *
 * Since: 2.2
 **/
const gchar *
hildon_picker_dialog_get_done_label (HildonPickerDialog * dialog)
{
  HildonPickerDialogPrivate *priv;

  g_return_val_if_fail (HILDON_IS_PICKER_DIALOG (dialog), NULL);

  priv = HILDON_PICKER_DIALOG_GET_PRIVATE (dialog);

  return gtk_button_get_label (GTK_BUTTON (priv->button));
}

static void
free_path_list (GList *list)
{
  g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (list);
}

static void
_clean_current_selection (HildonPickerDialog *dialog)
{
  if (dialog->priv->current_selection) {
    g_slist_foreach (dialog->priv->current_selection, (GFunc) free_path_list, NULL);
    g_slist_free (dialog->priv->current_selection);
    dialog->priv->current_selection = NULL;
  }
  if (dialog->priv->current_text) {
    g_free (dialog->priv->current_text);
    dialog->priv->current_text = NULL;
  }
}

static void
_save_current_selection (HildonPickerDialog *dialog)
{
  HildonTouchSelector *selector;
  gint i, columns;

  selector = HILDON_TOUCH_SELECTOR (dialog->priv->selector);

  _clean_current_selection (dialog);

  columns = hildon_touch_selector_get_num_columns (selector);
  for (i = 0; i  < columns; i++) {
    dialog->priv->current_selection
      = g_slist_append (dialog->priv->current_selection,
                        hildon_touch_selector_get_selected_rows (selector, i));
  }
  if (HILDON_IS_TOUCH_SELECTOR_ENTRY (selector)) {
	  HildonEntry *entry = hildon_touch_selector_entry_get_entry (HILDON_TOUCH_SELECTOR_ENTRY (selector));
	  dialog->priv->current_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
  }
}

static void
_restore_current_selection (HildonPickerDialog *dialog)
{
  GSList *current_selection, *iter;
  GList *selected, *selected_iter;
  GtkTreePath *current_path;
  HildonTouchSelector *selector;
  GtkTreeModel *model;
  GtkTreeIter tree_iter;
  gint i;

  if (dialog->priv->current_selection == NULL)
    return;

  current_selection = dialog->priv->current_selection;
  selector = HILDON_TOUCH_SELECTOR (dialog->priv->selector);

  if (hildon_touch_selector_get_num_columns (selector) !=
      g_slist_length (current_selection)) {
    /* We conclude that if the current selection has the same
       numbers of columns that the selector, all this ok
       Anyway this shouldn't happen. */
    g_critical ("Trying to restore the selection on a selector after change"
                " the number of columns. Are you removing columns while the"
                " dialog is open?");
    return;
  }

  if (dialog->priv->signal_changed_id)
    g_signal_handler_block (selector, dialog->priv->signal_changed_id);
  for (iter = current_selection, i = 0; iter; iter = g_slist_next (iter), i++) {
    selected = (GList *) (iter->data);
    model = hildon_touch_selector_get_model (selector, i);
    if (selected)
        hildon_touch_selector_unselect_all (selector, i);
    for (selected_iter = selected; selected_iter; selected_iter = g_list_next (selected_iter)) {
      current_path = (GtkTreePath *) selected_iter->data;
      if (gtk_tree_model_get_iter (model, &tree_iter, current_path))
          hildon_touch_selector_select_iter (selector, i, &tree_iter, FALSE);
    }
  }
  if (HILDON_IS_TOUCH_SELECTOR_ENTRY (selector) && dialog->priv->current_text != NULL) {
    HildonEntry *entry = hildon_touch_selector_entry_get_entry (HILDON_TOUCH_SELECTOR_ENTRY (selector));
    gtk_entry_set_text (GTK_ENTRY (entry), dialog->priv->current_text);
  }
  if (dialog->priv->signal_changed_id)
    g_signal_handler_unblock (selector, dialog->priv->signal_changed_id);
}

static gboolean
requires_done_button (HildonPickerDialog * dialog)
{
  return hildon_touch_selector_has_multiple_selection
    (HILDON_TOUCH_SELECTOR (dialog->priv->selector));
}

static void
prepare_action_area (HildonPickerDialog *dialog)
{
  if (requires_done_button (dialog)) {
    gtk_widget_show (GTK_DIALOG (dialog)->action_area);
  } else {
    gtk_widget_hide (GTK_DIALOG (dialog)->action_area);
  }
}

static void
setup_interaction_mode (HildonPickerDialog * dialog)
{
  if (dialog->priv->signal_changed_id) {
    g_signal_handler_disconnect (dialog->priv->selector,
                                 dialog->priv->signal_changed_id);
  }

  if (requires_done_button (dialog) == FALSE) {
    dialog->priv->signal_changed_id =
      g_signal_connect (G_OBJECT (dialog->priv->selector), "changed",
                        G_CALLBACK (_select_on_selector_changed_cb), dialog);
  }
}

/*------------------------- PUBLIC METHODS ---------------------------- */

/**
 * hildon_picker_dialog_new:
 * @parent: the parent window
 *
 * Creates a new #HildonPickerDialog
 *
 * Returns: a new #HildonPickerDialog
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_picker_dialog_new (GtkWindow * parent)
{
  GtkDialog *dialog = NULL;

  dialog = g_object_new (HILDON_TYPE_PICKER_DIALOG, NULL);

  if (parent) {
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
  }

  return GTK_WIDGET (dialog);
}


static gboolean
_hildon_picker_dialog_set_selector (HildonPickerDialog * dialog,
                                    HildonTouchSelector * selector)
{
  g_object_ref (selector);

  /* Remove the old selector, if any */
  if (dialog->priv->selector != NULL) {
    gtk_container_remove (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
                          dialog->priv->selector);
    if (dialog->priv->signal_columns_changed_id) {
            g_signal_handler_disconnect (dialog->priv->selector,
                                         dialog->priv->signal_columns_changed_id);
    }
  }

  dialog->priv->selector = GTK_WIDGET (selector);

  /* Pack the new selector */
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                      dialog->priv->selector, TRUE, TRUE, 0);

  g_object_unref (selector);

  gtk_widget_show (dialog->priv->selector);

  prepare_action_area (dialog);
  if (GTK_WIDGET_REALIZED (dialog)) {
    setup_interaction_mode (dialog);
  }

  dialog->priv->signal_columns_changed_id = g_signal_connect (G_OBJECT (dialog->priv->selector),
                                                              "columns-changed",
                                                              G_CALLBACK (on_selector_columns_changed), dialog);
  return TRUE;
}

/**
 * hildon_picker_dialog_set_selector:
 * @dialog: a #HildonPickerDialog
 * @selector: a #HildonTouchSelector
 *
 * Sets @selector as the #HildonTouchSelector to be shown in @dialog
 *
 * Returns: %TRUE if @selector was set, %FALSE otherwise
 *
 * Since: 2.2
 **/
gboolean
hildon_picker_dialog_set_selector (HildonPickerDialog * dialog,
                                   HildonTouchSelector * selector)
{
  g_return_val_if_fail (HILDON_IS_PICKER_DIALOG (dialog), FALSE);
  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), FALSE);

  return HILDON_PICKER_DIALOG_GET_CLASS (dialog)->set_selector (dialog, selector);
}

/**
 * hildon_picker_dialog_get_selector:
 * @dialog: a #HildonPickerDialog
 *
 * Retrieves the #HildonTouchSelector associated to @dialog.
 *
 * Returns: a #HildonTouchSelector
 *
 * Since: 2.2
 **/
HildonTouchSelector *
hildon_picker_dialog_get_selector (HildonPickerDialog * dialog)
{
  g_return_val_if_fail (HILDON_IS_PICKER_DIALOG (dialog), NULL);

  return HILDON_TOUCH_SELECTOR (dialog->priv->selector);
}
