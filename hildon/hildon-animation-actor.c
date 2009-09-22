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

/**
 * SECTION:hildon-animation-window
 * @short_description: Animation actor for
 * WM-assisted animation effects in the Hildon framework.
 *
 * The #HildonAnimationActor is an
 * animation actor for WM-assisted animation effects in the Hildon
 * framework. It derives from #GtkWindow and can host any widgets much
 * like a normal window. The special features available to the
 * animation actor is the ability to set its position, scale factor
 * and rotation. These parameters are interpreted by Hildon's
 * compositing window manager to alter the on-screen representation of
 * the animation actor window. Bear in mind, however, that by design
 * decision, animation actors are not reactive -- the widgets placed
 * in such window will not receive keyboard, motion or button
 * events. Animation actors are input-transparent -- the input events
 * will be sent to the underlying real windows and widgets.
 *
 * The animation actors may exist in a parented or an unparented
 * state. To be displayed, animations actors must be parented to
 * another top-level window widget. Animation actors display on top
 * (in front) of the standard window contents unless the position
 * (depth, z-coordinate) is specifically adjusted. Animation actors in
 * an unparented state do not display at all.
 *
 * Parented animation actors are placed in the coordinate space of the
 * parent window and visually become a part of the parent window
 * iteslf -- they inherit the transformations and effects enforced by
 * the window manager on the parent window (task switcher animations,
 * minimize events, etc.).
 *
 * All animation actor settings (position, scale, rotation, opacity,
 * depth) can be applied to unparented actors, but will only take
 * effect as the actor is parented to a top-level window and that
 * window is shown. All settings are preserved during
 * unparenting/reparenting.
 *
 * The #HildonAnimationActor API closely follows the #ClutterActor
 * API.  Please take a look at the #ClutterActor description for the
 * modelview transformations order that applies to
 * HildonAnimationActor and ClutterActor alike.
 *
 * Animation actor widget controls the animation actor as it is
 * transformed by the window manager using ClientMessage X11
 * events. It tries to minimize the amount of such events and couples
 * conceptually related parameters (visibility and opacity, position
 * and depth) to the same message.  The API, however, offers
 * convenience functions for the programmer to be able to modify every
 * parameter individually.
 *
 * <example>
 * <title>Basic HildonAnimationActor example</title>
 * <programlisting>
 * static void
 * animation_cb (void *obj)
 * {
 *     HildonAnimationActor *actor = HILDON_ANIMATION_ACTOR (obj);
 * <!-- -->
 *     static int x_inc = 1;
 *     static int y_inc = 1;
 *     static int x = 0;
 *     static int y = 0;
 *     static int r = 0;
 * <!-- -->
 *     if (((x_inc > 0) && (x > 800)) ||
 *         ((x_inc < 0) && (x < 1)))
 *         x_inc = -x_inc;
 *     if (((y_inc > 0) && (y > 480)) ||
 *         ((y_inc < 0) && (y < 1)))
 *         y_inc = -y_inc;
 * <!-- -->
 *     x += x_inc;
 *     y += y_inc;
 *     r ++;
 * <!-- -->
 *     // Set animation actor position and rotation
 *     hildon_animation_actor_set_position (actor, x, y);
 *     hildon_animation_actor_set_rotation (actor,
 *                                          HILDON_AA_Z_AXIS,
 *                                          r,
 *                                          0, 0, 0);
 * }
 * <!-- -->
 * int
 * main (int argc, char **argv)
 * {
 *     GtkWidget *win;
 *     GtkWidget *image;
 *     GtkWidget *actor;
 * <!-- -->
 *     gtk_init (&amp;argc, &amp;argv);
 * <!-- -->
 *     // ... set up a normal window
 *     win = hildon_window_new ();
 *     g_signal_connect (win, "destroy", G_CALLBACK (gtk_main_quit), NULL);
 *     gtk_widget_show_all (win);
 * <!-- -->
 *     // ... load an image
 *     image = gtk_image_new_from_file ("image.jpg");
 * <!-- -->
 *     actor = hildon_animation_actor_new();
 *     gtk_container_add (GTK_CONTAINER (actor), image);
 * <!-- -->
 *     // Parent the animation actor
 *     hildon_animation_actor_set_parent (HILDON_ANIMATION_ACTOR (actor), win);
 * <!-- -->
 *     // Set anchor point to the actor center
 *     hildon_animation_actor_set_anchor_from_gravity (HILDON_ANIMATION_ACTOR (actor),
						       HILDON_AA_CENTER_GRAVITY);
 * <!-- -->
 *     gtk_widget_show_all (actor);
 * <!-- -->
 *     // Set up animation
 *     g_timeout_add (100, (GSourceFunc)animation_cb, actor);
 * <!-- -->
 *     gtk_main ();
 * <!-- -->
 *     return 0;
 * }
 * </programlisting>
 * </example>
 */

