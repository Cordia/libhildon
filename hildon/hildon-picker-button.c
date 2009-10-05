/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
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
 * SECTION:hildon-picker-button
 * @short_description: A button that launches a #HildonPickerDialog and displays the
 * selected item
 * @see_also: #HildonTouchSelector, #HildonPickerDialog
 *
 * #HildonPickerButton is a widget that lets the user select a particular item from
 * a list. Visually, it's a button with title and value labels that brings up a
 * #HildonPickerDialog. The user can then use this dialog to choose an item, which
 * will be displayed in the value label of the button. The title of the dialog is
 * taken from the button's main label (see hildon_button_get_title()).
 *
 * You should create your own #HildonTouchSelector at convenience and set it
 * to the #HildonPickerButton with hildon_picker_button_set_selector(). For
 * the common use cases of buttons to select date and time, you can use #HildonDateButton
 * and #HildonTimeButton.
 *
 * <example>
 * <programlisting>
 * GtkWidget *
 * create_selector (void)
 * {
 *   GtkWidget *selector;
 * <!-- -->
 *   selector = hildon_touch_selector_new_text ();
 * <!-- -->
 *   hildon_touch_selector_append_text (HILDON_TOUCH_SELECTOR (selector), "America");
 *   hildon_touch_selector_append_text (HILDON_TOUCH_SELECTOR (selector), "Europe");
 *   hildon_touch_selector_append_text (HILDON_TOUCH_SELECTOR (selector), "Asia");
 *   hildon_touch_selector_append_text (HILDON_TOUCH_SELECTOR (selector), "Africa");
 *   hildon_touch_selector_append_text (HILDON_TOUCH_SELECTOR (selector), "Australia");
 * <!-- -->
 *   hildon_touch_selector_set_active (HILDON_TOUCH_SELECTOR (selector), 0, 2);
 * <!-- -->
 *   return selector;
 * }
 * <!-- -->
 * GtkWidget *
 * create_button (HildonTouchSelector *selector)
 * {
 *   GtkWidget *button;
 * <!-- -->
 *   button = hildon_picker_button_new (HILDON_SIZE_AUTO, HILDON_BUTTON_ARRANGEMENT_VERTICAL);
 *   hildon_button_set_title (HILDON_BUTTON (button), "Continent");
 * <!-- -->
 *   hildon_picker_button_set_selector (HILDON_PICKER_BUTTON (button),
 *                                      HILDON_TOUCH_SELECTOR (selector));
 * <!-- -->
 *   return button;
 * }
 * </programlisting>
 * </example>
 */

#include "hildon-picker-button.h"
#include "hildon-picker-button-private.h"
#include "hildon-picker-dialog.h"

G_DEFINE_TYPE (HildonPickerButton, hildon_picker_button, HILDON_TYPE_BUTTON)

#define GET_PRIVATE(o)                                                  \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), HILDON_TYPE_PICKER_BUTTON, HildonPickerButtonPrivate))

typedef struct _HildonPickerButtonPrivate HildonPickerButtonPrivate;

struct _HildonPickerButtonPrivate
{
  GtkWidget *selector;
  GtkWidget *dialog;
  gchar *done_button_text;
  guint disable_value_changed : 1;
};

/* Signals */
enum
{
  VALUE_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_SELECTOR = 1,
  PROP_DONE_BUTTON_TEXT
};

static guint picker_button_signals[LAST_SIGNAL] = { 0 };

static gboolean
_current_selector_empty                         (HildonPickerButton *button);

static void
hildon_picker_button_selector_selection_changed (HildonTouchSelector * selector,
                                                 gint column,
                                                 gpointer user_data);

static void
hildon_picker_button_selector_columns_changed   (HildonTouchSelector * selector,
                                                 gpointer user_data);


