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

#include                                        <hildon/hildon.h>

int
main                                            (int argc,
                                                 char **argv)
{
    GtkDialog *d, *d2;
    GtkWidget *label, *label2;

    hildon_gtk_init (&argc, &argv);

    /* First dialog, using gtk_dialog_new() */

    d = GTK_DIALOG (gtk_dialog_new ());
    label = gtk_label_new ("Hello, world!");

    gtk_window_set_title (GTK_WINDOW (d), "Hi!");
    gtk_dialog_add_button (GTK_DIALOG (d), GTK_STOCK_OK, GTK_RESPONSE_NONE);
    gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (d)), label);

    gtk_widget_show_all (GTK_WIDGET (d));

    gtk_dialog_run (GTK_DIALOG (d));

    /* Second dialog, using gtk_dialog_new_with_buttons() */

    d2 = GTK_DIALOG (gtk_dialog_new_with_buttons ("Hi again!",
                                                        GTK_WINDOW (d),
                                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                        GTK_STOCK_OK,
                                                        GTK_RESPONSE_ACCEPT,
                                                        GTK_STOCK_CANCEL,
                                                        GTK_RESPONSE_REJECT,
                                                        NULL));

    label2 = gtk_label_new ("Hello, again!");

    gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (d2)), label2);

    gtk_widget_show_all (GTK_WIDGET (d2));

    gtk_dialog_run (GTK_DIALOG (d2));

    return 0;
}
