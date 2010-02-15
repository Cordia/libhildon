/*
 * This file is a part of hildon
 *
 * Copyright (C) 2009 Nokia Corporation.
 *
 * Contact: Rodrigo Novo <rodrigo.novo@nokia.com>
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
#ifndef                                         __HILDON_TOUCH_SELECTOR_PRIVATE_H__
#define                                         __HILDON_TOUCH_SELECTOR_PRIVATE_H__


G_BEGIN_DECLS

void G_GNUC_INTERNAL
hildon_touch_selector_block_changed             (HildonTouchSelector *selector);

void G_GNUC_INTERNAL
hildon_touch_selector_unblock_changed           (HildonTouchSelector *selector);

void G_GNUC_INTERNAL
hildon_touch_selector_column_disable_focus      (HildonTouchSelectorColumn *col);

G_END_DECLS

#endif
