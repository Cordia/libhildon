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

#ifndef                                         __HILDON_PROGRAM_PRIVATE_H__
#define                                         __HILDON_PROGRAM_PRIVATE_H__

#include                                        "hildon-window.h"
#include                                        "hildon-stackable-window.h"

G_BEGIN_DECLS

#define                                         HILDON_PROGRAM_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                HILDON_TYPE_PROGRAM, HildonProgramPrivate));

typedef struct                                  _HildonProgramPrivate HildonProgramPrivate;

struct                                          _HildonProgramPrivate
{
    gboolean killable;
    gboolean is_topmost;
    guint window_count;
    GtkMenu *common_menu;
    HildonAppMenu *common_app_menu;
    GtkWidget *common_toolbar;
    GSList *windows;
};

G_END_DECLS

#endif                                          /* __HILDON_PROGRAM_PRIVATE_H__ */
