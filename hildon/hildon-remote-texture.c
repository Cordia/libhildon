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
 * SECTION:hildon-remote-texture
 * @short_description: Clutter/GLES texture created
 * from a shared memory area.
 *
 * The #HildonRemoteTexture is a GTK+ widget which allows the rendering of
 * a shared memory area within hildon-desktop. It allows the memory area to
 * be positioned and scaled, without altering its' contents.
 */

#include                                        <gdk/gdkx.h>
#include                                        <X11/Xatom.h>

#include                                        "hildon-remote-texture.h"
#include                                        "hildon-remote-texture-private.h"

G_DEFINE_TYPE (HildonRemoteTexture, hildon_remote_texture, GTK_TYPE_WINDOW);

static GdkFilterReturn
hildon_remote_texture_event_filter (GdkXEvent *xevent,
                                     GdkEvent *event,
                                     gpointer data);
static void
hildon_remote_texture_update_ready (HildonRemoteTexture *self);
static void
hildon_remote_texture_send_pending_messages (HildonRemoteTexture *self);
static void
hildon_remote_texture_send_all_messages (HildonRemoteTexture *self);
static gboolean
hildon_remote_texture_parent_map_event (GtkWidget *parent,
					 GdkEvent *event,
					 gpointer user_data);
static gboolean
hildon_remote_texture_map_event (GtkWidget *widget,
				  GdkEvent *event,
				  gpointer user_data);

static guint32 shm_atom;
static guint32 damage_atom;
static guint32 show_atom;
static guint32 position_atom;
static guint32 offset_atom;
static guint32 scale_atom;
static guint32 parent_atom;
static guint32 ready_atom;

static gboolean atoms_initialized = FALSE;

static void
hildon_remote_texture_realize                 (GtkWidget *widget)
{
    GdkDisplay *display;
    Atom wm_type, applet_type;

    GTK_WIDGET_CLASS (hildon_remote_texture_parent_class)->realize (widget);

    /* Set remote texture window type. */

    display = gdk_window_get_display (gtk_widget_get_window (widget));

    wm_type = gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_WINDOW_TYPE");
    applet_type = gdk_x11_get_xatom_by_name_for_display (display, "_HILDON_WM_WINDOW_TYPE_REMOTE_TEXTURE");

    XChangeProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (gtk_widget_get_window (widget)), wm_type,
                     XA_ATOM, 32, PropModeReplace,
                     (unsigned char *) &applet_type, 1);

    /* This is a bit of a hack, but for the sake of speed (it is assumed that
     * once HildonRemoteTexture is created, a lot of ClientMessages will
     * follow), we cache all ClientMessages atoms in static variables. */

    if (!atoms_initialized)
    {
        shm_atom =
            gdk_x11_get_xatom_by_name_for_display
            (display, "_HILDON_TEXTURE_CLIENT_MESSAGE_SHM");
        damage_atom =
            gdk_x11_get_xatom_by_name_for_display
            (display, "_HILDON_TEXTURE_CLIENT_MESSAGE_DAMAGE");
	show_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_TEXTURE_CLIENT_MESSAGE_SHOW");
	position_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_TEXTURE_CLIENT_MESSAGE_POSITION");
	offset_atom =
            gdk_x11_get_xatom_by_name_for_display
            (display, "_HILDON_TEXTURE_CLIENT_MESSAGE_OFFSET");
	scale_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_TEXTURE_CLIENT_MESSAGE_SCALE");
	parent_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_TEXTURE_CLIENT_MESSAGE_PARENT");
	ready_atom =
	    gdk_x11_get_xatom_by_name_for_display
	    (display, "_HILDON_TEXTURE_CLIENT_READY");
#if 0
	g_debug ("shm atom = %lu\n", shm_atom);
	g_debug ("damage atom = %lu\n", damage_atom);
	g_debug ("show atom = %lu\n", show_atom);
	g_debug ("position atom = %lu\n", position_atom);
	g_debug ("offset atom = %lu\n", offset_atom);
	g_debug ("scale atom = %lu\n", scale_atom);
	g_debug ("parent atom = %lu\n", parent_atom);
	g_debug ("ready atom = %lu\n", ready_atom);
#endif

	atoms_initialized = TRUE;
    }

    /* Wait for a ready message */

    gdk_window_add_filter (gtk_widget_get_window (widget),
			   hildon_remote_texture_event_filter,
			   widget);
}

