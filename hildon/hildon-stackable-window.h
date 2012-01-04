/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
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

#ifndef                                         __HILDON_STACKABLE_WINDOW_H__
#define                                         __HILDON_STACKABLE_WINDOW_H__

#include                                        "hildon-window.h"
#include                                        "hildon-app-menu.h"

G_BEGIN_DECLS

#define                                         HILDON_TYPE_STACKABLE_WINDOW \
                                                (hildon_stackable_window_get_type())

#define                                         HILDON_STACKABLE_WINDOW(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_STACKABLE_WINDOW, \
                                                HildonStackableWindow))

#define                                         HILDON_STACKABLE_WINDOW_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                HILDON_TYPE_STACKABLE_WINDOW, \
                                                HildonStackableWindowClass))

#define                                         HILDON_IS_STACKABLE_WINDOW(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                                HILDON_TYPE_STACKABLE_WINDOW))

#define                                         HILDON_IS_STACKABLE_WINDOW_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                HILDON_TYPE_STACKABLE_WINDOW))

#define                                         HILDON_STACKABLE_WINDOW_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                HILDON_TYPE_STACKABLE_WINDOW, \
                                                HildonStackableWindowClass))

#ifndef _TYPEDEF_HILDON_WINDOW_STACK_
#define _TYPEDEF_HILDON_WINDOW_STACK_
typedef struct                                  _HildonWindowStack HildonWindowStack;
#endif

typedef struct                                  _HildonStackableWindow HildonStackableWindow;
typedef struct                                  _HildonStackableWindowClass HildonStackableWindowClass;

struct                                          _HildonStackableWindowClass
{
    HildonWindowClass parent_class;

    /* Padding for future extension */
    void (*_hildon_reserved1)(void);
    void (*_hildon_reserved2)(void);
    void (*_hildon_reserved3)(void);
    void (*_hildon_reserved4)(void);
};

struct                                          _HildonStackableWindow
{
    HildonWindow parent;
};

GType
hildon_stackable_window_get_type                (void) G_GNUC_CONST;

GtkWidget*
hildon_stackable_window_new                     (void);

HildonWindowStack *
hildon_stackable_window_get_stack               (HildonStackableWindow *self);

G_END_DECLS

#endif                                 /* __HILDON_STACKABLE_WINDOW_H__ */
