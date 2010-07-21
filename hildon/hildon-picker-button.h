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

#ifndef                                         __HILDON_PICKER_BUTTON__
#define                                         __HILDON_PICKER_BUTTON__

#include                                        <glib-object.h>

#include                                        "hildon-button.h"
#include                                        "hildon-touch-selector.h"

G_BEGIN_DECLS

#define                                         HILDON_TYPE_PICKER_BUTTON \
                                                hildon_picker_button_get_type()

#define                                         HILDON_PICKER_BUTTON(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_PICKER_BUTTON, HildonPickerButton))

#define                                         HILDON_PICKER_BUTTON_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                HILDON_TYPE_PICKER_BUTTON, HildonPickerButtonClass))

#define                                         HILDON_IS_PICKER_BUTTON(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HILDON_TYPE_PICKER_BUTTON))

#define                                         HILDON_IS_PICKER_BUTTON_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), HILDON_TYPE_PICKER_BUTTON))

#define                                         HILDON_PICKER_BUTTON_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                HILDON_TYPE_PICKER_BUTTON, HildonPickerButtonClass))

typedef struct                                  _HildonPickerButton HildonPickerButton;
typedef struct                                  _HildonPickerButtonClass HildonPickerButtonClass;

struct                                          _HildonPickerButton
{
  HildonButton parent;
};

struct                                          _HildonPickerButtonClass
{
  HildonButtonClass parent_class;
};

GType
hildon_picker_button_get_type                   (void);

GtkWidget*
hildon_picker_button_new                        (HildonSizeType          size,
                                                 HildonButtonArrangement arrangement);

void
hildon_picker_button_set_selector               (HildonPickerButton * button,
                                                 HildonTouchSelector *selector);

HildonTouchSelector*
hildon_picker_button_get_selector               (HildonPickerButton *button);

void
hildon_picker_button_set_active                 (HildonPickerButton * button,
                                                 gint index);

gint
hildon_picker_button_get_active                 (HildonPickerButton * button);

void
hildon_picker_button_set_active_iter            (HildonPickerButton *button,
                                                 GtkTreeIter        *iter);

gboolean
hildon_picker_button_get_active_iter            (HildonPickerButton *button,
                                                 GtkTreeIter        *iter);

const gchar *
hildon_picker_button_get_done_button_text       (HildonPickerButton * button);

void
hildon_picker_button_set_done_button_text       (HildonPickerButton *button,
                                                 const gchar *done_button_text);

void
hildon_picker_button_value_changed              (HildonPickerButton *button);

G_END_DECLS

#endif /* __HILDON_PICKER_BUTTON__ */
