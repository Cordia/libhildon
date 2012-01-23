/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2005, 2006 Nokia Corporation, all rights reserved.
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

#include                                        <stdio.h>
#include                                        <stdlib.h>
#include                                        <glib.h>
#include                                        <gtk/gtk.h>
#include                                        <hildon/hildon.h>

gboolean                                        has_toolbar = FALSE;

HildonWindow*                                   window = NULL;

GtkToolbar*                                     toolbar = NULL;

static gboolean
on_add_clicked                                  (GtkWidget *widget)
{
    if (has_toolbar) 
        hildon_banner_show_information (widget, NULL, "Toolbar already added"); 
    else {
        hildon_window_add_toolbar (window, toolbar);
        has_toolbar = TRUE;
    }

    return TRUE;
}

static gboolean
on_remove_clicked                               (GtkWidget *widget)
{
    if (! has_toolbar) 
        hildon_banner_show_information (widget, NULL, "No toolbar added"); 
    else {
        g_object_ref (toolbar);
        hildon_window_remove_toolbar (window, toolbar);
        has_toolbar = FALSE;
    }

    return TRUE;
}

static gboolean
on_show_clicked                                 (GtkWidget *widget)
{
    if (! has_toolbar) 
        hildon_banner_show_information (widget, NULL, "No toolbar added"); 
    else
        gtk_widget_show_all (GTK_WIDGET (toolbar));

    return TRUE;
}

static gboolean
on_hide_clicked                                 (GtkWidget *widget)
{
    if (! has_toolbar) 
        hildon_banner_show_information (widget, NULL, "No toolbar added"); 
    else 
        gtk_widget_hide (GTK_WIDGET (toolbar));

    return TRUE;
}

int
main                                            (int argc,
                                                 char **argv)
{
    hildon_gtk_init (&argc, &argv);

    HildonProgram *program = hildon_program_get_instance ();

    window = HILDON_WINDOW (hildon_window_new ());
    hildon_program_add_window (program, window);    
    toolbar = GTK_TOOLBAR (hildon_find_toolbar_new ("Find"));

    gtk_container_set_border_width (GTK_CONTAINER (window), 6);

    GtkVBox *vbox = GTK_VBOX (gtk_vbox_new (6, FALSE));
    
    GtkButton *button1 = GTK_BUTTON (gtk_button_new_with_label ("Add toolbar"));
    g_signal_connect (G_OBJECT (button1), "clicked", G_CALLBACK (on_add_clicked), NULL);

    GtkButton *button2 = GTK_BUTTON (gtk_button_new_with_label ("Remove toolbar"));
    g_signal_connect (G_OBJECT (button2), "clicked", G_CALLBACK (on_remove_clicked), NULL);

    GtkButton *button3 = GTK_BUTTON (gtk_button_new_with_label ("Show toolbar"));
    g_signal_connect (G_OBJECT (button3), "clicked", G_CALLBACK (on_show_clicked), NULL);

    GtkButton *button4 = GTK_BUTTON (gtk_button_new_with_label ("Hide toolbar"));
    g_signal_connect (G_OBJECT (button4), "clicked", G_CALLBACK (on_hide_clicked), NULL);

    g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_main_quit), NULL);

    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button1), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button2), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button3), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button4), TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (vbox));

    gtk_widget_show_all (GTK_WIDGET (window));
    
    gtk_main ();
    return 0;
}