static void
hildon_remote_texture_unrealize               (GtkWidget *widget)
{
    gdk_window_remove_filter (gtk_widget_get_window (widget),
			      hildon_remote_texture_event_filter,
			      widget);

    GTK_WIDGET_CLASS (hildon_remote_texture_parent_class)->unrealize (widget);
}

static void
hildon_remote_texture_show                    (GtkWidget *widget)
{
    HildonRemoteTexture        *self = HILDON_REMOTE_TEXTURE (widget);

    GTK_WIDGET_CLASS (hildon_remote_texture_parent_class)->show (widget);
    hildon_remote_texture_set_show (self, 1);
}

static void
hildon_remote_texture_hide                    (GtkWidget *widget)
{
    HildonRemoteTexture        *self = HILDON_REMOTE_TEXTURE (widget);

    hildon_remote_texture_set_show (self, 0);
    GTK_WIDGET_CLASS (hildon_remote_texture_parent_class)->hide (widget);
}

static void
hildon_remote_texture_finalize                (GObject *object)
{
    HildonRemoteTexture        *self = HILDON_REMOTE_TEXTURE (object);
    HildonRemoteTexturePrivate
                       *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);

    if (priv->parent)
    {
        if (priv->parent_map_event_cb_id)
            g_signal_handler_disconnect (priv->parent,
                                         priv->parent_map_event_cb_id);

        g_object_unref (priv->parent);
    }
}

static void
hildon_remote_texture_class_init              (HildonRemoteTextureClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass    *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->finalize         = hildon_remote_texture_finalize;

    widget_class->realize           = hildon_remote_texture_realize;
    widget_class->unrealize         = hildon_remote_texture_unrealize;
    widget_class->show              = hildon_remote_texture_show;
    widget_class->hide              = hildon_remote_texture_hide;

    g_type_class_add_private (klass, sizeof (HildonRemoteTexturePrivate));
}

static void
hildon_remote_texture_init                    (HildonRemoteTexture *self)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);

    /* Default non-zero values for the private variables */

    priv->scale_x = 1;
    priv->scale_y = 1;
    priv->opacity = 0xff;
}

/**
 * hildon_remote_texture_new:
 *
 * Creates a new #HildonRemoteTexture.
 *
 * Return value: A #HildonRemoteTexture
 *
 * Since: 2.2
 **/
GtkWidget*
hildon_remote_texture_new                     (void)
{
    HildonRemoteTexture *newwindow = g_object_new (HILDON_TYPE_REMOTE_TEXTURE, NULL);

    gtk_window_set_decorated (GTK_WINDOW (newwindow), FALSE);

    return GTK_WIDGET (newwindow);
}

/*
 * An filter for GDK X11 events, waiting for PropertyNotify (window property
 * changes) events, keeping track of remote texture ready atom.
 * Having the ready atom set on the window by the window manager will trigger
 * updates of actor parameters (position/rotation/etc...) to be sent off
 * to the window manager for processing.
 */
static GdkFilterReturn
hildon_remote_texture_event_filter             (GdkXEvent *xevent,
						 GdkEvent *event,
						 gpointer data)
{
    HildonRemoteTexture *self = HILDON_REMOTE_TEXTURE (data);
    XAnyEvent *any = xevent;

    if (any->type == PropertyNotify)
    {
	XPropertyEvent *property = xevent;

	if (property->atom == ready_atom)
	{
	    hildon_remote_texture_update_ready (self);
	}
    }

    return GDK_FILTER_CONTINUE;
}

/*
 * Check for the ready atom on the remote texture X11 window.
 * If present, send all pending remote texture messages to the
 * window manager.
 */
