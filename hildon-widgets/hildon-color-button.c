/*
 * This file is part of hildon-libs
 *
 * Copyright (C) 2005, 2006 Nokia Corporation.
 *
 * Contact: Michael Dominic Kostrzewa <michael.kostrzewa@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License.
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
 * SECTION:hildon-color-button
 * @short_description: A widget to open HildonColorChooserDialog
 * @see_also: #HildonColorChooserDialog, #HildonColorPopup
 *
 * HildonColorButton is a widget to open a HildonColorChooserDialog.
 * The selected color is shown in the button.
 * The selected color is a property of the button.
 * The property name is "color" and its type is GtkColor.
 */
#include <config.h>

#include <gtk/gtkbutton.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtkdrawingarea.h>
#include <gtk/gtksignal.h>
#include <gdk/gdkkeysyms.h>
#include <hildon-widgets/hildon-defines.h>

#include "hildon-color-button.h"
#include "hildon-color-chooser-dialog.h"

#define HILDON_COLOR_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE\
              ((obj), HILDON_TYPE_COLOR_BUTTON, HildonColorButtonPrivate))

#define COLOR_FILLED_HEIGHT 22
#define COLOR_FILLED_WIDTH  22

#define COLOR_BUTTON_WIDTH  52
#define COLOR_BUTTON_HEIGHT 48

/* the outer border color */
#define OUTER_BORDER_RED   0
#define OUTER_BORDER_BLUE  0
#define OUTER_BORDER_GREEN 0
#define OUTER_BORDER_THICKNESS 1

/* the inner border color */
#define INNER_BORDER_RED   65535
#define INNER_BORDER_BLUE  65535
#define INNER_BORDER_GREEN 65535
#define INNER_BORDER_THICKNESS 2

struct _HildonColorButtonPrivate 
{
  GtkWidget *dialog;

  GdkColor color;
  GdkGC *gc;
};

enum 
{
  PROP_NONE,
  PROP_COLOR
};

static void
hildon_color_button_class_init(HildonColorButtonClass *klass);
static void
hildon_color_button_init(HildonColorButton *color_button);

static void
hildon_color_button_finalize(GObject *object);
static void
hildon_color_button_set_property(GObject *object, guint param_id,
                                 const GValue *value, GParamSpec *pspec);
static void
hildon_color_button_get_property(GObject *object, guint param_id,
                       		 GValue *value, GParamSpec *pspec);
static void
hildon_color_button_realize(GtkWidget *widget);
static void
hildon_color_button_unrealize(GtkWidget *widget);
static void
hildon_color_button_clicked(GtkButton *button);
static gboolean
hildon_color_button_key_pressed(GtkWidget * button, 
                                GdkEventKey * event,
                                gpointer data);
static gint
hildon_color_field_expose_event(GtkWidget *widget, GdkEventExpose *event,
                                HildonColorButton *cb);

static gboolean
hildon_color_button_mnemonic_activate( GtkWidget *widget,
                                       gboolean group_cycling );


static gpointer parent_class = NULL;

GType
hildon_color_button_get_type(void)
{
  static GType color_button_type = 0;
  
  if (!color_button_type)
    {
      static const GTypeInfo color_button_info =
      {
        sizeof (HildonColorButtonClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) hildon_color_button_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (HildonColorButton),
        0,              /* n_preallocs */
        (GInstanceInitFunc) hildon_color_button_init,
      };
      
      color_button_type =
        g_type_register_static (GTK_TYPE_BUTTON, "HildonColorButton",
                                &color_button_info, 0);
    }
  
  return color_button_type;
}

