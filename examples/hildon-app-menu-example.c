/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
 *
 * Author: Karl Lattimer <karl.lattimer@nokia.com>
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

#include                                        <gtk/gtk.h>
#include                                        <hildon/hildon.h>

static void
menu_button_clicked                             (GtkButton *button,
                                                 GtkLabel *label)
{
    const char *buttontext = gtk_button_get_label (button);
    char *text = g_strdup_printf("Last option selected:\n%s", buttontext);
    gtk_label_set_text (label, text);
    g_free (text);
    g_debug ("Button clicked: %s", buttontext);
}

static HildonAppMenu *
create_menu                                     (GtkWidget     *label,
                                                 GtkAccelGroup *accel)
{
    GtkWidget *button;
    HildonSizeType buttonsize = HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH;
    HildonAppMenu *menu = HILDON_APP_MENU (hildon_app_menu_new ());

    /* Options */
    button = hildon_gtk_button_new (buttonsize);
    gtk_button_set_label (GTK_BUTTON (button), "Menu command one");
    g_signal_connect_after (button, "clicked", G_CALLBACK (menu_button_clicked), label);
    hildon_app_menu_append (menu, GTK_BUTTON (button));

    gtk_widget_add_accelerator (button, "activate", accel, GDK_KEY_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    button = hildon_gtk_button_new (buttonsize);
    gtk_button_set_label (GTK_BUTTON (button), "Menu command two");
    g_signal_connect_after (button, "clicked", G_CALLBACK (menu_button_clicked), label);
    hildon_app_menu_append (menu, GTK_BUTTON (button));

    button = hildon_gtk_button_new (buttonsize);
    gtk_button_set_label (GTK_BUTTON (button), "Menu command three");
    g_signal_connect_after (button, "clicked", G_CALLBACK (menu_button_clicked), label);
    hildon_app_menu_append (menu, GTK_BUTTON (button));

    button = hildon_gtk_button_new (buttonsize);
    gtk_button_set_label (GTK_BUTTON (button), "Menu command four");
    g_signal_connect_after (button, "clicked", G_CALLBACK (menu_button_clicked), label);
    hildon_app_menu_append (menu, GTK_BUTTON (button));

    button = hildon_gtk_button_new (buttonsize);
    gtk_button_set_label (GTK_BUTTON (button), "Menu command five");
    g_signal_connect_after (button, "clicked", G_CALLBACK (menu_button_clicked), label);
    hildon_app_menu_append (menu, GTK_BUTTON (button));

    /* Filters */
    button = hildon_gtk_radio_button_new (buttonsize, NULL);
    gtk_button_set_label (GTK_BUTTON (button), "filter one");
    g_signal_connect_after (button, "clicked", G_CALLBACK (menu_button_clicked), label);
    hildon_app_menu_add_filter (menu, GTK_BUTTON (button));
    gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), FALSE);

    button = hildon_gtk_radio_button_new_from_widget (buttonsize, GTK_RADIO_BUTTON (button));
    gtk_button_set_label (GTK_BUTTON (button), "filter two");
    g_signal_connect_after (button, "clicked", G_CALLBACK (menu_button_clicked), label);
    hildon_app_menu_add_filter (menu, GTK_BUTTON (button));
    gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), FALSE);

    gtk_widget_show_all (GTK_WIDGET (menu));

    return menu;
}

int
main                                            (int argc,
                                                 char **argv)
{
    GtkWidget *win;
    GtkWidget *label;
    GtkWidget *label2;
    GtkBox *vbox;
    HildonAppMenu *menu;
    GtkAccelGroup *accel;

    hildon_gtk_init (&argc, &argv);

    label = gtk_label_new ("This is an example of the\nHildonAppMenu widget.\n\n"
                           "Click on the titlebar\nto pop up the menu.");
    label2 = gtk_label_new ("No menu option has been selected yet.");

    accel = gtk_accel_group_new ();

    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_CENTER);

    vbox = GTK_BOX (gtk_vbox_new (FALSE, 10));
    win = hildon_stackable_window_new ();

    menu = create_menu (label2, accel);

    hildon_window_set_app_menu (HILDON_WINDOW (win), menu);

    gtk_window_add_accel_group (GTK_WINDOW (win), accel);
    g_object_unref (accel);

    gtk_box_pack_start (vbox, label, TRUE, TRUE, 0);
    gtk_box_pack_start (vbox, label2, TRUE, TRUE, 0);

    gtk_container_set_border_width (GTK_CONTAINER (win), 20);
    gtk_container_add (GTK_CONTAINER (win), GTK_WIDGET (vbox));

    g_signal_connect (win, "delete_event", G_CALLBACK (gtk_main_quit), NULL);

    gtk_widget_show_all (win);

    gtk_main ();

    return 0;
}