static void
hildon_remote_texture_update_ready (HildonRemoteTexture *self)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);
    Display            *display = GDK_WINDOW_XDISPLAY (gtk_widget_get_window (widget));
    Window              window = GDK_WINDOW_XID (gtk_widget_get_window (widget));

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
	 * update the texture actor first. Then push all remote
	 * texture settings anew. */

	priv->map_event_cb_id =
	    g_signal_connect (G_OBJECT (self),
			      "map-event",
			      G_CALLBACK(hildon_remote_texture_map_event),
			      self);

	if (gtk_widget_get_mapped (GTK_WIDGET (self)))
	{
	    gtk_widget_unmap (GTK_WIDGET (self));
	    gtk_widget_map (GTK_WIDGET (self));
	}

	return;
    }

    priv->ready = 1;

    /* Send all pending messages */

    hildon_remote_texture_send_pending_messages (self);
}

static void
hildon_remote_texture_send_pending_messages (HildonRemoteTexture *self)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);

    if (priv->set_shm)
      hildon_remote_texture_set_image(self,
                                      priv->shm_key,
                                      priv->shm_width, priv->shm_height,
                                      priv->shm_bpp);

    if (priv->set_damage)
      hildon_remote_texture_update_area (self,
                                         priv->damage_x1,
                                         priv->damage_y1,
                                         priv->damage_x2 - priv->damage_x1,
                                         priv->damage_y2 - priv->damage_y1);

    if (priv->set_position)
	hildon_remote_texture_set_position (self,
                                            priv->x,
                                            priv->y,
                                            priv->width,
                                            priv->height);

    if (priv->set_offset)
        hildon_remote_texture_set_offset (self,
                                          priv->offset_x,
                                          priv->offset_y);

    if (priv->set_scale)
	hildon_remote_texture_set_scale (self,
					 priv->scale_x,
					 priv->scale_y);

    if (priv->set_parent)
	hildon_remote_texture_set_parent (self,
					   priv->parent);

    if (priv->set_show)
	hildon_remote_texture_set_show_full (self,
					      priv->show, priv->opacity);
}

static void
hildon_remote_texture_send_all_messages (HildonRemoteTexture *self)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);

    priv->set_shm = 1;
    priv->set_damage = 1;
    priv->set_position = 1;
    priv->set_scale = 1;
    priv->set_parent = 1;
    priv->set_show = 1;

    hildon_remote_texture_send_pending_messages (self);
}

/* ------------------------------------------------------------- */

/**
 * hildon_remote_texture_send_message:
 * @self: A #HildonRemoteTexture
 * @message_type: Message id for the remote texture message.
 * @l0: 1st remote texture message parameter.
 * @l1: 2nd remote texture message parameter.
 * @l2: 3rd remote texture message parameter.
 * @l3: 4th remote texture message parameter.
 * @l4: 5th remote texture message parameter.
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
hildon_remote_texture_send_message (HildonRemoteTexture *self,
                                     guint32 message_type,
                                     guint32 l0,
                                     guint32 l1,
                                     guint32 l2,
                                     guint32 l3,
                                     guint32 l4)
{
    GtkWidget          *widget = GTK_WIDGET (self);
    Display            *display = GDK_WINDOW_XDISPLAY (gtk_widget_get_window (widget));
    Window              window = GDK_WINDOW_XID (gtk_widget_get_window (widget));

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
 * hildon_remote_texture_set_image:
 * @self: A #HildonRemoteTexture
 * @key: The key that would be used with shmget in hildon-desktop. The key
 * should probably be created with ftok, and the relevant shared memory
 * area should be created before this call.
 * @width: width of image in pixels
 * @height: height of image in pixels
 * @bpp: BYTES per pixel - usually 2,3 or 4
 *
 * Since: 2.2
 */