#include                                        <gdk/gdkx.h>
#include                                        <X11/Xatom.h>

#include                                        "hildon-animation-actor.h"
#include                                        "hildon-animation-actor-private.h"

G_DEFINE_TYPE (HildonAnimationActor, hildon_animation_actor, GTK_TYPE_WINDOW);

static GdkFilterReturn
hildon_animation_actor_event_filter (GdkXEvent *xevent,
                                     GdkEvent *event,
                                     gpointer data);
static void
hildon_animation_actor_update_ready (HildonAnimationActor *self);
static void
hildon_animation_actor_send_pending_messages (HildonAnimationActor *self);
static void
hildon_animation_actor_send_all_messages (HildonAnimationActor *self);
static gboolean
hildon_animation_actor_parent_map_event (GtkWidget *parent,
					 GdkEvent *event,
					 gpointer user_data);
static gboolean
hildon_animation_actor_map_event (GtkWidget *widget,
				  GdkEvent *event,
				  gpointer user_data);

static guint32 show_atom;
static guint32 position_atom;
static guint32 rotation_atom;
static guint32 scale_atom;
static guint32 anchor_atom;
static guint32 parent_atom;
static guint32 ready_atom;

static gboolean atoms_initialized = FALSE;

static void
hildon_animation_actor_realize                 (GtkWidget *widget)
{
    GdkDisplay *display;
    Atom wm_type, applet_type;

    GTK_WIDGET_CLASS (hildon_animation_actor_parent_class)->realize (widget);

    /* Set animation actor window type. */

    display = gdk_drawable_get_display (widget->window);

    wm_type = gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_WINDOW_TYPE");
    applet_type = gdk_x11_get_xatom_by_name_for_display (display, "_HILDON_WM_WINDOW_TYPE_ANIMATION_ACTOR");

    XChangeProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (widget->window), wm_type,
                     XA_ATOM, 32, PropModeReplace,
                     (unsigned char *) &applet_type, 1);

    /* This is a bit of a hack, but for the sake of speed (it is assumed that
     * once HildonAnimationActor is created, a lot of ClientMessages will
     * follow), we cache all ClientMessages atoms in static variables. */

    if (!atoms_initialized)
    {
	show_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_ANIMATION_CLIENT_MESSAGE_SHOW");
	position_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_ANIMATION_CLIENT_MESSAGE_POSITION");
	rotation_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_ANIMATION_CLIENT_MESSAGE_ROTATION");
	scale_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_ANIMATION_CLIENT_MESSAGE_SCALE");
	anchor_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_ANIMATION_CLIENT_MESSAGE_ANCHOR");
	parent_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_ANIMATION_CLIENT_MESSAGE_PARENT");
	ready_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_ANIMATION_CLIENT_READY");
#if 0
	g_debug ("show atom = %lu\n", show_atom);
	g_debug ("position atom = %lu\n", position_atom);
	g_debug ("rotation atom = %lu\n", rotation_atom);
	g_debug ("scale atom = %lu\n", scale_atom);
	g_debug ("anchor atom = %lu\n", anchor_atom);
	g_debug ("parent atom = %lu\n", parent_atom);
	g_debug ("ready atom = %lu\n", ready_atom);
#endif

	atoms_initialized = TRUE;
    }

    /* Wait for a ready message */

    gdk_window_add_filter (widget->window,
			   hildon_animation_actor_event_filter,
			   widget);
}

static void
hildon_animation_actor_unrealize               (GtkWidget *widget)
{
    gdk_window_remove_filter (widget->window,
			      hildon_animation_actor_event_filter,
			      widget);

    GTK_WIDGET_CLASS (hildon_animation_actor_parent_class)->unrealize (widget);
}

