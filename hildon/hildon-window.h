/*
 * This file is a part of hildon
 *
 * Copyright (C) 2006 Nokia Corporation, all rights reserved.
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

#ifndef                                         __HILDON_WINDOW_H__
#define                                         __HILDON_WINDOW_H__

#include                                        <gtk/gtk.h>
#include                                        "hildon-app-menu.h"
#include                                        "hildon-edit-toolbar.h"

G_BEGIN_DECLS

#ifndef HILDON_DISABLE_DEPRECATED
/**
 * HILDON_WINDOW_LONG_PRESS_TIME:
 *
 * Time (in miliseconds) after which a hardware key press is
 * considered a long press.
 *
 * Deprecated: Hildon 2.2: this value is only meant to be used
 * internally by Hildon.
 */
#define                                         HILDON_WINDOW_LONG_PRESS_TIME 800
#endif

#define                                         HILDON_TYPE_WINDOW \
                                                (hildon_window_get_type())

#define                                         HILDON_WINDOW(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST (obj, \
                                                HILDON_TYPE_WINDOW, HildonWindow))

#define                                         HILDON_WINDOW_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass),\
                                                HILDON_TYPE_WINDOW, HildonWindowClass))

#define                                         HILDON_IS_WINDOW(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE (obj, HILDON_TYPE_WINDOW))

#define                                         HILDON_IS_WINDOW_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), HILDON_TYPE_WINDOW))

#define                                         HILDON_WINDOW_GET_CLASS(obj) \
                                                ((HildonWindowClass *) G_OBJECT_GET_CLASS(obj))

typedef struct                                  _HildonWindow HildonWindow;

typedef struct                                  _HildonWindowClass HildonWindowClass;

#ifndef HILDON_DISABLE_DEPRECATED
/**
 * HildonWindowClipboardOperation:
 *
 * Deprecated: Hildon 2.2: this value is only meant to be used
 * internally by Hildon
 */
typedef enum
{
    HILDON_WINDOW_CO_COPY,
    HILDON_WINDOW_CO_CUT,
    HILDON_WINDOW_CO_PASTE
}                                               HildonWindowClipboardOperation;
#endif /* HILDON_DISABLE_DEPRECATED */

struct                                          _HildonWindow
{
    GtkWindow parent;
};

struct                                          _HildonWindowClass
{
    GtkWindowClass parent_class;

    /* opera hacks for clip board operation */
    void (*clipboard_operation)(HildonWindow *hwindow, int operation);
    gboolean (*toggle_menu)(HildonWindow * self, guint button, guint32 time);
    /* Padding for future extension */
    void (*_hildon_reserved1)(void);
    void (*_hildon_reserved2)(void);
};

GType G_GNUC_CONST
hildon_window_get_type                          (void);

GtkWidget*
hildon_window_new                               (void);

void        
hildon_window_add_with_scrollbar                (HildonWindow *self,
                                                 GtkWidget    *child);

void
hildon_window_set_main_menu                     (HildonWindow *self,
						 GtkMenu      *menu);

GtkMenu*
hildon_window_get_main_menu                     (HildonWindow *self);

void
hildon_window_set_app_menu                      (HildonWindow  *self,
                                                 HildonAppMenu *menu);

HildonAppMenu *
hildon_window_get_app_menu                      (HildonWindow *self);

#ifndef HILDON_DISABLE_DEPRECATED
void        
hildon_window_set_menu                          (HildonWindow *self,
                                                 GtkMenu      *menu);

GtkMenu*
hildon_window_get_menu                          (HildonWindow *self);
#endif

void    
hildon_window_add_toolbar                       (HildonWindow *self,
                                                 GtkToolbar   *toolbar);

void        
hildon_window_remove_toolbar                    (HildonWindow *self,
                                                 GtkToolbar   *toolbar);

void
hildon_window_set_edit_toolbar                  (HildonWindow      *self,
                                                 HildonEditToolbar *toolbar);

gboolean    
hildon_window_get_is_topmost                    (HildonWindow *self);

const gchar *
hildon_window_get_markup                        (HildonWindow *window);

void
hildon_window_set_markup                        (HildonWindow *window,
                                                 const gchar  *markup);

G_END_DECLS

#endif                                          /* __HILDON_WINDOW_H__ */
