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

#ifndef                                         __HILDON_PICKER_DIALOG_H__
#define                                         __HILDON_PICKER_DIALOG_H__

#include                                        "hildon-dialog.h"
#include                                        "hildon-touch-selector.h"

G_BEGIN_DECLS

#define                                         HILDON_TYPE_PICKER_DIALOG \
                                                (hildon_picker_dialog_get_type ())

#define                                         HILDON_PICKER_DIALOG(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_PICKER_DIALOG, HildonPickerDialog))

#define                                         HILDON_PICKER_DIALOG_CLASS(vtable) \
                                                (G_TYPE_CHECK_CLASS_CAST ((vtable), \
                                                HILDON_TYPE_PICKER_DIALOG, HildonPickerDialogClass))

#define                                         HILDON_IS_PICKER_DIALOG(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HILDON_TYPE_PICKER_DIALOG))

#define                                         HILDON_IS_PICKER_DIALOG_CLASS(vtable) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((vtable), HILDON_TYPE_PICKER_DIALOG))

#define                                         HILDON_PICKER_DIALOG_GET_CLASS(inst) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((inst), \
                                                HILDON_TYPE_PICKER_DIALOG, HildonPickerDialogClass))

typedef struct                                  _HildonPickerDialog HildonPickerDialog;
typedef struct                                  _HildonPickerDialogClass HildonPickerDialogClass;
typedef struct                                  _HildonPickerDialogPrivate HildonPickerDialogPrivate;

struct                                          _HildonPickerDialog
{
  HildonDialog parent_instance;

  /*< private > */
  HildonPickerDialogPrivate *priv;
};

struct                                          _HildonPickerDialogClass
{
  HildonDialogClass parent_class;

  gboolean (* set_selector)                     (HildonPickerDialog  *dialog,
                                                 HildonTouchSelector *selector);
};

/* construction */
GType
hildon_picker_dialog_get_type                   (void) G_GNUC_CONST;

GtkWidget*
hildon_picker_dialog_new                        (GtkWindow * parent);

gboolean
hildon_picker_dialog_set_selector               (HildonPickerDialog  *dialog,
                                                 HildonTouchSelector *selector);

void
hildon_picker_dialog_set_done_label             (HildonPickerDialog *dialog,
                                                 const gchar        *label);

const gchar*
hildon_picker_dialog_get_done_label             (HildonPickerDialog *dialog);

HildonTouchSelector*
hildon_picker_dialog_get_selector               (HildonPickerDialog *dialog);

G_END_DECLS

#endif /* __HILDON_PICKER_SELECTOR_H__ */