static void
hildon_picker_button_get_property (GObject * object, guint property_id,
                                   GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
  case PROP_SELECTOR:
    g_value_set_object (value,
                        hildon_picker_button_get_selector (HILDON_PICKER_BUTTON (object)));
    break;
  case PROP_DONE_BUTTON_TEXT:
    g_value_set_string (value,
                        hildon_picker_button_get_done_button_text (HILDON_PICKER_BUTTON (object)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
hildon_picker_button_set_property (GObject * object, guint property_id,
                                   const GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
  case PROP_SELECTOR:
    hildon_picker_button_set_selector (HILDON_PICKER_BUTTON (object),
                                       g_value_get_object (value));
    break;
  case PROP_DONE_BUTTON_TEXT:
    hildon_picker_button_set_done_button_text (HILDON_PICKER_BUTTON (object),
                                               g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
hildon_picker_button_finalize (GObject * object)
{
  HildonPickerButtonPrivate *priv;

  priv = GET_PRIVATE (object);

  if (priv->selector) {
    g_signal_handlers_disconnect_by_func (priv->selector,
                                          hildon_picker_button_selector_selection_changed,
                                          object);
    g_signal_handlers_disconnect_by_func (priv->selector,
                                          hildon_picker_button_selector_columns_changed,
                                          object);
    g_object_unref (priv->selector);
    priv->selector = NULL;
  }
  if (priv->dialog) {
    gtk_widget_destroy (priv->dialog);
    priv->dialog = NULL;
  }

  if (priv->done_button_text) {
    g_free (priv->done_button_text);
    priv->done_button_text = NULL;
  }

  G_OBJECT_CLASS (hildon_picker_button_parent_class)->finalize (object);
}

/**
 * hildon_picker_button_value_changed:
 * @button: a #HildonPickerButton
 *
 * Emits a "#HildonPickerButton::value-changed" signal to the given
 * #HildonPickerButton
 *
 * Since: 2.2
 **/
void
hildon_picker_button_value_changed              (HildonPickerButton *button)
{
  HildonPickerButtonPrivate *priv;
  g_return_if_fail (HILDON_IS_PICKER_BUTTON (button));
  priv = GET_PRIVATE (button);

  if (!priv->disable_value_changed)
    g_signal_emit (button, picker_button_signals[VALUE_CHANGED], 0);
}

G_GNUC_INTERNAL void
hildon_picker_button_disable_value_changed      (HildonPickerButton *button,
                                                 gboolean            disable)
{
  HildonPickerButtonPrivate *priv;
  g_return_if_fail (HILDON_IS_PICKER_BUTTON (button));
  priv = GET_PRIVATE (button);

  priv->disable_value_changed = disable;
}

static void
_selection_changed (HildonPickerButton *button)
{
  HildonPickerButtonPrivate *priv = GET_PRIVATE (button);

  if (!GTK_IS_WINDOW (priv->dialog) ||
      !GTK_WIDGET_VISIBLE (GTK_WINDOW (priv->dialog))) {
    gchar *value = hildon_touch_selector_get_current_text (HILDON_TOUCH_SELECTOR (priv->selector));
    if (value) {
      hildon_button_set_value (HILDON_BUTTON (button), value);
      g_free (value);
      hildon_picker_button_value_changed (button);
    }
  }
}

static void
hildon_picker_button_on_dialog_response (GtkDialog *dialog,
                                         gint       response,
                                         gpointer   user_data)
{
  HildonPickerButton *button;
  HildonPickerButtonPrivate *priv;
  gchar *value;

  button = HILDON_PICKER_BUTTON (user_data);
  priv = GET_PRIVATE (button);

  if (response == GTK_RESPONSE_OK) {
    value = hildon_touch_selector_get_current_text
	    (HILDON_TOUCH_SELECTOR (priv->selector));
    hildon_button_set_value (HILDON_BUTTON (button), value);
    g_free (value);
    hildon_picker_button_value_changed (button);
  }

  gtk_widget_hide (GTK_WIDGET (dialog));
}

static void
hildon_picker_button_clicked (GtkButton * button)
{
  GtkWidget *parent;
  HildonPickerButtonPrivate *priv;

  priv = GET_PRIVATE (HILDON_PICKER_BUTTON (button));

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (priv->selector));

  /* Create the dialog if it doesn't exist already.  */
  if (!priv->dialog) {
    parent = gtk_widget_get_toplevel (GTK_WIDGET (button));
    if (GTK_WIDGET_TOPLEVEL (parent)) {
      priv->dialog = hildon_picker_dialog_new (GTK_WINDOW (parent));
    } else {
      priv->dialog = hildon_picker_dialog_new (NULL);
    }

    hildon_picker_dialog_set_selector (HILDON_PICKER_DIALOG (priv->dialog),
                                       HILDON_TOUCH_SELECTOR (priv->selector));
    if (priv->done_button_text) {
      hildon_picker_dialog_set_done_label (HILDON_PICKER_DIALOG (priv->dialog),
                                           priv->done_button_text);
    }

    gtk_window_set_modal (GTK_WINDOW (priv->dialog),
                          gtk_window_get_modal (GTK_WINDOW (parent)));
    gtk_window_set_title (GTK_WINDOW (priv->dialog),
                          hildon_button_get_title (HILDON_BUTTON (button)));
    g_signal_connect (priv->dialog, "response",
                      G_CALLBACK (hildon_picker_button_on_dialog_response),
                      button);
    g_signal_connect (priv->dialog, "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete),
                      NULL);
  }

  if (_current_selector_empty (HILDON_PICKER_BUTTON (button))) {
    g_warning ("There are no elements in the selector. Nothing to show.");
  } {
    gtk_window_present (GTK_WINDOW (priv->dialog));
  }
}

static void
hildon_picker_button_selector_selection_changed (HildonTouchSelector * selector,
                                                 gint column,
                                                 gpointer user_data)

{
  _selection_changed (HILDON_PICKER_BUTTON (user_data));
}

static void
hildon_picker_button_selector_columns_changed (HildonTouchSelector * selector,
                                               gpointer user_data)

{
  _selection_changed (HILDON_PICKER_BUTTON (user_data));
}

static void
hildon_picker_button_class_init (HildonPickerButtonClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkButtonClass *button_class = GTK_BUTTON_CLASS (klass);

  g_type_class_add_private (klass, sizeof (HildonPickerButtonPrivate));

  object_class->get_property = hildon_picker_button_get_property;
  object_class->set_property = hildon_picker_button_set_property;
  object_class->finalize = hildon_picker_button_finalize;

  button_class->clicked = hildon_picker_button_clicked;

  g_object_class_install_property (object_class,
                                   PROP_SELECTOR,
                                   g_param_spec_object ("touch-selector",
                                                        "HildonTouchSelector widget",
                                                        "HildonTouchSelector widget to be launched on button clicked",
                                                        HILDON_TYPE_TOUCH_SELECTOR,
                                                        G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_DONE_BUTTON_TEXT,
                                   g_param_spec_string ("done-button-text",
                                                        "HildonPickerDialog \"done\" button text",
                                                        "The text for the \"done\" button in the dialog launched",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  /**
   * HildonPickerButton::value-changed:
   * @widget: the widget that received the signal
   *
   * The ::value-changed signal is emitted each time the user chooses a different
   * item from the #HildonTouchSelector related, and the value label gets updated.
   *
   * Since: 2.2
   */
  picker_button_signals[VALUE_CHANGED] =
    g_signal_new ("value-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
}

static void
hildon_picker_button_init (HildonPickerButton * self)
{
  HildonPickerButtonPrivate *priv;

  priv = GET_PRIVATE (self);

  priv->dialog = NULL;
  priv->selector = NULL;
  priv->done_button_text = NULL;
  priv->disable_value_changed = FALSE;

  hildon_button_set_style (HILDON_BUTTON (self),
                           HILDON_BUTTON_STYLE_PICKER);
}

static gboolean
_current_selector_empty (HildonPickerButton *button)
{
  HildonPickerButtonPrivate *priv;
  HildonTouchSelector *selector = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  gint i = 0;

  priv = GET_PRIVATE (button);
  selector = HILDON_TOUCH_SELECTOR (priv->selector);

  g_return_val_if_fail (HILDON_IS_TOUCH_SELECTOR (selector), TRUE);

  if (hildon_touch_selector_has_multiple_selection (selector)) {
    return FALSE;
  } else {
    for (i=0; i < hildon_touch_selector_get_num_columns (selector); i++) {
      model = hildon_touch_selector_get_model (selector, i);

      if (gtk_tree_model_get_iter_first (model, &iter)) {
        return FALSE;
      }
    }
    return TRUE;
  }
}

/**
 * hildon_picker_button_new:
 * @size: One of #HildonSizeType, specifying the size of the new button.
 * @arrangement: one of #HildonButtonArrangement, specifying the placement of the
 * labels.
 *
 * Creates a new #HildonPickerButton. See hildon_button_new() for details on the
 * parameters.
 *
 * Returns: a newly created #HildonPickerButton
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_picker_button_new (HildonSizeType          size,
                          HildonButtonArrangement arrangement)
{
  GtkWidget *button;

  button = g_object_new (HILDON_TYPE_PICKER_BUTTON,
                         "arrangement", arrangement, "size", size,
                         NULL);

  return button;
}

/**
 * hildon_picker_button_set_selector:
 * @button: a #HildonPickerButton
 * @selector: a #HildonTouchSelector
 *
 * Sets @selector as the #HildonTouchSelector to be shown in the
 * #HildonPickerDialog that @button brings up.
 *
 * Since: 2.2
 **/
void
hildon_picker_button_set_selector (HildonPickerButton * button,
                                   HildonTouchSelector * selector)
{
  HildonPickerButtonPrivate *priv;
  gchar *value = NULL;

  g_return_if_fail (HILDON_IS_PICKER_BUTTON (button));
  g_return_if_fail (!selector || HILDON_IS_TOUCH_SELECTOR (selector));

  priv = GET_PRIVATE (button);

  if (priv->selector) {
    g_signal_handlers_disconnect_by_func (priv->selector,
                                          hildon_picker_button_selector_selection_changed,
                                          button);
    g_signal_handlers_disconnect_by_func (priv->selector,
                                          hildon_picker_button_selector_columns_changed,
                                          button);
    g_object_unref (priv->selector);
  }

  priv->selector = GTK_WIDGET (selector);

  if (selector) {
    g_object_ref_sink (selector);

    g_signal_connect (G_OBJECT (selector), "changed",
                      G_CALLBACK (hildon_picker_button_selector_selection_changed),
                      button);

    g_signal_connect (G_OBJECT (selector), "columns-changed",
                      G_CALLBACK (hildon_picker_button_selector_columns_changed),
                      button);

    value = hildon_touch_selector_get_current_text (HILDON_TOUCH_SELECTOR (priv->selector));
  }

  if (!value)
    value = g_strdup ("");

  hildon_button_set_value (HILDON_BUTTON (button), value);
  hildon_picker_button_value_changed (button);

  g_free (value);
}

/**
 * hildon_picker_button_get_selector:
 * @button: a #HildonPickerButton
 *
 * Retrieves the #HildonTouchSelector associated to @button.
 *
 * Returns: a #HildonTouchSelector
 *
 * Since: 2.2
 **/
HildonTouchSelector *
hildon_picker_button_get_selector (HildonPickerButton * button)
{
  HildonPickerButtonPrivate *priv;

  g_return_val_if_fail (HILDON_IS_PICKER_BUTTON (button), NULL);

  priv = GET_PRIVATE (button);

  return HILDON_TOUCH_SELECTOR (priv->selector);
}

/**
 * hildon_picker_button_get_active:
 * @button: a #HildonPickerButton
 *
 * Returns the index of the currently active item, or -1 if there's no
 * active item. If the selector has several columns, only the first
 * one is used.
 *
 * Returns: an integer which is the index of the currently active item, or -1 if there's no active item.
 *
 * Since: 2.2
 **/
gint
hildon_picker_button_get_active                 (HildonPickerButton * button)
{
  HildonTouchSelector *sel;
  g_return_val_if_fail (HILDON_IS_PICKER_BUTTON (button), -1);

  sel = hildon_picker_button_get_selector (button);

  return hildon_touch_selector_get_active (sel, 0);
}

/**
 * hildon_picker_button_set_active:
 * @button: a #HildonPickerButton
 * @index: the index of the item to select, or -1 to have no active item
 *
 * Sets the active item of the #HildonTouchSelector associated to
 * @button to @index. If the selector has several columns, only the
 * first one is used.
 *
 * Since: 2.2
 **/
void
hildon_picker_button_set_active                 (HildonPickerButton * button,
                                                 gint index)
{
  HildonTouchSelector *sel;
  gchar *text;
  g_return_if_fail (HILDON_IS_PICKER_BUTTON (button));

  sel = hildon_picker_button_get_selector (button);
  hildon_touch_selector_set_active (sel, 0, index);

  text = hildon_touch_selector_get_current_text (sel);
  hildon_button_set_value (HILDON_BUTTON (button), text);
  g_free (text);
}

/**
 * hildon_picker_button_get_done_button_text:
 * @button: a #HildonPickerButton
 *
 * Gets the text used in the #HildonPickerDialog that is launched by
 * @button. If no custom text is set, then %NULL is returned.
 *
 * Returns: the custom string to be used, or %NULL if the default
 * #HildonPickerDialog::done-button-text is to be used.
 *
 * Since: 2.2
 **/
const gchar *
hildon_picker_button_get_done_button_text (HildonPickerButton *button)
{
  HildonPickerButtonPrivate *priv;

  g_return_val_if_fail (HILDON_IS_PICKER_BUTTON (button), NULL);

  priv = GET_PRIVATE (button);

  return priv->done_button_text;
}

/**
 * hildon_picker_button_set_done_button_text:
 * @button: a #HildonPickerButton
 * @done_button_text: a string
 *
 * Sets a custom string to be used in the "done" button in #HildonPickerDialog.
 * If unset, the default HildonPickerButton::done-button-text property
 * value will be used.
 *
 * Since: 2.2
 **/
void
hildon_picker_button_set_done_button_text (HildonPickerButton *button,
                                           const gchar *done_button_text)
{
  HildonPickerButtonPrivate *priv;

  g_return_if_fail (HILDON_IS_PICKER_BUTTON (button));
  g_return_if_fail (done_button_text != NULL);

  priv = GET_PRIVATE (button);

  g_free (priv->done_button_text);
  priv->done_button_text = g_strdup (done_button_text);

  if (priv->dialog) {
    hildon_picker_dialog_set_done_label (HILDON_PICKER_DIALOG (priv->dialog),
                                         priv->done_button_text);
  }
}
