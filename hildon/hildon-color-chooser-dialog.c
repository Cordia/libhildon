/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006 Nokia Corporation, all rights reserved.
 *
 * Author: Kuisma Salonen <kuisma.salonen@nokia.com>
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
 * SECTION:hildon-color-chooser-dialog
 * @short_description: A dialog used to select a color from HSV colorspace.
 * @see_also: #HildonColorButton
 *
 * HildonColorChooserDialog enables the user to 
 * select an arbitrary color from a HSV colorspace.
 * The color is stored in one of the predefined color 
 * slots and can be reselected later on. 
 *
 * Additionally the user can choose one of the standard "factory"
 * colors.
 *
 */

#undef                                          HILDON_DISABLE_DEPRECATED

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <memory.h>
#include                                        <string.h>
#include                                        <libintl.h>
#include                                        <gdk/gdkkeysyms.h>

#include                                        "hildon-color-chooser-dialog.h"
#include                                        "hildon-color-chooser.h"
#include                                        "hildon-defines.h"
#include                                        "hildon-banner.h"
#include                                        "hildon-stock.h"
#include                                        "hildon-color-chooser-dialog-private.h"

#define                                         _(String) dgettext("hildon-libs", String)

static HildonColorChooserDialogClass*           parent_class = NULL;

/* darkened EGA palette to be used as predefined colors if style doesn't
   define anything else (darker colors are darkened 0x8000 -> 0x6666) */
static GdkColor hardcoded_colors[16] =          {{0, 0x0000, 0x0000, 0x0000},
                                                 {0, 0x6666, 0x6666, 0x6666},
                                                 {0, 0x6666, 0x0000, 0x0000},
                                                 {0, 0x0000, 0x6666, 0x0000},
                                                 {0, 0x0000, 0x0000, 0x6666},
                                                 {0, 0x6666, 0x6666, 0x0000},
                                                 {0, 0x6666, 0x0000, 0x6666},
                                                 {0, 0x0000, 0x6666, 0x6666},
                                                 {0, 0xffff, 0xffff, 0xffff},
                                                 {0, 0xc000, 0xc000, 0xc000},
                                                 {0, 0xffff, 0x0000, 0x0000},
                                                 {0, 0x0000, 0xffff, 0x0000},
                                                 {0, 0x0000, 0x0000, 0xffff},
                                                 {0, 0xffff, 0xffff, 0x0000},
                                                 {0, 0xffff, 0x0000, 0xffff},
                                                 {0, 0x0000, 0xffff, 0xffff}};

static void
hildon_color_chooser_dialog_init                (HildonColorChooserDialog *object);

static void 
hildon_color_chooser_dialog_class_init          (HildonColorChooserDialogClass *klass);

static void 
hildon_color_chooser_dialog_size_request        (GtkWidget *widget, 
                                                 GtkRequisition *req);

static void
hildon_color_chooser_dialog_size_allocate       (GtkWidget *widget, 
                                                 GtkAllocation *alloc);

static void 
hildon_color_chooser_dialog_realize             (GtkWidget *widget);

static void
hildon_color_chooser_dialog_unrealize           (GtkWidget *widget);

static void 
hildon_color_chooser_dialog_style_set           (GtkWidget *widget, 
                                                 GtkStyle *previous_style);

static void 
hildon_color_chooser_dialog_show                (GtkWidget *widget);

static void 
hildon_color_chooser_dialog_show_all            (GtkWidget *widget);

static gboolean 
hildon_color_chooser_dialog_key_press_event     (GtkWidget *widget, 
                                                 GdkEventKey *event);

static gboolean 
hildon_color_chooser_dialog_key_release_event   (GtkWidget *widget, 
                                                 GdkEventKey *event);

static void 
hildon_color_chooser_dialog_destroy             (GtkObject *object);

static gboolean 
hildon_color_chooser_dialog_area_expose         (GtkWidget *widget, 
                                                 GdkEventExpose *event, 
                                                 gpointer data);

static gboolean 
hildon_color_chooser_dialog_area_button_press   (GtkWidget *widget, 
                                                 GdkEventButton *event, 
                                                 gpointer data);

static void 
hildon_color_chooser_dialog_color_changed       (HildonColorChooser *chooser, 
                                                 gpointer data);

static void 
hildon_color_chooser_dialog_insensitive_press   (GtkWidget *widget, 
                                                 gpointer data);

static void 
hildon_color_chooser_dialog_refresh_style_info  (HildonColorChooserDialog *dialog);

static void 
hildon_color_chooser_dialog_set_color_num       (HildonColorChooserDialog *dialog, 
                                                 gint num);

static void 
hildon_color_chooser_dialog_ascii_hex_to_color  (gchar *s, 
                                                 GdkColor *color);

static void 
hildon_color_chooser_dialog_color_to_ascii_hex  (gchar *s, 
                                                 GdkColor *color);

