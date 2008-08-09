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
 * SECTION:hildon-time-button
 * @Short_Description: Button displaying and allowing selection of a time.
 * @See_Also: #HildonPickerButton, #HildonDateButton
 *
 * #HildonTimeButton is a widget that shows a text label and a time, and allows
 * the user to select a different time. Visually, it's a button that, once clicked,
 * presents a #HildonPickerDialog containing a #HildonTimeSelector. Once the user selects
 * a different time from the selector, this will be shown in the button.
 */

#include "hildon-time-selector.h"
#include "hildon-touch-selector.h"
#include "hildon-picker-button.h"
#include "hildon-time-button.h"

G_DEFINE_TYPE (HildonTimeButton, hildon_time_button, HILDON_TYPE_PICKER_BUTTON)

#if 0
#define GET_PRIVATE(o)                                                  \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), HILDON_TYPE_TIME_BUTTON, HildonTimeButtonPrivate))
typedef struct _HildonTimeButtonPrivate HildonTimeButtonPrivate;

struct _HildonTimeButtonPrivate
{
};
#endif

#if 0
static void
hildon_time_button_get_property (GObject * object, guint property_id,
                                 GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
hildon_time_button_set_property (GObject * object, guint property_id,
                                 const GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}
#endif

static void
hildon_time_button_class_init (HildonTimeButtonClass * klass)
{
#if 0
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (HildonTimeButtonPrivate));

  object_class->get_property = hildon_time_button_get_property;
  object_class->set_property = hildon_time_button_set_property;
#endif
}

static void
hildon_time_button_init (HildonTimeButton * self)
{
  GtkWidget *time_selector;

  time_selector = hildon_time_selector_new ();

  hildon_picker_button_set_selector (HILDON_PICKER_BUTTON (self),
                                     HILDON_TOUCH_SELECTOR (time_selector));
}

/**
 * hildon_time_button_new:
 * @size: One of #HildonSizeType
 * @arrangement: one of #HildonButtonArrangement
 *
 * Creates a new #HildonTimeButton. See hildon_button_new() for details on the
 * parameters.
 *
 * Returns: a new #HildonTimeButton
 **/
GtkWidget *
hildon_time_button_new (HildonSizeType          size,
                        HildonButtonArrangement arrangement)
{
  return g_object_new (HILDON_TYPE_TIME_BUTTON,
                       "title", "Time", "arrangement", arrangement, "size", size, NULL);
}

/**
 * hildon_time_button_get_time:
 * @button: a #HildonTimeButton
 * @hours: return location for the hours of the time selected
 * @minutes: return location for the minutes of the time selected
 *
 * Retrieves the time from @button.
 **/
void
hildon_time_button_get_time (HildonTimeButton * button,
                             guint * hours, guint * minutes)
{
  HildonTouchSelector *selector;

  g_return_if_fail (HILDON_IS_TIME_BUTTON (button));

  selector = hildon_picker_button_get_selector (HILDON_PICKER_BUTTON (button));

  hildon_time_selector_get_time (HILDON_TIME_SELECTOR (selector), hours, minutes);
}

/**
 * hildon_time_button_set_time:
 * @button: a #HildonTimeButton
 * @hours: the hours to be set
 * @minutes: the time to be set
 *
 * Sets the time to be displayed in @button. This time will
 * be selected by default on the #HildonTimeSelector.
 **/
void
hildon_time_button_set_time (HildonTimeButton * button,
                             guint hours, guint minutes)
{
  HildonTouchSelector *selector;
  gchar *time;

  g_return_if_fail (HILDON_IS_TIME_BUTTON (button));

  selector = hildon_picker_button_get_selector (HILDON_PICKER_BUTTON (button));

  hildon_time_selector_set_time (HILDON_TIME_SELECTOR (selector), hours, minutes);
  time = hildon_touch_selector_get_current_text (HILDON_TOUCH_SELECTOR (selector));

  hildon_button_set_value (HILDON_BUTTON (button), time);
}
