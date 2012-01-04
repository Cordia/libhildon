/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006, 2009 Nokia Corporation, all rights reserved.
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

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        "hildon-private.h"
#include                                        "hildon-defines.h"

G_GNUC_INTERNAL GtkWidget *
hildon_private_create_animation                 (gfloat       framerate,
                                                 const gchar *template,
                                                 gint         nframes)
{
    GtkWidget *image;
    GdkPixbufSimpleAnim *anim;
    GtkIconTheme *theme;
    gint i;

    anim = gdk_pixbuf_simple_anim_new (HILDON_ICON_PIXEL_SIZE_STYLUS,
                                       HILDON_ICON_PIXEL_SIZE_STYLUS,
                                       framerate);
    gdk_pixbuf_simple_anim_set_loop (anim, TRUE);
    theme = gtk_icon_theme_get_default ();

    for (i = 1; i <= nframes; i++) {
        GdkPixbuf *frame;
        GError *error = NULL;
        gchar *icon_name = g_strdup_printf (template, i);
        frame = gtk_icon_theme_load_icon (theme, icon_name,
                                          HILDON_ICON_PIXEL_SIZE_STYLUS,
                                          0, &error);

        if (error) {
            g_warning ("Icon theme lookup for icon `%s' failed: %s",
                       icon_name, error->message);
            g_error_free (error);
        } else {
            gdk_pixbuf_simple_anim_add_frame (anim, frame);
        }

        g_object_unref (frame);
        g_free (icon_name);
    }

    image = gtk_image_new_from_animation (GDK_PIXBUF_ANIMATION (anim));
    g_object_unref (anim);

    return image;
}


void
hildon_gtk_window_set_clear_window_flag                           (GtkWindow   *window,
                                                                   const gchar *atomname,
                                                                   Atom         xatom,
                                                                   gboolean     flag)
{
    GdkWindow *gdkwin = gtk_widget_get_window (GTK_WIDGET (window))	;
    GdkAtom atom = gdk_atom_intern (atomname, FALSE);

    if (flag) {
        guint32 set = 1;
        gdk_property_change (gdkwin, atom, gdk_x11_xatom_to_atom (xatom),
                             32, GDK_PROP_MODE_REPLACE, (const guchar *) &set, 1);
    } else {
        gdk_property_delete (gdkwin, atom);
    }
}

void
hildon_gtk_window_set_flag                                        (GtkWindow      *window,
                                                                   HildonFlagFunc  func,
                                                                   gpointer        userdata)
{
     g_return_if_fail (GTK_IS_WINDOW (window));
     if (gtk_widget_get_realized (GTK_WIDGET (window))) {
        (*func) (window, userdata);
     } else {
         g_signal_handlers_disconnect_matched (window, G_SIGNAL_MATCH_FUNC,
                                               0, 0, NULL, func, NULL);
         g_signal_connect (window, "realize", G_CALLBACK (func), userdata);
     }
}