static void
hildon_animation_actor_show                    (GtkWidget *widget)
{
    HildonAnimationActor        *self = HILDON_ANIMATION_ACTOR (widget);

    GTK_WIDGET_CLASS (hildon_animation_actor_parent_class)->show (widget);
    hildon_animation_actor_set_show (self, TRUE);
}

static void
hildon_animation_actor_hide                    (GtkWidget *widget)
{
    HildonAnimationActor        *self = HILDON_ANIMATION_ACTOR (widget);

    hildon_animation_actor_set_show (self, FALSE);
    GTK_WIDGET_CLASS (hildon_animation_actor_parent_class)->hide (widget);
}

static void
hildon_animation_actor_finalize                (GObject *object)
{
    HildonAnimationActor        *self = HILDON_ANIMATION_ACTOR (object);
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);

    if (priv->parent)
    {
	if (priv->parent_map_event_cb_id)
	    g_signal_handler_disconnect (priv->parent,
					 priv->parent_map_event_cb_id);

	g_object_unref (priv->parent);
    }
}

static void
hildon_animation_actor_class_init              (HildonAnimationActorClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass    *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->finalize         = hildon_animation_actor_finalize;

    widget_class->realize           = hildon_animation_actor_realize;
    widget_class->unrealize         = hildon_animation_actor_unrealize;
    widget_class->show              = hildon_animation_actor_show;
    widget_class->hide              = hildon_animation_actor_hide;

    g_type_class_add_private (klass, sizeof (HildonAnimationActorPrivate));
}

static void
hildon_animation_actor_init                    (HildonAnimationActor *self)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);

    /* Default non-zero values for the private variables */

    priv->scale_x = 1 << 16;
    priv->scale_y = 1 << 16;
    priv->opacity = 0xff;
}

/**
 * hildon_animation_actor_new:
 *
 * Creates a new #HildonAnimationActor.
 *
 * Return value: A #HildonAnimationActor
 *
 * Since: 2.2
 **/
GtkWidget*
hildon_animation_actor_new                     (void)
{
    HildonAnimationActor *newwindow = g_object_new (HILDON_TYPE_ANIMATION_ACTOR, NULL);

    gtk_window_set_decorated (GTK_WINDOW (newwindow), FALSE);

    return GTK_WIDGET (newwindow);
}

/*
 * An filter for GDK X11 events, waiting for PropertyNotify (window property
 * changes) events, keeping track of animation actor ready atom.
 * Having the ready atom set on the window by the window manager will trigger
 * updates of actor parameters (position/rotation/etc...) to be sent off
 * to the window manager for processing.
 */
static GdkFilterReturn
hildon_animation_actor_event_filter             (GdkXEvent *xevent,
						 GdkEvent *event,
						 gpointer data)
{
    HildonAnimationActor *self = HILDON_ANIMATION_ACTOR (data);
    XAnyEvent *any = xevent;

    if (any->type == PropertyNotify)
    {
	XPropertyEvent *property = xevent;

	if (property->atom == ready_atom)
	{
	    hildon_animation_actor_update_ready (self);
	}
    }

    return GDK_FILTER_CONTINUE;
}

/*
 * Check for the ready atom on the animation actor X11 window.
 * If present, send all pending animation actor messages to the
 * window manager.
 */
static void
hildon_animation_actor_update_ready (HildonAnimationActor *self)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);
    Display            *display = GDK_WINDOW_XDISPLAY (widget->window);
    Window              window = GDK_WINDOW_XID (widget->window);

    int status;
    gint xerror;

    Atom actual_type;
    int  actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;

    /* Check for the "ready" property */

    gdk_error_trap_push ();
    status = XGetWindowProperty (display, window,
				 ready_atom, 0, 32,
				 False, XA_ATOM,
				 &actual_type, &actual_format,
				 &nitems, &bytes_after, &prop);
    xerror = gdk_error_trap_pop();

    if (prop)
    {
	/* We do not actually use the property value for anything,
	 * it is enough that the property is set. */

	XFree (prop);
    }

    if (xerror ||
	(status != Success) || (actual_type != XA_ATOM) ||
       	(actual_format != 32) || (nitems != 1))
    {
	priv->ready = 0;
	return;
    }

    if (priv->ready)
    {
	/* The ready flag has been set once already. This means that
	 * the WM has restarted. Trigger re-mapping of the widget to
	 * update the texture actor first. Then push all animation
	 * actor settings anew. */

	priv->map_event_cb_id =
	    g_signal_connect (G_OBJECT (self),
			      "map-event",
			      G_CALLBACK(hildon_animation_actor_map_event),
			      self);

	if (GTK_WIDGET_MAPPED (GTK_WIDGET (self)))
	{
	    gtk_widget_unmap (GTK_WIDGET (self));
	    gtk_widget_map (GTK_WIDGET (self));
	}

	return;
    }

    priv->ready = 1;

    /* Send all pending messages */

    hildon_animation_actor_send_pending_messages (self);
}