GType G_GNUC_CONST
hildon_color_chooser_dialog_get_type            (void)
{
    static GType dialog_type = 0;

    if (!dialog_type) {
        static const GTypeInfo dialog_info =
        {
            sizeof (HildonColorChooserDialogClass),
            NULL,
            NULL,
            (GClassInitFunc) hildon_color_chooser_dialog_class_init,
            NULL,
            NULL,
            sizeof (HildonColorChooserDialog),
            0,
            (GInstanceInitFunc) hildon_color_chooser_dialog_init,
            NULL
        };

        dialog_type = g_type_register_static (GTK_TYPE_DIALOG, 
                "HildonColorChooserDialog", &dialog_info, 0);
    }

    return dialog_type;
}

static void 
hildon_color_chooser_dialog_init                (HildonColorChooserDialog *object)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (object);

    gtk_dialog_set_has_separator (GTK_DIALOG (object), FALSE);
    gtk_window_set_title (GTK_WINDOW (object), _("ecdg_ti_colour_selector"));

    g_assert (priv);

    priv->hbox = gtk_hbox_new (FALSE, 0);
    priv->vbox = gtk_vbox_new (FALSE, 0);
    priv->chooser = hildon_color_chooser_new ();

    gtk_box_pack_start (GTK_BOX (priv->hbox), priv->chooser, TRUE, TRUE, 0);
    gtk_box_pack_end (GTK_BOX (priv->hbox), priv->vbox, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (object)->vbox), priv->hbox, TRUE, TRUE, 0);


    priv->align_custom = gtk_alignment_new (0.5, 1.0, 0.0, 0.0);
    priv->align_defined = gtk_alignment_new (0.5, 1.0, 0.0, 0.0);

    priv->area_custom = gtk_drawing_area_new ();
    priv->area_defined = gtk_drawing_area_new ();

    gtk_container_add (GTK_CONTAINER (priv->align_custom), priv->area_custom);
    gtk_container_add (GTK_CONTAINER (priv->align_defined), priv->area_defined);

    priv->separator = gtk_hseparator_new ();

    gtk_box_pack_start (GTK_BOX (priv->vbox), priv->align_defined, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (priv->vbox), priv->separator, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (priv->vbox), priv->align_custom, FALSE, FALSE, 0);

    gtk_dialog_add_button (GTK_DIALOG (object), HILDON_STOCK_DONE, GTK_RESPONSE_OK);
    gtk_dialog_set_default_response (GTK_DIALOG (object), GTK_RESPONSE_OK);

    g_signal_connect (G_OBJECT (priv->chooser), 
            "query-tooltip", G_CALLBACK (hildon_color_chooser_dialog_insensitive_press), object);

    g_signal_connect (G_OBJECT (priv->area_custom), 
            "expose-event", G_CALLBACK (hildon_color_chooser_dialog_area_expose), object);

    g_signal_connect (G_OBJECT (priv->area_defined), 
            "expose-event", G_CALLBACK (hildon_color_chooser_dialog_area_expose), object);

    g_signal_connect (G_OBJECT (priv->area_custom), 
            "button-press-event", G_CALLBACK (hildon_color_chooser_dialog_area_button_press), object);

    g_signal_connect (G_OBJECT (priv->area_defined), 
            "button-press-event", G_CALLBACK (hildon_color_chooser_dialog_area_button_press), object);

    g_signal_connect (G_OBJECT (priv->chooser), 
            "color-changed", G_CALLBACK (hildon_color_chooser_dialog_color_changed), object);

    gtk_widget_add_events (priv->area_custom, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events (priv->area_defined, GDK_BUTTON_PRESS_MASK);

    priv->selected = 0;
    priv->gconf_client = gconf_client_get_default ();

    memset (&priv->style_info, 0, sizeof (HildonColorChooserStyleInfo));
    
    priv->colors_custom = NULL;
    priv->colors_defined = NULL;
    priv->gc_array = NULL;

    priv->has_style = 0;
}

