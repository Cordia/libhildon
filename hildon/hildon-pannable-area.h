/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
 *
 * Contact: Rodrigo Novo <rodrigo.novo@nokia.com>
 *
 * This widget is based on MokoFingerScroll from libmokoui
 * OpenMoko Application Framework UI Library
 * Authored by Chris Lord <chris@openedhand.com>
 * Copyright (C) 2006-2007 OpenMoko Inc.
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

#ifndef _HILDON_PANNABLE_AREA
#define _HILDON_PANNABLE_AREA

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define                                         HILDON_TYPE_PANNABLE_AREA \
                                                hildon_pannable_area_get_type()

#define                                         HILDON_PANNABLE_AREA(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_PANNABLE_AREA, HildonPannableArea))

#define                                         HILDON_PANNABLE_AREA_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                HILDON_TYPE_PANNABLE_AREA, HildonPannableAreaClass))

#define                                         HILDON_IS_PANNABLE_AREA(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HILDON_TYPE_PANNABLE_AREA))

#define                                         HILDON_IS_PANNABLE_AREA_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), HILDON_TYPE_PANNABLE_AREA))

#define                                         HILDON_PANNABLE_AREA_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                HILDON_TYPE_PANNABLE_AREA, HildonPannableAreaClass))

/**
 * HildonPannableAreaMode:
 * @HILDON_PANNABLE_AREA_MODE_PUSH: Areaing follows pointer
 * @HILDON_PANNABLE_AREA_MODE_ACCEL: Areaing uses physics to "spin" the widget
 * @HILDON_PANNABLE_AREA_MODE_AUTO: Automatically chooses between push and accel
 * modes, depending on input.
 *
 * Used to change the behaviour of the pannable areaing
 */
typedef enum {
  HILDON_PANNABLE_AREA_MODE_PUSH,
  HILDON_PANNABLE_AREA_MODE_ACCEL,
  HILDON_PANNABLE_AREA_MODE_AUTO
} HildonPannableAreaMode;

/**
 * HildonMovementMode:
 * @HILDON_MOVEMENT_MODE_HORIZ:
 * @HILDON_MOVEMENT_MODE_VERT:
 * @HILDON_MOVEMENT_MODE_BOTH:
 *
 * Used to control the movement of the pannable, we can allow or
 * disallow horizontal or vertical movement. This way the applications
 * can control the movement using scroll_to and jump_to functions
 */
typedef enum {
  HILDON_MOVEMENT_MODE_HORIZ = 1 << 1,
  HILDON_MOVEMENT_MODE_VERT = 1 << 2,
  HILDON_MOVEMENT_MODE_BOTH = 0x000006
} HildonMovementMode;

/**
 * HildonMovementDirection:
 * @HILDON_MOVEMENT_UP:
 * @HILDON_MOVEMENT_DOWN:
 * @HILDON_MOVEMENT_LEFT:
 * @HILDON_MOVEMENT_RIGHT:
 *
 * Used to point out the direction of the movement
 */
typedef enum {
  HILDON_MOVEMENT_UP,
  HILDON_MOVEMENT_DOWN,
  HILDON_MOVEMENT_LEFT,
  HILDON_MOVEMENT_RIGHT
} HildonMovementDirection;

/**
 * HildonPannableArea:
 *
 * HildonPannableArea has no publicly accessible fields
 */
typedef struct                                  _HildonPannableArea HildonPannableArea;
typedef struct                                  _HildonPannableAreaClass HildonPannableAreaClass;
typedef struct                                  _HildonPannableAreaPrivate HildonPannableAreaPrivate;

struct                                          _HildonPannableArea
{
    GtkBin parent;

    /* private */
    HildonPannableAreaPrivate *priv;
};

struct                                          _HildonPannableAreaClass
{
    GtkBinClass parent_class;

  void (* horizontal_movement) (HildonPannableArea *area,
                                HildonMovementDirection direction,
                                gdouble x, gdouble y);
  void (* vertical_movement) (HildonPannableArea *area,
                              HildonMovementDirection direction,
                              gdouble x, gdouble y);
};

GType hildon_pannable_area_get_type             (void);

GtkWidget* hildon_pannable_area_new             (void);
GtkWidget* hildon_pannable_area_new_full        (gint mode, gboolean enabled,
                                                 gdouble vel_min, gdouble vel_max,
                                                 gdouble decel, guint sps);
void hildon_pannable_area_add_with_viewport     (HildonPannableArea *area,
                                                 GtkWidget *child);
void hildon_pannable_area_scroll_to             (HildonPannableArea *area,
						 const gint x, const gint y);
void hildon_pannable_area_jump_to               (HildonPannableArea *area,
 						 const gint x, const gint y);
void hildon_pannable_area_scroll_to_child       (HildonPannableArea *area,
 						 GtkWidget *child);
void hildon_pannable_area_jump_to_child         (HildonPannableArea *area,
                                                 GtkWidget *child);
GtkWidget* hildon_pannable_get_child_widget_at  (HildonPannableArea *area,
                                                 gdouble x, gdouble y);
GtkAdjustment* hildon_pannable_area_get_hadjustment (HildonPannableArea *area);
GtkAdjustment* hildon_pannable_area_get_vadjustment (HildonPannableArea *area);
gboolean hildon_pannable_area_get_center_on_child_focus (HildonPannableArea *area);
void hildon_pannable_area_set_center_on_child_focus (HildonPannableArea *area,
                                                     gboolean value);

G_END_DECLS

#endif /* _HILDON_PANNABLE_AREA */
