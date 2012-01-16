/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006 Nokia Corporation, all rights reserved.
 *
 * Contact: Rodrigo Novo <rodrigo.novo@nokia.com>
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
 * SECTION:hildon-defines
 * @Title: Defines
 * @Short_Description: Definitions for icon sizes, margins, and hard keys definitions.
 *
 * The macros of this section should be used for a consistent sizing
 * of icons and spacing of between user interface elements.
 *
 * The icon sizes here presented must be registered during the
 * application initialization by calling hildon_init(). It is
 * recommended to use these sizes for a consistent look of
 * applications in the Hildon platform.
 *
 * The margin definitions are used internally in the widgets to define the spacing
 * and padding between widgets; as well as the margins for dialogs and other windows.
 * Using them to create a new interface layout will guarantee it to be consistent
 * with the widgets and other applications.
 *
 * The hardkey macros are mapping between the Hildon supported keys and the GDK
 * key definitions.
 */

#ifndef                                         __HILDON_DEFINES_H__
#define                                         __HILDON_DEFINES_H__

#include                                        <gtk/gtk.h>
#include                                        <gdk/gdkkeysyms.h>

G_BEGIN_DECLS

/* New hildon icon sizes. */



/**
 * HILDON_ICON_SIZE_XSMALL:
 *
 * #GtkIconSize for extra small icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_SIZE_XSMALL                  gtk_icon_size_from_name ("hildon-xsmall")

/**
 * HILDON_ICON_SIZE_SMALL:
 *
 * #GtkIconSize for small icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_SIZE_SMALL                   gtk_icon_size_from_name ("hildon-small")

/**
 * HILDON_ICON_SIZE_STYLUS:
 *
 * #GtkIconSize for stylus-size icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_SIZE_STYLUS                  gtk_icon_size_from_name ("hildon-stylus")

/**
 * HILDON_ICON_SIZE_FINGER:
 *
 * #GtkIconSize for finger-size icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_SIZE_FINGER                  gtk_icon_size_from_name ("hildon-finger")

/**
 * HILDON_ICON_SIZE_THUMB:
 *
 * #GtkIconSize for thumb-size icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_SIZE_THUMB                   gtk_icon_size_from_name ("hildon-thumb")

/**
 * HILDON_ICON_SIZE_LARGE:
 *
 * #GtkIconSize for large icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_SIZE_LARGE                   gtk_icon_size_from_name ("hildon-large")

/**
 * HILDON_ICON_SIZE_XLARGE:
 *
 * #GtkIconSize for extra large icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_SIZE_XLARGE                  gtk_icon_size_from_name ("hildon-xlarge")

/* Actual icon sizes */

/* New sizes */
/**
 * HILDON_ICON_PIXEL_SIZE_XSMALL:
 *
 * Pixel size for extra small icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_PIXEL_SIZE_XSMALL            hildon_get_icon_pixel_size (HILDON_ICON_SIZE_XSMALL)

/**
 * HILDON_ICON_PIXEL_SIZE_SMALL:
 *
 * Pixel size for small icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_PIXEL_SIZE_SMALL             hildon_get_icon_pixel_size (HILDON_ICON_SIZE_SMALL)

/**
 * HILDON_ICON_PIXEL_SIZE_STYLUS:
 *
 * Pixel size for stylus-size icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_PIXEL_SIZE_STYLUS            hildon_get_icon_pixel_size (HILDON_ICON_SIZE_STYLUS)

/**
 * HILDON_ICON_PIXEL_SIZE_FINGER:
 *
 * Pixel size for finger-size icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_PIXEL_SIZE_FINGER            hildon_get_icon_pixel_size (HILDON_ICON_SIZE_FINGER)

/**
 * HILDON_ICON_PIXEL_SIZE_THUMB:
 *
 * Pixel size for thumb-size icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_PIXEL_SIZE_THUMB             hildon_get_icon_pixel_size (HILDON_ICON_SIZE_THUMB)

/**
 * HILDON_ICON_PIXEL_SIZE_LARGE:
 *
 * Pixel size for large icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_PIXEL_SIZE_LARGE             hildon_get_icon_pixel_size (HILDON_ICON_SIZE_LARGE)

/**
 * HILDON_ICON_PIXEL_SIZE_XLARGE:
 *
 * Pixel size for extra large icons.
 *
 * Since: 2.2
 */
