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

/**
 * SECTION: hildon-pannable-area
 * @short_description: A scrolling widget designed for touch screens
 * @see_also: #GtkScrolledWindow
 *
 * #HildonPannableArea is a container widget that can be "panned" (scrolled)
 * up and down using the touchscreen with fingers. The widget has no scrollbars,
 * but it rather shows small scroll indicators to give an idea of the part of the
 * content that is visible at a time. The scroll indicators appear when a dragging
 * motion is started on the pannable area.
 *
 * The scrolling is "kinetic", meaning the motion can be "flicked" and it will
 * continue from the initial motion by gradually slowing down to an eventual stop.
 * The motion can also be stopped immediately by pressing the touchscreen over the
 * pannable area.
 */

#undef HILDON_DISABLE_DEPRECATED

#include <math.h>
#if USE_CAIRO_SCROLLBARS == 1
#include <cairo.h>
#endif
#include <gdk/gdkx.h>

#include "hildon-pannable-area.h"
#include "hildon-marshalers.h"
#include "hildon-enum-types.h"

#define USE_CAIRO_SCROLLBARS 0

#define SCROLL_BAR_MIN_SIZE 5
#define RATIO_TOLERANCE 0.000001
#define SCROLL_FADE_TIMEOUT 100
#define MOTION_EVENTS_PER_SECOND 25
#define CURSOR_STOPPED_TIMEOUT 200
#define MAX_SPEED_THRESHOLD 280
#define PANNABLE_MAX_WIDTH 788
#define PANNABLE_MAX_HEIGHT 378
#define ACCEL_FACTOR 27
#define MIN_ACCEL_THRESHOLD 40
#define FAST_CLICK 125

G_DEFINE_TYPE (HildonPannableArea, hildon_pannable_area, GTK_TYPE_BIN)

#define PANNABLE_AREA_PRIVATE(o)                                \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), HILDON_TYPE_PANNABLE_AREA, \
                                HildonPannableAreaPrivate))

struct _HildonPannableAreaPrivate {
  HildonPannableAreaMode mode;
  HildonMovementMode mov_mode;
  GdkWindow *event_window;
  gdouble x;		/* Used to store mouse co-ordinates of the first or */
  gdouble y;		/* previous events in a press-motion pair */
  gdouble ex;		/* Used to store mouse co-ordinates of the last */
  gdouble ey;		/* motion event in acceleration mode */
  gboolean enabled;
  gboolean button_pressed;
  guint32 last_time;	/* Last event time, to stop infinite loops */
  guint32 last_press_time;
  gint last_type;
  gboolean last_in;
  gboolean moved;
  gdouble vmin;
  gdouble vmax;
  gdouble vmax_overshooting;
  gdouble accel_vel_x;
  gdouble accel_vel_y;
  gdouble vfast_factor;
  gdouble decel;
  gdouble drag_inertia;
  gdouble scroll_time;
  gdouble vel_factor;
  guint sps;
  guint panning_threshold;
  guint scrollbar_fade_delay;
  guint bounce_steps;
  guint force;
  guint direction_error_margin;
  gdouble vel_x;
  gdouble vel_y;
  gdouble old_vel_x;
  gdouble old_vel_y;
  GdkWindow *child;
  gint child_width;
  gint child_height;
  gint ix;			/* Initial click mouse co-ordinates */
  gint iy;
  gint cx;			/* Initial click child window mouse co-ordinates */
  gint cy;
  guint idle_id;
  gdouble scroll_to_x;
  gdouble scroll_to_y;
  gdouble motion_x;
  gdouble motion_y;
  gint overshot_dist_x;
  gint overshot_dist_y;
  gint overshooting_y;
  gint overshooting_x;
  gdouble scroll_indicator_alpha;
  gint motion_event_scroll_timeout;
  gint scroll_indicator_timeout;
  gint scroll_indicator_event_interrupt;
  gint scroll_delay_counter;
  gint vovershoot_max;
  gint hovershoot_max;
  gboolean initial_hint;
  gboolean initial_effect;
  gboolean low_friction_mode;
  gboolean first_drag;

  gboolean size_request_policy;
  gboolean hscroll_visible;
  gboolean vscroll_visible;
  GdkRectangle hscroll_rect;
  GdkRectangle vscroll_rect;
  guint indicator_width;

  GtkAdjustment *hadjust;
  GtkAdjustment *vadjust;
  gint x_offset;
  gint y_offset;

  GtkPolicyType vscrollbar_policy;
  GtkPolicyType hscrollbar_policy;

  GdkGC *scrollbars_gc;
  GdkColor scroll_color;

  gboolean center_on_child_focus;
  gboolean center_on_child_focus_pending;

  gboolean selection_movement;
};

/*signals*/
enum {
  HORIZONTAL_MOVEMENT,
  VERTICAL_MOVEMENT,
  PANNING_STARTED,
  PANNING_FINISHED,
  LAST_SIGNAL
};

static guint pannable_area_signals [LAST_SIGNAL] = { 0 };

enum {
  PROP_ENABLED = 1,
  PROP_MODE,
  PROP_MOVEMENT_MODE,
  PROP_VELOCITY_MIN,
  PROP_VELOCITY_MAX,
  PROP_VEL_MAX_OVERSHOOTING,
  PROP_VELOCITY_FAST_FACTOR,
  PROP_DECELERATION,
  PROP_DRAG_INERTIA,
  PROP_SPS,
  PROP_PANNING_THRESHOLD,
  PROP_SCROLLBAR_FADE_DELAY,
  PROP_BOUNCE_STEPS,
  PROP_FORCE,
  PROP_DIRECTION_ERROR_MARGIN,
  PROP_VSCROLLBAR_POLICY,
  PROP_HSCROLLBAR_POLICY,
  PROP_VOVERSHOOT_MAX,
  PROP_HOVERSHOOT_MAX,
  PROP_SCROLL_TIME,
  PROP_INITIAL_HINT,
  PROP_LOW_FRICTION_MODE,
  PROP_SIZE_REQUEST_POLICY,
  PROP_HADJUSTMENT,
  PROP_VADJUSTMENT,
  PROP_CENTER_ON_CHILD_FOCUS,
  PROP_LAST
};

static void hildon_pannable_area_class_init (HildonPannableAreaClass * klass);
static void hildon_pannable_area_init (HildonPannableArea * area);
static void hildon_pannable_area_get_property (GObject * object,
                                               guint property_id,
                                               GValue * value,
                                               GParamSpec * pspec);
static void hildon_pannable_area_set_property (GObject * object,
                                               guint property_id,
                                               const GValue * value,
                                               GParamSpec * pspec);
static void hildon_pannable_area_remove_timeouts (GtkWidget * widget);
static void hildon_pannable_area_dispose (GObject * object);
static void hildon_pannable_area_realize (GtkWidget * widget);
static void hildon_pannable_area_unrealize (GtkWidget * widget);
static void hildon_pannable_area_size_request (GtkWidget * widget,
                                               GtkRequisition * requisition);
static void hildon_pannable_area_size_allocate (GtkWidget * widget,
                                                GtkAllocation * allocation);
static void hildon_pannable_area_child_allocate_calculate (GtkWidget * widget,
                                                           GtkAllocation * allocation,
                                                           GtkAllocation * child_allocation);
static void hildon_pannable_area_style_set (GtkWidget * widget,
                                            GtkStyle * previous_style);
static void hildon_pannable_area_map (GtkWidget * widget);
static void hildon_pannable_area_unmap (GtkWidget * widget);
static void hildon_pannable_area_grab_notify (GtkWidget *widget,
                                              gboolean was_grabbed,
                                              gpointer user_data);
#if USE_CAIRO_SCROLLBARS == 1
static void rgb_from_gdkcolor (GdkColor *color, gdouble *r, gdouble *g, gdouble *b);
#else /* USE_CAIRO_SCROLLBARS */
static void tranparency_color (GdkColor *color,
                               GdkColor colora,
                               GdkColor colorb,
                               gdouble transparency);
#endif /* USE_CAIRO_SCROLLBARS */
static void hildon_pannable_draw_vscroll (GtkWidget * widget,
                                          GdkColor *back_color,
                                          GdkColor *scroll_color);
static void hildon_pannable_draw_hscroll (GtkWidget * widget,
                                          GdkColor *back_color,
                                          GdkColor *scroll_color);
static void hildon_pannable_area_initial_effect (GtkWidget * widget);
static void hildon_pannable_area_redraw (HildonPannableArea * area);
static void hildon_pannable_area_launch_fade_timeout (HildonPannableArea * area,
                                                      gdouble alpha);
static void hildon_pannable_area_adjust_value_changed (HildonPannableArea * area,
                                                       gpointer data);
static void hildon_pannable_area_adjust_changed (HildonPannableArea * area,
                                                 gpointer data);
static gboolean hildon_pannable_area_scroll_indicator_fade(HildonPannableArea * area);
static gboolean hildon_pannable_area_expose_event (GtkWidget * widget,
                                                   GdkEventExpose * event);
static GdkWindow * hildon_pannable_area_get_topmost (GdkWindow * window,
                                                     gint x, gint y,
                                                     gint * tx, gint * ty,
                                                     GdkEventMask mask);
static void synth_crossing (GdkWindow * child,
                            gint x, gint y,
                            gint x_root, gint y_root,
                            guint32 time, gboolean in);
static gboolean hildon_pannable_area_button_press_cb (GtkWidget * widget,
                                                      GdkEventButton * event);
static void hildon_pannable_area_refresh (HildonPannableArea * area);
static gboolean hildon_pannable_area_check_scrollbars (HildonPannableArea * area);
static void hildon_pannable_axis_scroll (HildonPannableArea *area,
                                         GtkAdjustment *adjust,
                                         gdouble *vel,
                                         gdouble inc,
                                         gint *overshooting,
                                         gint *overshot_dist,
                                         gdouble *scroll_to,
                                         gint overshoot_max,
                                         gboolean *s);
static void hildon_pannable_area_scroll (HildonPannableArea *area,
                                         gdouble x, gdouble y);
static gboolean hildon_pannable_area_timeout (HildonPannableArea * area);
static void hildon_pannable_area_calculate_velocity (gdouble *vel,
                                                     gdouble delta,
                                                     gdouble dist,
                                                     gdouble vmax,
                                                     gdouble drag_inertia,
                                                     gdouble force,
                                                     guint sps);
static gboolean hildon_pannable_area_motion_event_scroll_timeout (HildonPannableArea *area);
static void hildon_pannable_area_motion_event_scroll (HildonPannableArea *area,
                                                      gdouble x, gdouble y);
static void hildon_pannable_area_check_move (HildonPannableArea *area,
                                             GdkEventMotion * event,
                                             gdouble *x,
                                             gdouble *y);
static void hildon_pannable_area_handle_move (HildonPannableArea *area,
                                              GdkEventMotion * event,
                                              gdouble *x,
                                              gdouble *y);
static gboolean hildon_pannable_area_motion_notify_cb (GtkWidget * widget,
                                                       GdkEventMotion * event);
static gboolean hildon_pannable_leave_notify_event (GtkWidget *widget,
                                                    GdkEventCrossing *event);
static gboolean hildon_pannable_area_key_release_cb (GtkWidget * widget,
                                                     GdkEventKey * event);
static gboolean hildon_pannable_area_button_release_cb (GtkWidget * widget,
                                                        GdkEventButton * event);
static gboolean hildon_pannable_area_scroll_cb (GtkWidget *widget,
                                                GdkEventScroll *event);
static void hildon_pannable_area_child_mapped (GtkWidget *widget,
                                               GdkEvent  *event,
                                               gpointer user_data);
static void hildon_pannable_area_add (GtkContainer *container, GtkWidget *child);
static void hildon_pannable_area_remove (GtkContainer *container, GtkWidget *child);
static void hildon_pannable_calculate_vel_factor (HildonPannableArea * self);
static void hildon_pannable_area_set_focus_child (GtkContainer *container,
                                                 GtkWidget *child);
static void hildon_pannable_area_center_on_child_focus (HildonPannableArea *area);


