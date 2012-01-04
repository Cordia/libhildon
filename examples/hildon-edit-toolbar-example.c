/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
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

typedef enum
{
  NORMAL_MODE,
  EDIT_MODE
} TreeViewMode;


static GtkTreeModel *
get_model                                       (void)
{
  int i;
  static GtkListStore *store = NULL;

  if (store != NULL)
    return GTK_TREE_MODEL (store);

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

static GtkWidget *
create_icon_view                                (TreeViewMode  tvmode)
{
  GtkWidget *icon_view;
  GtkTreeModel *model;
  GtkCellRenderer *renderer;

  if (tvmode == NORMAL_MODE)
    {
      icon_view = gtk_icon_view_new ();
    }
  else
    {
      icon_view = gtk_icon_view_new ();

      gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view),
                                        GTK_SELECTION_MULTIPLE);
    }

  model = get_model ();
  gtk_icon_view_set_model (GTK_ICON_VIEW (icon_view), model);

  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "stock-id", GTK_STOCK_NEW, NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (icon_view), renderer, TRUE);

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (icon_view), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (icon_view), renderer, "text", 0, NULL);

  return icon_view;
}

static void
delete_button_clicked                           (GtkButton   *button,
                                                 GtkIconView *iconview)
{
  GtkTreeModel *model;
  GList *items, *iter, *refs;
  GtkWidget *window;

  items = gtk_icon_view_get_selected_items (iconview);
  model = gtk_icon_view_get_model (iconview);
  refs = NULL;

  /* Get row references for all selected items */
  for (iter = items; iter != NULL; iter = iter->next)
    {
      GtkTreePath *path = (GtkTreePath *) iter->data;
      GtkTreeRowReference *ref = gtk_tree_row_reference_new (model, path);
      refs = g_list_prepend (refs, gtk_tree_row_reference_copy (ref));
      gtk_tree_row_reference_free (ref);
    }

  g_list_foreach (items, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (items);

  /* Remove all selected items from the model */
  for (iter = refs; iter != NULL; iter = iter->next)
    {
      GtkTreeIter treeiter;
      GtkTreeRowReference *ref = (GtkTreeRowReference *) iter->data;
      GtkTreePath *path = gtk_tree_row_reference_get_path (ref);
      gtk_tree_model_get_iter (model, &treeiter, path);
      gtk_list_store_remove (GTK_LIST_STORE (model), &treeiter);
    }

  g_list_foreach (refs, (GFunc) gtk_tree_row_reference_free, NULL);
  g_list_free (refs);

  /* After removing the items, close the window */
  window = gtk_widget_get_toplevel (GTK_WIDGET (iconview));
  gtk_widget_destroy (window);
}

static void
edit_window                                     (void)
{
  GtkWidget *window;
  GtkWidget *iconview;
  GtkWidget *toolbar;
  GtkWidget *area;

  window = hildon_stackable_window_new ();
  gtk_container_set_border_width (GTK_CONTAINER (window), 6);

  toolbar = hildon_edit_toolbar_new_with_text ("Choose items to delete", "Delete");
  area = hildon_pannable_area_new ();
  iconview = create_icon_view (EDIT_MODE);

  hildon_window_set_edit_toolbar (HILDON_WINDOW (window), HILDON_EDIT_TOOLBAR (toolbar));

  gtk_container_add (GTK_CONTAINER (area), iconview);
  gtk_container_add (GTK_CONTAINER (window), area);

  g_signal_connect (toolbar, "button-clicked", G_CALLBACK (delete_button_clicked), iconview);
  g_signal_connect_swapped (toolbar, "arrow-clicked", G_CALLBACK (gtk_widget_destroy), window);

  gtk_widget_show_all (window);
  gtk_window_fullscreen (GTK_WINDOW (window));
}

int
main                                            (int    argc,
                                                 char **argv)
{
  GtkWidget *window;
  GtkWidget *iconview;
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *area;

  hildon_gtk_init (&argc, &argv);

  window = hildon_stackable_window_new ();
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  gtk_container_set_border_width (GTK_CONTAINER (window), 6);

  vbox = gtk_vbox_new (FALSE, 10);
  area = hildon_pannable_area_new ();
  iconview = create_icon_view (NORMAL_MODE);
  button = hildon_gtk_button_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);
  gtk_button_set_label (GTK_BUTTON (button), "Delete some items");

  gtk_container_add (GTK_CONTAINER (area), iconview);
  gtk_box_pack_start (GTK_BOX (vbox), area, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (window), vbox);

  g_signal_connect (button, "clicked", G_CALLBACK (edit_window), NULL);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
