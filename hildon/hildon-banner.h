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

#ifndef                                         __HILDON_BANNER_H__
#define                                         __HILDON_BANNER_H__

#include                                        <gtk/gtk.h>

G_BEGIN_DECLS 

#define                                         HILDON_TYPE_BANNER \
                                                (hildon_banner_get_type())

#define                                         HILDON_BANNER(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                                                HILDON_TYPE_BANNER, HildonBanner))

#define                                         HILDON_IS_BANNER(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
                                                HILDON_TYPE_BANNER))

typedef struct                                  _HildonBanner HildonBanner;

typedef struct                                  _HildonBannerClass HildonBannerClass;

struct                                          _HildonBanner
{
   GtkWindow parent;
};

struct                                          _HildonBannerClass
{
   GtkWindowClass parent_class;
};

GType G_GNUC_CONST
hildon_banner_get_type                          (void);

GtkWidget* 
hildon_banner_show_information                  (GtkWidget *widget, 
                                                 const gchar *icon_name,
                                                 const gchar *text);

GtkWidget*
hildon_banner_show_informationf                 (GtkWidget *widget, 
                                                 const gchar *icon_name,
                                                 const gchar *format, 
                                                 ...);

GtkWidget*
hildon_banner_show_information_override_dnd     (GtkWidget *widget,
                                                 const gchar *text);

GtkWidget*       
hildon_banner_show_information_with_markup      (GtkWidget *widget, 
                                                 const gchar *icon_name,
                                                 const gchar *markup);

#ifndef HILDON_DISABLE_DEPRECATED
GtkWidget*
hildon_banner_show_animation                    (GtkWidget *widget, 
                                                 const gchar *animation_name,
                                                 const gchar *text);

GtkWidget*
hildon_banner_show_progress                     (GtkWidget *widget, 
                                                 GtkProgressBar *bar,
                                                 const gchar *text);
#endif /* HILDON_DISABLE_DEPRECATED */

void      
hildon_banner_set_text                          (HildonBanner *self,
                                                 const gchar *text);

void      
hildon_banner_set_markup                        (HildonBanner *self,
                                                 const gchar *markup);

#ifndef HILDON_DISABLE_DEPRECATED
void     
hildon_banner_set_fraction                      (HildonBanner *self,
                                                 gdouble fraction);

void     
hildon_banner_set_icon                          (HildonBanner *self,
                                                 const gchar *icon_name);

void     
hildon_banner_set_icon_from_file                (HildonBanner *self,
                                                 const gchar *icon_file);
#endif /* HILDON_DISABLE_DEPRECATED */

void
hildon_banner_set_timeout                       (HildonBanner *self,
                                                 guint timeout);

GtkWidget *
hildon_banner_show_custom_widget                (GtkWidget *widget,
                                                 GtkWidget *custom_widget);

G_END_DECLS

#endif                                          /* __HILDON_BANNER_H__ */
