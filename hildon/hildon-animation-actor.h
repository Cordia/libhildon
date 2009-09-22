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

#ifndef                                         __HILDON_ANIMATION_ACTOR_H__
#define                                         __HILDON_ANIMATION_ACTOR_H__

#include                                        "hildon-window.h"

G_BEGIN_DECLS

#define                                         HILDON_TYPE_ANIMATION_ACTOR \
                                                (hildon_animation_actor_get_type())

#define                                         HILDON_ANIMATION_ACTOR(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_ANIMATION_ACTOR, \
                                                HildonAnimationActor))

#define                                         HILDON_ANIMATION_ACTOR_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                HILDON_TYPE_ANIMATION_ACTOR, \
                                                HildonAnimationActorClass))

#define                                         HILDON_IS_ANIMATION_ACTOR(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                                HILDON_TYPE_ANIMATION_ACTOR))

#define                                         HILDON_IS_ANIMATION_ACTOR_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                HILDON_TYPE_ANIMATION_ACTOR))

#define                                         HILDON_ANIMATION_ACTOR_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                HILDON_TYPE_ANIMATION_ACTOR, \
                                                HildonAnimationActorClass))

typedef struct                                  _HildonAnimationActor HildonAnimationActor;
typedef struct                                  _HildonAnimationActorClass HildonAnimationActorClass;

struct                                          _HildonAnimationActorClass
{
    GtkWindowClass parent_class;

    /* Padding for future extension */
    void (*_hildon_reserved1)(void);
    void (*_hildon_reserved2)(void);
    void (*_hildon_reserved3)(void);
    void (*_hildon_reserved4)(void);
};

struct                                          _HildonAnimationActor
{
    GtkWindow parent;
};

#define HILDON_AA_X_AXIS	    0
#define HILDON_AA_Y_AXIS	    1
#define HILDON_AA_Z_AXIS	    2

#define HILDON_AA_N_GRAVITY	    1
#define HILDON_AA_NE_GRAVITY	    2
#define HILDON_AA_E_GRAVITY	    3
#define HILDON_AA_SE_GRAVITY	    4
#define HILDON_AA_S_GRAVITY	    5
#define HILDON_AA_SW_GRAVITY	    6
#define HILDON_AA_W_GRAVITY	    7
#define HILDON_AA_NW_GRAVITY	    8
#define HILDON_AA_CENTER_GRAVITY    9

GType
hildon_animation_actor_get_type                (void) G_GNUC_CONST;

GtkWidget*
hildon_animation_actor_new                     (void);

void
hildon_animation_actor_send_message (HildonAnimationActor *self,
                                     guint32 message_type,
                                     guint32 l0,
                                     guint32 l1,
                                     guint32 l2,
                                     guint32 l3,
                                     guint32 l4);
void
hildon_animation_actor_set_show_full (HildonAnimationActor *self,
				      gboolean show,
				      gint opacity);
void
hildon_animation_actor_set_show (HildonAnimationActor *self,
				 gboolean show);
void
hildon_animation_actor_set_opacity (HildonAnimationActor *self,
				    gint opacity);
void
hildon_animation_actor_set_position_full (HildonAnimationActor *self,
					  gint x,
					  gint y,
					  gint depth);
void
hildon_animation_actor_set_position (HildonAnimationActor *self,
                                     gint x,
                                     gint y);
void
hildon_animation_actor_set_depth (HildonAnimationActor *self,
				  gint depth);
void
hildon_animation_actor_set_scalex (HildonAnimationActor *self,
                                   gint32 x_scale,
                                   gint32 y_scale);
void
hildon_animation_actor_set_scale (HildonAnimationActor *self,
                                  double x_scale,
                                  double y_scale);
void
hildon_animation_actor_set_rotationx (HildonAnimationActor *self,
                                      gint   axis,
                                      gint32 degrees,
                                      gint   x,
                                      gint   y,
                                      gint   z);
void
hildon_animation_actor_set_rotation (HildonAnimationActor *self,
                                     gint   axis,
                                     double degrees,
                                     gint   x,
                                     gint   y,
                                     gint   z);
void
hildon_animation_actor_set_anchor (HildonAnimationActor *self,
                                   gint x,
                                   gint y);
void
hildon_animation_actor_set_anchor_from_gravity (HildonAnimationActor *self,
						guint gravity);
void
hildon_animation_actor_set_parent (HildonAnimationActor *self,
				   GtkWindow *parent);

G_END_DECLS

#endif                                 /* __HILDON_ANIMATION_ACTOR_H__ */
