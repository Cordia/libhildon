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

#include "check_utils.h"
#include <hildon/hildon-window.h>

/**
 * Creates a window of a fixed, well known size
 */
GtkWidget *
create_test_window ()
{
  GtkWidget *window = NULL;

  window = hildon_window_new ();
  /*  gtk_window_resize (GTK_WINDOW (window), TEST_WINDOW_WIDTH, TEST_WINDOW_HEIGHT);  */

  return window;
}

/**
 * Wrapper to gtk_widget_show, that call to gtk_events_pending serving events.
 */
void show_test_window(GtkWidget * window)
{
  
  gtk_widget_show (window);

  while (gtk_events_pending ())
    {
      gtk_main_iteration ();
    }
}

/**
 * Wrapper to gtk_widget_show_all, that call to gtk_events_pending serving events.
 */
void show_all_test_window(GtkWidget * window)
{

  gtk_widget_show_all (window);

  while (gtk_events_pending ())
    {
      gtk_main_iteration ();
    }
}
