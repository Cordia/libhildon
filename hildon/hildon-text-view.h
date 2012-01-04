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

#ifndef                                         __HILDON_TEXT_VIEW_H__
#define                                         __HILDON_TEXT_VIEW_H__

#include                                        <gtk/gtk.h>

G_BEGIN_DECLS

#define                                         HILDON_TYPE_TEXT_VIEW \
                                                (hildon_text_view_get_type())

#define                                         HILDON_TEXT_VIEW(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_TEXT_VIEW, HildonTextView))

#define                                         HILDON_TEXT_VIEW_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                HILDON_TYPE_TEXT_VIEW, HildonTextViewClass))

#define                                         HILDON_IS_TEXT_VIEW(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HILDON_TYPE_TEXT_VIEW))

#define                                         HILDON_IS_TEXT_VIEW_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), HILDON_TYPE_TEXT_VIEW))

#define                                         HILDON_TEXT_VIEW_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                HILDON_TYPE_TEXT_VIEW, HildonTextViewClass))

typedef struct                                  _HildonTextView HildonTextView;

typedef struct                                  _HildonTextViewClass HildonTextViewClass;

struct                                          _HildonTextViewClass
{
    GtkTextViewClass parent_class;
};

struct                                          _HildonTextView
{
    GtkTextView parent;
};


GType
hildon_text_view_get_type                       (void) G_GNUC_CONST;

GtkWidget *
hildon_text_view_new                            (void);

G_END_DECLS

#endif /* __HILDON_TEXT_VIEW_H__ */