void
hildon_remote_texture_set_image (HildonRemoteTexture *self,
                                 key_t key,
                                 guint width,
                                 guint height,
                                 guint bpp)
{
  HildonRemoteTexturePrivate
                       *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);
  GtkWidget          *widget = GTK_WIDGET (self);

  priv->set_shm = 1;
  priv->shm_key = key;
  priv->shm_width = width;
  priv->shm_height = height;
  priv->shm_bpp = bpp;

  if (gtk_widget_get_mapped (widget) && priv->ready)
    {
       /* Defer messages until the remote texture is parented
        * and the parent window is mapped */
        if (!priv->parent || !gtk_widget_get_mapped (GTK_WIDGET (priv->parent)))
            return;
        hildon_remote_texture_send_message (self,
                                            shm_atom,
                                            priv->shm_key,
                                            priv->shm_width,
                                            priv->shm_height,
                                            priv->shm_bpp,
                                            0);
        priv->set_shm = 0;
    }
}

/**
 * hildon_remote_texture_update_area:
 * @self: A #HildonRemoteTexture
 * @x: offset of damaged area in pixels
 * @y: offset of damaged area in pixels
 * @width: width of damaged area in pixels
 * @height: height of damaged area in pixels
 *
 * This signals to hildon-desktop that a specific region of the memory area
 * has changed. This will trigger a redraw and will update the relevant tiles
 * of the texture.
 *
 * Since: 2.2
 */
void
hildon_remote_texture_update_area (HildonRemoteTexture *self,
                                 gint x,
                                 gint y,
                                 gint width,
                                 gint height)
{
  HildonRemoteTexturePrivate
                     *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);
  GtkWidget          *widget = GTK_WIDGET (self);

  if (priv->damage_x1==priv->damage_x2 || priv->damage_y1==priv->damage_y2)
    {
      priv->damage_x1 = x;
      priv->damage_y1 = y;
      priv->damage_x2 = x+width;
      priv->damage_y2 = y+height;
    }
  else
    {
      if (x<priv->damage_x1) priv->damage_x1 = x;
      if (y<priv->damage_y1) priv->damage_y1 = y;
      if (x+width>priv->damage_x2) priv->damage_x2 = x+width;
      if (y+height>priv->damage_y2) priv->damage_y2 = y+height;
    }
  priv->set_damage = 1;

  if (gtk_widget_get_mapped (widget) && priv->ready)
  {
     /* Defer messages until the remote texture is parented
      * and the parent window is mapped */
      if (!priv->parent || !gtk_widget_get_mapped (GTK_WIDGET (priv->parent)))
          return;
      hildon_remote_texture_send_message (self,
                                          damage_atom,
                                          priv->damage_x1,
                                          priv->damage_y1,
                                          priv->damage_x2 - priv->damage_x1,
                                          priv->damage_y2 - priv->damage_y1,
                                          0);
      priv->set_damage = 0;
      priv->damage_x1 = 0;
      priv->damage_y1 = 0;
      priv->damage_x2 = 0;
      priv->damage_y2 = 0;
  }
}

/**
 * hildon_remote_texture_set_show_full:
 * @self: A #HildonRemoteTexture
 * @show: A boolean flag setting the visibility of the remote texture.
 * @opacity: Desired opacity setting
 *
 * Send a message to the window manager setting the visibility of
 * the remote texture. This will only affect the visibility of
 * the remote texture set by the compositing window manager in its own
 * rendering pipeline, after X has drawn the window to the off-screen
 * buffer. This setting, naturally, has no effect if the #HildonRemoteTexture
 * widget is not visible in X11 terms (i.e. realized and mapped).
 *
 * Furthermore, if a widget is parented, its final visibility will be
 * affected by that of the parent window.
 *
 * The opacity setting ranges from zero (0), being completely transparent
 * to 255 (0xff) being fully opaque.
 *
 * If the remote texture WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_remote_texture_set_show_full (HildonRemoteTexture *self,
				      gint show,
				      gint opacity)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    if (opacity > 255)
	opacity = 255;

    if (opacity < 0)
	opacity = 0;

    priv->show = show;
    priv->opacity = opacity;
    priv->set_show = 1;

    if (gtk_widget_get_mapped (widget) && priv->ready)
    {
	/* Defer show messages until the remote texture is parented
	 * and the parent window is mapped */
	if (!priv->parent || !gtk_widget_get_mapped (GTK_WIDGET (priv->parent)))
	    return;
	hildon_remote_texture_send_message (self,
					     show_atom,
					     show, opacity,
					     0, 0, 0);
	priv->set_show = 0;
    }
}

