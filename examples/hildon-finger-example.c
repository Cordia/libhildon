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

gboolean
on_button_press                                 (GtkWidget *widget, 
                                                 GdkEventButton *event);

gboolean
on_button_press                                 (GtkWidget *widget, 
                                                 GdkEventButton *event)
{
    HildonNote *note;
    if (hildon_helper_event_button_is_finger (event)) 
        note = HILDON_NOTE (hildon_note_new_information (NULL, "You clicked with finger!"));
    else
        note = HILDON_NOTE (hildon_note_new_information (NULL, "You clicked with stylus!"));

    gtk_dialog_run (GTK_DIALOG (note));
    gtk_widget_destroy (GTK_WIDGET (note));

    return TRUE;
}

int
main                                            (int argc,
                                                 char **argv)
{
    hildon_gtk_init (&argc, &argv);

    HildonProgram *program = hildon_program_get_instance ();
    GtkDrawingArea *area = GTK_DRAWING_AREA (gtk_drawing_area_new ());
    gtk_widget_set_size_request (GTK_WIDGET (area), 320, 240);

    gtk_widget_set_events (GTK_WIDGET (area), 
                           GDK_BUTTON_PRESS_MASK);

    GtkWidget *window = hildon_window_new ();
    hildon_program_add_window (program, HILDON_WINDOW (window));    

    gtk_window_set_title (GTK_WINDOW (window), "world");
    g_set_application_name ("hello");

    gtk_container_set_border_width (GTK_CONTAINER (window), 6);
    
    g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (G_OBJECT (area), "button-press-event", G_CALLBACK (on_button_press), NULL);
    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (area));
    gtk_widget_show_all (GTK_WIDGET (window));
    
    gtk_main ();
    return 0;
}


