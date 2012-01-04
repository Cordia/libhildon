/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 */

#ifndef                                         __HILDON_ENTRY_H__
#define                                         __HILDON_ENTRY_H__

#include                                        "hildon-gtk.h"

G_BEGIN_DECLS

#define                                         HILDON_TYPE_ENTRY \
                                                (hildon_entry_get_type())

#define                                         HILDON_ENTRY(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_ENTRY, HildonEntry))

#define                                         HILDON_ENTRY_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                HILDON_TYPE_ENTRY, HildonEntryClass))

#define                                         HILDON_IS_ENTRY(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HILDON_TYPE_ENTRY))

#define                                         HILDON_IS_ENTRY_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), HILDON_TYPE_ENTRY))

#define                                         HILDON_ENTRY_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                HILDON_TYPE_ENTRY, HildonEntryClass))

typedef struct                                  _HildonEntry HildonEntry;

typedef struct                                  _HildonEntryClass HildonEntryClass;

typedef struct                                  _HildonEntryPrivate HildonEntryPrivate;

struct                                          _HildonEntryClass
{
    GtkEntryClass parent_class;
};

struct                                          _HildonEntry
{
    GtkEntry parent;

    /* private */
    HildonEntryPrivate *priv;
};


GType
hildon_entry_get_type                           (void) G_GNUC_CONST;

GtkWidget *
hildon_entry_new                                (HildonSizeType size);

G_END_DECLS

#endif /* __HILDON_ENTRY_H__ */