static void
hildon_pannable_area_class_init (HildonPannableAreaClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);


  g_type_class_add_private (klass, sizeof (HildonPannableAreaPrivate));

  object_class->get_property = hildon_pannable_area_get_property;
  object_class->set_property = hildon_pannable_area_set_property;
  object_class->dispose = hildon_pannable_area_dispose;

  widget_class->realize = hildon_pannable_area_realize;
  widget_class->unrealize = hildon_pannable_area_unrealize;
  widget_class->map = hildon_pannable_area_map;
  widget_class->unmap = hildon_pannable_area_unmap;
  widget_class->size_request = hildon_pannable_area_size_request;
  widget_class->size_allocate = hildon_pannable_area_size_allocate;
  widget_class->expose_event = hildon_pannable_area_expose_event;
  widget_class->style_set = hildon_pannable_area_style_set;
  widget_class->button_press_event = hildon_pannable_area_button_press_cb;
  widget_class->button_release_event = hildon_pannable_area_button_release_cb;
  widget_class->key_release_event = hildon_pannable_area_key_release_cb;
  widget_class->motion_notify_event = hildon_pannable_area_motion_notify_cb;
  widget_class->leave_notify_event = hildon_pannable_leave_notify_event;
  widget_class->scroll_event = hildon_pannable_area_scroll_cb;

  container_class->add = hildon_pannable_area_add;
  container_class->remove = hildon_pannable_area_remove;
  container_class->set_focus_child = hildon_pannable_area_set_focus_child;

  klass->horizontal_movement = NULL;
  klass->vertical_movement = NULL;

  g_object_class_install_property (object_class,
				   PROP_ENABLED,
				   g_param_spec_boolean ("enabled",
							 "Enabled",
							 "Enable or disable finger-scroll.",
							 TRUE,
							 G_PARAM_READWRITE |
							 G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_VSCROLLBAR_POLICY,
				   g_param_spec_enum ("vscrollbar_policy",
						      "vscrollbar policy",
						      "Visual policy of the vertical scrollbar",
						      GTK_TYPE_POLICY_TYPE,
						      GTK_POLICY_AUTOMATIC,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_HSCROLLBAR_POLICY,
				   g_param_spec_enum ("hscrollbar_policy",
						      "hscrollbar policy",
						      "Visual policy of the horizontal scrollbar",
						      GTK_TYPE_POLICY_TYPE,
						      GTK_POLICY_AUTOMATIC,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_MODE,
				   g_param_spec_enum ("mode",
						      "Scroll mode",
						      "Change the finger-scrolling mode.",
						      HILDON_TYPE_PANNABLE_AREA_MODE,
						      HILDON_PANNABLE_AREA_MODE_AUTO,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_MOVEMENT_MODE,
				   g_param_spec_flags ("mov_mode",
                                                       "Scroll movement mode",
                                                       "Controls if the widget can scroll vertically, horizontally or both",
                                                       HILDON_TYPE_MOVEMENT_MODE,
                                                       HILDON_MOVEMENT_MODE_VERT,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_VELOCITY_MIN,
				   g_param_spec_double ("velocity_min",
							"Minimum scroll velocity",
							"Minimum distance the child widget should scroll "
							"per 'frame', in pixels per frame.",
							0, G_MAXDOUBLE, 10,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_VELOCITY_MAX,
				   g_param_spec_double ("velocity_max",
							"Maximum scroll velocity",
							"Maximum distance the child widget should scroll "
							"per 'frame', in pixels per frame.",
							0, G_MAXDOUBLE, 3500,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_VEL_MAX_OVERSHOOTING,
				   g_param_spec_double ("velocity_overshooting_max",
							"Maximum scroll velocity when overshooting",
							"Maximum distance the child widget should scroll "
							"per 'frame', in pixels per frame when it overshoots after hitting the edge.",
							0, G_MAXDOUBLE, 130,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_VELOCITY_FAST_FACTOR,
				   g_param_spec_double ("velocity_fast_factor",
							"Fast velocity factor",
							"Minimum velocity that is considered 'fast': "
							"children widgets won't receive button presses. "
							"Expressed as a fraction of the maximum velocity.",
							0, 1, 0.01,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_DECELERATION,
				   g_param_spec_double ("deceleration",
							"Deceleration multiplier",
							"The multiplier used when decelerating when in "
							"acceleration scrolling mode.",
							0, 1.0, 0.85,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_DRAG_INERTIA,
				   g_param_spec_double ("drag_inertia",
							"Inertia of the cursor dragging",
							"Percentage of the calculated speed in each moment we are are going to use"
                                                        "to calculate the launch speed, the other part would be the speed"
                                                        "calculated previously",
							0, 1.0, 0.85,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_SPS,
				   g_param_spec_uint ("sps",
						      "Scrolls per second",
						      "Amount of scroll events to generate per second.",
						      0, G_MAXUINT, 20,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_PANNING_THRESHOLD,
				   g_param_spec_uint ("panning_threshold",
						      "Threshold to consider a motion event an scroll",
						      "Amount of pixels to consider a motion event an scroll, if it is less"
                                                      "it is a click detected incorrectly by the touch screen.",
						      0, G_MAXUINT, 25,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_SCROLLBAR_FADE_DELAY,
				   g_param_spec_uint ("scrollbar_fade_delay",
						      "Time before starting to fade the scrollbar",
						      "Time the scrollbar is going to be visible if the widget is not in"
                                                      "action in miliseconds",
						      0, G_MAXUINT, 1000,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_BOUNCE_STEPS,
				   g_param_spec_uint ("bounce_steps",
						      "Bounce steps",
						      "Number of steps that is going to be used to bounce when hitting the"
                                                      "edge, the rubberband effect depends on it",
						      0, G_MAXUINT, 3,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_FORCE,
				   g_param_spec_uint ("force",
						      "Multiplier of the calculated speed",
						      "Force applied to the movement, multiplies the calculated speed of the"
                                                      "user movement the cursor in the screen",
						      0, G_MAXUINT, 50,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_DIRECTION_ERROR_MARGIN,
				   g_param_spec_uint ("direction_error_margin",
						      "Margin in the direction detection",
						      "After detecting the direction of the movement (horizontal or"
                                                      "vertical), we can add this margin of error to allow the movement in"
                                                      "the other direction even apparently it is not",
						      0, G_MAXUINT, 10,
						      G_PARAM_READWRITE |
						      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_VOVERSHOOT_MAX,
				   g_param_spec_int ("vovershoot_max",
                                                     "Vertical overshoot distance",
                                                     "Space we allow the widget to pass over its vertical limits when"
                                                     "hitting the edges, set 0 in order to deactivate overshooting.",
                                                     0, G_MAXINT, 150,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_HOVERSHOOT_MAX,
				   g_param_spec_int ("hovershoot_max",
                                                     "Horizontal overshoot distance",
                                                     "Space we allow the widget to pass over its horizontal limits when"
                                                     "hitting the edges, set 0 in order to deactivate overshooting.",
                                                     0, G_MAXINT, 150,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_SCROLL_TIME,
				   g_param_spec_double ("scroll_time",
							"Time to scroll to a position",
							"The time to scroll to a position when calling the hildon_pannable_scroll_to function",
							0.0, 20.0, 1.0,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
 				   PROP_INITIAL_HINT,
 				   g_param_spec_boolean ("initial-hint",
 							 "Initial hint",
 							 "Whether to hint the user about the pannability of the container.",
 							 TRUE,
							 G_PARAM_READWRITE |
 							 G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
 				   PROP_LOW_FRICTION_MODE,
 				   g_param_spec_boolean ("low-friction-mode",
 							 "Do not decelerate the initial velocity",
 							 "Avoid decelerating the panning movement, like no friction, the widget"
                                                         "will stop in the edges or if the user clicks.",
 							 FALSE,
							 G_PARAM_READWRITE |
 							 G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                                   PROP_SIZE_REQUEST_POLICY,
				   g_param_spec_enum ("size-request-policy",
                                                      "Size Requisition policy",
                                                      "Controls the size request policy of the widget",
                                                      HILDON_TYPE_SIZE_REQUEST_POLICY,
                                                      HILDON_SIZE_REQUEST_MINIMUM,
                                                      G_PARAM_READWRITE|
                                                      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
				   PROP_HADJUSTMENT,
				   g_param_spec_object ("hadjustment",
							"Horizontal Adjustment",
							"The GtkAdjustment for the horizontal position",
							GTK_TYPE_ADJUSTMENT,
							G_PARAM_READABLE));
  g_object_class_install_property (object_class,
				   PROP_VADJUSTMENT,
				   g_param_spec_object ("vadjustment",
							"Vertical Adjustment",
							"The GtkAdjustment for the vertical position",
							GTK_TYPE_ADJUSTMENT,
							G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                                   PROP_CENTER_ON_CHILD_FOCUS,
                                   g_param_spec_boolean ("center-on-child-focus",
                                                         "Center on the child with the focus",
                                                         "Whether to center the pannable on the child that receives the focus.",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));


  gtk_widget_class_install_style_property (widget_class,
					   g_param_spec_uint
					   ("indicator-width",
					    "Width of the scroll indicators",
					    "Pixel width used to draw the scroll indicators.",
					    0, G_MAXUINT, 8,
					    G_PARAM_READWRITE));

 /**
   * HildonPannableArea::horizontal-movement:
   * @hildonpannable: the object which received the signal
   * @direction: the direction of the movement #HILDON_MOVEMENT_LEFT or #HILDON_MOVEMENT_RIGHT
   * @initial_x: the x coordinate of the point where the user clicked to start the movement
   * @initial_y: the y coordinate of the point where the user clicked to start the movement
   *
   * The horizontal-movement signal is emitted when the pannable area
   * detects a horizontal movement. The detection does not mean the
   * widget is going to move (i.e. maybe the children are smaller
   * horizontally than the screen).
   *
   * Since: 2.2
   */
  pannable_area_signals[HORIZONTAL_MOVEMENT] =
    g_signal_new ("horizontal_movement",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (HildonPannableAreaClass, horizontal_movement),
		  NULL, NULL,
		  _hildon_marshal_VOID__INT_DOUBLE_DOUBLE,
		  G_TYPE_NONE, 3,
                  G_TYPE_INT,
		  G_TYPE_DOUBLE,
		  G_TYPE_DOUBLE);

  /**
   * HildonPannableArea::vertical-movement:
   * @hildonpannable: the object which received the signal
   * @direction: the direction of the movement #HILDON_MOVEMENT_UP or #HILDON_MOVEMENT_DOWN
   * @initial_x: the x coordinate of the point where the user clicked to start the movement
   * @initial_y: the y coordinate of the point where the user clicked to start the movement
   *
   * The vertical-movement signal is emitted when the pannable area
   * detects a vertical movement. The detection does not mean the
   * widget is going to move (i.e. maybe the children are smaller
   * vertically than the screen).
   *
   * Since: 2.2
   */
  pannable_area_signals[VERTICAL_MOVEMENT] =
    g_signal_new ("vertical_movement",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (HildonPannableAreaClass, vertical_movement),
		  NULL, NULL,
		  _hildon_marshal_VOID__INT_DOUBLE_DOUBLE,
		  G_TYPE_NONE, 3,
                  G_TYPE_INT,
		  G_TYPE_DOUBLE,
		  G_TYPE_DOUBLE);

 /**
   * HildonPannableArea::panning-started:
   * @hildonpannable: the pannable area object that is going to start
   * the panning
   *
   * This signal is emitted before the panning starts. Applications
   * can return %TRUE to avoid the panning. The main difference with
   * the vertical-movement and horizontal-movement signals is those
   * gesture signals are launched no matter if the widget is going to
   * move, this signal means the widget is going to start moving. It
   * could even happen that the widget moves and there was no gesture
   * (i.e. click meanwhile the pannable is overshooting).
   *
   * Returns: %TRUE to stop the panning launch. %FALSE to continue
   * with it.
   *
   * Since: 2.2
   */
  pannable_area_signals[PANNING_STARTED] =
    g_signal_new ("panning-started",
                  G_TYPE_FROM_CLASS (object_class),
                  0,
                  0,
                  NULL, NULL,
                  _hildon_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

 /**
   * HildonPannableArea::panning-finished:
   * @hildonpannable: the pannable area object that finished the
   * panning
   *
   * This signal is emitted after the kinetic panning has
   * finished.
   *
   * Since: 2.2
   */
  pannable_area_signals[PANNING_FINISHED] =
    g_signal_new ("panning-finished",
                  G_TYPE_FROM_CLASS (object_class),
                  0,
                  0,
                  NULL, NULL,
                  _hildon_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

}

static void
hildon_pannable_area_init (HildonPannableArea * area)
{
  HildonPannableAreaPrivate *priv = PANNABLE_AREA_PRIVATE (area);

  GTK_WIDGET_UNSET_FLAGS (area, GTK_NO_WINDOW);

  area->priv = priv;

  priv->moved = FALSE;
  priv->button_pressed = FALSE;
  priv->last_time = 0;
  priv->last_press_time = 0;
  priv->last_type = 0;
  priv->vscroll_visible = TRUE;
  priv->hscroll_visible = TRUE;
  priv->indicator_width = 6;
  priv->overshot_dist_x = 0;
  priv->overshot_dist_y = 0;
  priv->overshooting_y = 0;
  priv->overshooting_x = 0;
  priv->accel_vel_x = 0;
  priv->accel_vel_y = 0;
  priv->idle_id = 0;
  priv->vel_x = 0;
  priv->vel_y = 0;
  priv->old_vel_x = 0;
  priv->old_vel_y = 0;
  priv->scroll_indicator_alpha = 0.0;
  priv->scroll_indicator_timeout = 0;
  priv->motion_event_scroll_timeout = 0;
  priv->scroll_indicator_event_interrupt = 0;
  priv->scroll_delay_counter = 0;
  priv->scrollbar_fade_delay = 0;
  priv->scroll_to_x = -1;
  priv->scroll_to_y = -1;
  priv->first_drag = TRUE;
  priv->initial_effect = TRUE;
  priv->child_width = 0;
  priv->child_height = 0;
  priv->last_in = TRUE;
  priv->x_offset = 0;
  priv->y_offset = 0;
  priv->center_on_child_focus_pending = FALSE;
  priv->selection_movement = FALSE;

  gtk_style_lookup_color (GTK_WIDGET (area)->style,
			  "SecondaryTextColor", &priv->scroll_color);

  priv->hadjust =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
  priv->vadjust =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

  g_object_ref_sink (G_OBJECT (priv->hadjust));
  g_object_ref_sink (G_OBJECT (priv->vadjust));

  g_signal_connect_swapped (priv->hadjust, "value-changed",
			    G_CALLBACK (hildon_pannable_area_adjust_value_changed), area);
  g_signal_connect_swapped (priv->vadjust, "value-changed",
			    G_CALLBACK (hildon_pannable_area_adjust_value_changed), area);
  g_signal_connect_swapped (priv->hadjust, "changed",
			    G_CALLBACK (hildon_pannable_area_adjust_changed), area);
  g_signal_connect_swapped (priv->vadjust, "changed",
			    G_CALLBACK (hildon_pannable_area_adjust_changed), area);
  g_signal_connect (area, "grab-notify",
                    G_CALLBACK (hildon_pannable_area_grab_notify), NULL);
}

static void
hildon_pannable_area_get_property (GObject * object,
                                   guint property_id,
				   GValue * value,
                                   GParamSpec * pspec)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (object)->priv;

  switch (property_id) {
  case PROP_ENABLED:
    g_value_set_boolean (value, priv->enabled);
    break;
  case PROP_MODE:
    g_value_set_enum (value, priv->mode);
    break;
  case PROP_MOVEMENT_MODE:
    g_value_set_flags (value, priv->mov_mode);
    break;
  case PROP_VELOCITY_MIN:
    g_value_set_double (value, priv->vmin);
    break;
  case PROP_VELOCITY_MAX:
    g_value_set_double (value, priv->vmax);
    break;
  case PROP_VEL_MAX_OVERSHOOTING:
    g_value_set_double (value, priv->vmax_overshooting);
    break;
  case PROP_VELOCITY_FAST_FACTOR:
    g_value_set_double (value, priv->vfast_factor);
    break;
  case PROP_DECELERATION:
    g_value_set_double (value, priv->decel);
    break;
  case PROP_DRAG_INERTIA:
    g_value_set_double (value, priv->drag_inertia);
    break;
  case PROP_SPS:
    g_value_set_uint (value, priv->sps);
    break;
  case PROP_PANNING_THRESHOLD:
    g_value_set_uint (value, priv->panning_threshold);
    break;
  case PROP_SCROLLBAR_FADE_DELAY:
    /* convert to miliseconds */
    g_value_set_uint (value, priv->scrollbar_fade_delay * SCROLL_FADE_TIMEOUT);
    break;
  case PROP_BOUNCE_STEPS:
    g_value_set_uint (value, priv->bounce_steps);
    break;
  case PROP_FORCE:
    g_value_set_uint (value, priv->force);
    break;
  case PROP_DIRECTION_ERROR_MARGIN:
    g_value_set_uint (value, priv->direction_error_margin);
    break;
  case PROP_VSCROLLBAR_POLICY:
    g_value_set_enum (value, priv->vscrollbar_policy);
    break;
  case PROP_HSCROLLBAR_POLICY:
    g_value_set_enum (value, priv->hscrollbar_policy);
    break;
  case PROP_VOVERSHOOT_MAX:
    g_value_set_int (value, priv->vovershoot_max);
    break;
  case PROP_HOVERSHOOT_MAX:
    g_value_set_int (value, priv->hovershoot_max);
    break;
  case PROP_SCROLL_TIME:
    g_value_set_double (value, priv->scroll_time);
    break;
  case PROP_INITIAL_HINT:
    g_value_set_boolean (value, priv->initial_hint);
    break;
  case PROP_LOW_FRICTION_MODE:
    g_value_set_boolean (value, priv->low_friction_mode);
    break;
  case PROP_SIZE_REQUEST_POLICY:
    g_value_set_enum (value, priv->size_request_policy);
    break;
  case PROP_HADJUSTMENT:
    g_value_set_object (value,
                        hildon_pannable_area_get_hadjustment
                        (HILDON_PANNABLE_AREA (object)));
    break;
  case PROP_VADJUSTMENT:
    g_value_set_object (value,
                        hildon_pannable_area_get_vadjustment
                        (HILDON_PANNABLE_AREA (object)));
    break;
  case PROP_CENTER_ON_CHILD_FOCUS:
    g_value_set_boolean (value, priv->center_on_child_focus);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
hildon_pannable_area_set_property (GObject * object,
                                   guint property_id,
				   const GValue * value,
                                   GParamSpec * pspec)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (object)->priv;
  gboolean enabled;

  switch (property_id) {
  case PROP_ENABLED:
    enabled = g_value_get_boolean (value);

    if ((priv->enabled != enabled) && (GTK_WIDGET_REALIZED (object))) {
      if (enabled)
	gdk_window_raise (priv->event_window);
      else
	gdk_window_lower (priv->event_window);
    }

    priv->enabled = enabled;
    break;
  case PROP_MODE:
    priv->mode = g_value_get_enum (value);
    break;
  case PROP_MOVEMENT_MODE:
    priv->mov_mode = g_value_get_flags (value);
    break;
  case PROP_VELOCITY_MIN:
    priv->vmin = g_value_get_double (value);
    break;
  case PROP_VELOCITY_MAX:
    priv->vmax = g_value_get_double (value);
    break;
  case PROP_VEL_MAX_OVERSHOOTING:
    priv->vmax_overshooting = g_value_get_double (value);
    break;
  case PROP_VELOCITY_FAST_FACTOR:
    priv->vfast_factor = g_value_get_double (value);
    break;
  case PROP_DECELERATION:
    priv->decel = g_value_get_double (value);
    hildon_pannable_calculate_vel_factor (HILDON_PANNABLE_AREA (object));
    break;
  case PROP_DRAG_INERTIA:
    priv->drag_inertia = g_value_get_double (value);
    break;
  case PROP_SPS:
    priv->sps = g_value_get_uint (value);
    break;
  case PROP_PANNING_THRESHOLD:
    {
      GtkSettings *settings = gtk_settings_get_default ();
      GtkSettingsValue svalue = { NULL, { 0, }, };

      priv->panning_threshold = g_value_get_uint (value);

      /* insure gtk dnd is the same we are using, not allowed
         different thresholds in the same application */
      svalue.origin = "panning_threshold";
      g_value_init (&svalue.value, G_TYPE_LONG);
      g_value_set_long (&svalue.value, priv->panning_threshold);
      gtk_settings_set_property_value (settings, "gtk-dnd-drag-threshold", &svalue);
      g_value_unset (&svalue.value);
    }
    break;
  case PROP_SCROLLBAR_FADE_DELAY:
    /* convert to miliseconds */
    priv->scrollbar_fade_delay = g_value_get_uint (value)/(SCROLL_FADE_TIMEOUT);
    break;
  case PROP_BOUNCE_STEPS:
    priv->bounce_steps = g_value_get_uint (value);
    break;
  case PROP_FORCE:
    priv->force = g_value_get_uint (value);
    break;
  case PROP_DIRECTION_ERROR_MARGIN:
    priv->direction_error_margin = g_value_get_uint (value);
    break;
  case PROP_VSCROLLBAR_POLICY:
    priv->vscrollbar_policy = g_value_get_enum (value);

    gtk_widget_queue_resize (GTK_WIDGET (object));
    break;
  case PROP_HSCROLLBAR_POLICY:
    priv->hscrollbar_policy = g_value_get_enum (value);

    gtk_widget_queue_resize (GTK_WIDGET (object));
    break;
  case PROP_VOVERSHOOT_MAX:
    priv->vovershoot_max = g_value_get_int (value);
    break;
  case PROP_HOVERSHOOT_MAX:
    priv->hovershoot_max = g_value_get_int (value);
    break;
  case PROP_SCROLL_TIME:
    priv->scroll_time = g_value_get_double (value);

    hildon_pannable_calculate_vel_factor (HILDON_PANNABLE_AREA (object));
    break;
  case PROP_INITIAL_HINT:
    priv->initial_hint = g_value_get_boolean (value);
    break;
  case PROP_LOW_FRICTION_MODE:
    priv->low_friction_mode = g_value_get_boolean (value);
    break;
  case PROP_SIZE_REQUEST_POLICY:
    hildon_pannable_area_set_size_request_policy (HILDON_PANNABLE_AREA (object),
                                                  g_value_get_enum (value));
    break;
  case PROP_CENTER_ON_CHILD_FOCUS:
    priv->center_on_child_focus = g_value_get_boolean (value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
hildon_pannable_area_dispose (GObject * object)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (object)->priv;
  GtkWidget *child = gtk_bin_get_child (GTK_BIN (object));

  hildon_pannable_area_remove_timeouts (GTK_WIDGET (object));

  if (child) {
    g_signal_handlers_disconnect_by_func (child,
                                          hildon_pannable_area_child_mapped,
                                          object);
  }

  g_signal_handlers_disconnect_by_func (object,
                                        hildon_pannable_area_grab_notify,
                                        NULL);

  if (priv->hadjust) {
    g_signal_handlers_disconnect_by_func (priv->hadjust,
                                          hildon_pannable_area_adjust_value_changed,
                                          object);
    g_signal_handlers_disconnect_by_func (priv->hadjust,
                                          hildon_pannable_area_adjust_changed,
                                          object);
    g_object_unref (priv->hadjust);
    priv->hadjust = NULL;
  }

  if (priv->vadjust) {
    g_signal_handlers_disconnect_by_func (priv->vadjust,
                                          hildon_pannable_area_adjust_value_changed,
                                          object);
    g_signal_handlers_disconnect_by_func (priv->vadjust,
                                          hildon_pannable_area_adjust_changed,
                                          object);
    g_object_unref (priv->vadjust);
    priv->vadjust = NULL;
  }

  if (G_OBJECT_CLASS (hildon_pannable_area_parent_class)->dispose)
    G_OBJECT_CLASS (hildon_pannable_area_parent_class)->dispose (object);
}

static void
hildon_pannable_area_realize (GtkWidget * widget)
{
  GdkWindowAttr attributes;
  gint attributes_mask;
  gint border_width;
  HildonPannableAreaPrivate *priv;

  priv = HILDON_PANNABLE_AREA (widget)->priv;

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  border_width = GTK_CONTAINER (widget)->border_width;

  attributes.x = widget->allocation.x + border_width;
  attributes.y = widget->allocation.y + border_width;
  attributes.width = MAX (widget->allocation.width - 2 * border_width, 0);
  attributes.height = MAX (widget->allocation.height - 2 * border_width, 0);
  attributes.window_type = GDK_WINDOW_CHILD;

  /* avoid using the hildon_window */
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
  attributes.wclass = GDK_INPUT_OUTPUT;

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);

  /* create the events window */
  attributes.x = 0;
  attributes.y = 0;
  attributes.event_mask = gtk_widget_get_events (widget)
    | GDK_BUTTON_MOTION_MASK
    | GDK_BUTTON_PRESS_MASK
    | GDK_BUTTON_RELEASE_MASK
    | GDK_SCROLL_MASK
    | GDK_POINTER_MOTION_HINT_MASK
    | GDK_EXPOSURE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK;
  attributes.wclass = GDK_INPUT_ONLY;

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  priv->event_window = gdk_window_new (widget->window,
				       &attributes, attributes_mask);
  gdk_window_set_user_data (priv->event_window, widget);

  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);

  priv->scrollbars_gc = gdk_gc_new (GDK_DRAWABLE (widget->window));
  gdk_gc_copy (priv->scrollbars_gc, widget->style->fg_gc[GTK_STATE_INSENSITIVE]);
}


static void
hildon_pannable_area_remove_timeouts (GtkWidget * widget)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;

  if (priv->idle_id) {
    g_signal_emit (widget, pannable_area_signals[PANNING_FINISHED], 0);
    g_source_remove (priv->idle_id);
    priv->idle_id = 0;
  }

  if (priv->scroll_indicator_timeout){
    g_source_remove (priv->scroll_indicator_timeout);
    priv->scroll_indicator_timeout = 0;
  }

  if (priv->motion_event_scroll_timeout){
    g_source_remove (priv->motion_event_scroll_timeout);
    priv->motion_event_scroll_timeout = 0;
  }
}

static void
hildon_pannable_area_unrealize (GtkWidget * widget)
{
  HildonPannableAreaPrivate *priv;

  priv = HILDON_PANNABLE_AREA (widget)->priv;

  hildon_pannable_area_remove_timeouts (widget);

  if (priv->event_window != NULL) {
    gdk_window_set_user_data (priv->event_window, NULL);
    gdk_window_destroy (priv->event_window);
    priv->event_window = NULL;
  }

  gdk_gc_unref (priv->scrollbars_gc);

  if (GTK_WIDGET_CLASS (hildon_pannable_area_parent_class)->unrealize)
    (*GTK_WIDGET_CLASS (hildon_pannable_area_parent_class)->unrealize)(widget);
}

static void
hildon_pannable_area_size_request (GtkWidget * widget,
				   GtkRequisition * requisition)
{
  GtkRequisition child_requisition = {0};
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;
  GtkWidget *child = gtk_bin_get_child (GTK_BIN (widget));

  if (child && GTK_WIDGET_VISIBLE (child))
    {
      gtk_widget_size_request (child, &child_requisition);
    }

  if (priv->hscrollbar_policy == GTK_POLICY_NEVER) {
    requisition->width = child_requisition.width;
  } else {
    switch (priv->size_request_policy) {
      case HILDON_SIZE_REQUEST_CHILDREN:
        requisition->width = MIN (PANNABLE_MAX_WIDTH,
                                  child_requisition.width);
        break;
      case HILDON_SIZE_REQUEST_MINIMUM:
      default:
        requisition->width = priv->indicator_width;
      }
  }

  if (priv->vscrollbar_policy == GTK_POLICY_NEVER) {
    requisition->height = child_requisition.height;
  } else {
    switch (priv->size_request_policy) {
      case HILDON_SIZE_REQUEST_CHILDREN:
        requisition->height = MIN (PANNABLE_MAX_HEIGHT,
                                   child_requisition.height);
        break;
      case HILDON_SIZE_REQUEST_MINIMUM:
      default:
        requisition->height = priv->indicator_width;
      }
  }

  requisition->width += 2 * GTK_CONTAINER (widget)->border_width;
  requisition->height += 2 * GTK_CONTAINER (widget)->border_width;
}

static void
hildon_pannable_area_child_allocate_calculate (GtkWidget * widget,
                                               GtkAllocation * allocation,
                                               GtkAllocation * child_allocation)
{
  gint border_width;
  HildonPannableAreaPrivate *priv;

  border_width = GTK_CONTAINER (widget)->border_width;

  priv = HILDON_PANNABLE_AREA (widget)->priv;

  child_allocation->x = 0;
  child_allocation->y = 0;
  child_allocation->width = MAX (allocation->width - 2 * border_width -
                                 (priv->vscroll_visible ? priv->vscroll_rect.width : 0), 0);
  child_allocation->height = MAX (allocation->height - 2 * border_width -
                                  (priv->hscroll_visible ? priv->hscroll_rect.height : 0), 0);

  if (priv->overshot_dist_y > 0) {
    child_allocation->y = MIN (child_allocation->y + priv->overshot_dist_y,
                               child_allocation->height);
    child_allocation->height = MAX (child_allocation->height - priv->overshot_dist_y, 0);
  } else if (priv->overshot_dist_y < 0) {
    child_allocation->height = MAX (child_allocation->height + priv->overshot_dist_y, 0);
  }

  if (priv->overshot_dist_x > 0) {
    child_allocation->x = MIN (child_allocation->x + priv->overshot_dist_x,
                               child_allocation->width);
    child_allocation->width = MAX (child_allocation->width - priv->overshot_dist_x, 0);
  } else if (priv->overshot_dist_x < 0) {
    child_allocation->width = MAX (child_allocation->width + priv->overshot_dist_x, 0);
  }
}

static void
hildon_pannable_area_size_allocate (GtkWidget * widget,
				    GtkAllocation * allocation)
{
  GtkAllocation child_allocation;
  HildonPannableAreaPrivate *priv;
  GtkWidget *child = gtk_bin_get_child (GTK_BIN (widget));
  gint border_width;
  gdouble hv, vv;

  border_width = GTK_CONTAINER (widget)->border_width;

  widget->allocation = *allocation;

  priv = HILDON_PANNABLE_AREA (widget)->priv;

  if (GTK_WIDGET_REALIZED (widget)) {
      gdk_window_move_resize (widget->window,
			      allocation->x + border_width,
			      allocation->y  + border_width,
			      allocation->width  - border_width * 2,
			      allocation->height - border_width * 2);
      gdk_window_move_resize (priv->event_window,
			      0,
			      0,
			      allocation->width  - border_width * 2,
			      allocation->height - border_width * 2);
  }

  if (child && GTK_WIDGET_VISIBLE (child)) {

    hildon_pannable_area_check_scrollbars (HILDON_PANNABLE_AREA (widget));

    hildon_pannable_area_child_allocate_calculate (widget,
                                                   allocation,
                                                   &child_allocation);

    gtk_widget_size_allocate (child, &child_allocation);

    if (hildon_pannable_area_check_scrollbars (HILDON_PANNABLE_AREA (widget))) {
      hildon_pannable_area_child_allocate_calculate (widget,
                                                     allocation,
                                                     &child_allocation);

      gtk_widget_size_allocate (child, &child_allocation);
    }

    if (priv->vadjust->page_size >= 0) {
      priv->accel_vel_y = MIN (priv->vmax,
                               priv->vadjust->upper/priv->vadjust->page_size*ACCEL_FACTOR);
      priv->accel_vel_x = MIN (priv->vmax,
                               priv->hadjust->upper/priv->hadjust->page_size*ACCEL_FACTOR);
    }

    hv = priv->hadjust->value;
    vv = priv->vadjust->value;

    /* we have to do this after child size_allocate because page_size is
     * changed when we allocate the size of the children */
    if (priv->overshot_dist_y < 0) {
      priv->vadjust->value = priv->vadjust->upper - priv->vadjust->page_size;
    }

    if (priv->overshot_dist_x < 0) {
      priv->hadjust->value = priv->hadjust->upper - priv->hadjust->page_size;
    }

    if (hv != priv->hadjust->value)
      gtk_adjustment_value_changed (priv->hadjust);

    if (vv != priv->vadjust->value)
      gtk_adjustment_value_changed (priv->vadjust);

  } else {
    hildon_pannable_area_check_scrollbars (HILDON_PANNABLE_AREA (widget));
  }
}

static void
hildon_pannable_area_style_set (GtkWidget * widget,
                                GtkStyle * previous_style)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;

  GTK_WIDGET_CLASS (hildon_pannable_area_parent_class)->
    style_set (widget, previous_style);

  gtk_style_lookup_color (widget->style, "SecondaryTextColor", &priv->scroll_color);
  gtk_widget_style_get (widget, "indicator-width", &priv->indicator_width, NULL);
}

static void
hildon_pannable_area_map (GtkWidget * widget)
{
  HildonPannableAreaPrivate *priv;

  priv = HILDON_PANNABLE_AREA (widget)->priv;

  gdk_window_show (widget->window);

  if (priv->event_window != NULL && !priv->enabled)
    gdk_window_show (priv->event_window);

  (*GTK_WIDGET_CLASS (hildon_pannable_area_parent_class)->map) (widget);

  if (priv->event_window != NULL && priv->enabled)
    gdk_window_show (priv->event_window);
}

static void
hildon_pannable_area_unmap (GtkWidget * widget)
{
  HildonPannableAreaPrivate *priv;

  priv = HILDON_PANNABLE_AREA (widget)->priv;

  if (priv->event_window != NULL)
    gdk_window_hide (priv->event_window);

  gdk_window_hide (widget->window);

  (*GTK_WIDGET_CLASS (hildon_pannable_area_parent_class)->unmap) (widget);
}

static void
hildon_pannable_area_grab_notify (GtkWidget *widget,
                                  gboolean was_grabbed,
                                  gpointer user_data)
{
  /* an internal widget has grabbed the focus and now has returned it,
     we have to do some release actions */
  if (was_grabbed) {
    HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;

    priv->scroll_indicator_event_interrupt = 0;

    if ((!priv->scroll_indicator_timeout)&&(priv->scroll_indicator_alpha)>0.1) {
      priv->scroll_delay_counter = priv->scrollbar_fade_delay;

      hildon_pannable_area_launch_fade_timeout (HILDON_PANNABLE_AREA (widget),
                                                priv->scroll_indicator_alpha);
    }

    priv->last_type = 3;
    priv->moved = FALSE;
  }
}

#if USE_CAIRO_SCROLLBARS == 1

static void
rgb_from_gdkcolor (GdkColor *color, gdouble *r, gdouble *g, gdouble *b)
{
  *r = (color->red >> 8) / 255.0;
  *g = (color->green >> 8) / 255.0;
  *b = (color->blue >> 8) / 255.0;
}

static void
hildon_pannable_draw_vscroll (GtkWidget * widget,
                              GdkColor *back_color,
                              GdkColor *scroll_color)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;
  gfloat y, height;
  cairo_t *cr;
  cairo_pattern_t *pattern;
  gdouble r, g, b;
  gint radius = (priv->vscroll_rect.width/2) - 1;

  cr = gdk_cairo_create(widget->window);

  /* Draw the background */
  rgb_from_gdkcolor (back_color, &r, &g, &b);
  cairo_set_source_rgb (cr, r, g, b);
  cairo_rectangle(cr, priv->vscroll_rect.x, priv->vscroll_rect.y,
                  priv->vscroll_rect.width,
                  priv->vscroll_rect.height);
  cairo_fill_preserve (cr);
  cairo_clip (cr);

  /* Calculate the scroll bar height and position */
  y = ((priv->vadjust->value - priv->vadjust->lower) / (priv->vadjust->upper - priv->vadjust->lower)) *
    (widget->allocation.height -
     (priv->hscroll_visible ? priv->indicator_width : 0));
  height = ((((priv->vadjust->value - priv->vadjust->lower) +
              priv->vadjust->page_size) /
             (priv->vadjust->upper - priv->vadjust->lower)) *
            (widget->allocation.height -
             (priv->hscroll_visible ? priv->indicator_width : 0))) - y;

  /* Set a minimum height */
  height = MAX (SCROLL_BAR_MIN_SIZE, height);

  /* Check the max y position */
  y = MIN (y, widget->allocation.height -
           (priv->hscroll_visible ? priv->hscroll_rect.height : 0) -
           height);

  /* Draw the scrollbar */
  rgb_from_gdkcolor (scroll_color, &r, &g, &b);

  pattern = cairo_pattern_create_linear(radius+1, y, radius+1,y + height);
  cairo_pattern_add_color_stop_rgb(pattern, 0, r, g, b);
  cairo_pattern_add_color_stop_rgb(pattern, 1, r/2, g/2, b/2);
  cairo_set_source(cr, pattern);
  cairo_fill(cr);
  cairo_pattern_destroy(pattern);

  cairo_arc(cr, priv->vscroll_rect.x + radius + 1, y + radius + 1, radius, G_PI, 0);
  cairo_line_to(cr, priv->vscroll_rect.x + (radius * 2) + 1, y + height - radius);
  cairo_arc(cr, priv->vscroll_rect.x + radius + 1, y + height - radius, radius, 0, G_PI);
  cairo_line_to(cr, priv->vscroll_rect.x + 1, y + height - radius);
  cairo_clip (cr);

  cairo_paint_with_alpha(cr, priv->scroll_indicator_alpha);

  cairo_destroy(cr);
}

static void
hildon_pannable_draw_hscroll (GtkWidget * widget,
                              GdkColor *back_color,
                              GdkColor *scroll_color)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;
  gfloat x, width;
  cairo_t *cr;
  cairo_pattern_t *pattern;
  gdouble r, g, b;
  gint radius = (priv->hscroll_rect.height/2) - 1;

  cr = gdk_cairo_create(widget->window);

  /* Draw the background */
  rgb_from_gdkcolor (back_color, &r, &g, &b);
  cairo_set_source_rgb (cr, r, g, b);
  cairo_rectangle(cr, priv->hscroll_rect.x, priv->hscroll_rect.y,
                  priv->hscroll_rect.width,
                  priv->hscroll_rect.height);
  cairo_fill_preserve (cr);
  cairo_clip (cr);

  /* calculate the scrollbar width and position */
  x = ((priv->hadjust->value - priv->hadjust->lower) / (priv->hadjust->upper - priv->hadjust->lower)) *
    (widget->allocation.width - (priv->vscroll_visible ? priv->indicator_width : 0));
  width =((((priv->hadjust->value - priv->hadjust->lower) +
            priv->hadjust->page_size) / (priv->hadjust->upper - priv->hadjust->lower)) *
          (widget->allocation.width -
           (priv->vscroll_visible ? priv->indicator_width : 0))) - x;

  /* Set a minimum width */
  width = MAX (SCROLL_BAR_MIN_SIZE, width);

  /* Check the max x position */
  x = MIN (x, widget->allocation.width -
           (priv->vscroll_visible ? priv->vscroll_rect.width : 0) -
           width);

  /* Draw the scrollbar */
  rgb_from_gdkcolor (scroll_color, &r, &g, &b);

  pattern = cairo_pattern_create_linear(x, radius+1, x+width, radius+1);
  cairo_pattern_add_color_stop_rgb(pattern, 0, r, g, b);
  cairo_pattern_add_color_stop_rgb(pattern, 1, r/2, g/2, b/2);
  cairo_set_source(cr, pattern);
  cairo_fill(cr);
  cairo_pattern_destroy(pattern);

  cairo_arc_negative(cr, x + radius + 1, priv->hscroll_rect.y + radius + 1, radius, 3*G_PI_2, G_PI_2);
  cairo_line_to(cr, x + width - radius, priv->hscroll_rect.y + (radius * 2) + 1);
  cairo_arc_negative(cr, x + width - radius, priv->hscroll_rect.y + radius + 1, radius, G_PI_2, 3*G_PI_2);
  cairo_line_to(cr, x + width - radius, priv->hscroll_rect.y + 1);
  cairo_clip (cr);

  cairo_paint_with_alpha(cr, priv->scroll_indicator_alpha);

  cairo_destroy(cr);
}

#else /* USE_CAIRO_SCROLLBARS */

static void
tranparency_color (GdkColor *color,
                   GdkColor colora,
                   GdkColor colorb,
                   gdouble transparency)
{
  gdouble diff;

  diff = colora.red - colorb.red;
  color->red = colora.red-diff*transparency;

  diff = colora.green - colorb.green;
  color->green = colora.green-diff*transparency;

  diff = colora.blue - colorb.blue;
  color->blue = colora.blue-diff*transparency;
}

static void
hildon_pannable_draw_vscroll (GtkWidget *widget,
                              GdkColor *back_color,
                              GdkColor *scroll_color)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;
  gfloat y, height;
  GdkColor transp_color;
  GdkGC *gc = priv->scrollbars_gc;

  gdk_draw_rectangle (widget->window,
                      widget->style->bg_gc[GTK_STATE_NORMAL],
                      TRUE,
                       priv->vscroll_rect.x, priv->vscroll_rect.y,
                      priv->vscroll_rect.width,
                      priv->vscroll_rect.height);

  y = ((priv->vadjust->value - priv->vadjust->lower) / (priv->vadjust->upper - priv->vadjust->lower)) *
    (widget->allocation.height - (priv->hscroll_visible ? priv->indicator_width : 0));
  height = ((((priv->vadjust->value - priv->vadjust->lower) + priv->vadjust->page_size) /
             (priv->vadjust->upper - priv->vadjust->lower)) *
            (widget->allocation.height -
             (priv->hscroll_visible ? priv->indicator_width : 0))) - y;

  /* Set a minimum height */
  height = MAX (SCROLL_BAR_MIN_SIZE, height);

  /* Check the max y position */
  y = MIN (y, widget->allocation.height -
           (priv->hscroll_visible ? priv->hscroll_rect.height : 0) -
           height);

  if (priv->scroll_indicator_alpha == 1.0) {
    transp_color = priv->scroll_color;
  } else if (priv->scroll_indicator_alpha < 1.0) {
    tranparency_color (&transp_color, *back_color, *scroll_color,
                       priv->scroll_indicator_alpha);
  }
  gdk_gc_set_rgb_fg_color (gc, &transp_color);

  gdk_draw_rectangle (widget->window, gc,
                      TRUE, priv->vscroll_rect.x, y,
                      priv->vscroll_rect.width, height);
}

static void
hildon_pannable_draw_hscroll (GtkWidget *widget,
                              GdkColor *back_color,
                              GdkColor *scroll_color)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;
  gfloat x, width;
  GdkColor transp_color;
  GdkGC *gc = priv->scrollbars_gc;

  gdk_draw_rectangle (widget->window,
                      widget->style->bg_gc[GTK_STATE_INSENSITIVE],
                      TRUE,
                      priv->hscroll_rect.x, priv->hscroll_rect.y,
                      priv->hscroll_rect.width,
                      priv->hscroll_rect.height);

  /* calculate the scrollbar width and position */
  x = ((priv->hadjust->value - priv->hadjust->lower) / (priv->hadjust->upper - priv->hadjust->lower)) *
    (widget->allocation.width - (priv->vscroll_visible ? priv->indicator_width : 0));
  width =((((priv->hadjust->value - priv->hadjust->lower) +
            priv->hadjust->page_size) / (priv->hadjust->upper - priv->hadjust->lower)) *
          (widget->allocation.width -
           (priv->vscroll_visible ? priv->indicator_width : 0))) - x;

  /* Set a minimum width */
  width = MAX (SCROLL_BAR_MIN_SIZE, width);

  /* Check the max x position */
  x = MIN (x, widget->allocation.width -
           (priv->vscroll_visible ? priv->vscroll_rect.width : 0) -
           width);

  if (priv->scroll_indicator_alpha == 1.0) {
    transp_color = priv->scroll_color;
  } else if (priv->scroll_indicator_alpha < 1.0) {
    tranparency_color (&transp_color, *back_color, *scroll_color,
                       priv->scroll_indicator_alpha);
  }
  gdk_gc_set_rgb_fg_color (gc, &transp_color);

  gdk_draw_rectangle (widget->window, gc,
                      TRUE, x, priv->hscroll_rect.y, width,
                      priv->hscroll_rect.height);
}

#endif /* USE_CAIRO_SCROLLBARS */

static void
hildon_pannable_area_initial_effect (GtkWidget * widget)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;

  if (priv->initial_hint) {
    if (priv->vscroll_visible || priv->hscroll_visible) {

      priv->scroll_indicator_event_interrupt = 0;
      priv->scroll_delay_counter = priv->scrollbar_fade_delay;

      hildon_pannable_area_launch_fade_timeout (HILDON_PANNABLE_AREA (widget), 1.0);
    }
  }
}

static void
hildon_pannable_area_launch_fade_timeout (HildonPannableArea * area,
                                          gdouble alpha)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (area)->priv;

  priv->scroll_indicator_alpha = alpha;

  if (!priv->scroll_indicator_timeout)
    priv->scroll_indicator_timeout =
      gdk_threads_add_timeout_full (G_PRIORITY_HIGH_IDLE + 20,
				    SCROLL_FADE_TIMEOUT,
				    (GSourceFunc) hildon_pannable_area_scroll_indicator_fade,
				    area,
				    NULL);
}

static void
hildon_pannable_area_adjust_changed (HildonPannableArea * area,
                                     gpointer data)
{
  if (GTK_WIDGET_REALIZED (area))
    hildon_pannable_area_refresh (area);
}

static void
hildon_pannable_area_adjust_value_changed (HildonPannableArea * area,
                                           gpointer data)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (area)->priv;
  gint xdiff, ydiff;
  gint x = priv->x_offset;
  gint y = priv->y_offset;

  priv->x_offset = priv->hadjust->value;
  xdiff = x - priv->x_offset;
  priv->y_offset = priv->vadjust->value;
  ydiff = y - priv->y_offset;

  if ((xdiff || ydiff) && GTK_WIDGET_DRAWABLE (area)) {
    hildon_pannable_area_redraw (area);

    if ((priv->vscroll_visible) || (priv->hscroll_visible)) {
      priv->scroll_indicator_event_interrupt = 0;
      priv->scroll_delay_counter = priv->scrollbar_fade_delay;

      hildon_pannable_area_launch_fade_timeout (area, 1.0);
    }
  }
}

static void
hildon_pannable_area_redraw (HildonPannableArea * area)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (area)->priv;

  /* Redraw scroll indicators */
  if (GTK_WIDGET_DRAWABLE (area)) {
      if (priv->hscroll_visible) {
        gdk_window_invalidate_rect (GTK_WIDGET (area)->window,
                                    &priv->hscroll_rect, FALSE);
      }

      if (priv->vscroll_visible) {
        gdk_window_invalidate_rect (GTK_WIDGET (area)->window,
                                    &priv->vscroll_rect, FALSE);
      }
  }
}

static gboolean
hildon_pannable_area_scroll_indicator_fade(HildonPannableArea * area)
{
  HildonPannableAreaPrivate *priv = area->priv;

  /* if moving do not fade out */
  if (((ABS (priv->vel_y)>priv->vmin)||
       (ABS (priv->vel_x)>priv->vmin))&&(!priv->button_pressed)) {

    return TRUE;
  }

  if (priv->scroll_indicator_event_interrupt) {
    /* Stop a fade out, and fade back in */
    if (priv->scroll_indicator_alpha > 0.9) {
      priv->scroll_indicator_alpha = 1.0;
      priv->scroll_indicator_timeout = 0;

      return FALSE;
    } else {
      priv->scroll_indicator_alpha += 0.2;
      hildon_pannable_area_redraw (area);

      return TRUE;
    }
  }

  if ((priv->scroll_indicator_alpha > 0.9) &&
      (priv->scroll_delay_counter > 0)) {
    priv->scroll_delay_counter--;

    return TRUE;
  }

  if (!priv->scroll_indicator_event_interrupt) {
    /* Continue fade out */
    if (priv->scroll_indicator_alpha < 0.1) {
      priv->scroll_indicator_timeout = 0;
      priv->scroll_indicator_alpha = 0.0;

      return FALSE;
    } else {
      priv->scroll_indicator_alpha -= 0.2;
      hildon_pannable_area_redraw (area);

      return TRUE;
    }
  }

  return TRUE;
}

static gboolean
hildon_pannable_area_expose_event (GtkWidget * widget,
                                   GdkEventExpose * event)
{

  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;
#if USE_CAIRO_SCROLLBARS == 1
  GdkColor back_color = widget->style->bg[GTK_STATE_NORMAL];
  GdkColor scroll_color = widget->style->base[GTK_STATE_SELECTED];
#else /* USE_CAIRO_SCROLLBARS */
  GdkColor back_color = widget->style->bg[GTK_STATE_NORMAL];
  GdkColor scroll_color = priv->scroll_color;
#endif

  if (G_UNLIKELY (priv->initial_effect)) {
    hildon_pannable_area_initial_effect (widget);

    priv->initial_effect = FALSE;
  }

  if (gtk_bin_get_child (GTK_BIN (widget))) {

    if (priv->scroll_indicator_alpha > 0.1) {
      if (priv->vscroll_visible) {
        hildon_pannable_draw_vscroll (widget, &back_color, &scroll_color);
      }
      if (priv->hscroll_visible) {
        hildon_pannable_draw_hscroll (widget, &back_color, &scroll_color);
      }
    }

    /* draw overshooting rectangles */
    if (priv->overshot_dist_y > 0) {
      gint overshot_height;

      overshot_height = MIN (priv->overshot_dist_y, widget->allocation.height -
                             (priv->hscroll_visible ? priv->hscroll_rect.height : 0));

      gdk_draw_rectangle (widget->window,
                          widget->style->bg_gc[GTK_STATE_NORMAL],
                          TRUE,
                          0,
                          0,
                          widget->allocation.width -
                          (priv->vscroll_visible ? priv->vscroll_rect.width : 0),
                          overshot_height);
    } else if (priv->overshot_dist_y < 0) {
      gint overshot_height;
      gint overshot_y;

      overshot_height =
        MAX (priv->overshot_dist_y,
             -(widget->allocation.height -
               (priv->hscroll_visible ? priv->hscroll_rect.height : 0)));

      overshot_y = MAX (widget->allocation.height +
                        overshot_height -
                        (priv->hscroll_visible ? priv->hscroll_rect.height : 0), 0);

      gdk_draw_rectangle (widget->window,
                          widget->style->bg_gc[GTK_STATE_NORMAL],
                          TRUE,
                          0,
                          overshot_y,
                          widget->allocation.width -
                          priv->vscroll_rect.width,
                          -overshot_height);
    }

    if (priv->overshot_dist_x > 0) {
      gint overshot_width;

      overshot_width = MIN (priv->overshot_dist_x, widget->allocation.width -
                             (priv->vscroll_visible ? priv->vscroll_rect.width : 0));

      gdk_draw_rectangle (widget->window,
			  widget->style->bg_gc[GTK_STATE_NORMAL],
			  TRUE,
			  0,
			  0,
                          overshot_width,
			  widget->allocation.height -
                          (priv->hscroll_visible ? priv->hscroll_rect.height : 0));
    } else if (priv->overshot_dist_x < 0) {
      gint overshot_width;
      gint overshot_x;

      overshot_width =
        MAX (priv->overshot_dist_x,
             -(widget->allocation.width -
               (priv->vscroll_visible ? priv->vscroll_rect.width : 0)));

      overshot_x = MAX (widget->allocation.width +
                        overshot_width -
                        (priv->vscroll_visible ? priv->vscroll_rect.width : 0), 0);

      gdk_draw_rectangle (widget->window,
			  widget->style->bg_gc[GTK_STATE_NORMAL],
			  TRUE,
                          overshot_x,
			  0,
			  -overshot_width,
			  widget->allocation.height -
			  priv->hscroll_rect.height);
    }

  }

  return GTK_WIDGET_CLASS (hildon_pannable_area_parent_class)->expose_event (widget, event);
}

static GdkWindow *
hildon_pannable_area_get_topmost (GdkWindow * window,
                                  gint x, gint y,
                                  gint * tx, gint * ty,
                                  GdkEventMask mask)
{
  /* Find the GdkWindow at the given point, by recursing from a given
   * parent GdkWindow. Optionally return the co-ordinates transformed
   * relative to the child window.
   */
  gint width, height;
  GList *c, *children;
  GdkWindow *selected_window = NULL;

  gdk_drawable_get_size (GDK_DRAWABLE (window), &width, &height);
  if ((x < 0) || (x >= width) || (y < 0) || (y >= height))
    return NULL;

  children = gdk_window_peek_children (window);

  if (!children) {
    if (tx)
      *tx = x;
    if (ty)
      *ty = y;
    selected_window = window;
  }

  for (c = children; c; c = c->next) {
    GdkWindow *child = (GdkWindow *) c->data;
    gint wx, wy;

    gdk_drawable_get_size (GDK_DRAWABLE (child), &width, &height);
    gdk_window_get_position (child, &wx, &wy);

    if ((x >= wx) && (x < (wx + width)) && (y >= wy) && (y < (wy + height)) &&
        (gdk_window_is_visible (child))) {

      if (gdk_window_peek_children (child)) {
        selected_window = hildon_pannable_area_get_topmost (child, x-wx, y-wy,
                                                            tx, ty, mask);
        if (!selected_window) {
          if (tx)
            *tx = x-wx;
          if (ty)
            *ty = y-wy;
          selected_window = child;
        }
      } else {
        if ((gdk_window_get_events (child)&mask)) {
          if (tx)
            *tx = x-wx;
          if (ty)
            *ty = y-wy;
          selected_window = child;
        }
      }
    }
  }

  return selected_window;
}

static void
synth_crossing (GdkWindow * child,
		gint x, gint y,
		gint x_root, gint y_root,
                guint32 time, gboolean in)
{
  GdkEvent *event;
  GdkEventType type = in ? GDK_ENTER_NOTIFY : GDK_LEAVE_NOTIFY;

  /* Send synthetic enter event */
  event = gdk_event_new (type);
  event->any.type = type;
  event->any.window = g_object_ref (child);
  event->any.send_event = FALSE;
  event->crossing.subwindow = g_object_ref (child);
  event->crossing.time = time;
  event->crossing.x = x;
  event->crossing.y = y;
  event->crossing.x_root = x_root;
  event->crossing.y_root = y_root;
  event->crossing.mode = GDK_CROSSING_NORMAL;
  event->crossing.detail = GDK_NOTIFY_UNKNOWN;
  event->crossing.focus = FALSE;
  event->crossing.state = 0;
  gdk_event_put (event);
  gdk_event_free (event);
}

static gboolean
hildon_pannable_area_button_press_cb (GtkWidget * widget,
				      GdkEventButton * event)
{
  gint x, y;
  HildonPannableArea *area = HILDON_PANNABLE_AREA (widget);
  HildonPannableAreaPrivate *priv = area->priv;

  priv->selection_movement =
      (event->state & GDK_SHIFT_MASK) &&
      (event->time == priv->last_time) &&
      (priv->last_type == 1);

  if ((!priv->enabled) || (event->button != 1) || (priv->selection_movement) ||
      ((event->time == priv->last_time) &&
       (priv->last_type == 1)) ||
      (gtk_bin_get_child (GTK_BIN (widget)) == NULL))
    return TRUE;

  priv->scroll_indicator_event_interrupt = 1;

  hildon_pannable_area_launch_fade_timeout (area,
                                            priv->scroll_indicator_alpha);

  priv->last_time = event->time;
  priv->last_press_time = event->time;
  priv->last_type = 1;

  priv->scroll_to_x = -1;
  priv->scroll_to_y = -1;

  if (priv->button_pressed && priv->child) {
    /* Widget stole focus on last click, send crossing-out event */
    synth_crossing (priv->child, 0, 0, event->x_root, event->y_root,
		    event->time, FALSE);
  }

  priv->x = event->x;
  priv->y = event->y;
  priv->ix = priv->x;
  priv->iy = priv->y;

  /* Don't allow a click if we're still moving fast */
  if ((ABS (priv->vel_x) <= (priv->vmax * priv->vfast_factor)) &&
      (ABS (priv->vel_y) <= (priv->vmax * priv->vfast_factor)))
    priv->child =
      hildon_pannable_area_get_topmost (gtk_bin_get_child (GTK_BIN (widget))->window,
					event->x, event->y, &x, &y, GDK_BUTTON_PRESS_MASK);
  else
    priv->child = NULL;

  priv->button_pressed = TRUE;

  /* Stop scrolling on mouse-down (so you can flick, then hold to stop) */
  priv->old_vel_x = priv->vel_x;
  priv->old_vel_y = priv->vel_y;
  priv->vel_x = 0;
  priv->vel_y = 0;
  if (priv->idle_id) {
    g_source_remove (priv->idle_id);
    priv->idle_id = 0;
    g_signal_emit (area, pannable_area_signals[PANNING_FINISHED], 0);
  }

  if (priv->child) {

    gdk_drawable_get_size (priv->child, &priv->child_width,
                           &priv->child_height);
    priv->last_in = TRUE;

    g_object_add_weak_pointer ((GObject *) priv->child,
			       (gpointer) & priv->child);

    synth_crossing (priv->child, x, y, event->x_root,
		    event->y_root, event->time, TRUE);

    /* Avoid reinjecting the event to create an infinite loop */
    if (priv->event_window == ((GdkEvent*) event)->any.window) {
      event = (GdkEventButton *) gdk_event_copy ((GdkEvent *) event);
      /* remove the reference we added with the copy */
      g_object_unref (((GdkEvent*) event)->any.window);
      event->x = x;
      event->y = y;
      priv->cx = x;
      priv->cy = y;

      /* Send synthetic click (button press/release) event */
      ((GdkEvent*) event)->any.window = g_object_ref (priv->child);

      gdk_event_put ((GdkEvent *) event);
      gdk_event_free ((GdkEvent *) event);
    }
  } else
    priv->child = NULL;

  return TRUE;
}

static gboolean
hildon_pannable_area_check_scrollbars (HildonPannableArea * area)
{
  HildonPannableAreaPrivate *priv = area->priv;
  gboolean prev_hscroll_visible, prev_vscroll_visible;

  prev_hscroll_visible = priv->hscroll_visible;
  prev_vscroll_visible = priv->vscroll_visible;

  if (!gtk_bin_get_child (GTK_BIN (area))) {
    priv->vscroll_visible = FALSE;
    priv->hscroll_visible = FALSE;
  } else {
    switch (priv->hscrollbar_policy) {
    case GTK_POLICY_ALWAYS:
      priv->hscroll_visible = TRUE;
      break;
    case GTK_POLICY_NEVER:
      priv->hscroll_visible = FALSE;
      break;
    default:
      priv->hscroll_visible = (priv->hadjust->upper - priv->hadjust->lower >
                               priv->hadjust->page_size);
    }

    switch (priv->vscrollbar_policy) {
    case GTK_POLICY_ALWAYS:
      priv->vscroll_visible = TRUE;
      break;
    case GTK_POLICY_NEVER:
      priv->vscroll_visible = FALSE;
      break;
    default:
      priv->vscroll_visible = (priv->vadjust->upper - priv->vadjust->lower >
                               priv->vadjust->page_size);
    }

    /* Store the vscroll/hscroll areas for redrawing */
    if (priv->vscroll_visible) {
      GtkAllocation *allocation = &GTK_WIDGET (area)->allocation;
      priv->vscroll_rect.x = allocation->width - priv->indicator_width;
      priv->vscroll_rect.y = 0;
      priv->vscroll_rect.width = priv->indicator_width;
      priv->vscroll_rect.height = allocation->height -
        (priv->hscroll_visible ? priv->indicator_width : 0);
    }
    if (priv->hscroll_visible) {
      GtkAllocation *allocation = &GTK_WIDGET (area)->allocation;
      priv->hscroll_rect.y = allocation->height - priv->indicator_width;
      priv->hscroll_rect.x = 0;
      priv->hscroll_rect.height = priv->indicator_width;
      priv->hscroll_rect.width = allocation->width -
        (priv->vscroll_visible ? priv->indicator_width : 0);
    }
  }

  return ((priv->hscroll_visible != prev_hscroll_visible) ||
          (priv->vscroll_visible != prev_vscroll_visible));
}

static void
hildon_pannable_area_refresh (HildonPannableArea * area)
{
  if (GTK_WIDGET_DRAWABLE (area) &&
      hildon_pannable_area_check_scrollbars (area)) {
    HildonPannableAreaPrivate *priv = area->priv;

    gtk_widget_queue_resize (GTK_WIDGET (area));

    if ((priv->vscroll_visible) || (priv->hscroll_visible)) {
      priv->scroll_indicator_event_interrupt = 0;
      priv->scroll_delay_counter = area->priv->scrollbar_fade_delay;

      hildon_pannable_area_launch_fade_timeout (area, 1.0);
    }
  } else {
    hildon_pannable_area_redraw (area);
  }
}

/* Scroll by a particular amount (in pixels). Optionally, return if
 * the scroll on a particular axis was successful.
 */
static void
hildon_pannable_axis_scroll (HildonPannableArea *area,
                             GtkAdjustment *adjust,
                             gdouble *vel,
                             gdouble inc,
                             gint *overshooting,
                             gint *overshot_dist,
                             gdouble *scroll_to,
                             gint overshoot_max,
                             gboolean *s)
{
  gdouble dist;
  HildonPannableAreaPrivate *priv = area->priv;

  dist = gtk_adjustment_get_value (adjust) - inc;

  /* Overshooting
   * We use overshot_dist to define the distance of the current overshoot,
   * and overshooting to define the direction/whether or not we are overshot
   */
  if (!(*overshooting)) {

    /* Initiation of the overshoot happens when the finger is released
     * and the current position of the pannable contents are out of range
     */
    if (dist < adjust->lower) {
      if (s) *s = FALSE;

      dist = adjust->lower;

      if (overshoot_max!=0) {
        *overshooting = 1;
        *scroll_to = -1;
        *overshot_dist = CLAMP (*overshot_dist + *vel, 0, overshoot_max);
        *vel = MIN (priv->vmax_overshooting, *vel);
        gtk_widget_queue_resize (GTK_WIDGET (area));
      } else {
        *vel = 0.0;
        *scroll_to = -1;
      }
    } else if (dist > adjust->upper - adjust->page_size) {
      if (s) *s = FALSE;

      dist = adjust->upper - adjust->page_size;

      if (overshoot_max!=0) {
        *overshooting = 1;
        *scroll_to = -1;
        *overshot_dist = CLAMP (*overshot_dist + *vel, -overshoot_max, 0);
        *vel = MAX (-priv->vmax_overshooting, *vel);
        gtk_widget_queue_resize (GTK_WIDGET (area));
      } else {
        *vel = 0.0;
        *scroll_to = -1;
      }
    } else {
      if ((*scroll_to) != -1) {
        if (((inc < 0)&&(*scroll_to <= dist))||
            ((inc > 0)&&(*scroll_to >= dist))) {
          dist = *scroll_to;
          *scroll_to = -1;
          *vel = 0;
        }
      }
    }

    adjust->value = dist;
  } else {
    if (!priv->button_pressed) {

      /* When the overshoot has started we continue for
       * PROP_BOUNCE_STEPS more steps into the overshoot before we
       * reverse direction. The deceleration factor is calculated
       * based on the percentage distance from the first item with
       * each iteration, therefore always returning us to the
       * top/bottom most element
       */
      if (*overshot_dist > 0) {

        if ((*overshooting < priv->bounce_steps) && (*vel > 0)) {
          (*overshooting)++;
          *vel = (((gdouble)*overshot_dist)/overshoot_max) * (*vel);
        } else if ((*overshooting >= priv->bounce_steps) && (*vel > 0)) {
          *vel *= -1;
        } else if ((*overshooting > 1) && (*vel < 0)) {
          /* we add the MIN in order to avoid very small speeds */
          *vel = MIN (((((gdouble)*overshot_dist)*0.8) * -1), -10.0);
        }

        *overshot_dist = CLAMP (*overshot_dist + *vel, 0, overshoot_max);

        gtk_widget_queue_resize (GTK_WIDGET (area));

      } else if (*overshot_dist < 0) {

        if ((*overshooting < priv->bounce_steps) && (*vel < 0)) {
          (*overshooting)++;
          *vel = (((gdouble)*overshot_dist)/overshoot_max) * (*vel) * -1;
        } else if ((*overshooting >= priv->bounce_steps) && (*vel < 0)) {
          *vel *= -1;
        } else if ((*overshooting > 1) && (*vel > 0)) {
          /* we add the MAX in order to avoid very small speeds */
          *vel = MAX (((((gdouble)*overshot_dist)*0.8) * -1), 10.0);
        }

        *overshot_dist = CLAMP (*overshot_dist + (*vel), -overshoot_max, 0);

        gtk_widget_queue_resize (GTK_WIDGET (area));

      } else {
        *overshooting = 0;
        *vel = 0;
        gtk_widget_queue_resize (GTK_WIDGET (area));
      }
    } else {

      gint overshot_dist_old = *overshot_dist;

      if (*overshot_dist > 0) {
        *overshot_dist = CLAMP ((*overshot_dist) + inc, 0, overshoot_max);
      } else if (*overshot_dist < 0) {
        *overshot_dist = CLAMP ((*overshot_dist) + inc, -1 * overshoot_max, 0);
      } else {
        *overshooting = 0;
        adjust->value = CLAMP (dist,
                               adjust->lower,
                               adjust->upper -
                               adjust->page_size);
      }

      if (*overshot_dist != overshot_dist_old)
        gtk_widget_queue_resize (GTK_WIDGET (area));
    }
  }
}

static void
hildon_pannable_area_scroll (HildonPannableArea *area,
                             gdouble x, gdouble y)
{
  gboolean sx, sy;
  HildonPannableAreaPrivate *priv = area->priv;
  gboolean hscroll_visible, vscroll_visible;
  gdouble hv, vv;

  if (gtk_bin_get_child (GTK_BIN (area)) == NULL)
    return;

  vscroll_visible = (priv->vadjust->upper - priv->vadjust->lower >
	     priv->vadjust->page_size);
  hscroll_visible = (priv->hadjust->upper - priv->hadjust->lower >
	     priv->hadjust->page_size);

  sx = TRUE;
  sy = TRUE;

  hv = priv->hadjust->value;
  vv = priv->vadjust->value;

  if (vscroll_visible) {
    hildon_pannable_axis_scroll (area, priv->vadjust, &priv->vel_y, y,
                                 &priv->overshooting_y, &priv->overshot_dist_y,
                                 &priv->scroll_to_y, priv->vovershoot_max, &sy);
  } else {
    priv->vel_y = 0.0;
    priv->scroll_to_y = -1;
  }

  if (hscroll_visible) {
    hildon_pannable_axis_scroll (area, priv->hadjust, &priv->vel_x, x,
                                 &priv->overshooting_x, &priv->overshot_dist_x,
                                 &priv->scroll_to_x, priv->hovershoot_max, &sx);
  } else {
    priv->vel_x = 0.0;
    priv->scroll_to_x = -1;
  }

  if (hv != priv->hadjust->value)
    gtk_adjustment_value_changed (priv->hadjust);

  if (vv != priv->vadjust->value)
    gtk_adjustment_value_changed (priv->vadjust);

  /* If the scroll on a particular axis wasn't succesful, reset the
   * initial scroll position to the new mouse co-ordinate. This means
   * when you get to the top of the page, dragging down works immediately.
   */
  if (priv->mode == HILDON_PANNABLE_AREA_MODE_ACCEL) {
      if (!sx) {
        priv->x = priv->ex;
      }

      if (!sy) {
        priv->y = priv->ey;
      }
    }
}

static gboolean
hildon_pannable_area_timeout (HildonPannableArea * area)
{
  HildonPannableAreaPrivate *priv = area->priv;

  if ((!priv->enabled) || (priv->mode == HILDON_PANNABLE_AREA_MODE_PUSH)) {
    priv->idle_id = 0;
    g_signal_emit (area, pannable_area_signals[PANNING_FINISHED], 0);

    return FALSE;
  }

  hildon_pannable_area_scroll (area, priv->vel_x, priv->vel_y);

  gdk_window_process_updates (GTK_WIDGET (area)->window, FALSE);

  if (!priv->button_pressed) {
    /* Decelerate gradually when pointer is raised */
    if ((!priv->overshot_dist_y) &&
        (!priv->overshot_dist_x)) {

      /* in case we move to a specific point do not decelerate when arriving */
      if ((priv->scroll_to_x != -1)||(priv->scroll_to_y != -1)) {

        if (ABS (priv->vel_x) >= 1.5) {
          priv->vel_x *= priv->decel;
        }

        if (ABS (priv->vel_y) >= 1.5) {
          priv->vel_y *= priv->decel;
        }

      } else {
        if ((!priv->low_friction_mode) ||
            ((priv->mov_mode&HILDON_MOVEMENT_MODE_HORIZ) &&
             (ABS (priv->vel_x) < 0.8*priv->vmax)))
          priv->vel_x *= priv->decel;

        if ((!priv->low_friction_mode) ||
            ((priv->mov_mode&HILDON_MOVEMENT_MODE_VERT) &&
             (ABS (priv->vel_y) < 0.8*priv->vmax)))
          priv->vel_y *= priv->decel;

        if ((ABS (priv->vel_x) < 1.0) && (ABS (priv->vel_y) < 1.0)) {
          priv->vel_x = 0;
          priv->vel_y = 0;
          priv->idle_id = 0;

          g_signal_emit (area, pannable_area_signals[PANNING_FINISHED], 0);

          return FALSE;
        }
      }
    }
  } else if (priv->mode == HILDON_PANNABLE_AREA_MODE_AUTO) {
    priv->idle_id = 0;

    return FALSE;
  }

  return TRUE;
}

static void
hildon_pannable_area_calculate_velocity (gdouble *vel,
                                         gdouble delta,
                                         gdouble dist,
                                         gdouble vmax,
                                         gdouble drag_inertia,
                                         gdouble force,
                                         guint sps)
{
  gdouble rawvel;

  if (ABS (dist) >= RATIO_TOLERANCE) {
    rawvel = (dist / ABS (delta)) * force;
    *vel = *vel * (1 - drag_inertia) +
      rawvel * drag_inertia;
    *vel = *vel > 0 ? MIN (*vel, vmax)
      : MAX (*vel, -1 * vmax);
  }
}

static gboolean
hildon_pannable_area_motion_event_scroll_timeout (HildonPannableArea *area)
{
  HildonPannableAreaPrivate *priv = area->priv;

  if ((priv->motion_x != 0)||(priv->motion_y != 0))
    hildon_pannable_area_scroll (area, priv->motion_x, priv->motion_y);

  priv->motion_event_scroll_timeout = 0;

  return FALSE;
}

static void
hildon_pannable_area_motion_event_scroll (HildonPannableArea *area,
                                          gdouble x, gdouble y)
{
  HildonPannableAreaPrivate *priv = area->priv;

  if (priv->motion_event_scroll_timeout) {

    priv->motion_x += x;
    priv->motion_y += y;

  } else {

  /* we do not delay the first event but the next ones */
    hildon_pannable_area_scroll (area, x, y);

    priv->motion_x = 0;
    priv->motion_y = 0;

    priv->motion_event_scroll_timeout = gdk_threads_add_timeout_full
      (G_PRIORITY_HIGH_IDLE + 20,
       (gint) (1000.0 / (gdouble) MOTION_EVENTS_PER_SECOND),
       (GSourceFunc) hildon_pannable_area_motion_event_scroll_timeout, area, NULL);
  }
}

static void
hildon_pannable_area_check_move (HildonPannableArea *area,
                                 GdkEventMotion * event,
                                 gdouble *x,
                                 gdouble *y)
{
  HildonPannableAreaPrivate *priv = area->priv;

  if (priv->first_drag && (!priv->moved) &&
      ((ABS (*x) > (priv->panning_threshold))
       || (ABS (*y) > (priv->panning_threshold)))) {
    priv->moved = TRUE;
    *x = 0;
    *y = 0;

    if (priv->first_drag) {
        gboolean vscroll_visible;
        gboolean hscroll_visible;

      if (ABS (priv->iy - event->y) >=
          ABS (priv->ix - event->x)) {

        g_signal_emit (area,
                       pannable_area_signals[VERTICAL_MOVEMENT],
                       0, (priv->iy > event->y) ?
                       HILDON_MOVEMENT_UP :
                       HILDON_MOVEMENT_DOWN,
                       (gdouble)priv->ix, (gdouble)priv->iy);

        vscroll_visible = (priv->vadjust->upper - priv->vadjust->lower >
                   priv->vadjust->page_size);

        if (!((vscroll_visible)&&
              (priv->mov_mode&HILDON_MOVEMENT_MODE_VERT))) {

          hscroll_visible = (priv->hadjust->upper - priv->hadjust->lower >
                             priv->hadjust->page_size);

          /* even in case we do not have to move we check if this
             could be a fake horizontal movement */
          if (!((hscroll_visible)&&
                (priv->mov_mode&HILDON_MOVEMENT_MODE_HORIZ)) ||
              (ABS (priv->iy - event->y) -
               ABS (priv->ix - event->x) >= priv->direction_error_margin))
            priv->moved = FALSE;
        }
      } else {

        g_signal_emit (area,
                       pannable_area_signals[HORIZONTAL_MOVEMENT],
                       0, (priv->ix > event->x) ?
                       HILDON_MOVEMENT_LEFT :
                       HILDON_MOVEMENT_RIGHT,
                       (gdouble)priv->ix, (gdouble)priv->iy);

        hscroll_visible = (priv->hadjust->upper - priv->hadjust->lower >
                           priv->hadjust->page_size);

        if (!((hscroll_visible)&&
              (priv->mov_mode&HILDON_MOVEMENT_MODE_HORIZ))) {

          vscroll_visible = (priv->vadjust->upper - priv->vadjust->lower >
                             priv->vadjust->page_size);

          /* even in case we do not have to move we check if this
             could be a fake vertical movement */
          if (!((vscroll_visible) &&
                (priv->mov_mode&HILDON_MOVEMENT_MODE_VERT)) ||
              (ABS (priv->ix - event->x) -
               ABS (priv->iy - event->y) >= priv->direction_error_margin))
            priv->moved = FALSE;
        }
      }

      if ((priv->moved)&&(priv->child)) {
        gint pos_x, pos_y;

        pos_x = priv->cx + (event->x - priv->ix);
        pos_y = priv->cy + (event->y - priv->iy);

        synth_crossing (priv->child, pos_x, pos_y, event->x_root,
                        event->y_root, event->time, FALSE);
      }

      if (priv->moved) {
        gboolean result_val;

        g_signal_emit (area,
                       pannable_area_signals[PANNING_STARTED],
                       0, &result_val);

        priv->moved = !result_val;
      }
    }

    priv->first_drag = FALSE;

    if ((priv->mode != HILDON_PANNABLE_AREA_MODE_PUSH) &&
	(priv->mode != HILDON_PANNABLE_AREA_MODE_AUTO)) {

      if (!priv->idle_id)
        priv->idle_id = gdk_threads_add_timeout_full
          (G_PRIORITY_HIGH_IDLE + 20,
	   (gint)(1000.0 / (gdouble) priv->sps),
	   (GSourceFunc)
	   hildon_pannable_area_timeout, area, NULL);
    }
  }
}

static void
hildon_pannable_area_handle_move (HildonPannableArea *area,
                                  GdkEventMotion * event,
                                  gdouble *x,
                                  gdouble *y)
{
  HildonPannableAreaPrivate *priv = area->priv;
  gdouble delta;

  switch (priv->mode) {
  case HILDON_PANNABLE_AREA_MODE_PUSH:
    /* Scroll by the amount of pixels the cursor has moved
     * since the last motion event.
     */
    hildon_pannable_area_motion_event_scroll (area, *x, *y);
    priv->x = event->x;
    priv->y = event->y;
    break;
  case HILDON_PANNABLE_AREA_MODE_ACCEL:
    /* Set acceleration relative to the initial click */
    priv->ex = event->x;
    priv->ey = event->y;
    priv->vel_x = ((*x > 0) ? 1 : -1) *
      (((ABS (*x) /
         (gdouble) GTK_WIDGET (area)->allocation.width) *
        (priv->vmax - priv->vmin)) + priv->vmin);
    priv->vel_y = ((*y > 0) ? 1 : -1) *
      (((ABS (*y) /
         (gdouble) GTK_WIDGET (area)->allocation.height) *
        (priv->vmax - priv->vmin)) + priv->vmin);
    break;
  case HILDON_PANNABLE_AREA_MODE_AUTO:

    delta = event->time - priv->last_time;

    if (priv->mov_mode&HILDON_MOVEMENT_MODE_VERT) {
      gdouble dist = event->y - priv->y;

      hildon_pannable_area_calculate_velocity (&priv->vel_y,
                                               delta,
                                               dist,
                                               priv->vmax,
                                               priv->drag_inertia,
                                               priv->force,
                                               priv->sps);
    } else {
      *y = 0;
      priv->vel_y = 0;
    }

    if (priv->mov_mode&HILDON_MOVEMENT_MODE_HORIZ) {
      gdouble dist = event->x - priv->x;

      hildon_pannable_area_calculate_velocity (&priv->vel_x,
                                               delta,
                                               dist,
                                               priv->vmax,
                                               priv->drag_inertia,
                                               priv->force,
                                               priv->sps);
    } else {
      *x = 0;
      priv->vel_x = 0;
    }

    hildon_pannable_area_motion_event_scroll (area, *x, *y);

    if (priv->mov_mode&HILDON_MOVEMENT_MODE_HORIZ)
      priv->x = event->x;
    if (priv->mov_mode&HILDON_MOVEMENT_MODE_VERT)
      priv->y = event->y;

    break;
  default:
    break;
  }
}

static gboolean
hildon_pannable_area_motion_notify_cb (GtkWidget * widget,
				       GdkEventMotion * event)
{
  HildonPannableArea *area = HILDON_PANNABLE_AREA (widget);
  HildonPannableAreaPrivate *priv = area->priv;
  gdouble x, y;

  if (gtk_bin_get_child (GTK_BIN (widget)) == NULL)
    return TRUE;

  if ((!priv->enabled) || (!priv->button_pressed) ||
      ((event->time == priv->last_time) && (priv->last_type == 2))) {
    gdk_window_get_pointer (widget->window, NULL, NULL, 0);
    return TRUE;
  }

  if (!priv->selection_movement) {

    if (priv->last_type == 1) {
      priv->first_drag = TRUE;
    }

    x = event->x - priv->x;
    y = event->y - priv->y;

    if (!priv->moved) {
      hildon_pannable_area_check_move (area, event, &x, &y);
    }

    if (priv->moved) {
      hildon_pannable_area_handle_move (area, event, &x, &y);
    } else if (priv->child) {
      gboolean in;
      gint pos_x, pos_y;

      pos_x = priv->cx + (event->x - priv->ix);
      pos_y = priv->cy + (event->y - priv->iy);

      in = (((0 <= pos_x)&&(priv->child_width >= pos_x)) &&
            ((0 <= pos_y)&&(priv->child_height >= pos_y)));

      if (((!priv->last_in)&&in)||((priv->last_in)&&(!in))) {

        synth_crossing (priv->child, pos_x, pos_y, event->x_root,
                        event->y_root, event->time, in);

        priv->last_in = in;
      }
    }

    priv->last_time = event->time;
    priv->last_type = 2;
  }

  if (priv->child && priv->event_window == ((GdkEvent*) event)->any.window) {
      /* Send motion notify to child */
      event = (GdkEventMotion *) gdk_event_copy ((GdkEvent *) event);
      /* remove the reference we added with the copy */
      g_object_unref (((GdkEvent*) event)->any.window);
      event->x = priv->cx + (event->x - priv->ix);
      event->y = priv->cy + (event->y - priv->iy);
      ((GdkEvent*) event)->any.window = g_object_ref (priv->child);
      gdk_event_put ((GdkEvent *) event);
      gdk_event_free ((GdkEvent *) event);
  }

  gdk_window_get_pointer (widget->window, NULL, NULL, 0);

  return TRUE;
}

static gboolean
hildon_pannable_leave_notify_event (GtkWidget *widget,
                                    GdkEventCrossing *event)
{
  HildonPannableArea *area = HILDON_PANNABLE_AREA (widget);
  HildonPannableAreaPrivate *priv = area->priv;

  if ((priv->child)&&(priv->last_in)) {
    priv->last_in = FALSE;

    synth_crossing (priv->child, 0, 0, event->x_root,
                    event->y_root, event->time, FALSE);
  }

  return FALSE;
}

static gboolean
hildon_pannable_area_key_release_cb (GtkWidget * widget,
                                     GdkEventKey * event)
{
  HildonPannableArea *area = HILDON_PANNABLE_AREA (widget);

  if (G_UNLIKELY (area->priv->center_on_child_focus_pending)) {
    hildon_pannable_area_center_on_child_focus (area);
    area->priv->center_on_child_focus_pending = FALSE;
  }

  return FALSE;
}

static gboolean
hildon_pannable_area_button_release_cb (GtkWidget * widget,
					GdkEventButton * event)
{
  HildonPannableArea *area = HILDON_PANNABLE_AREA (widget);
  HildonPannableAreaPrivate *priv = area->priv;
  gint x, y;
  gdouble dx, dy;
  GdkWindow *child;
  gboolean force_fast = TRUE;

  if  (((event->time == priv->last_time) && (priv->last_type == 3))
       || (gtk_bin_get_child (GTK_BIN (widget)) == NULL)
       || (!priv->button_pressed) || (!priv->enabled) || (event->button != 1))
    return TRUE;

  if (!priv->selection_movement) {
    /* if last event was a motion-notify we have to check the movement
       and launch the animation */
    if (priv->last_type == 2) {

      dx = event->x - priv->x;
      dy = event->y - priv->y;

      hildon_pannable_area_check_move (area, (GdkEventMotion *) event, &dx, &dy);

      if (priv->moved) {
        gdouble delta = event->time - priv->last_time;

        hildon_pannable_area_handle_move (area, (GdkEventMotion *) event, &dx, &dy);

        /* move all the way to the last position now */
        if (priv->motion_event_scroll_timeout) {
          g_source_remove (priv->motion_event_scroll_timeout);
          hildon_pannable_area_motion_event_scroll_timeout (HILDON_PANNABLE_AREA (widget));
          priv->motion_x = 0;
          priv->motion_y = 0;
        }

        if ((ABS (dx) < 4.0) && (delta >= CURSOR_STOPPED_TIMEOUT))
          priv->vel_x = 0;

        if ((ABS (dy) < 4.0) && (delta >= CURSOR_STOPPED_TIMEOUT))
          priv->vel_y = 0;
      }
    }

    /* If overshoot has been initiated with a finger down, on release set max speed */
    if (priv->overshot_dist_y != 0) {
      priv->overshooting_y = priv->bounce_steps; /* Hack to stop a bounce in the finger down case */
      priv->vel_y = priv->overshot_dist_y * 0.9;
    }

    if (priv->overshot_dist_x != 0) {
      priv->overshooting_x = priv->bounce_steps; /* Hack to stop a bounce in the finger down case */
      priv->vel_x = priv->overshot_dist_x * 0.9;
    }

    priv->button_pressed = FALSE;

    /* if widget was moving fast in the panning, increase speed even more */
    if ((event->time - priv->last_press_time < FAST_CLICK) &&
        ((ABS (priv->old_vel_x) > priv->vmin) ||
         (ABS (priv->old_vel_y) > priv->vmin)) &&
        ((ABS (priv->old_vel_x) > MIN_ACCEL_THRESHOLD) ||
         (ABS (priv->old_vel_y) > MIN_ACCEL_THRESHOLD)))
      {
        gint symbol = 0;

        if (priv->vel_x != 0)
          symbol = ((priv->vel_x * priv->old_vel_x) > 0) ? 1 : -1;

        priv->vel_x = symbol *
          (priv->old_vel_x + ((priv->old_vel_x > 0) ? priv->accel_vel_x
                              : -priv->accel_vel_x));

        symbol = 0;

        if (priv->vel_y != 0)
          symbol = ((priv->vel_y * priv->old_vel_y) > 0) ? 1 : -1;

        priv->vel_y = symbol *
          (priv->old_vel_y + ((priv->old_vel_y > 0) ? priv->accel_vel_y
                              : -priv->accel_vel_y));

        force_fast = FALSE;
      }

    if  ((ABS (priv->vel_y) >= priv->vmin) ||
         (ABS (priv->vel_x) >= priv->vmin)) {

      /* we have to move because we are in overshooting position*/
      if (!priv->moved) {
        gboolean result_val;

        g_signal_emit (area,
                       pannable_area_signals[PANNING_STARTED],
                       0, &result_val);
      }

      priv->scroll_indicator_alpha = 1.0;

      if (force_fast) {
        if ((ABS (priv->vel_x) > MAX_SPEED_THRESHOLD) &&
            (priv->accel_vel_x > MAX_SPEED_THRESHOLD))
          priv->vel_x = (priv->vel_x > 0) ? priv->accel_vel_x : -priv->accel_vel_x;

        if ((ABS (priv->vel_y) > MAX_SPEED_THRESHOLD) &&
            (priv->accel_vel_y > MAX_SPEED_THRESHOLD))
          priv->vel_y = (priv->vel_y > 0) ? priv->accel_vel_y : -priv->accel_vel_y;
      }

      if (!priv->idle_id)
        priv->idle_id = gdk_threads_add_timeout_full (G_PRIORITY_HIGH_IDLE + 20,
                                                      (gint) (1000.0 / (gdouble) priv->sps),
                                                      (GSourceFunc) hildon_pannable_area_timeout,
                                                      widget, NULL);
    } else {
      if (priv->center_on_child_focus_pending) {
        hildon_pannable_area_center_on_child_focus (area);
      }

      if (priv->moved)
        g_signal_emit (widget, pannable_area_signals[PANNING_FINISHED], 0);
    }

    area->priv->center_on_child_focus_pending = FALSE;

    priv->scroll_indicator_event_interrupt = 0;
    priv->scroll_delay_counter = priv->scrollbar_fade_delay;

    hildon_pannable_area_launch_fade_timeout (HILDON_PANNABLE_AREA (widget),
                                              priv->scroll_indicator_alpha);
  }

  priv->last_time = event->time;
  priv->last_type = 3;

  if (!priv->child) {
    priv->moved = FALSE;
    return TRUE;
  }

  child =
    hildon_pannable_area_get_topmost (gtk_bin_get_child (GTK_BIN (widget))->window,
				      event->x, event->y, &x, &y, GDK_BUTTON_RELEASE_MASK);

  event = (GdkEventButton *) gdk_event_copy ((GdkEvent *) event);
  /* remove the reference we added with the copy */
  g_object_unref (((GdkEvent*) event)->any.window);
  event->x = x;
  event->y = y;

  /* Leave the widget if we've moved - This doesn't break selection,
   * but stops buttons from being clicked.
   */
  if ((child != priv->child) || (priv->moved)) {
    /* Send synthetic leave event */
    synth_crossing (priv->child, x, y, event->x_root,
		    event->y_root, event->time, FALSE);
    /* insure no click will happen for widgets that do not handle
       leave-notify */
    event->x = -16384;
    event->y = -16384;
    /* Send synthetic button release event */
    ((GdkEvent *) event)->any.window = g_object_ref (priv->child);
    gdk_event_put ((GdkEvent *) event);
  } else {
    /* Send synthetic button release event */
    ((GdkEvent *) event)->any.window = g_object_ref (child);
    gdk_event_put ((GdkEvent *) event);
    /* Send synthetic leave event */
    synth_crossing (priv->child, x, y, event->x_root,
		    event->y_root, event->time, FALSE);
  }
  g_object_remove_weak_pointer ((GObject *) priv->child,
				(gpointer) & priv->child);

  priv->moved = FALSE;
  gdk_event_free ((GdkEvent *) event);

  return TRUE;
}

/* utility event handler */
static gboolean
hildon_pannable_area_scroll_cb (GtkWidget *widget,
                                GdkEventScroll *event)
{
  GtkAdjustment *adj = NULL;
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (widget)->priv;

  if ((!priv->enabled) ||
      (gtk_bin_get_child (GTK_BIN (widget)) == NULL))
    return TRUE;

  priv->scroll_indicator_event_interrupt = 0;
  priv->scroll_delay_counter = priv->scrollbar_fade_delay + 20;

  hildon_pannable_area_launch_fade_timeout (HILDON_PANNABLE_AREA (widget), 1.0);

  /* Stop inertial scrolling */
  if (priv->idle_id) {
    priv->vel_x = 0.0;
    priv->vel_y = 0.0;
    priv->overshooting_x = 0;
    priv->overshooting_y = 0;

    if ((priv->overshot_dist_x>0)||(priv->overshot_dist_y>0)) {
      priv->overshot_dist_x = 0;
      priv->overshot_dist_y = 0;

      gtk_widget_queue_resize (GTK_WIDGET (widget));
    }

    g_signal_emit (widget, pannable_area_signals[PANNING_FINISHED], 0);

    g_source_remove (priv->idle_id);
    priv->idle_id = 0;
  }

  if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN)
    adj = priv->vadjust;
  else
    adj = priv->hadjust;

  if (adj)
    {
      gdouble delta, new_value;

      /* from gtkrange.c calculate delta*/
      delta = pow (adj->page_size, 2.0 / 3.0);

      if (event->direction == GDK_SCROLL_UP ||
          event->direction == GDK_SCROLL_LEFT)
        delta = - delta;

      new_value = CLAMP (adj->value + delta, adj->lower, adj->upper - adj->page_size);

      gtk_adjustment_set_value (adj, new_value);
    }

  return TRUE;
}

static void
hildon_pannable_area_child_mapped (GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer user_data)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (user_data)->priv;

  if (priv->event_window != NULL && priv->enabled)
    gdk_window_raise (priv->event_window);
}

static void
hildon_pannable_area_add (GtkContainer *container, GtkWidget *child)
{
  HildonPannableAreaPrivate *priv = HILDON_PANNABLE_AREA (container)->priv;

  g_return_if_fail (gtk_bin_get_child (GTK_BIN (container)) == NULL);

  gtk_widget_set_parent (child, GTK_WIDGET (container));
  GTK_BIN (container)->child = child;

  g_signal_connect_after (child, "map-event",
                          G_CALLBACK (hildon_pannable_area_child_mapped),
                          container);

  if (!gtk_widget_set_scroll_adjustments (child, priv->hadjust, priv->vadjust)) {
    g_warning ("%s: cannot add non scrollable widget, "
               "wrap it in a viewport", __FUNCTION__);
  }
}

/* call this function if you are not panning */
static void
hildon_pannable_area_center_on_child_focus      (HildonPannableArea *area)
{
  GtkWidget *focused_child = NULL;
  GtkWidget *window = NULL;

  window = gtk_widget_get_toplevel (GTK_WIDGET (area));

  if (GTK_WIDGET_TOPLEVEL (window)) {
    focused_child = gtk_window_get_focus (GTK_WINDOW (window));
  }

  if (focused_child) {
    hildon_pannable_area_scroll_to_child (area, focused_child);
  }
}

static void
hildon_pannable_area_set_focus_child            (GtkContainer     *container,
                                                 GtkWidget        *child)
{
  HildonPannableArea *area = HILDON_PANNABLE_AREA (container);

  if (!area->priv->center_on_child_focus) {
    return;
  }

  if (GTK_IS_WIDGET (child)) {
    area->priv->center_on_child_focus_pending = TRUE;
  }
}

static void
hildon_pannable_area_remove (GtkContainer *container, GtkWidget *child)
{
  g_return_if_fail (HILDON_IS_PANNABLE_AREA (container));
  g_return_if_fail (child != NULL);
  g_return_if_fail (gtk_bin_get_child (GTK_BIN (container)) == child);

  gtk_widget_set_scroll_adjustments (child, NULL, NULL);

  g_signal_handlers_disconnect_by_func (child,
                                        hildon_pannable_area_child_mapped,
                                        container);

  /* chain parent class handler to remove child */
  GTK_CONTAINER_CLASS (hildon_pannable_area_parent_class)->remove (container, child);
}

/*
 * This method calculates a factor necessary to determine the initial distance
 * to jump in hildon_pannable_area_scroll_to(). For fixed time and frames per
 * second, we know in how many frames 'n' we need to reach the destination
 * point. We know that, for a distance d,
 *
 *   d = d_0 + d_1 + ... + d_n
 *
 * where d_i is the distance travelled in the i-th frame and decel_factor is
 * the deceleration factor. This can be rewritten as
 *
 *   d = d_0 + (d_0 * decel_factor) + ... + (d_n-1 * decel_factor),
 *
 * since the distance travelled on each frame is the distance travelled in the
 * previous frame reduced by the deceleration factor. Reducing this and
 * factoring d_0 out, we get
 *
 *   d = d_0 (1 + decel_factor + ... + decel_factor^(n-1)).
 *
 * Since the sum is independent of the distance to be travelled, we can define
 * vel_factor as
 *
 *   vel_factor = 1 + decel_factor + ... + decel_factor^(n-1).
 *
 * That's the gem we calculate in this method.
 */
static void
hildon_pannable_calculate_vel_factor (HildonPannableArea * self)
{
  HildonPannableAreaPrivate *priv = self->priv;
  gfloat fct = 1;
  gfloat fct_i = 1;
  gint i, n;

  n = ceil (priv->sps * priv->scroll_time);

  for (i = 1; i < n && fct_i >= RATIO_TOLERANCE; i++) {
    fct_i *= priv->decel;
    fct += fct_i;
  }

  priv->vel_factor = fct;
}

/**
 * hildon_pannable_area_new:
 *
 * Create a new pannable area widget
 *
 * Returns: the newly created #HildonPannableArea
 *
 * Since: 2.2
 */

GtkWidget *
hildon_pannable_area_new (void)
{
  return g_object_new (HILDON_TYPE_PANNABLE_AREA, NULL);
}

/**
 * hildon_pannable_area_new_full:
 * @mode: #HildonPannableAreaMode
 * @enabled: Value for the enabled property
 * @vel_min: Value for the velocity-min property
 * @vel_max: Value for the velocity-max property
 * @decel: Value for the deceleration property
 * @sps: Value for the sps property
 *
 * Create a new #HildonPannableArea widget and set various properties
 *
 * returns: the newly create #HildonPannableArea
 *
 * Since: 2.2
 */

GtkWidget *
hildon_pannable_area_new_full (gint mode, gboolean enabled,
			       gdouble vel_min, gdouble vel_max,
			       gdouble decel, guint sps)
{
  return g_object_new (HILDON_TYPE_PANNABLE_AREA,
		       "mode", mode,
		       "enabled", enabled,
		       "velocity_min", vel_min,
		       "velocity_max", vel_max,
		       "deceleration", decel, "sps", sps, NULL);
}

/**
 * hildon_pannable_area_add_with_viewport:
 * @area: A #HildonPannableArea
 * @child: Child widget to add to the viewport
 *
 *
 * Convenience function used to add a child to a #GtkViewport, and add
 * the viewport to the scrolled window for childreen without native
 * scrolling capabilities.
 *
 * See gtk_scrolled_window_add_with_viewport() for more information.
 *
 * Since: 2.2
 */

void
hildon_pannable_area_add_with_viewport (HildonPannableArea * area,
					GtkWidget * child)
{
  GtkBin *bin;
  GtkWidget *viewport;

  g_return_if_fail (HILDON_IS_PANNABLE_AREA (area));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (child->parent == NULL);

  bin = GTK_BIN (area);

  if (bin->child != NULL)
    {
      g_return_if_fail (GTK_IS_VIEWPORT (bin->child));
      g_return_if_fail (GTK_BIN (bin->child)->child == NULL);

      viewport = bin->child;
    }
  else
    {
      HildonPannableAreaPrivate *priv = area->priv;

      viewport = gtk_viewport_new (priv->hadjust,
                                   priv->vadjust);
      gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_NONE);
      gtk_container_add (GTK_CONTAINER (area), viewport);
    }

  gtk_widget_show (viewport);
  gtk_container_add (GTK_CONTAINER (viewport), child);
}

/**
 * hildon_pannable_area_scroll_to:
 * @area: A #HildonPannableArea.
 * @x: The x coordinate of the destination point or -1 to ignore this axis.
 * @y: The y coordinate of the destination point or -1 to ignore this axis.
 *
 * Smoothly scrolls @area to ensure that (@x, @y) is a visible point
 * on the widget. To move in only one coordinate, you must set the other one
 * to -1. Notice that, in %HILDON_PANNABLE_AREA_MODE_PUSH mode, this function
 * works just like hildon_pannable_area_jump_to().
 *
 * This function is useful if you need to present the user with a particular
 * element inside a scrollable widget, like #GtkTreeView. For instance,
 * the following example shows how to scroll inside a #GtkTreeView to
 * make visible an item, indicated by the #GtkTreeIter @iter.
 *
 * <example>
 * <programlisting>
 *  GtkTreePath *path;
 *  GdkRectangle *rect;
 *  <!-- -->
 *  path = gtk_tree_model_get_path (model, &amp;iter);
 *  gtk_tree_view_get_background_area (GTK_TREE_VIEW (treeview),
 *                                     path, NULL, &amp;rect);
 *  gtk_tree_view_convert_bin_window_to_tree_coords (GTK_TREE_VIEW (treeview),
 *                                                   0, rect.y, NULL, &amp;y);
 *  hildon_pannable_area_scroll_to (panarea, -1, y);
 *  gtk_tree_path_free (path);
 * </programlisting>
 * </example>
 *
 * If you want to present a child widget in simpler scenarios,
 * use hildon_pannable_area_scroll_to_child() instead.
 *
 * There is a precondition to this function: the widget must be
 * already realized. Check the hildon_pannable_area_jump_to_child() for
 * more tips regarding how to call this function during
 * initialization.
 *
 * Since: 2.2
 **/
void
hildon_pannable_area_scroll_to (HildonPannableArea *area,
				const gint x, const gint y)
{
  HildonPannableAreaPrivate *priv;
  gint width, height;
  gint dist_x, dist_y;
  gboolean hscroll_visible, vscroll_visible;

  g_return_if_fail (GTK_WIDGET_REALIZED (area));
  g_return_if_fail (HILDON_IS_PANNABLE_AREA (area));

  priv = area->priv;

  vscroll_visible = (priv->vadjust->upper - priv->vadjust->lower >
	     priv->vadjust->page_size);
  hscroll_visible = (priv->hadjust->upper - priv->hadjust->lower >
	     priv->hadjust->page_size);

  if (((!vscroll_visible)&&(!hscroll_visible)) ||
      (x == -1 && y == -1)) {
    return;
  }

  if (priv->mode == HILDON_PANNABLE_AREA_MODE_PUSH)
    hildon_pannable_area_jump_to (area, x, y);

  width = priv->hadjust->upper - priv->hadjust->lower;
  height = priv->vadjust->upper - priv->vadjust->lower;

  g_return_if_fail (x < width || y < height);

  if ((x > -1)&&(hscroll_visible)) {
    priv->scroll_to_x = CLAMP (x - priv->hadjust->page_size/2,
                               priv->hadjust->lower,
                               priv->hadjust->upper - priv->hadjust->page_size);
    dist_x = priv->scroll_to_x - priv->hadjust->value;
    if (dist_x == 0) {
      priv->scroll_to_x = -1;
    } else {
      priv->vel_x = - dist_x/priv->vel_factor;
    }
  } else {
    priv->scroll_to_x = -1;
  }

  if ((y > -1)&&(vscroll_visible)) {
    priv->scroll_to_y = CLAMP (y - priv->vadjust->page_size/2,
                               priv->vadjust->lower,
                               priv->vadjust->upper - priv->vadjust->page_size);
    dist_y = priv->scroll_to_y - priv->vadjust->value;
    if (dist_y == 0) {
      priv->scroll_to_y = -1;
    } else {
      priv->vel_y = - dist_y/priv->vel_factor;
    }
  } else {
    priv->scroll_to_y = y;
  }

  if ((priv->scroll_to_y == -1) && (priv->scroll_to_x == -1)) {
    return;
  }

  hildon_pannable_area_launch_fade_timeout (area, 1.0);

  if (!priv->idle_id)
    priv->idle_id = gdk_threads_add_timeout_full (G_PRIORITY_HIGH_IDLE + 20,
						  (gint) (1000.0 / (gdouble) priv->sps),
						  (GSourceFunc) hildon_pannable_area_timeout,
						  area, NULL);
}

/**
 * hildon_pannable_area_jump_to:
 * @area: A #HildonPannableArea.
 * @x: The x coordinate of the destination point or -1 to ignore this axis.
 * @y: The y coordinate of the destination point or -1 to ignore this axis.
 *
 * Jumps the position of @area to ensure that (@x, @y) is a visible
 * point in the widget. In order to move in only one coordinate, you
 * must set the other one to -1. See hildon_pannable_area_scroll_to()
 * function for an example of how to calculate the position of
 * children in scrollable widgets like #GtkTreeview.
 *
 * There is a precondition to this function: the widget must be
 * already realized. Check the hildon_pannable_area_jump_to_child() for
 * more tips regarding how to call this function during
 * initialization.
 *
 * Since: 2.2
 **/
void
hildon_pannable_area_jump_to (HildonPannableArea *area,
                              const gint x, const gint y)
{
  HildonPannableAreaPrivate *priv;
  gint width, height;
  gdouble hv, vv;

  g_return_if_fail (HILDON_IS_PANNABLE_AREA (area));
  g_return_if_fail (GTK_WIDGET_REALIZED (area));
  g_return_if_fail (x >= -1 && y >= -1);

  if (x == -1 && y == -1) {
    return;
  }

  priv = area->priv;

  width = priv->hadjust->upper - priv->hadjust->lower;
  height = priv->vadjust->upper - priv->vadjust->lower;

  g_return_if_fail (x < width || y < height);

  hv = priv->hadjust->value;
  vv = priv->vadjust->value;

  if (x != -1) {
    gdouble jump_to = x - priv->hadjust->page_size/2;

    priv->hadjust->value = CLAMP (jump_to,
                                  priv->hadjust->lower,
                                  priv->hadjust->upper -
                                  priv->hadjust->page_size);
  }

  if (y != -1) {
    gdouble jump_to =  y - priv->vadjust->page_size/2;

    priv->vadjust->value = CLAMP (jump_to,
                                  priv->vadjust->lower,
                                  priv->vadjust->upper -
                                  priv->vadjust->page_size);
  }

  if (hv != priv->hadjust->value)
    gtk_adjustment_value_changed (priv->hadjust);

  if (vv != priv->vadjust->value)
    gtk_adjustment_value_changed (priv->vadjust);

  hildon_pannable_area_launch_fade_timeout (area, 1.0);

  if (priv->idle_id) {
    priv->vel_x = 0.0;
    priv->vel_y = 0.0;
    priv->overshooting_x = 0;
    priv->overshooting_y = 0;

    if ((priv->overshot_dist_x>0)||(priv->overshot_dist_y>0)) {
      priv->overshot_dist_x = 0;
      priv->overshot_dist_y = 0;

      gtk_widget_queue_resize (GTK_WIDGET (area));
    }

    g_signal_emit (area, pannable_area_signals[PANNING_FINISHED], 0);
    g_source_remove (priv->idle_id);
    priv->idle_id = 0;
  }
}

/**
 * hildon_pannable_area_scroll_to_child:
 * @area: A #HildonPannableArea.
 * @child: A #GtkWidget, descendant of @area.
 *
 * Smoothly scrolls until @child is visible inside @area. @child must
 * be a descendant of @area. If you need to scroll inside a scrollable
 * widget, e.g., #GtkTreeview, see hildon_pannable_area_scroll_to().
 *
 * There is a precondition to this function: the widget must be
 * already realized. Check the hildon_pannable_area_jump_to_child() for
 * more tips regarding how to call this function during
 * initialization.
 *
 * Since: 2.2
 **/
void
hildon_pannable_area_scroll_to_child (HildonPannableArea *area, GtkWidget *child)
{
  GtkWidget *bin_child;
  gint x, y;

  g_return_if_fail (GTK_WIDGET_REALIZED (area));
  g_return_if_fail (HILDON_IS_PANNABLE_AREA (area));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_is_ancestor (child, GTK_WIDGET (area)));

  if (GTK_BIN (area)->child == NULL)
    return;

  /* We need to get to check the child of the inside the area */
  bin_child = GTK_BIN (area)->child;

  /* we check if we added a viewport */
  if (GTK_IS_VIEWPORT (bin_child)) {
    bin_child = GTK_BIN (bin_child)->child;
  }

  if (gtk_widget_translate_coordinates (child, bin_child, 0, 0, &x, &y))
    hildon_pannable_area_scroll_to (area, x, y);
}

/**
 * hildon_pannable_area_jump_to_child:
 * @area: A #HildonPannableArea.
 * @child: A #GtkWidget, descendant of @area.
 *
 * Jumps to make sure @child is visible inside @area. @child must
 * be a descendant of @area. If you want to move inside a scrollable
 * widget, like, #GtkTreeview, see hildon_pannable_area_scroll_to().
 *
 * There is a precondition to this function: the widget must be
 * already realized. You can control if the widget is ready with the
 * GTK_WIDGET_REALIZED macro. If you want to call this function during
 * the initialization process of the widget do it inside a callback to
 * the ::realize signal, using g_signal_connect_after() function.
 *
 * Since: 2.2
 **/
void
hildon_pannable_area_jump_to_child (HildonPannableArea *area, GtkWidget *child)
{
  GtkWidget *bin_child;
  gint x, y;

  g_return_if_fail (GTK_WIDGET_REALIZED (area));
  g_return_if_fail (HILDON_IS_PANNABLE_AREA (area));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_is_ancestor (child, GTK_WIDGET (area)));

  if (gtk_bin_get_child (GTK_BIN (area)) == NULL)
    return;

  /* We need to get to check the child of the inside the area */
  bin_child = gtk_bin_get_child (GTK_BIN (area));

  /* we check if we added a viewport */
  if (GTK_IS_VIEWPORT (bin_child)) {
    bin_child = gtk_bin_get_child (GTK_BIN (bin_child));
  }

  if (gtk_widget_translate_coordinates (child, bin_child, 0, 0, &x, &y))
    hildon_pannable_area_jump_to (area, x, y);
}

/**
 * hildon_pannable_get_child_widget_at:
 * @area: A #HildonPannableArea.
 * @x: horizontal coordinate of the point
 * @y: vertical coordinate of the point
 *
 * Get the widget at the point (x, y) inside the pannable area. In
 * case no widget found it returns NULL.
 *
 * returns: the #GtkWidget if we find a widget, NULL in any other case
 *
 * Since: 2.2
 **/
GtkWidget*
hildon_pannable_get_child_widget_at (HildonPannableArea *area,
                                     gdouble x, gdouble y)
{
  GdkWindow *window = NULL;
  GtkWidget *child_widget = NULL;

  window = hildon_pannable_area_get_topmost
    (gtk_bin_get_child (GTK_BIN (area))->window,
     x, y, NULL, NULL, GDK_ALL_EVENTS_MASK);

  gdk_window_get_user_data (window, (gpointer) &child_widget);

  return child_widget;
}


/**
 * hildon_pannable_area_get_hadjustment:
 * @area: A #HildonPannableArea.
 *
 * Returns the horizontal adjustment. This adjustment is the internal
 * widget adjustment used to control the animations. Do not modify it
 * directly to change the position of the pannable, to do that use the
 * pannable API. If you modify the object directly it could cause
 * artifacts in the animations.
 *
 * returns: The horizontal #GtkAdjustment
 *
 * Since: 2.2
 **/
GtkAdjustment*
hildon_pannable_area_get_hadjustment            (HildonPannableArea *area)
{

  g_return_val_if_fail (HILDON_IS_PANNABLE_AREA (area), NULL);

  return area->priv->hadjust;
}

/**
 * hildon_pannable_area_get_vadjustment:
 * @area: A #HildonPannableArea.
 *
 * Returns the vertical adjustment. This adjustment is the internal
 * widget adjustment used to control the animations. Do not modify it
 * directly to change the position of the pannable, to do that use the
 * pannable API. If you modify the object directly it could cause
 * artifacts in the animations.
 *
 * returns: The vertical #GtkAdjustment
 *
 * Since: 2.2
 **/
GtkAdjustment*
hildon_pannable_area_get_vadjustment            (HildonPannableArea *area)
{
  g_return_val_if_fail (HILDON_IS_PANNABLE_AREA (area), NULL);

  return area->priv->vadjust;
}


/**
 * hildon_pannable_area_get_size_request_policy:
 * @area: A #HildonPannableArea.
 *
 * This function returns the current size request policy of the
 * widget. That policy controls the way the size_request is done in
 * the pannable area. Check
 * hildon_pannable_area_set_size_request_policy() for a more detailed
 * explanation.
 *
 * returns: the policy is currently being used in the widget
 * #HildonSizeRequestPolicy.
 *
 * Since: 2.2
 **/
HildonSizeRequestPolicy
hildon_pannable_area_get_size_request_policy (HildonPannableArea *area)
{
  HildonPannableAreaPrivate *priv;

  g_return_val_if_fail (HILDON_IS_PANNABLE_AREA (area), FALSE);

  priv = area->priv;

  return priv->size_request_policy;
}

/**
 * hildon_pannable_area_set_size_request_policy:
 * @area: A #HildonPannableArea.
 * @size_request_policy: One of the allowed #HildonSizeRequestPolicy
 *
 * This function sets the pannable area size request policy. That
 * policy controls the way the size_request is done in the pannable
 * area. Pannable can use the size request of its children
 * (#HILDON_SIZE_REQUEST_CHILDREN) or the minimum size required for
 * the area itself (#HILDON_SIZE_REQUEST_MINIMUM), the latter is the
 * default. Recall this size depends on the scrolling policy you are
 * requesting to the pannable area, if you set #GTK_POLICY_NEVER this
 * parameter will not have any effect with
 * #HILDON_SIZE_REQUEST_MINIMUM set.
 *
 * Since: 2.2
 *
 * Deprecated: This method and the policy request is deprecated, DO
 * NOT use it in future code, the only policy properly supported in
 * gtk+ nowadays is the minimum size. Use #gtk_window_set_default_size
 * or #gtk_window_set_geometry_hints with the proper size in your case
 * to define the height of your dialogs.
 **/
void
hildon_pannable_area_set_size_request_policy (HildonPannableArea *area,
                                              HildonSizeRequestPolicy size_request_policy)
{
  HildonPannableAreaPrivate *priv;

  g_return_if_fail (HILDON_IS_PANNABLE_AREA (area));

  priv = area->priv;

  if (priv->size_request_policy == size_request_policy)
    return;

  priv->size_request_policy = size_request_policy;

  gtk_widget_queue_resize (GTK_WIDGET (area));

  g_object_notify (G_OBJECT (area), "size-request-policy");
}

/**
 * hildon_pannable_area_get_center_on_child_focus
 * @area: A #HildonPannableArea
 *
 * Gets the @area #HildonPannableArea:center-on-child-focus property
 * value.
 *
 * See #HildonPannableArea:center-on-child-focus for more information.
 *
 * Returns: the @area #HildonPannableArea:center-on-child-focus value
 *
 * Since: 2.2
 **/
gboolean
hildon_pannable_area_get_center_on_child_focus  (HildonPannableArea *area)
{
  g_return_val_if_fail (HILDON_IS_PANNABLE_AREA (area), FALSE);

  return area->priv->center_on_child_focus;
}

/**
 * hildon_pannable_area_set_center_on_child_focus
 * @area: A #HildonPannableArea
 * @value: the new value
 *
 * Sets the @area #HildonPannableArea:center-on-child-focus property
 * to @value.
 *
 * See #HildonPannableArea:center-on-child-focus for more information.
 *
 * Since: 2.2
 **/
void
hildon_pannable_area_set_center_on_child_focus  (HildonPannableArea *area,
                                                 gboolean value)
{
  g_return_if_fail (HILDON_IS_PANNABLE_AREA (area));

  area->priv->center_on_child_focus = value;
}
