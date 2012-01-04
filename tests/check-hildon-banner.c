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
#include <gtk/gtkmain.h>
#include "test_suites.h"
#include "check_utils.h"
#include <hildon/hildon-banner.h>
#include <hildon/hildon-window.h>

/* -------------------- Fixtures -------------------- */

static GtkWidget * b_window = NULL;

static void 
fx_setup_default_banner ()
{
  int argc = 0;
  gtk_init(&argc, NULL);
  
  b_window = create_test_window();

 /* Check window object has been created properly */
  fail_if(!HILDON_IS_WINDOW(b_window),
          "hildon-banner: Window creation failed.");

  
}

static void 
fx_teardown_default_banner ()
{

  gtk_widget_destroy(b_window);

}

/* -------------------- Test cases -------------------- */

/* ----- Test case for show_animation -----*/

/**
 * Purpose: Check creation of new animation banner with regular values
 * Cases considered:
 *    - Create an animation banner with NULL animation name and TEST_STRING text. 
 *    - Create an animation banner with dummy animation name and "" text.
 */
START_TEST (test_show_animation_regular)
{
  gchar * animation_name=NULL;
  gchar * text=NULL;
    
  HildonBanner * hildon_banner = NULL;


  /*Test 1: Create an animation banner with NULL animation name. */
  text = TEST_STRING;
  hildon_banner = HILDON_BANNER(hildon_banner_show_animation(b_window,NULL,text));

  fail_if(!HILDON_IS_BANNER(hildon_banner),
          "hildon-banner: hildon_banner_show_animation failed creating banner.");
    
  hildon_banner_set_text(hildon_banner,text);
  
  gtk_widget_destroy(GTK_WIDGET(hildon_banner));

  /*Test 2: Create an animation banner with animation name set to "dummy" and text set to "". */
  text="";
  animation_name = "dummy";
  hildon_banner = HILDON_BANNER(hildon_banner_show_animation(b_window,animation_name,text));
    
  fail_if(!HILDON_IS_BANNER(hildon_banner),
          "hildon-banner: hildon_banner_show_animation failed creating banner.");
    
  hildon_banner_set_text(hildon_banner,text);
  gtk_widget_destroy(GTK_WIDGET(hildon_banner));
    
}
END_TEST

/**
 * Purpose: Check creation of new animation banner with invalid values
 * Cases considered:
 *    - Create an animation banner with NULL text. 
 *    - Create an animation banner with NULL window.
 */
START_TEST (test_show_animation_invalid)
{
  gchar * animation_name=NULL;
  gchar * text=NULL;
    
  HildonBanner * hildon_banner = NULL;

  /*Test 1: Create an animation banner with NULL text. */
    
  hildon_banner = HILDON_BANNER(hildon_banner_show_animation(b_window,NULL,NULL));
    
  fail_if(HILDON_IS_BANNER(hildon_banner),
          "hildon-banner: hildon_banner_show_animation failed creating banner.");
    
  hildon_banner = NULL;
    
  /*Test 2: Create an animation banner with NULL window. */
  text="";
  animation_name = "dummy";
  hildon_banner = HILDON_BANNER(hildon_banner_show_animation(NULL,animation_name,text));
    
  fail_if(!HILDON_IS_BANNER(hildon_banner),
          "hildon-banner: hildon_banner_show_animation failed creating banner.");
    
  hildon_banner_set_text(hildon_banner,text);
  gtk_widget_destroy(GTK_WIDGET(hildon_banner));
    
}
END_TEST

/* ----- Test case for show_progress -----*/
/**
 * Purpose: Check creation of new banner with progress bar with regular values.
 * Cases considered:
 *    - Create new progress banner with standard progress bar and "" as text.
 *    - Create new progress banner with NULL progress bar and TEST_STRING as text.
 */