#define HILDON_ICON_PIXEL_SIZE_XLARGE            hildon_get_icon_pixel_size (HILDON_ICON_SIZE_XLARGE)

/* Margins */

/**
 * HILDON_MARGIN_HALF:
 *
 * Half of the default pixel-size margin to be used for padding between widgets.
 *
 * Since: 2.2
 */
#define HILDON_MARGIN_HALF                       4

/**
 * HILDON_MARGIN_DEFAULT:
 *
 * Default pixel-size margin to be used for padding between widgets.
 *
 * Since: 2.2
 */
#define HILDON_MARGIN_DEFAULT                    8

/**
 * HILDON_MARGIN_DOUBLE:
 *
 * Double pixel-size margin to be used for padding between widgets.
 *
 * Since: 2.2
 */
#define HILDON_MARGIN_DOUBLE                     16

/**
 * HILDON_MARGIN_TRIPLE:
 *
 * Triple pixel-size margin to be used for padding between widgets.
 *
 * Since: 2.2
 */
#define HILDON_MARGIN_TRIPLE                     24

/**
 * HILDON_WINDOW_TITLEBAR_HEIGHT:
 *
 * Height of the window titlebar according to Hildon Desktop.
 *
 * Since: 2.2
 */
#define HILDON_WINDOW_TITLEBAR_HEIGHT            56

/* Hard keys */

/**
 * HILDON_HARDKEY_UP:
 *
 * Key definition for the Up hardkey.
 */
#define HILDON_HARDKEY_UP                        GDK_KEY_Up

/**
 * HILDON_HARDKEY_LEFT:
 *
 * Key definition for the Left hardkey.
 */
#define HILDON_HARDKEY_LEFT                      GDK_KEY_Left

/**
 * HILDON_HARDKEY_RIGHT:
 *
 * Key definition for the Right hardkey.
 */
#define HILDON_HARDKEY_RIGHT                     GDK_KEY_Right

/**
 * HILDON_HARDKEY_DOWN:
 *
 * Key definition for the Down hardkey.
 */
#define HILDON_HARDKEY_DOWN                      GDK_KEY_Down

/**
 * HILDON_HARDKEY_SELECT:
 *
 * Key definition for the Select hardkey.
 */
#define HILDON_HARDKEY_SELECT                    GDK_KEY_Return

/**
 * HILDON_HARDKEY_Menu:
 *
 * Key definition for the Menu hardkey.
 */
#define HILDON_HARDKEY_MENU                      GDK_KEY_F4

/**
 * HILDON_HARDKEY_HOME:
 *
 * Key definition for the Home hardkey.
 */
#define HILDON_HARDKEY_HOME                      GDK_KEY_F5

/**
 * HILDON_HARDKEY_ESC:
 *
 * Key definition for the Esc hardkey.
 */
#define HILDON_HARDKEY_ESC                       GDK_KEY_Escape

/**
 * HILDON_HARDKEY_FULLSCREEN:
 *
 * Key definition for the Fullscreen hardkey.
 */
#define HILDON_HARDKEY_FULLSCREEN                GDK_KEY_F6

/**
 * HILDON_HARDKEY_INCREASE:
 *
 * Key definition for the Increase hardkey.
 */
#define HILDON_HARDKEY_INCREASE                  GDK_KEY_F7

/**
 * HILDON_HARDKEY_DECREASE:
 *
 * Key definition for the Decrease hardkey.
 */
#define HILDON_HARDKEY_DECREASE                  GDK_KEY_F8

gint
hildon_get_icon_pixel_size                      (GtkIconSize size);

G_END_DECLS

#endif                                          /* HILDON_DEFINES_H */