/**
 * hildon_remote_texture_set_show:
 * @self: A #HildonRemoteTexture
 * @show: A boolean flag setting the visibility of the remote texture.
 *
 * This function is a shortcut for hildon_remote_texture_set_show_full(),
 * setting the overall actor visibility without changing it's opacity
 * setting.
 *
 * Since: 2.2
 **/
void
hildon_remote_texture_set_show (HildonRemoteTexture *self,
				 gint show)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);

    hildon_remote_texture_set_show_full (self,
					  show, priv->opacity);
}

/**
 * hildon_remote_texture_set_opacity:
 * @self: A #HildonRemoteTexture
 * @opacity: Desired opacity setting
 *
 * This function is a shortcut for hildon_remote_texture_set_show_full(),
 * setting actor opacity without changing it's overall visibility.
 *
 * See hildon_remote_texture_set_show_full() for description of the range
 * of values @opacity argument takes.
 *
 * Since: 2.2
 **/
void
hildon_remote_texture_set_opacity (HildonRemoteTexture *self,
				    gint opacity)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);

    hildon_remote_texture_set_show_full (self,
					  priv->show, opacity);
}

/**
 * hildon_remote_texture_set_position:
 * @self: A #HildonRemoteTexture
 * @x: Desired X coordinate
 * @y: Desired Y coordinate
 * @width: Desired width
 * @height: Desired height
 *
 * Send a message to the window manager setting the offset of the remote
 * texture in the window (in Remote texture's pixels). The texture
 * is also subject to the animation effects rendered by the compositing
 * window manager on that window (like those by task switcher).
 *
 * If the remote texture WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_remote_texture_set_position (HildonRemoteTexture *self,
                                        gint x,
                                        gint y,
                                        gint width,
                                        gint height)
{
    HildonRemoteTexturePrivate
                       *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    priv->x = x;
    priv->y = y;
    priv->width = width;
    priv->height = height;
    priv->set_position = 1;

    if (gtk_widget_get_mapped (widget) && priv->ready)
    {
        /* Defer messages until the remote texture is parented
         * and the parent window is mapped */

        if (!priv->parent || !gtk_widget_get_mapped (GTK_WIDGET (priv->parent)))
            return;
        hildon_remote_texture_send_message (self,
                                            position_atom,
                                            x, y,
                                            width, height, 0);
        priv->set_position = 0;
    }
}

/**
 * hildon_remote_texture_set_offset:
 * @self: A #HildonRemoteTexture
 * @x: Desired X offset
 * @y: Desired Y offset
 *
 * Send a message to the window manager setting the offset of the remote
 * texture in the window (in Remote texture's pixels). The texture
 * is also subject to the animation effects rendered by the compositing
 * window manager on that window (like those by task switcher).
 *
 * If the remote texture WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_remote_texture_set_offset (HildonRemoteTexture *self,
                                    double x,
                                    double y)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    priv->offset_x = x;
    priv->offset_y = y;
    priv->set_offset = 1;

    if (gtk_widget_get_mapped (widget) && priv->ready)
    {
        /* Defer messages until the remote texture is parented
         * and the parent window is mapped */

        if (!priv->parent || !gtk_widget_get_mapped (GTK_WIDGET (priv->parent)))
            return;
        hildon_remote_texture_send_message (self,
                                            offset_atom,
                                            (gint)(x*65536), (gint)(y*65536),
                                            0, 0, 0);
        priv->set_offset = 0;
    }
}

/**
 * hildon_remote_texture_set_scalex:
 * @self: A #HildonRemoteTexture
 * @x_scale: The scale factor for the memory area to be rendered in the X-axis
 * @y_scale: The scale factor for the memory area to be rendered in the X-axis
 *
 * Since: 2.2
 **/
