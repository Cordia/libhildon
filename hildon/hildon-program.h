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

#ifndef                                         __HILDON_PROGRAM_H__
#define                                         __HILDON_PROGRAM_H__

#include                                        <glib-object.h>

#include                                        "hildon-window.h"
#include                                        "hildon-stackable-window.h"

G_BEGIN_DECLS

typedef struct                                  _HildonProgram HildonProgram;

typedef struct                                  _HildonProgramClass HildonProgramClass;

#define                                         HILDON_TYPE_PROGRAM \
                                                (hildon_program_get_type())

#define                                         HILDON_PROGRAM(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_PROGRAM, HildonProgram))

#define                                         HILDON_PROGRAM_CLASS(obj) \
                                                (G_TYPE_CHECK_CLASS_CAST ((obj), \
                                                HILDON_TYPE_PROGRAM, HildonProgramClass))

#define                                         HILDON_IS_PROGRAM(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HILDON_TYPE_PROGRAM))

#define                                         HILDON_IS_PROGRAM_CLASS(klass) \
                                                (G_CHECK_CLASS_TYPE ((klass), HILDON_TYPE_PROGRAM))

#define                                         HILDON_PROGRAM_GET_CLASS(obj) \
                                                ((HildonProgramClass *) G_OBJECT_GET_CLASS(obj))

struct                                          _HildonProgram
{
    GObject parent;
};

struct                                          _HildonProgramClass
{
    GObjectClass parent;

    /* Padding for future extension */
    void (*_hildon_reserved1)(void);
    void (*_hildon_reserved2)(void);
    void (*_hildon_reserved3)(void);
    void (*_hildon_reserved4)(void);
};

GType G_GNUC_CONST
hildon_program_get_type                         (void);

HildonProgram*
hildon_program_get_instance                     (void);

void
hildon_program_add_window                       (HildonProgram *self, 
                                                 HildonWindow *window);

void
hildon_program_remove_window                    (HildonProgram *self, 
                                                 HildonWindow *window);

void
hildon_program_set_can_hibernate                (HildonProgram *self, 
                                                 gboolean can_hibernate);

gboolean
hildon_program_get_can_hibernate                (HildonProgram *self);

void
hildon_program_set_common_menu                  (HildonProgram *self, 
                                                 GtkMenu *menu);

GtkMenu*
hildon_program_get_common_menu                  (HildonProgram *self);

void
hildon_program_set_common_app_menu              (HildonProgram *self,
                                                 HildonAppMenu *menu);

HildonAppMenu*
hildon_program_get_common_app_menu              (HildonProgram *self);

void
hildon_program_set_common_toolbar               (HildonProgram *self, 
                                                 GtkToolbar *toolbar);

GtkToolbar*
hildon_program_get_common_toolbar               (HildonProgram *self);

gboolean
hildon_program_get_is_topmost                   (HildonProgram *self);

G_END_DECLS

#endif                                          /* __HILDON_PROGRAM_H__ */