static void
hildon_animation_actor_send_pending_messages (HildonAnimationActor *self)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);

    if (priv->set_anchor)
    {
	if (priv->gravity == 0)
	    hildon_animation_actor_set_anchor (self,
					       priv->anchor_x,
					       priv->anchor_y);
	else
	    hildon_animation_actor_set_anchor_from_gravity (self,
							    priv->gravity);
    }

    if (priv->set_position)
	hildon_animation_actor_set_position_full (self,
						  priv->position_x,
						  priv->position_y,
						  priv->depth);

    if (priv->set_rotation & (1 << HILDON_AA_X_AXIS))
	hildon_animation_actor_set_rotationx (self,
					      HILDON_AA_X_AXIS,
					      priv->x_rotation_angle,
					      0,
					      priv->x_rotation_y,
					      priv->x_rotation_z);

    if (priv->set_rotation & (1 << HILDON_AA_Y_AXIS))
	hildon_animation_actor_set_rotationx (self,
					      HILDON_AA_Y_AXIS,
					      priv->y_rotation_angle,
					      priv->y_rotation_x,
					      0,
					      priv->y_rotation_z);

    if (priv->set_rotation & (1 << HILDON_AA_Z_AXIS))
	hildon_animation_actor_set_rotationx (self,
					      HILDON_AA_Z_AXIS,
					      priv->z_rotation_angle,
					      priv->z_rotation_x,
					      priv->z_rotation_y,
					      0);

    if (priv->set_scale)
	hildon_animation_actor_set_scalex (self,
					   priv->scale_x,
					   priv->scale_y);

    if (priv->set_parent)
	hildon_animation_actor_set_parent (self,
					   priv->parent);

    if (priv->set_show)
	hildon_animation_actor_set_show_full (self,
					      priv->show, priv->opacity);
}

static void
hildon_animation_actor_send_all_messages (HildonAnimationActor *self)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);

    priv->set_anchor = 1;
    priv->set_position = 1;
    priv->set_rotation = (1 << HILDON_AA_X_AXIS) |
	                 (1 << HILDON_AA_Y_AXIS) |
	                 (1 << HILDON_AA_Z_AXIS);
    priv->set_scale = 1;
    priv->set_parent = 1;
    priv->set_show = 1;

    hildon_animation_actor_send_pending_messages (self);
}

/* ------------------------------------------------------------- */

