/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2005, 2006, 2009 Nokia Corporation, all rights reserved.
 *
 * Author: Michael Dominic Kostrzewa <michael.kostrzewa@nokia.com>
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

#include                                        <hildon/hildon.h>

#ifndef HILDON_DISABLE_DEPRECATED

static gboolean
on_animation_idle                               (GtkWidget *banner)
{
    gtk_widget_destroy (banner);
    g_object_unref (banner);
    return FALSE;
}

static gboolean
on_progress_idle                                (GtkWidget *banner)
{
    gtk_widget_destroy (banner);
    g_object_unref (banner);
    return FALSE;
}

#endif

static void
on_information_clicked                          (GtkWidget *widget)
{
    GtkWidget* banner = hildon_banner_show_information (widget, NULL, "Information banner");
    hildon_banner_set_timeout (HILDON_BANNER (banner), 9000);
}

#ifndef HILDON_DISABLE_DEPRECATED

static void
on_animation_clicked                            (GtkWidget *widget)
{
    GtkWidget *banner = hildon_banner_show_animation (widget, NULL, "Animation banner");
    g_object_ref (banner);
    gdk_threads_add_timeout (5000, (GSourceFunc) on_animation_idle, banner);
}

static void
on_progress_clicked                             (GtkWidget *widget)
{
    GtkWidget *banner = hildon_banner_show_progress (widget, NULL, "Progress banner");
    g_object_ref (banner);
    gdk_threads_add_timeout (5000, (GSourceFunc) on_progress_idle, banner);
}

#endif

int
main                                            (int argc,
                                                 char **argv)
{
    HildonProgram *program;
    GtkWidget *window, *vbox, *button1;
#ifndef HILDON_DISABLE_DEPRECATED
    GtkWidget *button2, *button3;
#endif

    hildon_gtk_init (&argc, &argv);

    window = hildon_window_new ();
    program = hildon_program_get_instance ();
    hildon_program_add_window (program, HILDON_WINDOW (window));

    button1 = gtk_button_new_with_label ("Information");
    g_signal_connect (button1, "clicked", G_CALLBACK (on_information_clicked), NULL);

#ifndef HILDON_DISABLE_DEPRECATED
    button2 = gtk_button_new_with_label ("Animation");
    g_signal_connect (button2, "clicked", G_CALLBACK (on_animation_clicked), NULL);

    button3 = gtk_button_new_with_label ("Progress");
    g_signal_connect (button3, "clicked", G_CALLBACK (on_progress_clicked), NULL);
#endif

    vbox = gtk_vbox_new (6, FALSE);
    gtk_box_pack_start (GTK_BOX (vbox), button1, TRUE, TRUE, 0);
#ifndef HILDON_DISABLE_DEPRECATED
    gtk_box_pack_start (GTK_BOX (vbox), button2, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), button3, TRUE, TRUE, 0);
#endif

    gtk_container_set_border_width (GTK_CONTAINER (window), 6);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    gtk_widget_show_all (window);

    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    gtk_main ();
    return 0;
}