static void
hildon_color_chooser_dialog_class_init          (HildonColorChooserDialogClass *klass)
{
    GtkWidgetClass *widget_klass = GTK_WIDGET_CLASS (klass);
    GtkObjectClass *object_klass = GTK_OBJECT_CLASS (klass);
    gchar tmp[32];
    gint i;

    widget_klass->size_request      = hildon_color_chooser_dialog_size_request;
    widget_klass->size_allocate     = hildon_color_chooser_dialog_size_allocate;
    widget_klass->realize           = hildon_color_chooser_dialog_realize;
    widget_klass->unrealize         = hildon_color_chooser_dialog_unrealize;
    widget_klass->style_set         = hildon_color_chooser_dialog_style_set;
    widget_klass->show              = hildon_color_chooser_dialog_show;
    widget_klass->show_all          = hildon_color_chooser_dialog_show_all;
    widget_klass->key_press_event   = hildon_color_chooser_dialog_key_press_event;
    widget_klass->key_release_event = hildon_color_chooser_dialog_key_release_event;


    object_klass->destroy           = hildon_color_chooser_dialog_destroy;

    parent_class = g_type_class_peek_parent (klass);

    gtk_widget_class_install_style_property (widget_klass,
            g_param_spec_boxed ("container_sizes",
                "Container sizes",
                "Container specific sizes",
                GTK_TYPE_BORDER,
                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_klass,
            g_param_spec_boxed ("radio_sizes",
                "Color radio sizes",
                "Color radio specific sizes",
                GTK_TYPE_BORDER,
                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_klass,
            g_param_spec_boxed ("num_buttons",
                "Number of buttons",
                "Number of color store buttons",
                GTK_TYPE_BORDER,
                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_klass,
            g_param_spec_boxed ("default_color", "Default color",
                "Default color for nonpainted custom colors",
                GDK_TYPE_COLOR,
                G_PARAM_READABLE));

    for (i = 0; i < 32; i++) {
        memset (tmp, 0, 32);
        g_snprintf (tmp, 32, "defined_color%d", i);

        gtk_widget_class_install_style_property (widget_klass,
                g_param_spec_boxed (tmp, "Defined color",
                    "Pre-defined colors for the dialog",
                    GDK_TYPE_COLOR,
                    G_PARAM_READABLE));
    }

    g_type_class_add_private (object_klass, sizeof (HildonColorChooserDialogPrivate));
}

static void 
hildon_color_chooser_dialog_size_request        (GtkWidget *widget, 
                                                 GtkRequisition *req)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (widget);

    g_assert (priv);

    gtk_container_set_border_width (GTK_CONTAINER (priv->hbox), priv->style_info.cont_sizes.left);

    gtk_box_set_spacing (GTK_BOX (priv->hbox), priv->style_info.cont_sizes.right);
    gtk_box_set_spacing (GTK_BOX (priv->vbox), priv->style_info.cont_sizes.top);
    gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (widget)->vbox), priv->style_info.cont_sizes.bottom);

    gtk_widget_set_size_request (priv->area_custom,
            (priv->style_info.radio_sizes.left + 
             2 * priv->style_info.radio_sizes.bottom) * 
            (priv->style_info.num_buttons.top) +
            (priv->style_info.num_buttons.top-1) *
            priv->style_info.radio_sizes.top,
            (priv->style_info.radio_sizes.right + 
             2 * priv->style_info.radio_sizes.bottom) * 
            (priv->style_info.num_buttons.bottom) +
            (priv->style_info.num_buttons.bottom-1) * 
            priv->style_info.radio_sizes.top);

    gtk_widget_set_size_request (priv->area_defined,
            (priv->style_info.radio_sizes.left + 
             2 * priv->style_info.radio_sizes.bottom) * 
            (priv->style_info.num_buttons.left) +
            (priv->style_info.num_buttons.left-1) * 
            priv->style_info.radio_sizes.top,
            (priv->style_info.radio_sizes.right +  
             2 * priv->style_info.radio_sizes.bottom) * 
            (priv->style_info.num_buttons.right) +
            (priv->style_info.num_buttons.right-1) * 
            priv->style_info.radio_sizes.top);

    GTK_WIDGET_CLASS (parent_class)->size_request (widget, req);
}

static void 
hildon_color_chooser_dialog_size_allocate       (GtkWidget *widget,
                                                 GtkAllocation *alloc)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (widget);

    GdkRectangle rect;
    int i, tmp, tmp2;

    g_assert (priv);

    GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, alloc);

    if (GTK_WIDGET_REALIZED (widget)) {
        tmp  = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right);
        tmp2 = (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);

        for (i = 0; i < tmp; i++) {
            rect.x = ((i % priv->style_info.num_buttons.left) * 
                    (priv->style_info.radio_sizes.left + 
                     priv->style_info.radio_sizes.top + 
                     2 * priv->style_info.radio_sizes.bottom)) + 
                priv->style_info.radio_sizes.bottom;

            rect.y = ((i / priv->style_info.num_buttons.left) * 
                    (priv->style_info.radio_sizes.right + 
                     priv->style_info.radio_sizes.top +
                     2 * priv->style_info.radio_sizes.bottom)) + 
                priv->style_info.radio_sizes.bottom;

            rect.width = priv->style_info.radio_sizes.left;
            rect.height = priv->style_info.radio_sizes.right;

            gdk_gc_set_clip_rectangle (priv->gc_array[i], &rect);
        }

        for (i = 0; i < tmp2; i++) {
            rect.x = ((i % priv->style_info.num_buttons.top) * 
                    (priv->style_info.radio_sizes.left + 
                     priv->style_info.radio_sizes.top +
                     2 * priv->style_info.radio_sizes.bottom)) + 
                priv->style_info.radio_sizes.bottom;

            rect.y = ((i / priv->style_info.num_buttons.top) * 
                    (priv->style_info.radio_sizes.right + 
                     priv->style_info.radio_sizes.top +
                     2 * priv->style_info.radio_sizes.bottom)) + priv->style_info.radio_sizes.bottom;

            rect.width = priv->style_info.radio_sizes.left;
            rect.height = priv->style_info.radio_sizes.right;

            gdk_gc_set_clip_rectangle (priv->gc_array[i + tmp], &rect);
        }
    }
}