/**
 * hildon_animation_actor_send_message:
 * @self: A #HildonAnimationActor
 * @message_type: Message id for the animation actor message.
 * @l0: 1st animation actor message parameter.
 * @l1: 2nd animation actor message parameter.
 * @l2: 3rd animation actor message parameter.
 * @l3: 4th animation actor message parameter.
 * @l4: 5th animation actor message parameter.
 *
 * Sends an X11 ClientMessage event to the window manager with
 * the specified parameters -- id (@message_type) and data (@l0,
 * @l1, @l2, @l3, @l4).
 *
 * This is an internal utility function that application will
 * not need to call directly.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_send_message (HildonAnimationActor *self,
                                     guint32 message_type,
                                     guint32 l0,
                                     guint32 l1,
                                     guint32 l2,
                                     guint32 l3,
                                     guint32 l4)
{
    GtkWidget          *widget = GTK_WIDGET (self);
    Display            *display = GDK_WINDOW_XDISPLAY (widget->window);
    Window              window = GDK_WINDOW_XID (widget->window);
#if 0
    XClientMessageEvent event;

    event.type = ClientMessage;
    event.window = window;
    event.message_type = (Atom)message_type;
    event.format = 32;
    event.data.l[0] = l0;
    event.data.l[1] = l1;
    event.data.l[2] = l2;
    event.data.l[3] = l3;
    event.data.l[4] = l4;
#else
    XEvent event = { 0 };

    event.xclient.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = (Atom)message_type;
    event.xclient.format = 32;
    event.xclient.data.l[0] = l0;
    event.xclient.data.l[1] = l1;
    event.xclient.data.l[2] = l2;
    event.xclient.data.l[3] = l3;
    event.xclient.data.l[4] = l4;
#endif

#if 0
    g_debug ("%lu (%lu %lu %lu %lu %lu) -> %lu\n",
             message_type,
             l0, l1, l2, l3, l4,
             window);
#endif

    XSendEvent (display, window, True,
                StructureNotifyMask,
                (XEvent *)&event);
}

/**
 * hildon_animation_actor_set_show_full:
 * @self: A #HildonAnimationActor
 * @show: A boolean flag setting the visibility of the animation actor.
 * @opacity: Desired opacity setting
 *
 * Send a message to the window manager setting the visibility of
 * the animation actor. This will only affect the visibility of
 * the animation actor set by the compositing window manager in its own
 * rendering pipeline, after X has drawn the window to the off-screen
 * buffer. This setting, naturally, has no effect if the #HildonAnimationActor
 * widget is not visible in X11 terms (i.e. realized and mapped).
 *
 * Furthermore, if a widget is parented, its final visibility will be
 * affected by that of the parent window.
 *
 * The opacity setting ranges from zero (0), being completely transparent
 * to 255 (0xff) being fully opaque.
 *
 * If the animation actor WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_show_full (HildonAnimationActor *self,
				      gboolean show,
				      gint opacity)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    if (opacity > 255)
	opacity = 255;

    if (opacity < 0)
	opacity = 0;

    priv->show = show;
    priv->opacity = opacity;
    priv->set_show = 1;

    if (GTK_WIDGET_MAPPED (widget) && priv->ready)
    {
	/* Defer show messages until the animation actor is parented
	 * and the parent window is mapped */

	if (!priv->parent || !GTK_WIDGET_MAPPED (GTK_WIDGET (priv->parent)))
	    return;

	hildon_animation_actor_send_message (self,
					     show_atom,
					     show, opacity,
					     0, 0, 0);
	priv->set_show = 0;
    }
}

