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
#include <gtk/gtk.h>
#include "test_suites.h"

#include <hildon/hildon-program.h>


/* -------------------- Fixtures -------------------- */

static HildonProgram *program = NULL;

static void 
fx_setup_default_program ()
{
  int argc = 0;
  gtk_init(&argc, NULL);

  program = hildon_program_get_instance();
  /* Check program object has been created properly */
  fail_if(!HILDON_IS_PROGRAM(program),
          "hildon-program: Creation failed.");

}

static void 
fx_teardown_default_program ()
{
  g_object_unref(program);
}


/* -------------------- Test cases -------------------- */

/* ----- Test case for hildon_program_add_window -----*/

/**
 * Purpose: Test regular usage of the add_window interface
 * Cases considered:
 *    - Add a window object to the program
 *    - Add another window object to the program
 *    - Add the same window object to the program
 */
START_TEST (test_add_window_regular)
{
  HildonWindow *window; 
  HildonWindow *window1; 

  /* Test1: Add a window object to the program */  
  window = HILDON_WINDOW(hildon_window_new());
  hildon_program_add_window(program, window);

  /* Test2: Add another window object to the program */  
  window1 = HILDON_WINDOW(hildon_window_new());
  hildon_program_add_window(program, window1);

  /* Test3: Add the same window object to the program */  
  hildon_program_add_window(program, window);
  hildon_program_remove_window(program, window);

  gtk_widget_destroy (GTK_WIDGET (window));
  gtk_widget_destroy (GTK_WIDGET (window1));
}
END_TEST

/**
 * Purpose: Check invalid values of the add_window interface
 * Cases considered:
 *    - Add to a NULL program
 *    - Add a NULL window
 *    - Add a label instead of a window
 */
START_TEST (test_add_window_invalid)
{
  GtkWidget *label;

  /* Test1: Add to a NULL program */  
  hildon_program_add_window(NULL, NULL);

  /* Test2: Add a NULL window */
  hildon_program_add_window(program, NULL);

  /* Test3: Add a label instead of a window */
  label = gtk_label_new("This is an invalid example widget");
  hildon_program_add_window(program, HILDON_WINDOW(label));

  gtk_widget_destroy (GTK_WIDGET (label));
}
END_TEST

/* ----- Test case for hildon_program_remove_window -----*/

/**
 * Purpose: Test regular usage of the remove_window interface
 * Cases considered:
 *    - Add a window object to the program and remove it
 *    - Add another window object to the program and remove the first one
 *    - Remove a window two times
 */
START_TEST (test_remove_window_regular)
{
  HildonWindow *window; 
  HildonWindow *window1; 

  /* Test1: Add a window object to the program and remove it */  
  window = HILDON_WINDOW(hildon_window_new());
  hildon_program_add_window(program, window);
  hildon_program_remove_window(program, window);

  /* Test2: Add another window object to the program and remove the first one */  
  window1 = HILDON_WINDOW(hildon_window_new());
  hildon_program_add_window(program, window);
  hildon_program_add_window(program, window1);
  hildon_program_remove_window(program, window);

  /* Test3: Remove a window two times */  
  hildon_program_remove_window(program, window1);
  hildon_program_remove_window(program, window1);

  gtk_widget_destroy (GTK_WIDGET (window));
  gtk_widget_destroy (GTK_WIDGET (window1));
}
END_TEST

/**
 * Purpose: Check invalid values of the remove_window interface
 * Cases considered:
 *    - Remove from a NULL program
 *    - Remove a NULL window
 *    - Remove a label instead of a window
 */
START_TEST (test_remove_window_invalid)
{
  GtkWidget *label;

  /* Test1: Remove from a NULL program */
  hildon_program_remove_window(NULL, NULL);

  /* Test2: Remove a NULL window */  
  hildon_program_remove_window(program, NULL);

  /* Test3: Remove a label instead of a window */  
  label = gtk_label_new("This is an invalid example widget");
  hildon_program_remove_window(program, HILDON_WINDOW(label));

  gtk_widget_destroy (GTK_WIDGET (label));
}
END_TEST

/* ----- Test case for hildon_program_set_can_hibernate -----*/

/**
 * Purpose: Test regular usage of the set_cant_hibernate interface
 * Cases considered:
 *    - Test the initial value of the property, it must be FALSE
 *    - Set a value and test if the value is correct
 */
START_TEST (test_set_can_hibernate_regular)
{

  /* Test1: Test the initial value of the property, it must be FALSE */  
  fail_if(hildon_program_get_can_hibernate(program), 
          "hildon-program: The initial value of the hibernate property must be FALSE and it is not");

  /* Test2: Set a value and test if the value is correct */  
  hildon_program_set_can_hibernate(program, TRUE);
  fail_if(!hildon_program_get_can_hibernate(program), 
          "hildon-program: We set the hibernate property to TRUE and it is FALSE");
    
}
END_TEST

/**
 * Purpose: Check invalid values of the set_can_hibernate interface
 * Cases considered:
 *    - Set the property to a NULL object
 */
START_TEST (test_set_can_hibernate_invalid)
{
  hildon_program_set_can_hibernate(NULL, TRUE);
}
END_TEST

/* ---------- Suite creation ---------- */

Suite *create_hildon_program_suite()
{
  /* Create the suite */
  Suite *s = suite_create("HildonProgram");

  /* Create test cases */
  TCase *tc1 = tcase_create("add_window");
  TCase *tc2 = tcase_create("remove_window");
  TCase *tc3 = tcase_create("set_can_hibernate");

  /* Create test case for add_window and add it to the suite */
  tcase_add_checked_fixture(tc1, fx_setup_default_program, fx_teardown_default_program);
  tcase_add_test(tc1, test_add_window_regular);
  tcase_add_test(tc1, test_add_window_invalid);
  suite_add_tcase (s, tc1);

  /* Create test case for remove_window and add it to the suite */
  tcase_add_checked_fixture(tc2, fx_setup_default_program, fx_teardown_default_program);
  tcase_add_test(tc2, test_remove_window_regular);
  tcase_add_test(tc2, test_remove_window_invalid);
  suite_add_tcase (s, tc2);

  /* Create test case for set_can_hibernate and add it to the suite */
  tcase_add_checked_fixture(tc3, fx_setup_default_program, fx_teardown_default_program);
  tcase_add_test(tc3, test_set_can_hibernate_regular);
  tcase_add_test(tc3, test_set_can_hibernate_invalid);
  suite_add_tcase (s, tc3);

  /* Return created suite */
  return s;
}
