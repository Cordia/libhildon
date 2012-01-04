/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version. or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef                                         __HILDON_TOUCH_SELECTOR_ENTRY__
#define                                         __HILDON_TOUCH_SELECTOR_ENTRY__

#include                                        "hildon-touch-selector.h"
#include                                        "hildon-entry.h"

G_BEGIN_DECLS

#define                                         HILDON_TYPE_TOUCH_SELECTOR_ENTRY \
                                                (hildon_touch_selector_entry_get_type())

#define                                         HILDON_TOUCH_SELECTOR_ENTRY(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_TOUCH_SELECTOR_ENTRY, HildonTouchSelectorEntry))

#define                                         HILDON_TOUCH_SELECTOR_ENTRY_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                HILDON_TYPE_TOUCH_SELECTOR_ENTRY, HildonTouchSelectorEntryClass))

#define                                         HILDON_IS_TOUCH_SELECTOR_ENTRY(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                                HILDON_TYPE_TOUCH_SELECTOR_ENTRY))

#define                                         HILDON_IS_TOUCH_SELECTOR_ENTRY_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                HILDON_TYPE_TOUCH_SELECTOR_ENTRY))

#define                                         HILDON_TOUCH_SELECTOR_ENTRY_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                HILDON_TYPE_TOUCH_SELECTOR_ENTRY, HildonTouchSelectorEntryClass))

typedef struct                                  _HildonTouchSelectorEntry HildonTouchSelectorEntry;
typedef struct                                  _HildonTouchSelectorEntryClass HildonTouchSelectorEntryClass;

struct                                          _HildonTouchSelectorEntry
{
  HildonTouchSelector parent;

  /*< private > */
};

struct                                          _HildonTouchSelectorEntryClass
{
  HildonTouchSelectorClass parent_class;
};

GType
hildon_touch_selector_entry_get_type            (void);

GtkWidget*
hildon_touch_selector_entry_new                 (void);

GtkWidget*
hildon_touch_selector_entry_new_text            (void);

void
hildon_touch_selector_entry_set_text_column     (HildonTouchSelectorEntry *selector,
                                                 gint                      text_column);
gint
hildon_touch_selector_entry_get_text_column     (HildonTouchSelectorEntry *selector);

HildonEntry *
hildon_touch_selector_entry_get_entry (HildonTouchSelectorEntry * selector);

G_END_DECLS

#endif /* __HILDON_TOUCH_SELECTOR_ENTRY__ */