static void 
hildon_color_chooser_dialog_realize             (GtkWidget *widget)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (widget);

    GdkRectangle rect;
    int i, tmp, tmp2;

    g_assert (priv);
    
    GTK_WIDGET_CLASS(parent_class)->realize (widget);

    tmp = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right) +
        (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);

    for (i = 0; i < tmp; i++) {
        priv->gc_array[i] = gdk_gc_new (widget->window);
    }

    tmp  = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right);
    tmp2 = (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);

    for (i = 0; i < tmp; i++) {
        gdk_gc_set_rgb_fg_color (priv->gc_array[i], &priv->colors_defined[i]);

        rect.x = ((i % priv->style_info.num_buttons.left) * 
                (priv->style_info.radio_sizes.left + 
                 priv->style_info.radio_sizes.top +
                 2 * priv->style_info.radio_sizes.bottom)) + 
            priv->style_info.radio_sizes.bottom;

        rect.y = ((i / priv->style_info.num_buttons.left) * 
                (priv->style_info.radio_sizes.right + 
                 priv->style_info.radio_sizes.top +
                 2 * priv->style_info.radio_sizes.bottom)) + 
            priv->style_info.radio_sizes.bottom;

        rect.width = priv->style_info.radio_sizes.left;
        rect.height = priv->style_info.radio_sizes.right;

        gdk_gc_set_clip_rectangle (priv->gc_array[i], &rect);
    }

    for (i = 0; i < tmp2; i++) {
        gdk_gc_set_rgb_fg_color (priv->gc_array[i + tmp], &priv->colors_custom[i]);

        rect.x = ((i % priv->style_info.num_buttons.top) * 
                (priv->style_info.radio_sizes.left + 
                 priv->style_info.radio_sizes.top +
                 2 * priv->style_info.radio_sizes.bottom)) + 
            priv->style_info.radio_sizes.bottom;

        rect.y = ((i / priv->style_info.num_buttons.top) * 
                (priv->style_info.radio_sizes.right + 
                 priv->style_info.radio_sizes.top +
                 2 * priv->style_info.radio_sizes.bottom)) + 
            priv->style_info.radio_sizes.bottom;

        rect.width = priv->style_info.radio_sizes.left;
        rect.height = priv->style_info.radio_sizes.right;

        gdk_gc_set_clip_rectangle (priv->gc_array[i + tmp], &rect);
    }
}

static void 
hildon_color_chooser_dialog_unrealize           (GtkWidget *widget)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (widget);

    int i, tmp;

    tmp = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right) +
        (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);

    for (i = 0; i < tmp; i++) {
        g_object_unref (priv->gc_array[i]);
	priv->gc_array[i] = NULL;
    }

    GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void 
