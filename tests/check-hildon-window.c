/*
 * This file is a part of hildon tests
 *
 * Copyright (C) 2006, 2007 Nokia Corporation, all rights reserved.
 *
 * Contact: Michael Dominic Kostrzewa <michael.kostrzewa@nokia.com>
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

#include <stdlib.h>
#include <check.h>
#include "test_suites.h"
#include "check_utils.h"

#include <hildon/hildon-window.h>


/* -------------------- Fixtures -------------------- */

static HildonWindow *window = NULL;

static void
fx_setup_default_window ()
{
  int argc = 0;
  gtk_init(&argc, NULL);

  window = HILDON_WINDOW(hildon_window_new());
  /* Check window object has been created properly */
  fail_if(!HILDON_IS_WINDOW(window),
          "hildon-window: Creation failed.");

  show_all_test_window(GTK_WIDGET(window));
}

static void
fx_teardown_default_window ()
{
  gtk_widget_destroy(GTK_WIDGET(window));
}


/* -------------------- Test cases -------------------- */

/* ----- Test case for add_with_scrollbar -----*/

/**
 * Purpose: Test regular usage
 * Cases considered:
 *    - Add a non viewport child
 *    - Add a viewport child
 */
START_TEST (test_add_with_scrollbar_regular)
{
  GtkWidget *box;
  GtkWidget *scrolled_window;
  GtkWidget *viewport;
  GList *children, *first;

  /* Test1: Check addition of a non viewport child */
  box = gtk_hbox_new(FALSE, 0);
  hildon_window_add_with_scrollbar(window, box);

  show_all_test_window(GTK_WIDGET(window));

  children = gtk_container_get_children(GTK_CONTAINER(window));
  first = g_list_first(children);
  fail_if(first == NULL,
          "hildon-window: Added a hbox with scrollbar, but window has not children");
  fail_if (!GTK_IS_SCROLLED_WINDOW(first->data),
	   "hildon-window: Added a hbox with scrollbar, but retrieved child is not a scrolled window");
  scrolled_window = GTK_WIDGET(first->data);
  g_list_free(children);
  children = gtk_container_get_children(GTK_CONTAINER(scrolled_window));
  first = g_list_first(children);
  fail_if(first == NULL,
          "hildon-window: Added a hbox with scrollbar, but scrolled window has not children");
  fail_if (!GTK_IS_VIEWPORT(first->data),
	   "hildon-window: Added a hbox with scrollbar, but retrieved child from the scrolled window is not a viewport");  
  viewport = GTK_WIDGET(first->data);
  g_list_free(children);
  children = gtk_container_get_children(GTK_CONTAINER(viewport));
  first = g_list_first(children);
  fail_if(first == NULL,
          "hildon-window: Added a hbox with scrollbar, but viewport child of the scrolled window has not children");
  fail_if (!GTK_IS_HBOX(first->data),
	   "hildon-window: Added a hbox with scrollbar, but retrieved child from the viewport of the scrolled window is not a hbox");
  g_list_free(children);
  gtk_container_remove(GTK_CONTAINER(window), scrolled_window);

  /* Test2: Check addition of a viewport child */
  box = gtk_hbox_new(FALSE, 0);
  viewport = gtk_viewport_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 100, 10, 10, 1000)),
                              GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 100, 10, 10, 1000)));
  gtk_container_add(GTK_CONTAINER(viewport), box);
  hildon_window_add_with_scrollbar(window, viewport);

  show_all_test_window(GTK_WIDGET(window));

  children = gtk_container_get_children(GTK_CONTAINER(window));
  first = g_list_first(children);
  fail_if(first == NULL,
          "hildon-window: Added a hbox inside a viewport with scrollbar, but window has not children");
  fail_if (!GTK_IS_SCROLLED_WINDOW(first->data),
	   "hildon-window: Added a hbox inside a viewport with scrollbar, but retrieved child is not a scrolled window");
  scrolled_window = GTK_WIDGET(first->data);
  g_list_free(children);
  children = gtk_container_get_children(GTK_CONTAINER(scrolled_window));
  first = g_list_first(children);
  fail_if(first == NULL,
          "hildon-window: Added a hbox inside a viewportwith scrollbar, but scrolled window has not children");
  fail_if (!GTK_IS_VIEWPORT(first->data),
	   "hildon-window: Added a hbox inside a viewport with scrollbar, but retrieved child from the scrolled window is not a viewport");
  viewport = GTK_WIDGET(first->data);
  g_list_free(children);
  children = gtk_container_get_children(GTK_CONTAINER(viewport));
  first = g_list_first(children);
  fail_if(first == NULL,
          "hildon-window: Added a hbox inside a viewport with scrollbar, but viewport child of the scrolled window has not children");
  fail_if (!GTK_IS_HBOX(first->data),
              "hildon-window: Added a hbox inside a viewport with scrollbar, but retrieved child from the viewport of the scrolled window is not a hbox");
  g_list_free(children);
  gtk_container_remove(GTK_CONTAINER(window), scrolled_window);
}
END_TEST

/**
 * Purpose: Check invalid values
 * Cases considered:
 *    - Add a NULL child
 *    - Add a child to a NULL window
 *    - Add a child with a parent already set
 */
START_TEST (test_add_with_scrollbar_invalid)
{
  GList *children, *first;
  GtkWidget *hbox;
  GtkWidget *entry;

  /* Test1: Add NULL object */
  hildon_window_add_with_scrollbar(window, NULL);
  children = gtk_container_get_children(GTK_CONTAINER(window));
  first = g_list_first(children);
  fail_if (first != NULL,
              "hildon-window: Addition of a NULL child is allowed");
  g_list_free(children);

  /* Test2: Add to a NULL window */
  hildon_window_add_with_scrollbar(NULL, gtk_hbox_new(FALSE, 0));

  /* Test3: Add a widget with a parent already set */
  hbox = gtk_hbox_new(FALSE, 0);
  entry = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(hbox), entry);
  hildon_window_add_with_scrollbar(window, entry);

  show_all_test_window(GTK_WIDGET(window));

  children = gtk_container_get_children(GTK_CONTAINER(window));
  first = g_list_first(children);
  fail_if (first != NULL,
              "hildon-window: Addition of a child with parent already set is allowed");
  g_list_free(children);
}
END_TEST

/* ---------- Suite creation ---------- */

Suite *create_hildon_window_suite()
{
  /* Create the suite */
  Suite *s = suite_create("HildonWindow");

  /* Create test cases */
  TCase *tc1 = tcase_create("add_with_scrollbar");

  /* Create test case for add_with_scrollbar and add it to the suite */
  tcase_add_checked_fixture(tc1, fx_setup_default_window, fx_teardown_default_window);
  tcase_add_test(tc1, test_add_with_scrollbar_regular);
  tcase_add_test(tc1, test_add_with_scrollbar_invalid);
  suite_add_tcase (s, tc1);

  /* Return created suite */
  return s;
}
