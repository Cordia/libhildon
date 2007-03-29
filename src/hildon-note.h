/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006 Nokia Corporation, all rights reserved.
 *
 * Contact: Michael Dominic Kostrzewa <michael.kostrzewa@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
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

#ifndef                                         __HILDON_NOTE_H__
#define                                         __HILDON_NOTE_H__

#include                                        <gtk/gtkdialog.h>
#include                                        <gtk/gtkprogressbar.h>

G_BEGIN_DECLS

#define                                         HILDON_TYPE_NOTE \
                                                (hildon_note_get_type())

#define                                         HILDON_NOTE(obj) \
                                                (GTK_CHECK_CAST (obj, HILDON_TYPE_NOTE, HildonNote))

#define                                         HILDON_NOTE_CLASS(klass) \
                                                (GTK_CHECK_CLASS_CAST ((klass), \
                                                HILDON_TYPE_NOTE, HildonNoteClass))

#define                                         HILDON_IS_NOTE(obj) \
                                                (GTK_CHECK_TYPE (obj, HILDON_TYPE_NOTE))

#define                                         HILDON_IS_NOTE_CLASS(klass) \
                                                (GTK_CHECK_CLASS_TYPE ((klass), HILDON_TYPE_NOTE))

typedef struct                                  _HildonNote HildonNote;

typedef struct                                  _HildonNoteClass HildonNoteClass;

struct                                          _HildonNote 
{
    GtkDialog parent;
};

struct _HildonNoteClass 
{
    GtkDialogClass parent_class;
};

GType G_GNUC_CONST
hildon_note_get_type                            (void);

GtkWidget*
hildon_note_new_confirmation                    (GtkWindow *parent,
                                                 const gchar *description);

GtkWidget*
hildon_note_new_confirmation_add_buttons        (GtkWindow *parent,
                                                 const gchar *description, 
                                                 ...);

GtkWidget*
hildon_note_new_confirmation_with_icon_name     (GtkWindow *parent,
                                                 const gchar *description,
                                                 const gchar *icon_name);

GtkWidget*
hildon_note_new_cancel_with_progress_bar        (GtkWindow *parent,
                                                 const gchar *description,
                                                 GtkProgressBar *progressbar);

GtkWidget*
hildon_note_new_information                     (GtkWindow *parent,
                                                 const gchar *description);
                                       
GtkWidget*
hildon_note_new_information_with_icon_name      (GtkWindow *parent,
                                                 const gchar *description,
                                                 const gchar *icon_name);
                                                       
void 
hildon_note_set_button_text                     (HildonNote *note, 
                                                 const gchar *text);

void 
hildon_note_set_button_texts                    (HildonNote *note, 
                                                 const gchar *text_ok,
                                                 const gchar *text_cancel);

typedef enum
{
    HILDON_NOTE_TYPE_CONFIRMATION = 0,
    HILDON_NOTE_TYPE_CONFIRMATION_BUTTON,
    HILDON_NOTE_TYPE_INFORMATION,
    HILDON_NOTE_TYPE_INFORMATION_THEME,
    HILDON_NOTE_TYPE_PROGRESSBAR
}                                               HildonNoteType;

G_END_DECLS

#endif                                          /* __HILDON_NOTE_H__ */