hildon_color_chooser_dialog_style_set           (GtkWidget *widget, 
                                                 GtkStyle *previous_style)
{
    HildonColorChooserDialog *dialog = HILDON_COLOR_CHOOSER_DIALOG (widget);
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (widget);

    GdkColor *tmpcolor;
    gchar tmp[32], key[128], *val;
    int i, tmpn, setcolor = 0;

    g_assert (priv);

    if (! priv->has_style)
        setcolor = 1;

    priv->has_style = 1;

    gtk_widget_style_get (widget, "default_color", &tmpcolor, NULL);

    if (tmpcolor) {
        priv->style_info.default_color = *tmpcolor;
	gdk_color_free (tmpcolor);
    } else {
        priv->style_info.default_color.red   = 0x0000;
        priv->style_info.default_color.green = 0x0000;
        priv->style_info.default_color.blue  = 0x0000;
        priv->style_info.default_color.pixel = 0x00000000;
    }

    hildon_color_chooser_dialog_refresh_style_info (dialog);

    if (memcmp (&priv->style_info.num_buttons, &priv->style_info.last_num_buttons, sizeof (GtkBorder))) {
        if (priv->colors_custom) {
            g_free (priv->colors_custom);
        } if (priv->colors_defined) {
            g_free (priv->colors_defined);
        } if (priv->gc_array) {
            if (GTK_WIDGET_REALIZED (widget)) {
                tmpn = (priv->style_info.last_num_buttons.left * priv->style_info.last_num_buttons.right) +
                    (priv->style_info.last_num_buttons.top * priv->style_info.last_num_buttons.bottom);

                for (i = 0; i < tmpn; i++) {
                    g_object_unref (priv->gc_array[i]);
		    priv->gc_array[i] = NULL;
                }
            }

            g_free (priv->gc_array);
        }

        priv->colors_custom = (GdkColor *)
            g_malloc0 (sizeof (GdkColor) * (priv->style_info.num_buttons.top * 
                        priv->style_info.num_buttons.bottom));

        priv->colors_defined = (GdkColor *)
            g_malloc0 (sizeof(GdkColor) * (priv->style_info.num_buttons.left * 
                        priv->style_info.num_buttons.right));

        tmpn = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right) +
            (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);

        priv->gc_array = (GdkGC **) g_malloc0 (sizeof (GdkGC *) * tmpn);
	if (GTK_WIDGET_REALIZED (widget)) {
                for (i = 0; i < tmpn; i++) {
		    priv->gc_array[i] = gdk_gc_new (widget->window);
                }
	}

        if (priv->gconf_client) {

            for (i = 0; i < (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom); i++) {
                memset (key, 0, 128);
                /* FIXME Extremally bad hardcoding */
                g_snprintf (key, 128, "/system/osso/af/color_chooser/custom_color%d", i);
                val = gconf_client_get_string (priv->gconf_client, key, NULL);

                if (val) {
                    hildon_color_chooser_dialog_ascii_hex_to_color (val, &priv->colors_custom[i]);
                    g_free (val);
                } else {
                    priv->colors_custom[i] = priv->style_info.default_color;
                }
            }
        } else {
            for (i = 0; i < (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom); i++) {
                priv->colors_custom[i] = priv->style_info.default_color;
            }
        }
    }

    tmpn = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right);

    hildon_color_chooser_set_color (HILDON_COLOR_CHOOSER (priv->chooser),
            (priv->selected < tmpn) ? 
            &priv->colors_defined[priv->selected] : 
            &priv->colors_custom[priv->selected - tmpn]);

    for (i = 0; i < (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right); i++) {
        memset (tmp, 0, 32);
        g_snprintf (tmp, 32, "defined_color%d", i);

        gtk_widget_style_get (widget, tmp, &tmpcolor, NULL);

        if (tmpcolor) {
            priv->colors_defined[i] = *tmpcolor;
	    gdk_color_free (tmpcolor);
        } else {
            if(i < 16) {
                priv->colors_defined[i] = hardcoded_colors[i];
            } else { /* fallback to prevent segfault */
                priv->colors_defined[i].red = 0x0000;
                priv->colors_defined[i].green = 0x0000;
                priv->colors_defined[i].blue = 0x0000;
                priv->colors_defined[i].pixel = 0x00000000;
            }
        }
    }

    if (GTK_WIDGET_REALIZED (widget)) {
        for (i = 0; i < (priv->style_info.num_buttons.left * 
                    priv->style_info.num_buttons.right); i++) {
            gdk_gc_set_rgb_fg_color (priv->gc_array[i], &priv->colors_defined[i]);
        }
    }

    if (setcolor)
        hildon_color_chooser_dialog_set_color (HILDON_COLOR_CHOOSER_DIALOG (dialog), 
                &priv->pending_color);

    gtk_widget_queue_resize (widget);

    GTK_WIDGET_CLASS (parent_class)->style_set (widget, previous_style);
}

static void 
hildon_color_chooser_dialog_show                (GtkWidget *widget)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (widget);

    g_assert (priv);

    gtk_widget_show (priv->hbox);
    gtk_widget_show (priv->vbox);

    gtk_widget_show (priv->chooser);

    gtk_widget_show (priv->align_custom);
    gtk_widget_show (priv->align_defined);

    gtk_widget_show (priv->separator);

    gtk_widget_show (priv->area_custom);
    gtk_widget_show (priv->area_defined);

    GTK_WIDGET_CLASS (parent_class)->show (widget);
}

/* FIXME WTF this function is even needed here? */
static void 
hildon_color_chooser_dialog_show_all            (GtkWidget *widget)
{
    hildon_color_chooser_dialog_show (widget);
}

static gboolean 
hildon_color_chooser_dialog_key_press_event     (GtkWidget *widget, 
                                                 GdkEventKey *event)
{
    HildonColorChooserDialog *dialog = HILDON_COLOR_CHOOSER_DIALOG (widget);
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (widget);
    int tmp, tot, sel;

    g_assert (priv);

    if (event->keyval == HILDON_HARDKEY_UP || event->keyval == HILDON_HARDKEY_DOWN ||
            event->keyval == HILDON_HARDKEY_LEFT || event->keyval == HILDON_HARDKEY_RIGHT) {
        tmp = (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);
        tot = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right) + tmp;

        switch (event->keyval) {

            case HILDON_HARDKEY_UP:
                if(priv->selected >= priv->style_info.num_buttons.top) {
                    if(priv->selected - priv->style_info.num_buttons.left >= tmp) {
                        sel = priv->selected - priv->style_info.num_buttons.left;
                    } else {
                        sel = priv->selected - priv->style_info.num_buttons.top;
                    }

                    hildon_color_chooser_dialog_set_color_num (dialog, sel);
                }
                break;

            case HILDON_HARDKEY_DOWN:
                if(priv->selected < tot - priv->style_info.num_buttons.left) {
                    if(priv->selected < tmp) {
                        sel = priv->selected + priv->style_info.num_buttons.top;
                    } else {
                        sel = priv->selected + priv->style_info.num_buttons.left;
                    }

                    hildon_color_chooser_dialog_set_color_num (dialog, sel);
                }
                break;

            case HILDON_HARDKEY_LEFT:
                if ((priv->selected < tmp ? 
                            (priv->selected % priv->style_info.num_buttons.top) : 
                            ((priv->selected - tmp) % priv->style_info.num_buttons.left)) > 0) {
                    sel = priv->selected - 1;

                    hildon_color_chooser_dialog_set_color_num (dialog, sel);
                }
                break;

            case HILDON_HARDKEY_RIGHT:
                if ((priv->selected < tmp) ? 
                        (priv->selected % priv->style_info.num_buttons.top < priv->style_info.num_buttons.top - 1) :
                        ((priv->selected - tmp) % priv->style_info.num_buttons.left < priv->style_info.num_buttons.left - 1)) {
                    sel = priv->selected + 1;

                    hildon_color_chooser_dialog_set_color_num (dialog, sel);
                }
                break;

            default:
                break;
        }

        return FALSE;
    }

    return GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, event);
}