void
hildon_remote_texture_set_scale (HildonRemoteTexture *self,
                               double x_scale,
                               double y_scale)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);
    GtkWidget          *widget = GTK_WIDGET (self);

    priv->scale_x = x_scale;
    priv->scale_y = y_scale;
    priv->set_scale = 1;

    if (gtk_widget_get_mapped (widget) && priv->ready)
    {
        /* Defer messages until the remote texture is parented
         * and the parent window is mapped */
        if (!priv->parent || !gtk_widget_get_mapped (GTK_WIDGET (priv->parent)))
            return;
        hildon_remote_texture_send_message (self,
                                             scale_atom,
                                             priv->scale_x * (1 << 16),
                                             priv->scale_y * (1 << 16),
                                             0, 0, 0);
        priv->set_scale = 0;
    }
}

/*
 * This callback will be triggered by the parent widget of
 * an remote texture when it is mapped. The compositing
 * window manager is now ready to parent the remote texture
 * into the target parent window.
 */
static gboolean
hildon_remote_texture_parent_map_event (GtkWidget *parent,
					 GdkEvent *event,
					 gpointer user_data)
{
    hildon_remote_texture_set_parent (HILDON_REMOTE_TEXTURE (user_data),
				       GTK_WINDOW (parent));
    return FALSE;
}

/*
 * This callback will be triggered by the widget re-mapping
 * itself in case of WM restarting. The point is to push all
 * remote texture parameters anew to the WM.
 */
static gboolean
hildon_remote_texture_map_event (GtkWidget *widget,
				  GdkEvent *event,
				  gpointer user_data)
{
    HildonRemoteTexture
	               *self = HILDON_REMOTE_TEXTURE (user_data);
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);

    hildon_remote_texture_send_all_messages (self);

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
 * hildon_remote_texture_set_parent:
 * @self: A #HildonRemoteTexture
 * @parent: A #GtkWindow that the actor will be parented to.
 *
 * Send a message to the window manager setting the parent window
 * for the remote texture. Parenting an actor will not affect the
 * X window that the HildonRemoteTexture represents, but it's off-screen
 * bitmap as it is handled by the compositing window manager.
 *
 * Parenting an remote texture will affect its visibility as set
 * by the gtk_widget_show(), gtk_widget_hide() and
 * hildon_remote_texture_set_show(). The remote texture will only be
 * visible when the top-level window it is parented is visible.
 *
 * Passing %NULL as a @parent argument will unparent the remote texture.
 * This will restore the actor's visibility if it was suppressed by
 * being unparented or parented to an unmapped window.
 *
 * If the remote texture WM-counterpart is not ready, the show message
 * will be queued until the WM is ready for it.
 *
 * Since: 2.2
 **/
void
hildon_remote_texture_set_parent (HildonRemoteTexture *self,
				   GtkWindow *parent)
{
    HildonRemoteTexturePrivate
	               *priv = HILDON_REMOTE_TEXTURE_GET_PRIVATE (self);
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
				  G_CALLBACK(hildon_remote_texture_parent_map_event),
				  self);
	}
	else
	{
	    priv->parent_map_event_cb_id = 0;
	}
    }

    if (gtk_widget_get_mapped (widget) && priv->ready)
    {
	Window win = 0;

	/* If the remote texture is being unparented or parented to an
	 * unmapped widget, force its visibility to "hidden". */

	if (!priv->parent || !gtk_widget_get_mapped (GTK_WIDGET (priv->parent)))
	{
	    hildon_remote_texture_send_message (self,
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
	    if (!gtk_widget_get_mapped (GTK_WIDGET (priv->parent)))
		return;

	    GdkWindow *gdk = gtk_widget_get_window (GTK_WIDGET (parent));
	    win = GDK_WINDOW_XID (gdk);
	}

	hildon_remote_texture_send_message (self,
					     parent_atom,
					     win,
					     0, 0, 0, 0);
	priv->set_parent = 0;

	/* Set remote texture visibility to desired value (in case it was
	 * forced off when the actor was parented into an unmapped widget). */

	hildon_remote_texture_send_message (self,
					     show_atom,
					     priv->show, priv->opacity,
					     0, 0, 0);
	priv->set_show = 0;
    }
}

