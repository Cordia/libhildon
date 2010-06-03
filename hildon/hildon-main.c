/*
 * This file is part of the hildon library
 *
 * Copyright (C) 2008, 2009 Nokia Corporation, all rights reserved.
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

/**
 * SECTION:hildon-main
 * @Short_Description: Library initialization.
 *
 * Before using Hildon, you need to initialize it; initialization connects
 * to the window system display, and parses some standard command line
 * arguments. See also gtk_init() to know more details on this topic.
 *
 * Hildon should be initialized by using hildon_gtk_init(). Note that
 * this function also initializes gtk by calling gtk_init(). In case
 * you need a customized initialization of the GTK+ library you have
 * to use hildon_init() after the customized GTK+ initialization.
 *
 * <example>
 * <title>Typical <function>main</function> function for a Hildon application</title>
 *   <programlisting>
 * int
 * main (int argc, char **argv)
 * {
 *   /<!-- -->* Initialize i18n support *<!-- -->/
 *   gtk_set_locale (<!-- -->);
 * <!-- -->
 *   /<!-- -->* Initialize the widget set *<!-- -->/
 *   hildon_gtk_init (&amp;argc, &amp;argv);
 * <!-- -->
 *   /<!-- -->* Create the main window *<!-- -->/
 *   mainwin = hildon_stackable_window_new();
 * <!-- -->
 *   /<!-- -->* Set up our GUI elements *<!-- -->/
 *  ...
 * <!-- -->
 *   /<!-- -->* Show the application window *<!-- -->/
 *   gtk_widget_show_all (mainwin);
 * <!-- -->
 *   /<!-- -->* Enter the main event loop, and wait for user interaction *<!-- -->/
 *   gtk_main (<!-- -->);
 * <!-- -->
 *   /<!-- -->* The user lost interest *<!-- -->/
 *   return 0;
 *}
 *  </programlisting>
 * </example>
 */


#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "hildon-main.h"
#include "hildon-stock.h"

#define GETTEXT_PACKAGE "hildon-libs"

static const GtkStockItem hildon_items[] = {
  { HILDON_STOCK_DATE, N_("wdgt_ti_date"), 0, 0, GETTEXT_PACKAGE },
  { HILDON_STOCK_TIME, N_("wdgt_ti_time"), 0, 0, GETTEXT_PACKAGE },
  { HILDON_STOCK_STOP, N_("wdgt_bd_stop"), 0, 0, GETTEXT_PACKAGE },
  { HILDON_STOCK_YES,  N_("wdgt_bd_yes"),  0, 0, GETTEXT_PACKAGE },
  { HILDON_STOCK_NO,   N_("wdgt_bd_no"),   0, 0, GETTEXT_PACKAGE },
  { HILDON_STOCK_DONE, N_("wdgt_bd_done"), 0, 0, GETTEXT_PACKAGE },
  { HILDON_STOCK_SORT, N_("wdgt_bd_sort"), 0, 0, GETTEXT_PACKAGE },
  { HILDON_STOCK_FINISH,   N_("wdgt_bd_finish"),   0, 0, GETTEXT_PACKAGE },
  { HILDON_STOCK_PREVIOUS, N_("wdgt_bd_previous"), 0, 0, GETTEXT_PACKAGE },
  { HILDON_STOCK_NEXT, N_("wdgt_bd_next"), 0, 0, GETTEXT_PACKAGE },
};

/**
 * hildon_init:
 *
 * Initializes the hildon library. Call this function after calling gtk_init()
 * and before using any hildon or GTK+ functions in your program.
 *
 * Since: 2.2
 **/
void
hildon_init (void)
{
  static gboolean initialized = FALSE;

  if (initialized) {
    g_critical ("Tried to initialized Hildon more than once.");
    return;
  } else {
    initialized = TRUE;
  }

  /* Register icon sizes */
  gtk_icon_size_register ("hildon-xsmall", 16, 16);
  gtk_icon_size_register ("hildon-small", 24, 24);
  gtk_icon_size_register ("hildon-stylus", 32, 32);
  gtk_icon_size_register ("hildon-finger", 48, 48);
  gtk_icon_size_register ("hildon-thumb", 64, 64);
  gtk_icon_size_register ("hildon-large", 96, 96);
  gtk_icon_size_register ("hildon-xlarge", 128, 128);

  /* Add Hildon stock items */
  gtk_stock_add_static (hildon_items, G_N_ELEMENTS (hildon_items));
}

/**
 * hildon_gtk_init:
 * @argc: Address of the <parameter>argc</parameter>
 * parameter of your main() function. Changed if any arguments were
 * handled.
 * @argv: Address of the <parameter>argv</parameter>
 * parameter of main().  Any parameters understood by gtk_init()
 * are stripped before return.
 *
 * Convenience function to initialize the GTK+ and hildon
 * libraries. Use this function to replace a call to gtk_init() and also
 * initialize the hildon library. See hildon_init() and gtk_init() for details.
 *
 * Since: 2.2
 **/
void
hildon_gtk_init (int *argc, char ***argv)
{
  gtk_init (argc, argv);
  hildon_init ();
}
