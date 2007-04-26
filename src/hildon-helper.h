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

#ifndef                                         __HILDON_HELPER_H__
#define                                         __HILDON_HELPER_H__

#include                                        <gtk/gtkwindow.h>
#include                                        <gdk/gdkkeysyms.h>
#include                                        <gdk/gdkevents.h>
#include                                        "hildon-defines.h"

G_BEGIN_DECLS

gulong
hildon_helper_set_logical_font                  (GtkWidget *widget, 
                                                 const gchar *logicalfontname);

gulong
hildon_helper_set_logical_color                 (GtkWidget *widget, 
                                                 GtkRcFlags rcflags,
                                                 GtkStateType state, 
                                                 const gchar *logicalcolorname);
gboolean 
hildon_helper_event_button_is_finger            (GdkEventButton *event);

void
hildon_helper_set_insensitive_message           (GtkWidget *widget,
                                                 const gchar *message);

void
hildon_helper_set_insensitive_messagef          (GtkWidget *widget,
						 const gchar *format,
						 ...);

void
hildon_helper_set_thumb_scrollbar               (GtkScrolledWindow *win, 
                                                 gboolean thumb);

G_END_DECLS

#endif                                          /* __HILDON_HELPER_H__ */