static gboolean
hildon_color_chooser_dialog_key_release_event   (GtkWidget *widget, 
                                                 GdkEventKey *event)
{
    if (event->keyval == HILDON_HARDKEY_UP || 
        event->keyval == HILDON_HARDKEY_DOWN ||
        event->keyval == HILDON_HARDKEY_LEFT || 
        event->keyval == HILDON_HARDKEY_RIGHT) 
    {
        return FALSE;
    }

    return GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, event);
}

static void 
hildon_color_chooser_dialog_destroy             (GtkObject *object)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (object);

    gchar key[128], color[13];
    int i, tmp;

    g_assert (priv);

    if (priv->gconf_client) {
        memset (color, 0, 13);

        tmp = (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);

        for (i = 0; i < tmp; i++) {
            memset (key, 0, 128);
            /* FIXME Extremally bad hardcoding */
            g_snprintf (key, 128, "/system/osso/af/color_chooser/custom_color%d", i);
            hildon_color_chooser_dialog_color_to_ascii_hex (color, &priv->colors_custom[i]);
            gconf_client_set_string (priv->gconf_client, key, color, NULL);
        }

        g_object_unref (priv->gconf_client);
        priv->gconf_client = NULL;
    }

    if (priv->gc_array) {
        g_free (priv->gc_array);
        priv->gc_array = NULL;
    } if (priv->colors_defined) {
        g_free (priv->colors_defined);
        priv->colors_defined = NULL;
    } if (priv->colors_custom) {
        g_free (priv->colors_custom);
        priv->colors_custom = NULL;
    }

    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

/**
 * hildon_color_chooser_dialog_set_color:
 * @dialog: a #HildonColorChooserDialog
 * @color: a color to set on the #HildonColorChooserDialog
 *
 * Sets the dialog to point at the given color. It'll first try to
 * search the palette of the existing colors to match the passed color. 
 * If the color is not found in the pallette, the color in the currently 
 * selected box will be modified.
 *
 */
void 
hildon_color_chooser_dialog_set_color           (HildonColorChooserDialog *dialog, 
                                                 GdkColor *color)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (dialog);

    int i, found = -1, tmp, tmp2;

    g_assert (priv);

    if (! priv->has_style) {
        priv->pending_color = *color;
        return;
    }

    tmp  = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right);
    tmp2 = (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);

    for (i = 0; i < tmp; i++) {
        if (priv->colors_defined[i].red == color->red &&
                priv->colors_defined[i].green == color->green &&
                priv->colors_defined[i].blue == color->blue) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        for (i = 0; i < tmp2; i++) {
            if (priv->colors_custom[i].red == color->red &&
                    priv->colors_custom[i].green == color->green &&
                    priv->colors_custom[i].blue == color->blue) {
                found = i + tmp;
                break;
            }
        }
    }

    if (found == -1) {
        priv->colors_custom[tmp2-1] = *color;
        if (GTK_WIDGET_REALIZED (GTK_WIDGET (dialog))) {
            gdk_gc_set_rgb_fg_color (priv->gc_array[tmp2-1], color);
        }
        hildon_color_chooser_dialog_set_color_num (dialog, tmp2 - 1);
    } else {
        hildon_color_chooser_dialog_set_color_num (dialog, found);
    }
}

