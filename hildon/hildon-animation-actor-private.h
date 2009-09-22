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

#ifndef                                         __HILDON_ANIMATION_ACTOR_PRIVATE_H__
#define                                         __HILDON_ANIMATION_ACTOR_PRIVATE_H__

G_BEGIN_DECLS

typedef struct                                  _HildonAnimationActorPrivate HildonAnimationActorPrivate;

#define                                         HILDON_ANIMATION_ACTOR_GET_PRIVATE(obj) \
                                                (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                HILDON_TYPE_ANIMATION_ACTOR, HildonAnimationActorPrivate));

struct                                          _HildonAnimationActorPrivate
{
    guint      ready : 1;

    guint      set_show : 1;
    guint      set_position : 1;
    guint      set_rotation : 3;
    guint      set_scale : 1;
    guint      set_anchor : 1;
    guint      set_parent : 1;

    gboolean   show;
    guint      opacity;

    guint      gravity;
    guint      anchor_x;
    guint      anchor_y;

    guint      position_x;
    guint      position_y;
    guint      depth;

    guint32    x_rotation_angle;
    guint      x_rotation_y;
    guint      x_rotation_z;

    guint32    y_rotation_angle;
    guint      y_rotation_x;
    guint      y_rotation_z;

    guint32    z_rotation_angle;
    guint      z_rotation_x;
    guint      z_rotation_y;

    guint32    scale_x;
    guint32    scale_y;

    GtkWindow *parent;
    gulong     parent_map_event_cb_id;

    gulong     map_event_cb_id;
};

G_END_DECLS

#endif                                          /* __HILDON_ANIMATION_ACTOR_PRIVATE_H__ */
