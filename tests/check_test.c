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

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlibint.h>
#include <gtk/gtk.h>
#include <check.h>
#include <gconf/gconf-client.h>

#include "test_suites.h"

/* Define environment checking results defines */
#define ENVIRONMENT_X_ERROR       1
#define ENVIRONMENT_MAEMO_ERROR   2
#define ENVIRONMENT_OK            3

/* This is used for the Gconf availability check */
#define GCONF_TEST_PATH "/hildon/tests/flag"

/* ------------------------ Helper functions ------------------------ */

/**
 * Creates the list of suites to be run.
 */
static SRunner *
configure_tests(gint environment)
{
  SRunner *sr;
  
  /* Create srunner object with the first test suite */

  sr = srunner_create(create_hildon_caption_suite());

  srunner_add_suite(sr, create_hildon_find_toolbar_suite());
  srunner_add_suite(sr, create_hildon_note_suite());
  srunner_add_suite(sr, create_hildon_wizard_dialog_suite());
  /* srunner_add_suite(sr, create_hildon_scroll_area_suite()); */
  srunner_add_suite(sr, create_hildon_window_suite());
  srunner_add_suite(sr, create_hildon_helper_suite());
  srunner_add_suite(sr, create_hildon_picker_button_suite());

  /* Disable tests that need maemo environment to be up if it is not running */
  if (environment != ENVIRONMENT_MAEMO_ERROR)
    {
      /* srunner_add_suite(sr, create_hildon_system_sound_suite()); */
      /* srunner_add_suite(sr, create_hildon_color_selector_suite()); */
      srunner_add_suite(sr, create_hildon_program_suite());
    }
 
  return sr;
}


/**
 * Checks environment configuration for tests execution
 */
static gint 
check_environment()
{
  Display *display = NULL;
  GConfClient *client = NULL;
  GError *gconf_error = NULL;

  /* Check X server availability */
  if ((display = XOpenDisplay(NULL)) == NULL)
    return ENVIRONMENT_X_ERROR;
  else
    XCloseDisplay(display);

  /* Check maemo environment is up. We do this checking gconf is available */
  g_type_init();
  client = gconf_client_get_default();
  gconf_client_get(client, GCONF_TEST_PATH ,&gconf_error);
  if (gconf_error)
    return ENVIRONMENT_MAEMO_ERROR;

  /* Environment is ok */
  return ENVIRONMENT_OK;
}

/**
 * --------------------------------------------------------------------------
 * Main program
 * --------------------------------------------------------------------------
 */
int main(void)
{
  int nf = 0;
  gint environment = 0;

  /* Show test start header */
  printf("\n");
  printf("******************************************************************\n");
  printf(" Executing hildon-libs unit tests.... \n");
  printf("******************************************************************\n\n");

  /* Check environment is ok to run the tests */
  environment = check_environment();
  if (environment == ENVIRONMENT_X_ERROR)
    {
      printf("\n-------------------------- ERROR ------------------------------------");
      printf("\nNo X server found. Check you have an X server up and running and the");
      printf("\nDISPLAY environment variable set properly.");
      printf("\n---------------------------------------------------------------------\n");
      return EXIT_FAILURE;
    }
  else if (environment == ENVIRONMENT_MAEMO_ERROR)
    {
      printf("\n------------------------- WARNING -----------------------------------");
      printf("\nMaemo environment is not running. Some tests that depend on Gconf and");
      printf("\nDbus will be disabled. To fix this you should startup the environment");
      printf("\nexecuting \"af-sb-init.sh start\" before running the tests.");
      printf("\n---------------------------------------------------------------------\n");
    }

  /* Configure test suites to be executed */
  SRunner *sr = configure_tests(environment);

  /* Run tests */
  srunner_run_all(sr, CK_NORMAL);//CK_VERBOSE);

  /* Retrieve number of failed tests */
  nf = srunner_ntests_failed(sr);

  /* Free resouces */
  srunner_free(sr);

  /* Return global success or failure */
  return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE; 
}