static gboolean 
hildon_color_chooser_dialog_area_expose         (GtkWidget *widget, 
                                                 GdkEventExpose *event, 
                                                 gpointer data)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (data);

    int i, num_selected, tot_w, tot_h, spacing, brd, x, y;
    GdkGC **start_gc;
    int tmp, w, h;

    g_assert (priv);

    tmp = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right);

    if (widget == priv->area_custom) {
        num_selected = priv->selected - tmp;
        start_gc = priv->gc_array + tmp;
        tmp = (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);
        w = priv->style_info.num_buttons.top; 
        h = priv->style_info.num_buttons.bottom;
    } else { /* widget == dialog->area_defined */
        num_selected = priv->selected;
        start_gc = priv->gc_array;
        w = priv->style_info.num_buttons.left; 
        h = priv->style_info.num_buttons.right;
    }

    spacing = priv->style_info.radio_sizes.top;
    brd = priv->style_info.radio_sizes.bottom;
    tot_w = priv->style_info.radio_sizes.left + 2 * brd;
    tot_h = priv->style_info.radio_sizes.right + 2 * brd;

    for (i = 0; i < tmp; i++) {
        x = ((i % w) * (tot_w + spacing));
        y = ((i / w) * (tot_h + spacing));

        gdk_draw_rectangle (widget->window,
                (i == num_selected) ? widget->style->bg_gc[GTK_STATE_SELECTED] : widget->style->bg_gc[GTK_STATE_NORMAL],
                TRUE,
                (i == num_selected) ? x : x + 2,  
                (i == num_selected) ? y : y + 2,
                (i == num_selected) ? tot_w : tot_w - 4,
                (i == num_selected) ? tot_h : tot_h - 4);

        gdk_draw_rectangle(widget->window,
                (i == num_selected) ? widget->style->bg_gc[GTK_STATE_NORMAL] : widget->style->white_gc,
                TRUE,
                x + 3,  
                y + 3,
                tot_w - 6,
                tot_h - 6);

        gdk_draw_rectangle(widget->window,
                start_gc [i],
                TRUE,
                x + 3 + 1,  
                y + 3 + 1,
                tot_w - 6 - 2,
                tot_h - 6 - 2);
    }

    return FALSE;
}

static gboolean 
hildon_color_chooser_dialog_area_button_press   (GtkWidget *widget, 
                                                 GdkEventButton *event, 
                                                 gpointer data)
{
    HildonColorChooserDialog *dialog = HILDON_COLOR_CHOOSER_DIALOG (data);
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (data);

    int i, hskip, vskip, brd, selection = -1;
    int x, y, tmp, tmp2, w;

    g_assert (priv);

    x = event->x;
    y = event->y;

    brd = priv->style_info.radio_sizes.bottom;
    hskip = priv->style_info.radio_sizes.left + 
        priv->style_info.radio_sizes.top + 2 * brd;
    vskip = priv->style_info.radio_sizes.right + 
        priv->style_info.radio_sizes.top + 2 * brd;

    tmp  = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right);
    tmp2 = (priv->style_info.num_buttons.top * priv->style_info.num_buttons.bottom);

    if (widget == priv->area_defined) {
        w = priv->style_info.num_buttons.left;

        for (i = 0; i < tmp; i++) {
            if (x >= hskip * (i % w) + brd && x < hskip * (i % w) + brd + priv->style_info.radio_sizes.left &&
                y >= vskip * (i / w) + brd && y < hskip * (i / w) + brd + priv->style_info.radio_sizes.right) {
                selection = i;
                break;
            }
        }
    } else {
        w = priv->style_info.num_buttons.top;
        for (i = 0; i < tmp2; i++) {
            if (x >= hskip * (i % w) + brd && x < hskip * (i % w) + brd + priv->style_info.radio_sizes.left &&
                y >= vskip * (i / w) + brd && y < hskip * (i / w) + brd + priv->style_info.radio_sizes.right) {
                selection = i + tmp;
                break;
            }
        }
    }

    if (selection != -1) {
        hildon_color_chooser_dialog_set_color_num (dialog, selection);
    }

    return FALSE;
}

static void 
hildon_color_chooser_dialog_color_changed       (HildonColorChooser *chooser, 
                                                 gpointer data)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (data);
    char key[128], color_str[13];
    int tmp;
    GdkColor color;

    g_assert (priv);

    hildon_color_chooser_get_color (chooser, &color);

    tmp = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right);

    if (priv->selected >= tmp) {
        priv->colors_custom[priv->selected - tmp] = color;

        gdk_gc_set_rgb_fg_color (priv->gc_array[priv->selected], &priv->colors_custom[priv->selected - tmp]);
        gtk_widget_queue_draw (priv->area_custom);

        if (priv->gconf_client) {
            memset (key, 0, 128);
            memset (color_str, 0, 13);
            /* FIXME Ugly hardcoded stuff! */
            g_snprintf (key, 128, "/system/osso/af/color_chooser/custom_color%d", priv->selected - tmp);
            hildon_color_chooser_dialog_color_to_ascii_hex (color_str, &priv->colors_custom[priv->selected - tmp]);
            gconf_client_set_string (priv->gconf_client, key, color_str, NULL);
        }
    }
}

static void 
hildon_color_chooser_dialog_insensitive_press   (GtkWidget *widget, 
                                                 gpointer data)
{
    hildon_banner_show_information (widget, NULL, _("ecdg_ib_colour_selector_predefined"));
}

