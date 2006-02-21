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

#ifndef __HILDON_PROGRAM_H__
#define __HILDON_PROGRAM_H__

#include<glib-object.h>
#include"hildon-window.h"

G_BEGIN_DECLS
typedef struct _HildonProgram HildonProgram;
typedef struct _HildonProgramClass HildonProgramClass;

#define HILDON_TYPE_PROGRAM (hildon_program_get_type())
#define HILDON_PROGRAM(obj) (G_TYPE_CHECK_INSTANCE_CAST (obj, \
            HILDON_TYPE_PROGRAM, \
            HildonProgram))
#define HILDON_IS_PROGRAM(obj) (G_TYPE_CHECK_INSTANCE_TYPE (obj, \
            HILDON_TYPE_PROGRAM))

/* deprecated */
#define hildon_program_new hildon_program_get_instance



struct _HildonProgram
{
    GObject parent;
};

struct _HildonProgramClass
{
    GObjectClass parent;

    /* Padding for future extension */
    void (*_hildon_reserved1)(void);
    void (*_hildon_reserved2)(void);
    void (*_hildon_reserved3)(void);
    void (*_hildon_reserved4)(void);

};

GType
hildon_program_get_type (void);

/* Public methods */

HildonProgram *
hildon_program_get_instance (void);

void
hildon_program_add_window (HildonProgram *self, HildonWindow *window);

void
hildon_program_remove_window (HildonProgram *self, HildonWindow *window);

void
hildon_program_set_can_hibernate (HildonProgram *self, gboolean killable);

gboolean
hildon_program_get_can_hibernate (HildonProgram *self);

void
hildon_program_set_common_menu (HildonProgram *self, GtkMenu *menu);

GtkMenu *
hildon_program_get_common_menu (HildonProgram *self);

void
hildon_program_set_common_toolbar (HildonProgram *self, GtkToolbar *toolbar);

GtkToolbar *
hildon_program_get_common_toolbar (HildonProgram *self);

gboolean
hildon_program_get_is_topmost (HildonProgram *self);

G_END_DECLS
#endif /* HILDON_PROGRAM_H */
