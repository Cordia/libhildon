/*
 * This file is part of hildon-libs
 *
 * Copyright (C) 2006 Nokia Corporation.
 *
 * Contact: Luc Pionchon <luc.pionchon@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef HILDON_CODE_DIALOG_H
#define HILDON_CODE_DIALOG_H

#include <gtk/gtkdialog.h>

G_BEGIN_DECLS


#define HILDON_TYPE_CODE_DIALOG ( hildon_code_dialog_get_type() )
#define HILDON_CODE_DIALOG(obj) \
    (GTK_CHECK_CAST (obj, HILDON_TYPE_CODE_DIALOG, HildonCodeDialog))
#define HILDON_CODE_DIALOG_CLASS(klass) \
    (GTK_CHECK_CLASS_CAST ((klass),\
     HILDON_TYPE_CODE_DIALOG, HildonCodeDialogClass))
#define HILDON_IS_CODE_DIALOG(obj) (GTK_CHECK_TYPE (obj, HILDON_TYPE_CODE_DIALOG))
#define HILDON_IS_CODE_DIALOG_CLASS(klass) \
    (GTK_CHECK_CLASS_TYPE ((klass), HILDON_TYPE_CODE_DIALOG))


typedef struct _HildonCodeDialogPrivate HildonCodeDialogPrivate;
typedef struct _HildonCodeDialog HildonCodeDialog;
typedef struct _HildonCodeDialogClass HildonCodeDialogClass;


struct _HildonCodeDialog
{
    GtkDialog parent;

    HildonCodeDialogPrivate *priv;
};

struct _HildonCodeDialogClass
{
    GtkDialogClass parent_class;
};


GType hildon_code_dialog_get_type (void);
GtkWidget *hildon_code_dialog_new(void);
const gchar *hildon_code_dialog_get_code (HildonCodeDialog *dialog);
void hildon_code_dialog_clear_code (HildonCodeDialog *dialog);
void hildon_code_dialog_set_help_text (HildonCodeDialog *dialog,
                                       const gchar *text);


G_END_DECLS
#endif