/**
 * hildon_animation_actor_set_show:
 * @self: A #HildonAnimationActor
 * @show: A boolean flag setting the visibility of the animation actor.
 *
 * This function is a shortcut for hildon_animation_actor_set_show_full(),
 * setting the overall actor visibility without changing it's opacity
 * setting.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_show (HildonAnimationActor *self,
				 gboolean show)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);

    hildon_animation_actor_set_show_full (self,
					  show, priv->opacity);
}

/**
 * hildon_animation_actor_set_opacity:
 * @self: A #HildonAnimationActor
 * @opacity: Desired opacity setting
 *
 * This function is a shortcut for hildon_animation_actor_set_show_full(),
 * setting actor opacity without changing it's overall visibility.
 *
 * See hildon_animation_actor_set_show_full() for description of the range
 * of values @opacity argument takes.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_opacity (HildonAnimationActor *self,
				    gint opacity)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);

    hildon_animation_actor_set_show_full (self,
					  priv->show, opacity);
}

/**
 * hildon_animation_actor_set_position_full:
 * @self: A #HildonAnimationActor
 * @x: Desired X coordinate
 * @y: Desired Y coordinate
 * @depth: Desired window depth (Z coordinate)
 *
 * Send a message to the window manager setting the position of the
 * animation actor. This will set the position of the animation
 * actor off-screen bitmap as it is rendered to the screen. The
 * position of the actor is relative to the parent window. The actor
 * is also subject to the animation effects rendered by the compositing
 * window manager on that window (like those by task switcher).
 *
 * The window depth affects the stacking of animation actors within
 * a parent window and, more generally, the stacking of clutter actors
 * within a stage/container. The default depth is 0 and a parent
 * window's container will have it's window texture stacked at that level.
 * The stacking at any depth level is sequential -- animation actor B
 * created/parented after animation actor A will obscure the latter
 * if they overlap.
 *
 * Animation actors with non-zero depth settings are subject to scaling as
 * per the global scene perspective setup, which limits the depth setting
 * as the primary parameter to control the stacking order. Since the
 * stacking order follows the parenting order, it may be better to use
 * hildon_animation_actor_set_parent() for setting the stacking.
 *
 * If the animation actor WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_position_full (HildonAnimationActor *self,
                                          gint x,
                                          gint y,
					  gint depth)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    priv->position_x = x;
    priv->position_y = y;
    priv->depth = depth;
    priv->set_position = 1;

    if (GTK_WIDGET_MAPPED (widget) && priv->ready)
    {
	hildon_animation_actor_send_message (self,
					     position_atom,
					     x, y, depth,
					     0, 0);
	priv->set_position = 0;
    }
}

/**
 * hildon_animation_actor_set_position:
 * @self: A #HildonAnimationActor
 * @x: Desired window X coordinate
 * @y: Desired window Y coordinate
 *
 * A shortcut for hildon_animation_actor_set_position_full(),
 * changing the window position, but preserving it's depth setting.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_position (HildonAnimationActor *self,
                                     gint x,
                                     gint y)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);

    hildon_animation_actor_set_position_full (self,
					      x, y, priv->depth);
}

/**
 * hildon_animation_actor_set_depth:
 * @self: A #HildonAnimationActor
 * @depth: Desired window depth (Z coordinate)
 *
 * A shortcut for hildon_animation_actor_set_position_full(),
 * changing the window depth, but preserving it's position.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_depth (HildonAnimationActor *self,
				  gint depth)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);

    hildon_animation_actor_set_position_full (self,
					      priv->position_x,
					      priv->position_y,
					      depth);
}

/**
 * hildon_animation_actor_set_scalex:
 * @self: A #HildonAnimationActor
 * @x_scale: Window's desired scale factor along the X-axis
 * @y_scale: Window's desired scale factor along the Y-axis
 *
 * This function is just like hildon_animation_actor_set_scale(),
 * but the scale factors are given as 16-bit fixed-point number.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_scalex (HildonAnimationActor *self,
                                   gint32 x_scale,
                                   gint32 y_scale)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    priv->scale_x = x_scale;
    priv->scale_y = y_scale;
    priv->set_scale = 1;

    if (GTK_WIDGET_MAPPED (widget) && priv->ready)
    {
	hildon_animation_actor_send_message (self,
					     scale_atom,
					     x_scale, y_scale,
					     0, 0, 0);
	priv->set_scale = 0;
    }
}

/**
 * hildon_animation_actor_set_scale:
 * @self: A #HildonAnimationActor
 * @x_scale: Window's desired scale factor along the X-axis
 * @y_scale: Window's desired scale factor along the Y-axis
 *
 * Send a message to the window manager setting the scale factors of the
 * animation actor. This will set the scale factors on the animation
 * actor off-screen bitmap as it is rendered to the screen. If the
 * animation actor is parented to another top-level window, the
 * animation effects rendered by the compositing window manager
 * on that top-level window (like those by task switcher) will
 * also affect the animation actor.
 *
 * If the animation actor WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_scale (HildonAnimationActor *self,
                                  double x_scale,
                                  double y_scale)
{
    gint32 f_x_scale = x_scale * (1 << 16);
    gint32 f_y_scale = y_scale * (1 << 16);

    hildon_animation_actor_set_scalex (self, f_x_scale, f_y_scale);
}

/**
 * hildon_animation_actor_set_rotationx:
 * @self: A #HildonAnimationActor
 * @axis: The rotation axis.
 * @degrees: The rotation angle in degrees.
 * @x: Center of the rotation, X coordinate.
 * @y: Center of the rotation, Y coordinate.
 * @z: Center of the rotation, Z coordinate.
 *
 * This function is just like hildon_animation_actor_set_rotation(),
 * but the rotation angle is given as 16-bit fixed-point number.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_rotationx (HildonAnimationActor *self,
                                      gint   axis,
                                      gint32 degrees,
                                      gint   x,
                                      gint   y,
                                      gint   z)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    guint mask = 0;

    switch (axis)
    {
	case HILDON_AA_X_AXIS:
	    priv->x_rotation_angle = degrees;
	    priv->x_rotation_y = y;
	    priv->x_rotation_z = z;
	    mask = (1 << HILDON_AA_X_AXIS);
	    break;
	case HILDON_AA_Y_AXIS:
	    priv->y_rotation_angle = degrees;
	    priv->y_rotation_x = x;
	    priv->y_rotation_z = z;
	    mask = (1 << HILDON_AA_Y_AXIS);
	    break;
	case HILDON_AA_Z_AXIS:
	    priv->z_rotation_angle = degrees;
	    priv->z_rotation_x = x;
	    priv->z_rotation_y = y;
	    mask = (1 << HILDON_AA_Z_AXIS);
	    break;
	default:
	    return;
    }

    priv->set_rotation |= mask;

    if (GTK_WIDGET_MAPPED (widget) && priv->ready)
    {
	hildon_animation_actor_send_message (self,
					     rotation_atom,
					     axis, degrees, x, y, z);
	priv->set_rotation &= ~mask;
    }
}

/**
 * hildon_animation_actor_set_rotation:
 * @self: A #HildonAnimationActor
 * @axis: The rotation axis.
 * @degrees: The rotation angle in degrees.
 * @x: Center of the rotation, X coordinate.
 * @y: Center of the rotation, Y coordinate.
 * @z: Center of the rotation, Z coordinate.
 *
 * Send a message to the window manager setting the animation actor
 * rotation around one of the three axes. The rotation center coordinates
 * depend on the axis of rotation:
 *
 *   * %HILDON_AA_X_AXIS requires @y and @z coordinates.
 *   * %HILDON_AA_Y_AXIS requires @x and @z coordinates.
 *   * %HILDON_AA_Z_AXIS requires @x and @y coordinates.
 *
 * If the animation actor WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_rotation (HildonAnimationActor *self,
                                     gint   axis,
                                     double degrees,
                                     gint   x,
                                     gint   y,
                                     gint   z)
{
    gint32 f_degrees = degrees * (1 << 16);

    hildon_animation_actor_set_rotationx (self, axis, f_degrees, x, y, z);
}

/**
 * hildon_animation_actor_set_anchor:
 * @self: A #HildonAnimationActor
 * @x: The X coordinate of the anchor point.
 * @y: The Y coordinate of the anchor point.
 *
 * Send a message to the window manager setting the anchor point for
 * the animation actor. The anchor point is the point to which the
 * actor position within its parent it is relative.
 *
 * If the animation actor WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_anchor (HildonAnimationActor *self,
                                   gint x,
                                   gint y)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    priv->gravity = 0;
    priv->anchor_x = x;
    priv->anchor_y = y;
    priv->set_anchor = 1;

    if (GTK_WIDGET_MAPPED (widget) && priv->ready)
    {
	hildon_animation_actor_send_message (self,
					     anchor_atom,
					     0, x, y,
					     0, 0);
	priv->set_anchor = 0;
    }
}

/**
 * hildon_animation_actor_set_anchor_from_gravity:
 * @self: A #HildonAnimationActor
 * @gravity: The gravity constant.
 *
 * Send a message to the window manager setting the anchor point for
 * the animation actor. The anchor point is the point to which the
 * actor position within its parent it is relative. Instead of being
 * defined in (x, y)-coordinates, the anchor point is defined in the
 * relative "gravity" constant as:
 *
 *   * %HILDON_AA_N_GRAVITY translates to (width / 2, 0) coordinate
 *   * %HILDON_AA_NE_GRAVITY translates to (width, 0) coordinate
 *   * %HILDON_AA_E_GRAVITY translates to (width, height / 2) coordinate
 *   * %HILDON_AA_SE_GRAVITY translates to (width, height) coordinate
 *   * %HILDON_AA_S_GRAVITY translates to (width / 2, height) coordinate
 *   * %HILDON_AA_SW_GRAVITY translates to (0, height) coordinate
 *   * %HILDON_AA_W_GRAVITY translates to (0, height / 2) coordinate
 *   * %HILDON_AA_NW_GRAVITY translates to (0, 0) coordinate
 *   * %HILDON_AA_CENTER_GRAVITY translates to (width / 2, height / 2) coordinate
 *
 * If the animation actor WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_anchor_from_gravity (HildonAnimationActor *self,
						guint gravity)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    priv->gravity = gravity;
    priv->set_anchor = 1;

    if (GTK_WIDGET_MAPPED (widget) && priv->ready)
    {
	hildon_animation_actor_send_message (self,
					     anchor_atom,
					     gravity, 0, 0,
					     0, 0);
	priv->set_anchor = 0;
    }
}

/*
 * This callback will be triggered by the parent widget of
 * an animation actor when it is mapped. The compositing
 * window manager is now ready to parent the animation actor
 * into the target parent window.
 */
