/*
 * This file is a part of hildon
 *
 * Copyright (C) 2006 Nokia Corporation, all rights reserved.
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

#ifndef                                         __HILDON_WINDOW_PRIVATE_H__
#define                                         __HILDON_WINDOW_PRIVATE_H__

#include                                        "hildon-program.h"

#include                                        <X11/Xlib.h>

G_BEGIN_DECLS

typedef struct                                  _HildonWindowPrivate HildonWindowPrivate;

struct                                          _HildonWindowPrivate
{
    GtkMenu *menu;
    HildonAppMenu *app_menu;
    GtkWidget *vbox;
    GtkWidget *edit_toolbar;

    GtkBorder *borders;
    GtkBorder *toolbar_borders;

    GtkAllocation allocation;

    gchar *markup;

    guint fullscreen;
    guint is_topmost;
    gint visible_toolbars;
    gint previous_vbox_y;

    HildonProgram *program;
};

#define                                         HILDON_WINDOW_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
                                                HILDON_TYPE_WINDOW, HildonWindowPrivate))

void G_GNUC_INTERNAL
hildon_window_set_program                       (HildonWindow *self, 
                                                 GObject *program);

void G_GNUC_INTERNAL
hildon_window_unset_program                     (HildonWindow *self);

void G_GNUC_INTERNAL
hildon_window_set_can_hibernate_property        (HildonWindow *self, 
                                                 gpointer can_hibernate);

void G_GNUC_INTERNAL
hildon_window_take_common_toolbar               (HildonWindow *self);

void G_GNUC_INTERNAL
hildon_window_update_topmost                    (HildonWindow *self, 
                                                 Window window_id);

Window G_GNUC_INTERNAL
hildon_window_get_active_window                 (void);

void G_GNUC_INTERNAL
hildon_window_update_title                      (HildonWindow *window);

void G_GNUC_INTERNAL
hildon_window_set_menu_flag                     (HildonWindow *window,
                                                 gboolean   has_menu);
G_END_DECLS

#endif                                          /* __HILDON_WINDOW_PRIVATE_H__ */