/* function has size defaults */
static void 
hildon_color_chooser_dialog_refresh_style_info  (HildonColorChooserDialog *dialog)
{
    GtkBorder *tmp1, *tmp2, *tmp3;
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (dialog);

    g_assert (priv);

    gtk_widget_style_get (GTK_WIDGET (dialog), "container_sizes", &tmp1,
            "radio_sizes", &tmp2, "num_buttons", &tmp3, NULL);

    priv->style_info.last_num_buttons = priv->style_info.num_buttons;

    if (tmp1) {
        priv->style_info.cont_sizes = *tmp1;
        gtk_border_free (tmp1);
    } else {
        priv->style_info.cont_sizes.left = 0;
        priv->style_info.cont_sizes.right = 8;
        priv->style_info.cont_sizes.top = 4;
        priv->style_info.cont_sizes.bottom = 0;
    }

    if (tmp2) {
        priv->style_info.radio_sizes = *tmp2;
        gtk_border_free (tmp2);
    } else {
        priv->style_info.radio_sizes.left = 16;
        priv->style_info.radio_sizes.right = 16;
        priv->style_info.radio_sizes.top = 4;
        priv->style_info.radio_sizes.bottom = 2;
    }

    if (tmp3) {
        priv->style_info.num_buttons = *tmp3;
        gtk_border_free (tmp3);
    } else {
        priv->style_info.num_buttons.left = 8;
        priv->style_info.num_buttons.right = 2;
        priv->style_info.num_buttons.top = 8;
        priv->style_info.num_buttons.bottom = 2;
    }
}

static void 
hildon_color_chooser_dialog_set_color_num       (HildonColorChooserDialog *dialog, 
                                                 gint num)
{
    HildonColorChooserDialogPrivate *priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (dialog);

    int tmp;

    g_assert (priv);

    tmp = (priv->style_info.num_buttons.left * priv->style_info.num_buttons.right);

    if (num < tmp) {
        gtk_widget_set_sensitive (priv->chooser, FALSE);
    } else {
        gtk_widget_set_sensitive (priv->chooser, TRUE);
    }

    priv->selected = num;

    gtk_widget_queue_draw (priv->area_custom);
    gtk_widget_queue_draw (priv->area_defined);

    priv->color = (num < tmp) ? priv->colors_defined[num] : priv->colors_custom[num - tmp];

    hildon_color_chooser_set_color (HILDON_COLOR_CHOOSER (priv->chooser), 
            (num < tmp) ? &priv->colors_defined[num] : &priv->colors_custom[num - tmp]);
}

static void 
hildon_color_chooser_dialog_ascii_hex_to_color  (gchar *s, 
                                                 GdkColor *color)
{
    int vals[12], i;

    for (i = 0; i < 12; i++) {
        if (s[i] >= '0' && s[i] <= '9') {
            vals[i] = s[i] - 0x30;
        } else if (s[i] >= 'a' && s[i] <= 'f') {
            vals[i] = s[i] - 0x57;
        } else {
            vals[i] = 0;
        }
    }

    color->red   = (vals[0] << 12) | (vals[1] <<  8) | (vals[2 ] <<  4) | (vals[3 ]);
    color->green = (vals[4] << 12) | (vals[5] <<  8) | (vals[6 ] <<  4) | (vals[7 ]);
    color->blue  = (vals[8] << 12) | (vals[9] <<  8) | (vals[10] <<  4) | (vals[11]);
}

static void 
hildon_color_chooser_dialog_color_to_ascii_hex  (gchar *s, 
                                                 GdkColor *color)
{
    g_snprintf (s, 13, "%x%x%x%x%x%x%x%x%x%x%x%x",
         (color->red >> 12) & 0xf, (color->red >>  8) & 0xf,
         (color->red >>  4) & 0xf, (color->red      ) & 0xf,
         (color->green >> 12) & 0xf, (color->green >>  8) & 0xf,
         (color->green >>  4) & 0xf, (color->green      ) & 0xf,
         (color->blue >> 12) & 0xf, (color->blue >>  8) & 0xf,
         (color->blue >>  4) & 0xf, (color->blue      ) & 0xf);
}

/**
 * hildon_color_chooser_dialog_new:
 *
 * Creates a new color chooser dialog.
 *
 * Returns: a new color chooser dialog.
 */
GtkWidget*
hildon_color_chooser_dialog_new                 (void)
{
    return g_object_new (HILDON_TYPE_COLOR_CHOOSER_DIALOG, NULL);
}

/**
 * hildon_color_chooser_dialog_get_color:
 * @dialog: a #HildonColorChooserDialog
 * @color: a color structure to fill with the currently selected color
 *
 * Retrives the currently selected color in the color chooser dialog.
 *
 */
void
hildon_color_chooser_dialog_get_color           (HildonColorChooserDialog *dialog, 
                                                 GdkColor *color)
{
    /* FIXME Should return pending color? */
    HildonColorChooserDialogPrivate *priv;

    g_return_if_fail (HILDON_IS_COLOR_CHOOSER_DIALOG (dialog));
    priv = HILDON_COLOR_CHOOSER_DIALOG_GET_PRIVATE (dialog);
    g_assert (priv);

    hildon_color_chooser_get_color (HILDON_COLOR_CHOOSER (priv->chooser), color);
}