static void
hildon_color_button_class_init(HildonColorButtonClass *klass)
{
  GObjectClass *gobject_class;
  GtkButtonClass *button_class;
  GtkWidgetClass *widget_class;
  
  gobject_class = G_OBJECT_CLASS (klass);
  button_class = GTK_BUTTON_CLASS (klass);
  widget_class = GTK_WIDGET_CLASS (klass);
  
  parent_class = g_type_class_peek_parent (klass);

  gobject_class->get_property = hildon_color_button_get_property;
  gobject_class->set_property = hildon_color_button_set_property;
  gobject_class->finalize = hildon_color_button_finalize;
  widget_class->realize = hildon_color_button_realize;
  widget_class->unrealize = hildon_color_button_unrealize;
  button_class->clicked = hildon_color_button_clicked;
  widget_class->mnemonic_activate = hildon_color_button_mnemonic_activate;
  
  /**
   * HildonColorButton:color:
   *
   * The selected color.
   */
  g_object_class_install_property (gobject_class, PROP_COLOR,
                                   g_param_spec_boxed ("color",
                                     "Current Color",
                                     "The selected color",
                                     GDK_TYPE_COLOR,
                                     G_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (HildonColorButtonPrivate));
}

/* Handle exposure events for the color picker's drawing area */
static gint
hildon_color_field_expose_event(GtkWidget *widget, GdkEventExpose *event,
                                HildonColorButton *cb)
{
    GdkColor outer_border, inner_border;

    /* Create the outer border color */
    outer_border.pixel = 0;
    outer_border.red   = OUTER_BORDER_RED;
    outer_border.blue  = OUTER_BORDER_BLUE;
    outer_border.green = OUTER_BORDER_GREEN;

    /* Create the inner border color */
    inner_border.pixel = 0;
    inner_border.red   = INNER_BORDER_RED;
    inner_border.blue  = INNER_BORDER_BLUE;
    inner_border.green = INNER_BORDER_GREEN;

    /* serve the outer border color to the Graphic Context */
    gdk_gc_set_rgb_fg_color(cb->priv->gc, &outer_border);
    /* draw the outer border as a filled rectangle */
    gdk_draw_rectangle(widget->window,
            cb->priv->gc,
            TRUE,
            event->area.x,
            event->area.y,
            event->area.width,
            event->area.height);

    /* serve the inner border color to the Graphic Context */
    gdk_gc_set_rgb_fg_color(cb->priv->gc, &inner_border);
    /* draw the inner border as a filled rectangle */
    gdk_draw_rectangle(widget->window,
            cb->priv->gc,
            TRUE,
            event->area.x + OUTER_BORDER_THICKNESS,
            event->area.y + OUTER_BORDER_THICKNESS,
            event->area.width  - (OUTER_BORDER_THICKNESS*2),
            event->area.height - (OUTER_BORDER_THICKNESS*2));

    /* serve the actual color to the Graphic Context */
    gdk_gc_set_rgb_fg_color(cb->priv->gc, &cb->priv->color);
    /* draw the actual rectangle */
    gdk_draw_rectangle(widget->window,
            cb->priv->gc,
            TRUE,
            event->area.x + (INNER_BORDER_THICKNESS + OUTER_BORDER_THICKNESS),
            event->area.y + (INNER_BORDER_THICKNESS + OUTER_BORDER_THICKNESS),
            event->area.width  - ((INNER_BORDER_THICKNESS + OUTER_BORDER_THICKNESS)*2),
            event->area.height - ((INNER_BORDER_THICKNESS + OUTER_BORDER_THICKNESS)*2));

    return FALSE;
}

static void
hildon_color_button_init(HildonColorButton *cb)
{
  GtkWidget *align;
  GtkWidget *drawing_area;
  
  cb->priv = HILDON_COLOR_BUTTON_GET_PRIVATE(cb);

  cb->priv->dialog = NULL;
  cb->priv->gc = NULL;

  gtk_widget_push_composite_child();
  
  /* create widgets and pixbuf */
  align = gtk_alignment_new(0.5, 0.5, 0, 0); /*composite widget*/

  drawing_area = gtk_drawing_area_new(); /*composite widget*/

  /* setting minimum sizes */
  gtk_widget_set_size_request(GTK_WIDGET(cb), COLOR_BUTTON_WIDTH,
                              COLOR_BUTTON_HEIGHT);
  gtk_widget_set_size_request(GTK_WIDGET(drawing_area),
                              COLOR_FILLED_WIDTH, COLOR_FILLED_HEIGHT);

  /* Connect the callback function for exposure event */
  g_signal_connect(drawing_area, "expose-event",
                   G_CALLBACK(hildon_color_field_expose_event), cb);

  /* Connect to callback function for key press event */
  g_signal_connect(G_OBJECT(cb), "key-press-event",
		   G_CALLBACK(hildon_color_button_key_pressed), cb);
  
  /* packing */
  gtk_container_add(GTK_CONTAINER(align), drawing_area);
  gtk_container_add(GTK_CONTAINER(cb), align);
  
  gtk_widget_show_all(align);
  
  gtk_widget_pop_composite_child();
}

/* Free memory used by HildonColorButton */
static void
hildon_color_button_finalize(GObject *object)
{
  HildonColorButton *cb = HILDON_COLOR_BUTTON(object);

  if (cb->priv->dialog)
  {
    gtk_widget_destroy(cb->priv->dialog);
    cb->priv->dialog = NULL;
  }

  if( G_OBJECT_CLASS(parent_class)->finalize )
    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void
hildon_color_button_realize(GtkWidget *widget)
{
  HildonColorButton *cb = HILDON_COLOR_BUTTON(widget);

  GTK_WIDGET_CLASS(parent_class)->realize(widget);

  cb->priv->gc = gdk_gc_new(widget->window);
}

static void
hildon_color_button_unrealize(GtkWidget *widget)
{
  HildonColorButton *cb = HILDON_COLOR_BUTTON(widget);

  g_object_unref(cb->priv->gc);
  cb->priv->gc = NULL;

  GTK_WIDGET_CLASS(parent_class)->unrealize(widget);
}

/* Make the widget sensitive with the keyboard event */
static gboolean
hildon_color_button_mnemonic_activate( GtkWidget *widget,
                                       gboolean group_cycling )
{
  gtk_widget_grab_focus( widget );
  return TRUE;
}

/* Popup a color selector dialog on button click */
static void
hildon_color_button_clicked(GtkButton *button)
{
  HildonColorButton *cb = HILDON_COLOR_BUTTON(button);
  HildonColorChooserDialog *cs_dialog = HILDON_COLOR_CHOOSER_DIALOG(cb->priv->dialog);
  
  /* Popup the color selector dialog */
  if (!cs_dialog)
  {
    /* The dialog hasn't been created yet, do it. */
    GtkWidget *parent = gtk_widget_get_toplevel(GTK_WIDGET(cb));
    cb->priv->dialog = hildon_color_chooser_dialog_new(GTK_WINDOW(parent));
    cs_dialog = HILDON_COLOR_CHOOSER_DIALOG(cb->priv->dialog);
    if (parent)
      gtk_window_set_transient_for(GTK_WINDOW(cs_dialog), GTK_WINDOW(parent));
  }

  /* Set the initial color for the color selector dialog */
  hildon_color_chooser_dialog_set_color(cs_dialog, &cb->priv->color);
 
  /* Update the color for color button if selection was made */
  if (gtk_dialog_run(GTK_DIALOG(cs_dialog)) == GTK_RESPONSE_OK)
  {
    hildon_color_chooser_dialog_get_color(cs_dialog, &cb->priv->color);
    hildon_color_button_set_color( HILDON_COLOR_BUTTON( button ), 
            &(cb->priv->color) );
  } else 
  {
          // FIXME: Fixes 39778, but I think the bug lies elsewhere
          gtk_widget_queue_draw (GTK_WIDGET (button)); 
  }

  gtk_widget_hide(GTK_WIDGET(cs_dialog));
}

/* Popup a color selector dialog on hardkey Select press */
static gboolean
hildon_color_button_key_pressed(GtkWidget * button, 
				 GdkEventKey * event,
				 gpointer data)
{
  g_return_val_if_fail (HILDON_IS_COLOR_BUTTON(button), FALSE);

  if (event->keyval == HILDON_HARDKEY_SELECT)
  {
    hildon_color_button_clicked(GTK_BUTTON(button));
    return TRUE;
  }
  
  return FALSE;
}

/* Set_property function for HildonColorButtonClass initialization */
static void
hildon_color_button_set_property(GObject *object, guint param_id,
                                 const GValue *value, GParamSpec *pspec)
{
  HildonColorButton *cb = HILDON_COLOR_BUTTON(object);

  switch (param_id) 
    {
    case PROP_COLOR:
      cb->priv->color = *(GdkColor*)g_value_get_boxed(value);
      gtk_widget_queue_draw(GTK_WIDGET(cb));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
      break;
    }
}

/* Get_property function for HildonColorButtonClass initialization */
static void
hildon_color_button_get_property(GObject *object, guint param_id,
                                 GValue *value, GParamSpec *pspec)
{
  HildonColorButton *cb = HILDON_COLOR_BUTTON(object);

  switch (param_id) 
    {
    case PROP_COLOR:
      g_value_set_boxed(value, &cb->priv->color);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
      break;
    }
}

/**
 * hildon_color_button_new:
 *
 * Creates a new color button. This returns a widget in the form of a
 * small button containing a swatch representing the selected color.
 * When the button is clicked, a color-selection dialog will open,
 * allowing the user to select a color. The swatch will be updated to
 * reflect the new color when the user finishes.
 *
 * Returns: a new color button
 */
GtkWidget *
hildon_color_button_new(void)
{
  return g_object_new( HILDON_TYPE_COLOR_BUTTON, NULL );
}

/**
 * hildon_color_button_new_with_color:
 * @color: a #GdkColor for the initial color
 *
 * Creates a new color button with @color as the initial color. 
 *
 * Returns: a new color button
 */
GtkWidget *
hildon_color_button_new_with_color(const GdkColor *color)
{
  return g_object_new( HILDON_TYPE_COLOR_BUTTON, "color", color, NULL );
}

/**
 * hildon_color_button_set_color:
 * @button: a #HildonColorButton
 * @color: a color to be set
 *
 * Sets the color selected by the button.
 */
void
hildon_color_button_set_color( HildonColorButton *button, GdkColor *color )
{
  g_object_set( G_OBJECT(button), "color", color, NULL );
}

/**
 * hildon_color_button_get_color:
 * @button: a #HildonColorButton
 *
 * Returns: the color selected by the button
 */
GdkColor *
hildon_color_button_get_color( HildonColorButton *button )
{
  GdkColor *color = NULL;
  g_object_get( G_OBJECT(button), "color", &color, NULL );
  return color;
}