static gboolean
hildon_animation_actor_parent_map_event (GtkWidget *parent,
					 GdkEvent *event,
					 gpointer user_data)
{
    hildon_animation_actor_set_parent (HILDON_ANIMATION_ACTOR (user_data),
				       GTK_WINDOW (parent));
    return FALSE;
}

/*
 * This callback will be triggered by the widget re-mapping
 * itself in case of WM restarting. The point is to push all
 * animation actor parameters anew to the WM.
 */
static gboolean
hildon_animation_actor_map_event (GtkWidget *widget,
				  GdkEvent *event,
				  gpointer user_data)
{
    HildonAnimationActor
	               *self = HILDON_ANIMATION_ACTOR (user_data);
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);

    hildon_animation_actor_send_all_messages (self);

    /* Disconnect the "map-event" handler after the "emergency resend all
     * actor parameters" drill is over. */

    if (priv->map_event_cb_id)
    {
	g_signal_handler_disconnect (self,
				     priv->map_event_cb_id);
	priv->map_event_cb_id = 0;
    }

    return FALSE;
}

/**
 * hildon_animation_actor_set_parent:
 * @self: A #HildonAnimationActor
 * @parent: A #GtkWindow that the actor will be parented to.
 *
 * Send a message to the window manager setting the parent window
 * for the animation actor. Parenting an actor will not affect the
 * X window that the HildonAnimationActor represents, but it's off-screen
 * bitmap as it is handled by the compositing window manager.
 *
 * Parenting an animation actor will affect its visibility as set
 * by the gtk_widget_show(), gtk_widget_hide() and
 * hildon_animation_actor_set_show(). The animation actor will only be
 * visible when the top-level window it is parented is visible.
 *
 * Passing %NULL as a @parent argument will unparent the animation actor.
 * This will restore the actor's visibility if it was suppressed by
 * being unparented or parented to an unmapped window.
 *
 * If the animation actor WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_animation_actor_set_parent (HildonAnimationActor *self,
				   GtkWindow *parent)
{
    HildonAnimationActorPrivate
	               *priv = HILDON_ANIMATION_ACTOR_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    gtk_window_set_transient_for (GTK_WINDOW (self), parent);

    if (priv->parent != parent)
    {
	/* Setting a new parent */

	if (priv->parent)
	{
	    if (priv->parent_map_event_cb_id)
		g_signal_handler_disconnect (priv->parent,
					     priv->parent_map_event_cb_id);

	    /* Might need a synchronized "parent(0)" or "parent(new parent)"
	     * message here before we can safely decrease the reference count. */

	    g_object_unref (priv->parent);
	}

	priv->parent = parent;
	priv->set_parent = 1;

	if (parent != 0)
	{
	    /* The widget is being (re)parented, not unparented. */

	    g_object_ref (parent);

	    priv->parent_map_event_cb_id =
		g_signal_connect (G_OBJECT (priv->parent),
				  "map-event",
				  G_CALLBACK(hildon_animation_actor_parent_map_event),
				  self);
	}
	else
	{
	    priv->parent_map_event_cb_id = 0;
	}
    }

    if (GTK_WIDGET_MAPPED (widget) && priv->ready)
    {
	Window win = 0;

	/* If the animation actor is being unparented or parented to an
	 * unmapped widget, force its visibility to "hidden". */

	if (!priv->parent || !GTK_WIDGET_MAPPED (GTK_WIDGET (priv->parent)))
	{
	    hildon_animation_actor_send_message (self,
						 show_atom,
						 0, priv->opacity,
						 0, 0, 0);
	}

	/* If the widget is being parented (parent != 0), only proceed when
	 * the parent widget is realized, since we need the X window id of
	 * the parent. If the widget is being unparented (parent == 0), pass
	 * the "special" window id of 0 in the message. */

	if (priv->parent)
	{
	    if (!GTK_WIDGET_MAPPED (GTK_WIDGET (priv->parent)))
		return;

	    GdkWindow *gdk = GTK_WIDGET (parent)->window;
	    win = GDK_WINDOW_XID (gdk);
	}

	hildon_animation_actor_send_message (self,
					     parent_atom,
					     win,
					     0, 0, 0, 0);
	priv->set_parent = 0;

	/* Set animation actor visibility to desired value (in case it was
	 * forced off when the actor was parented into an unmapped widget). */

	hildon_animation_actor_send_message (self,
					     show_atom,
					     priv->show, priv->opacity,
					     0, 0, 0);
	priv->set_show = 0;
    }
}

