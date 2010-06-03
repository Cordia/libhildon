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

#include <libintl.h>

#include "hildon-stock.h"
#include "hildon-date-button.h"
#include "hildon-date-selector.h"
#include "hildon-touch-selector.h"
#include "hildon-picker-button-private.h"

/**
 * SECTION:hildon-date-button
 * @Short_Description: Button displaying and allowing selection of a date.
 * @See_Also: #HildonPickerButton, #HildonTimeButton
 *
 * #HildonDateButton is a widget that shows a text label and a date, and allows
 * the user to select a different date. Visually, it's a button that, once clicked,
 * presents a #HildonPickerDialog containing a #HildonDateSelector. Once the user selects
 * a different date from the selector, this will be shown in the button.
 */

G_DEFINE_TYPE (HildonDateButton, hildon_date_button, HILDON_TYPE_PICKER_BUTTON)

#if 0
#define GET_PRIVATE(o)                                                  \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), HILDON_TYPE_DATE_BUTTON, HildonDateButtonPrivate))

typedef struct _HildonDateButtonPrivate HildonDateButtonPrivate;

struct _HildonDateButtonPrivate
{
};
#endif

#if 0
static void
hildon_date_button_get_property (GObject * object, guint property_id,
                                 GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
hildon_date_button_set_property (GObject * object, guint property_id,
                                 const GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}
#endif

static GObject *
hildon_date_button_constructor (GType type,
				guint n_construct_params,
				GObjectConstructParam *construct_params)
{
  GObject *object;

  object = G_OBJECT_CLASS (hildon_date_button_parent_class)->constructor (type,
                                                                          n_construct_params,
                                                                          construct_params);
  gtk_button_set_use_stock (GTK_BUTTON (object), TRUE);

  return object;
}

static void
hildon_date_button_class_init (HildonDateButtonClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = hildon_date_button_constructor;
#if 0
  g_type_class_add_private (klass, sizeof (HildonDateButtonPrivate));

  object_class->get_property = hildon_date_button_get_property;
  object_class->set_property = hildon_date_button_set_property;
#endif
}

static void
hildon_date_button_init (HildonDateButton * self)
{
  GtkWidget *date_selector;

  date_selector = hildon_date_selector_new ();

  hildon_picker_button_set_selector (HILDON_PICKER_BUTTON (self),
                                     HILDON_TOUCH_SELECTOR (date_selector));
}

static GtkWidget *
hildon_date_button_new_full (HildonSizeType           size,
                             HildonButtonArrangement  arrangement,
                             GtkWidget               *selector)
{
  return g_object_new (HILDON_TYPE_DATE_BUTTON,
                       "title", HILDON_STOCK_DATE,
                       "arrangement", arrangement,
                       "size", size,
                       "touch-selector", selector,
                       NULL);
}

/**
 * hildon_date_button_new:
 * @size: One of #HildonSizeType
 * @arrangement: one of #HildonButtonArrangement
 *
 * Creates a new #HildonDateButton. See hildon_button_new() for details on the
 * parameters.
 *
 * Returns: a new #HildonDateButton
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_date_button_new (HildonSizeType          size,
                        HildonButtonArrangement arrangement)
{
  GtkWidget *selector = hildon_date_selector_new ();
  return hildon_date_button_new_full (size, arrangement, selector);
}

/**
 * hildon_date_button_new_with_year_range:
 * @size: One of #HildonSizeType
 * @arrangement: one of #HildonButtonArrangement
 * @min_year: the minimum available year or -1 to ignore
 * @max_year: the maximum available year or -1 to ignore
 *
 * Creates a new #HildonDateButton with a specific valid range of years.
 * See hildon_date_selector_new_with_year_range() for details on the range.
 *
 * Returns: a new #HildonDateButton
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_date_button_new_with_year_range (HildonSizeType size,
                                        HildonButtonArrangement arrangement,
                                        gint min_year,
                                        gint max_year)
{
  GtkWidget *selector;
  selector = hildon_date_selector_new_with_year_range (min_year, max_year);
  return hildon_date_button_new_full (size, arrangement, selector);
}

/**
 * hildon_date_button_get_date:
 * @button: a #HildonDateButton
 * @year: return location for the selected year
 * @month: return location for the selected month
 * @day: return location for the selected day
 *
 * Retrieves currently selected date from @button.
 *
 * Since: 2.2
 **/
void
hildon_date_button_get_date (HildonDateButton * button,
                             guint * year, guint * month, guint * day)
{
  HildonTouchSelector *selector;

  g_return_if_fail (HILDON_IS_DATE_BUTTON (button));

  selector = hildon_picker_button_get_selector (HILDON_PICKER_BUTTON (button));

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));

  hildon_date_selector_get_date (HILDON_DATE_SELECTOR (selector), year, month, day);
}

/**
 * hildon_date_button_set_date:
 * @button: a #HildonDateButton
 * @year: the year to set.
 * @month: the month number to set.
 * @day: the day of the month to set.
 *
 * Sets the date in @button. The date set will be displayed
 * and will be the default selected option on the shown #HildonDateSelector.
 *
 * Since: 2.2
 **/
void
hildon_date_button_set_date (HildonDateButton * button,
                             guint year, guint month, guint day)
{
  HildonTouchSelector *selector;
  gchar *date;

  g_return_if_fail (HILDON_IS_DATE_BUTTON (button));

  selector = hildon_picker_button_get_selector (HILDON_PICKER_BUTTON (button));

  g_return_if_fail (HILDON_IS_TOUCH_SELECTOR (selector));

  hildon_picker_button_disable_value_changed (HILDON_PICKER_BUTTON (button), TRUE);
  hildon_date_selector_select_current_date (HILDON_DATE_SELECTOR (selector),
                                            year, month, day);
  hildon_picker_button_disable_value_changed (HILDON_PICKER_BUTTON (button), FALSE);

  date = hildon_touch_selector_get_current_text (HILDON_TOUCH_SELECTOR (selector));

  hildon_button_set_value (HILDON_BUTTON (button), date);

  g_free (date);

  hildon_picker_button_value_changed (HILDON_PICKER_BUTTON (button));
}