START_TEST (test_show_progress_regular)
{
  gchar * text=NULL;
    
  GtkProgressBar * progress_bar = NULL;
  HildonBanner * hildon_banner = NULL;

  progress_bar = GTK_PROGRESS_BAR(gtk_progress_bar_new());

  /*Test 1: Create progress banner with TEST_STRING as text and basic progress_bar. */
  text = TEST_STRING;
  hildon_banner = HILDON_BANNER(hildon_banner_show_progress(b_window,progress_bar,text));

  fail_if(!HILDON_IS_BANNER(hildon_banner),
          "hildon-banner: hildon_banner_show_progress failed creating banner.");
    
  hildon_banner_set_text(hildon_banner,text);
  hildon_banner_set_fraction(hildon_banner,0.5);
  gtk_widget_destroy(GTK_WIDGET(hildon_banner));

  hildon_banner = NULL;

  /*Test 2: Create progress banner with "" as text and NULL progress_bar. */
  text = "";
  hildon_banner = HILDON_BANNER(hildon_banner_show_progress(b_window,NULL,text));
    
  fail_if(!HILDON_IS_BANNER(hildon_banner),
          "hildon-banner: hildon_banner_show_progress failed creating banner.");
    
  hildon_banner_set_text(hildon_banner,text);
  hildon_banner_set_fraction(hildon_banner,0.2);
  gtk_widget_destroy(GTK_WIDGET(hildon_banner));
    
}
END_TEST

/**
 * Purpose: Check creation of new banner with progress bar with invalid values
 * Cases considered:
 *    - Create new progress banner with NULL text.
 *    - Create new progress banner with NULL window.
 */
START_TEST (test_show_progress_invalid)
{
  gchar * text=NULL;
    
  GtkProgressBar * progress_bar = NULL;
  HildonBanner * hildon_banner = NULL;

  progress_bar = GTK_PROGRESS_BAR(gtk_progress_bar_new());
  /*Test 1: Create progress banner with TEST_STRING as text and basic progress_bar. */
  text = TEST_STRING;
  hildon_banner = HILDON_BANNER(hildon_banner_show_progress(b_window,progress_bar,NULL));

  /* NULL text avoid create correct banner. */
  fail_if(HILDON_IS_BANNER(hildon_banner),
          "hildon-banner: hildon_banner_show_progress failed creating banner.");
    
  /*Test 2: Create progress banner with TEST_STRING as text, NULL progress_bar and NULL window */
  text = TEST_STRING;
  hildon_banner = HILDON_BANNER(hildon_banner_show_progress(NULL,NULL,text));
    
  fail_if(!HILDON_IS_BANNER(hildon_banner),
          "hildon-banner: hildon_banner_show_progress failed creating banner.");
    
  hildon_banner_set_text(hildon_banner,text);
  hildon_banner_set_fraction(hildon_banner,0.2);

  gtk_widget_destroy(GTK_WIDGET(hildon_banner));
}
END_TEST

/* ---------- Suite creation ---------- */
Suite *create_hildon_banner_suite()
{
  /* Create the suite */
  Suite *s = suite_create("HildonBanner");

  /* Create test cases */
  TCase *tc1 = tcase_create("show_animation");
  TCase *tc2 = tcase_create("show_progress");

  /* Create unit tests for hildon_banner_show_animation and add it to the suite */
  tcase_add_checked_fixture(tc1, fx_setup_default_banner, fx_teardown_default_banner);
  tcase_add_test(tc1, test_show_animation_regular);
  tcase_add_test(tc1, test_show_animation_invalid);
  suite_add_tcase (s, tc1);

  /* Create unit tests for hildon_banner_show_animation and add it to the suite */
  tcase_add_checked_fixture(tc2, fx_setup_default_banner, fx_teardown_default_banner);
  tcase_add_test(tc2, test_show_progress_regular);
  tcase_add_test(tc2, test_show_progress_invalid);
  suite_add_tcase (s, tc2);



  /* Return created suite */
  return s;             
}

