/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2009 Nokia Corporation, all rights reserved.
 *
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
#include                                        <string.h>
#include                                        <hildon/hildon.h>

enum { TEXT_COLUMN, N_COLUMNS };

static void
on_add_clicked (GtkButton *button,
                gpointer user_data)
{
  GtkListStore *store = NULL;
  GtkTreeIter   iter;

  store = GTK_LIST_STORE (user_data);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      TEXT_COLUMN, "Extra row", -1);
}

static void
on_remove_clicked (GtkButton *button,
                   gpointer user_data)
{
  GtkListStore *store = NULL;
  GtkTreeIter   iter;

  store = GTK_LIST_STORE (user_data);

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store),
                                     &iter)) {
    gtk_list_store_remove (store, &iter);
  }
}


static GtkWidget*
create_content ()
{
  gint               i;
  GtkWidget         *tv       = NULL;
  GtkWidget         *panarea  = NULL;
  GtkWidget         *button   = NULL;;
  GtkTreeViewColumn *col      = NULL;
  GtkCellRenderer   *renderer = NULL;
  GtkListStore      *store    = NULL;
  GtkWidget         *vbox     = NULL;

  /* Create a treeview */
#ifdef MAEMO_GTK
  tv = hildon_gtk_tree_view_new (HILDON_UI_MODE_NORMAL);
#else
  tv = gtk_tree_view_new ();
#endif /* MAEMO_GTK */

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer, "width", 1, "xalign", 0.5, NULL);

  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, "Title");

  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_set_attributes (col, renderer, "text", TEXT_COLUMN, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);

  /* Add some rows to the treeview */
  store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);

  for (i = 0; i < 6; i++) {
    GtkTreeIter iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
                        TEXT_COLUMN, "One row", -1);
  }

  gtk_tree_view_set_model (GTK_TREE_VIEW (tv), GTK_TREE_MODEL (store));
  g_object_unref (store);

  /* Put everything in a pannable area */
  panarea = hildon_pannable_area_new ();
  gtk_container_add (GTK_CONTAINER (panarea), GTK_WIDGET (tv));

  vbox = gtk_vbox_new (FALSE, 5);

  button = gtk_button_new_with_label ("Add a row");
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_add_clicked), store);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("Remove a row");
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_remove_clicked), store);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), panarea, TRUE, TRUE, 6);

  return vbox;
}

int
main (int argc, char **argv)
{
  HildonProgram *program = NULL;
  GtkWidget     *window  = NULL;
  GtkWidget     *content = NULL;

  hildon_gtk_init (&argc, &argv);

  program = hildon_program_get_instance ();

  /* Create the main window */
  window = hildon_window_new ();
  hildon_program_add_window (program, HILDON_WINDOW (window));
  gtk_container_set_border_width (GTK_CONTAINER (window), 5);

  content = create_content ();

  gtk_container_add (GTK_CONTAINER (window), content);

  g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (GTK_WIDGET (window));

  gtk_main ();

  return 0;
}
