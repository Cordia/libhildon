/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006, 2009 Nokia Corporation, all rights reserved.
 *
 * Contact: Rodrigo Novo <rodrigo.novo@nokia.com>
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

#ifndef                                         __HILDON_PRIVATE_H__
#define                                         __HILDON_PRIVATE_H__

#include                                        <gtk/gtk.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL GtkWidget *
hildon_private_create_animation                 (gfloat       framerate,
                                                 const gchar *template,
                                                 gint         nframes);

G_GNUC_INTERNAL void
hildon_gtk_window_set_clear_window_flag                           (GtkWindow   *window,
                                                                   const gchar *atomname,
                                                                   Atom         xatom,
                                                                   gboolean     flag);

typedef void (*HildonFlagFunc) (GtkWindow *window, gpointer userdata);

G_GNUC_INTERNAL void
hildon_gtk_window_set_flag                                        (GtkWindow      *window,
                                                                   HildonFlagFunc  func,
                                                                   gpointer        userdata);

G_END_DECLS

#endif                                          /* __HILDON_PRIVATE_H__ */
