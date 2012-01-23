/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
 *
 * Author: Karl Lattimer <karl.lattimer@nokia.com>
 *
 * Based on testhildoniconview.c by Kristian Rietveld <kris@imendio.com>
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

enum
{
  MULTI_SELECT  = 1 << 0,
  NORMAL_MODE   = 1 << 1,
  EDIT_MODE     = 1 << 2
};


static GtkTreeModel *
create_model (void)
{
  int i;
  GtkListStore *store;

  store = gtk_list_store_new (1, G_TYPE_STRING);

  for (i = 0; i < 50; i++)
    {
      gchar *str;

      str = g_strdup_printf ("\nRow %d\n", i);
      gtk_list_store_insert_with_values (store, NULL, i, 0, str, -1);
      g_free (str);
    }

  return GTK_TREE_MODEL (store);
}

static void
item_activated_callback (GtkWidget         *icon_view,
                         GtkTreePath       *path,
                         gpointer           user_data)
{
  g_print ("item-activated emitted.\n");
}

static GtkWidget *
create_icon_view (gboolean      multi_select)
{
  GtkWidget *icon_view;
  GtkCellRenderer *renderer;
  GtkTreeModel *model;

  icon_view = gtk_icon_view_new ();

  model = create_model ();
  gtk_icon_view_set_model (GTK_ICON_VIEW (icon_view), model);
  g_object_unref (model);

  if (multi_select)
    gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view),
                                      GTK_SELECTION_MULTIPLE);
  else
    gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view),
                                      GTK_SELECTION_SINGLE);

  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "stock-id", GTK_STOCK_NEW, NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (icon_view),
                              renderer,
                              TRUE);

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (icon_view),
                              renderer,
                              FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (icon_view),
                                  renderer,
                                  "text", 0,
                                  NULL);

  return icon_view;
}

static void
create_icon_view_window (GtkWidget *button,
                         gpointer   user_data)
{
  GtkWidget *window;
  GtkWidget *sw;
  GtkWidget *icon_view;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "delete-event",
                    G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_set_border_width (GTK_CONTAINER (window), 6);

  sw = hildon_pannable_area_new ();
  gtk_container_add (GTK_CONTAINER (window), sw);

  icon_view = create_icon_view ((GPOINTER_TO_INT (user_data) & MULTI_SELECT) == MULTI_SELECT);

  /* Some signals doing printfs() to see if the behavior is correct. */
  g_signal_connect (icon_view, "item-activated",
                    G_CALLBACK (item_activated_callback), NULL);

  gtk_widget_set_size_request (icon_view, 480, 800);
  gtk_container_add (GTK_CONTAINER (sw), icon_view);

  gtk_widget_show_all (window);
}

int
main (int argc, char **argv)
{
  GtkWidget *window;
  GtkWidget *mainbox;
  GtkWidget *label;
  GtkWidget *vbox;
  GtkWidget *padbox;
  GtkWidget *button;

  hildon_gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "delete-event",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_container_set_border_width (GTK_CONTAINER (window), 6);

  mainbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (window), mainbox);

  /* old-style */
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "<b>Old-style behavior</b>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);

  padbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (mainbox), padbox, FALSE, FALSE, 6);

  gtk_box_pack_start (GTK_BOX (padbox), gtk_label_new ("   "),
                      FALSE, FALSE, 6);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (padbox), vbox, TRUE, TRUE, 6);

  button = gtk_button_new_with_label ("Single selection");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window), 0x0);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("Multiple selections");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window),
                    GINT_TO_POINTER (MULTI_SELECT));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  /* normal-mode */
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "<b>Fremantle Normal mode</b>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);

  padbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (mainbox), padbox, FALSE, FALSE, 6);

  gtk_box_pack_start (GTK_BOX (padbox), gtk_label_new ("   "),
                      FALSE, FALSE, 6);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (padbox), vbox, TRUE, TRUE, 6);

  button = gtk_button_new_with_label ("Direct activation");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window),
                    GINT_TO_POINTER (NORMAL_MODE));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  /* edit-mode */
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "<b>Fremantle Edit mode</b>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);

  padbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (mainbox), padbox, FALSE, FALSE, 6);

  gtk_box_pack_start (GTK_BOX (padbox), gtk_label_new ("   "),
                      FALSE, FALSE, 6);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (padbox), vbox, TRUE, TRUE, 6);

  button = gtk_button_new_with_label ("Single selection");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window),
                    GINT_TO_POINTER (EDIT_MODE));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("Multiple selections");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window),
                    GINT_TO_POINTER (EDIT_MODE | MULTI_SELECT));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);


  button = gtk_button_new_with_label ("Close");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_box_pack_end (GTK_BOX (mainbox), button, FALSE, FALSE, 0);

  gtk_box_pack_end (GTK_BOX (mainbox), gtk_hseparator_new (),
                    FALSE, FALSE, 6);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
