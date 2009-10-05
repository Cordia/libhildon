/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2008 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version. or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * SECTION:hildon-time-selector
 * @short_description: A widget to select the current time.
 *
 * #HildonTimeSelector allows users to choose a time by selecting hour
 * and minute. It also allows choosing between AM or PM format.
 *
 * The currently selected time can be altered with
 * hildon_time_selector_set_time(), and retrieved using
 * hildon_time_selector_get_time().
 *
 * Use this widget instead of deprecated HildonTimeEditor widget.
 */

#define _GNU_SOURCE     /* needed for GNU nl_langinfo_l */
#define __USE_GNU       /* needed for locale */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <libintl.h>
#include <time.h>
#include <langinfo.h>
#include <locale.h>
#include <gconf/gconf-client.h>

#include "hildon-enum-types.h"
#include "hildon-time-selector.h"
#include "hildon-touch-selector-private.h"

#define HILDON_TIME_SELECTOR_GET_PRIVATE(obj)                           \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HILDON_TYPE_TIME_SELECTOR, HildonTimeSelectorPrivate))

G_DEFINE_TYPE (HildonTimeSelector, hildon_time_selector, HILDON_TYPE_TOUCH_SELECTOR)

#define INIT_YEAR 100
#define LAST_YEAR 50    /* since current year */

#define _(String)  dgettext("hildon-libs", String)
#define N_(String) String


/* FIXME: we should get this two props from the clock ui headers */
#define CLOCK_GCONF_PATH "/apps/clock"
#define CLOCK_GCONF_IS_24H_FORMAT CLOCK_GCONF_PATH  "/time-format"

enum {
  COLUMN_STRING,
  COLUMN_INT,
  TOTAL_MODEL_COLUMNS
};

enum {
  COLUMN_HOURS = 0,
  COLUMN_MINUTES,
  COLUMN_AMPM,
  TOTAL_TIME_COLUMNS
};

enum
{
  PROP_0,
  PROP_MINUTES_STEP,
  PROP_TIME_FORMAT_POLICY
};

struct _HildonTimeSelectorPrivate
{
  GtkTreeModel *hours_model;
  GtkTreeModel *minutes_model;
  GtkTreeModel *ampm_model;

  guint minutes_step;
  HildonTimeSelectorFormatPolicy format_policy;
  gboolean ampm_format;         /* if using am/pm format or 24 h one */

  gboolean pm;                  /* if we are on pm (only useful if ampm_format == TRUE) */

  gint creation_hours;
  gint creation_minutes;
};

static void hildon_time_selector_finalize (GObject * object);
static GObject* hildon_time_selector_constructor (GType type,
                                                  guint n_construct_properties,
                                                  GObjectConstructParam *construct_params);
static void hildon_time_selector_get_property (GObject *object,
                                               guint param_id,
                                               GValue *value,
                                               GParamSpec *pspec);
static void hildon_time_selector_set_property (GObject *object,
                                               guint param_id,
                                               const GValue *value,
                                               GParamSpec *pspec);

/* private functions */
static GtkTreeModel *_create_hours_model (HildonTimeSelector * selector);
static GtkTreeModel *_create_minutes_model (guint minutes_step);
static GtkTreeModel *_create_ampm_model (HildonTimeSelector * selector);

static void _get_real_time (gint * hours, gint * minutes);
static void _manage_ampm_selection_cb (HildonTouchSelector * selector,
                                       gint num_column, gpointer data);
static void _set_pm (HildonTimeSelector * selector, gboolean pm);

static gchar *_custom_print_func (HildonTouchSelector * selector,
                                  gpointer user_data);

static void
check_automatic_ampm_format                     (HildonTimeSelector * selector);

static void
update_format_policy                            (HildonTimeSelector *selector,
                                                 HildonTimeSelectorFormatPolicy new_policy);
static void
update_format_dependant_columns                 (HildonTimeSelector *selector,
                                                 guint hours,
                                                 guint minutes);

static void
hildon_time_selector_class_init (HildonTimeSelectorClass * class)
{
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  gobject_class = (GObjectClass *) class;
  object_class = (GtkObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;
  container_class = (GtkContainerClass *) class;

  /* GObject */
  gobject_class->get_property = hildon_time_selector_get_property;
  gobject_class->set_property = hildon_time_selector_set_property;
  gobject_class->constructor = hildon_time_selector_constructor;
  gobject_class->finalize = hildon_time_selector_finalize;

  g_object_class_install_property (gobject_class,
                                   PROP_MINUTES_STEP,
                                   g_param_spec_uint ("minutes-step",
                                                      "Step between minutes in the model",
                                                      "Step between the minutes in the list of"
                                                      " options of the widget ",
                                                      1, 30, 1,
                                                      G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY));

  /**
   * HildonTimeSelector:time-format-policy:
   *
   * The visual policy of the time format
   *
   * Since: 2.2
   */
  g_object_class_install_property (gobject_class,
                                   PROP_TIME_FORMAT_POLICY,
                                   g_param_spec_enum ("time_format_policy",
                                                      "time format policy",
                                                      "Visual policy of the time format",
                                                      HILDON_TYPE_TIME_SELECTOR_FORMAT_POLICY,
                                                      HILDON_TIME_SELECTOR_FORMAT_POLICY_AUTOMATIC,
                                                      G_PARAM_READWRITE|G_PARAM_CONSTRUCT));

  /* GtkWidget */

  /* GtkContainer */

  /* signals */

  g_type_class_add_private (object_class, sizeof (HildonTimeSelectorPrivate));
}

/* FIXME: the constructor was required because as we need the initial values
   of the properties passed on g_object_new. But, probably use the method
   constructed could be easier */
static GObject*
hildon_time_selector_constructor (GType type,
                                  guint n_construct_properties,
                                  GObjectConstructParam *construct_params)
{
  GObject *object;
  HildonTimeSelector *selector;
  HildonTouchSelectorColumn *column;

  object = (* G_OBJECT_CLASS (hildon_time_selector_parent_class)->constructor)
    (type, n_construct_properties, construct_params);

  selector = HILDON_TIME_SELECTOR (object);

  selector->priv->hours_model = _create_hours_model (selector);

  column = hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector),
                                                     selector->priv->hours_model, TRUE);
  hildon_touch_selector_column_set_text_column (column, 0);

  /* we need initialization parameters in order to create minute models*/
  selector->priv->minutes_step = selector->priv->minutes_step ? selector->priv->minutes_step : 1;

  selector->priv->minutes_model = _create_minutes_model (selector->priv->minutes_step);

  column = hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector),
                                                     selector->priv->minutes_model, TRUE);
  hildon_touch_selector_column_set_text_column (column, 0);

  if (selector->priv->ampm_format) {
    selector->priv->ampm_model = _create_ampm_model (selector);

    hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector),
                                              selector->priv->ampm_model, TRUE);

    g_signal_connect (G_OBJECT (selector),
                      "changed", G_CALLBACK (_manage_ampm_selection_cb),
                      NULL);
  }


  /* By default we should select the current day */
  hildon_time_selector_set_time (selector,
                                 selector->priv->creation_hours,
                                 selector->priv->creation_minutes);

  return object;

}

static void
hildon_time_selector_init (HildonTimeSelector * selector)
{
  selector->priv = HILDON_TIME_SELECTOR_GET_PRIVATE (selector);

  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (selector), GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (selector), FALSE);

  hildon_touch_selector_set_print_func (HILDON_TOUCH_SELECTOR (selector),
                                        _custom_print_func);

  /* By default we use the automatic ampm format */
  selector->priv->pm = TRUE;
  check_automatic_ampm_format (selector);

  _get_real_time (&selector->priv->creation_hours,
                  &selector->priv->creation_minutes);
}

static void
hildon_time_selector_get_property (GObject *object,
                                   guint param_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  HildonTimeSelectorPrivate *priv = HILDON_TIME_SELECTOR_GET_PRIVATE (object);

  switch (param_id)
    {
    case PROP_MINUTES_STEP:
      g_value_set_uint (value, priv->minutes_step);
      break;
    case PROP_TIME_FORMAT_POLICY:
      g_value_set_enum (value, priv->format_policy);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
hildon_time_selector_set_property (GObject *object,
                                   guint param_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  HildonTimeSelectorPrivate *priv = HILDON_TIME_SELECTOR_GET_PRIVATE (object);

  switch (param_id)
    {
    case PROP_MINUTES_STEP:
      priv->minutes_step = g_value_get_uint (value);
      break;
    case PROP_TIME_FORMAT_POLICY:
      update_format_policy (HILDON_TIME_SELECTOR (object),
                            g_value_get_enum (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
hildon_time_selector_finalize (GObject * object)
{
  HildonTimeSelectorPrivate *priv = HILDON_TIME_SELECTOR_GET_PRIVATE (object);

  if (priv->hours_model) {
    g_object_unref (priv->hours_model);
    priv->hours_model = NULL;
  }

  if (priv->minutes_model) {
    g_object_unref (priv->minutes_model);
    priv->minutes_model = NULL;
  }

  if (priv->ampm_model) {
    g_object_unref (priv->ampm_model);
    priv->ampm_model = NULL;
  }

  (*G_OBJECT_CLASS (hildon_time_selector_parent_class)->finalize) (object);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */

static gchar *
_custom_print_func (HildonTouchSelector * touch_selector,
                    gpointer user_data)
{
  gchar *result = NULL;
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  HildonTimeSelector *selector = NULL;
  static gchar string[255];
  guint hours = 0;
  guint minutes = 0;

  selector = HILDON_TIME_SELECTOR (touch_selector);

  hildon_time_selector_get_time (selector, &hours, &minutes);

  tm.tm_min = minutes;
  tm.tm_hour = hours;

  if (selector->priv->ampm_format) {
    if (selector->priv->pm) {
      strftime (string, 255, _("wdgt_va_12h_time_pm"), &tm);
    } else {
      strftime (string, 255, _("wdgt_va_12h_time_am"), &tm);
    }
  } else {
    strftime (string, 255, _("wdgt_va_24h_time"), &tm);
  }


  result = g_strdup (string);

  return result;
}

static GtkTreeModel *
_create_minutes_model (guint minutes_step)
{
  GtkListStore *store_minutes = NULL;
  gint i = 0;
  static gchar label[255];
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  GtkTreeIter iter;

  store_minutes = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  for (i = 0; i <= 59; i=i+minutes_step) {
    tm.tm_min = i;
    strftime (label, 255, _("wdgt_va_minutes"), &tm);

    gtk_list_store_append (store_minutes, &iter);
    gtk_list_store_set (store_minutes, &iter,
                        COLUMN_STRING, label, COLUMN_INT, i, -1);
  }

  return GTK_TREE_MODEL (store_minutes);
}

static GtkTreeModel *
_create_hours_model (HildonTimeSelector * selector)
{
  GtkListStore *store_hours = NULL;
  gint i = 0;
  GtkTreeIter iter;
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  static gchar label[255];
  static gint range_12h[12] = {12, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11};
  static gint range_24h[24] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
                               12,13,14,15,16,17,18,19,20,21,22,23};
  gint *range = NULL;
  gint num_elements = 0;
  gchar *format_string = NULL;

  if (selector->priv->ampm_format) {
    range = range_12h;
    num_elements = 12;
    format_string = N_("wdgt_va_12h_hours");
  } else {
    range = range_24h;
    num_elements = 24;
    format_string = N_("wdgt_va_24h_hours");
  }

  store_hours = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  for (i = 0; i < num_elements; i++) {
    tm.tm_hour = range[i];
    strftime (label, 255, _(format_string), &tm);

    gtk_list_store_append (store_hours, &iter);
    gtk_list_store_set (store_hours, &iter,
                        COLUMN_STRING, label, COLUMN_INT, range[i], -1);
  }

  return GTK_TREE_MODEL (store_hours);
}

static GtkTreeModel *
_create_ampm_model (HildonTimeSelector * selector)
{
  GtkListStore *store_ampm = NULL;
  GtkTreeIter iter;
  static gchar label[255];

  store_ampm = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);

  snprintf (label, 255, _("wdgt_va_am"));
  gtk_list_store_append (store_ampm, &iter);
  gtk_list_store_set (store_ampm, &iter,
                      COLUMN_STRING, label,
                      COLUMN_INT, 0, -1);

  snprintf (label, 255, _("wdgt_va_pm"));
  gtk_list_store_append (store_ampm, &iter);
  gtk_list_store_set (store_ampm, &iter,
                      COLUMN_STRING, label,
                      COLUMN_INT, 1, -1);

  return GTK_TREE_MODEL (store_ampm);
}

static void
_get_real_time (gint * hours, gint * minutes)
{
  time_t secs;
  struct tm *tm = NULL;

  secs = time (NULL);
  tm = localtime (&secs);

  if (hours != NULL) {
    *hours = tm->tm_hour;
  }

  if (minutes != NULL) {
    *minutes = tm->tm_min;
  }
}

static void
_manage_ampm_selection_cb (HildonTouchSelector * touch_selector,
                           gint num_column, gpointer data)
{
  HildonTimeSelector *selector = NULL;
  gint pm;
  GtkTreeIter iter;

  g_return_if_fail (HILDON_IS_TIME_SELECTOR (touch_selector));
  selector = HILDON_TIME_SELECTOR (touch_selector);

  if (num_column == COLUMN_AMPM &&
      hildon_touch_selector_get_selected (HILDON_TOUCH_SELECTOR (selector),
					  COLUMN_AMPM, &iter)) {
     gtk_tree_model_get (selector->priv->ampm_model, &iter, COLUMN_INT, &pm, -1);

    selector->priv->pm = pm;
  }
}

static void
check_automatic_ampm_format (HildonTimeSelector * selector)
{
  GConfClient *client = NULL;
  gboolean value = TRUE;
  GError *error = NULL;

  client = gconf_client_get_default ();
  value = gconf_client_get_bool (client, CLOCK_GCONF_IS_24H_FORMAT, &error);
  if (error != NULL) {
    g_warning
      ("Error trying to get gconf variable %s, using 24h format by default",
       CLOCK_GCONF_IS_24H_FORMAT);
    g_error_free (error);
  }

  selector->priv->ampm_format = !value;
}

static void
_set_pm (HildonTimeSelector * selector, gboolean pm)
{
  GtkTreeIter iter;

  selector->priv->pm = pm;

  if (selector->priv->ampm_model != NULL) {
    gtk_tree_model_iter_nth_child (selector->priv->ampm_model, &iter, NULL, pm);

    hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector),
                                       COLUMN_AMPM, &iter, FALSE);
  }
}

static void
update_format_policy                            (HildonTimeSelector *selector,
                                                 HildonTimeSelectorFormatPolicy new_policy)
{
  gboolean prev_ampm_format = FALSE;
  gint num_columns = -1;

  num_columns = hildon_touch_selector_get_num_columns (HILDON_TOUCH_SELECTOR (selector));
  prev_ampm_format = selector->priv->ampm_format;

  if (new_policy != selector->priv->format_policy) {
    guint hours;
    guint minutes;

    selector->priv->format_policy = new_policy;

    /* We get the hour previous all the changes, to avoid problems with the
       changing widget structure */
    if (num_columns >= 2) {/* we are on the object construction */
      hildon_time_selector_get_time (selector, &hours, &minutes);
    }

    switch (new_policy)
      {
      case HILDON_TIME_SELECTOR_FORMAT_POLICY_AMPM:
        selector->priv->ampm_format = TRUE;
        break;
      case HILDON_TIME_SELECTOR_FORMAT_POLICY_24H:
        selector->priv->ampm_format = FALSE;
        break;
      case HILDON_TIME_SELECTOR_FORMAT_POLICY_AUTOMATIC:
        check_automatic_ampm_format (selector);
        break;
      }

    if (prev_ampm_format != selector->priv->ampm_format && num_columns >= 2) {
      update_format_dependant_columns (selector, hours, minutes);
    }
  }
}

static void
update_format_dependant_columns                 (HildonTimeSelector *selector,
                                                 guint hours,
                                                 guint minutes)
{
  /* To avoid an extra and wrong VALUE_CHANGED signal on the model update */
  hildon_touch_selector_block_changed (HILDON_TOUCH_SELECTOR(selector));

  if (selector->priv->hours_model) {
    g_object_unref (selector->priv->hours_model);
  }
  selector->priv->hours_model = _create_hours_model (selector);
  hildon_touch_selector_set_model (HILDON_TOUCH_SELECTOR (selector),
                                   0,
                                   selector->priv->hours_model);

  /* We need to set NOW the correct hour on the hours column, because the number of
     columns will be updated too, so a signal COLUMNS_CHANGED will be emitted. Some
     other widget could have connected to this signal and ask for the hour, so this
     emission could have a wrong hour. We could use a custom func to only modify the
     hours selection, but hildon_time_selector_time manage yet all the ampm issues
     to select the correct one */
  hildon_time_selector_set_time (selector, hours, minutes);

  /* if we are at this function, we are sure that a change on the number of columns
     will happen, so check the column number is not required */
  if (selector->priv->ampm_model) {
    g_object_unref (selector->priv->ampm_model);
  }
  if (selector->priv->ampm_format) {
    selector->priv->ampm_model = _create_ampm_model (selector);

    hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector),
                                              selector->priv->ampm_model, TRUE);

    g_signal_connect (G_OBJECT (selector),
                      "changed", G_CALLBACK (_manage_ampm_selection_cb),
                      NULL);
  } else {
    selector->priv->ampm_model = NULL;
    hildon_touch_selector_remove_column (HILDON_TOUCH_SELECTOR (selector), 2);
  }

  _set_pm (selector, hours >= 12);

  hildon_touch_selector_unblock_changed (HILDON_TOUCH_SELECTOR (selector));
}
/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * hildon_time_selector_new:
 *
 * Creates a new #HildonTimeSelector
 *
 * Returns: a new #HildonTimeSelector
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_time_selector_new ()
{
  return g_object_new (HILDON_TYPE_TIME_SELECTOR, NULL);
}


/**
 * hildon_time_selector_new_step:
 * @minutes_step: step between the minutes in the selector.

 * Creates a new #HildonTimeSelector, with @minutes_step steps between
 * the minutes in the minutes column.
 *
 * Returns: a new #HildonTimeSelector
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_time_selector_new_step (guint minutes_step)
{
  return g_object_new (HILDON_TYPE_TIME_SELECTOR, "minutes-step",
                       minutes_step, NULL);
}

/**
 * hildon_time_selector_set_time
 * @selector: the #HildonTimeSelector
 * @hours:  the current hour (0-23)
 * @minutes: the current minute (0-59)
 *
 * Sets the current active hour on the #HildonTimeSelector widget
 *
 * The format of the hours accepted is always 24h format, with a range
 * (0-23):(0-59).
 *
 * Since: 2.2
 *
 * Returns: %TRUE on success, %FALSE otherwise
 **/
gboolean
hildon_time_selector_set_time (HildonTimeSelector * selector,
                               guint hours, guint minutes)
{
  GtkTreeIter iter;
  gint hours_item = 0;

  g_return_val_if_fail (HILDON_IS_TIME_SELECTOR (selector), FALSE);
  g_return_val_if_fail (hours <= 23, FALSE);
  g_return_val_if_fail (minutes <= 59, FALSE);

  _set_pm (selector, hours >= 12);

  if (selector->priv->ampm_format) {
    hours_item = hours - selector->priv->pm * 12;
  } else {
    hours_item = hours;
  }

  gtk_tree_model_iter_nth_child (selector->priv->hours_model, &iter, NULL,
                                 hours_item);
  hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector),
                                     COLUMN_HOURS, &iter, FALSE);

  g_assert (selector->priv->minutes_step>0);
  minutes = minutes/selector->priv->minutes_step;
  gtk_tree_model_iter_nth_child (selector->priv->minutes_model, &iter, NULL,
                                 minutes);
  hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector),
                                     COLUMN_MINUTES, &iter, FALSE);

  return TRUE;
}

/**
 * hildon_time_selector_get_time
 * @selector: the #HildonTimeSelector
 * @hours:  to set the current hour (0-23)
 * @minutes: to set the current minute (0-59)
 *
 * Gets the current active hour on the #HildonTimeSelector widget. Both @year
 * and @minutes can be NULL.
 *
 * This method returns the date always in 24h format, with a range (0-23):(0-59)
 *
 * Since: 2.2
 **/
void
hildon_time_selector_get_time (HildonTimeSelector * selector,
                               guint * hours, guint * minutes)
{
  GtkTreeIter iter;

  g_return_if_fail (HILDON_IS_TIME_SELECTOR (selector));

  if (hours != NULL) {
    if (hildon_touch_selector_get_selected (HILDON_TOUCH_SELECTOR (selector),
                                            COLUMN_HOURS, &iter)) {

      gtk_tree_model_get (selector->priv->hours_model,
                          &iter, COLUMN_INT, hours, -1);
    }
    if (selector->priv->ampm_format) {
      *hours %= 12;
      *hours += selector->priv->pm * 12;
    }
  }

  if (minutes != NULL) {
    if (hildon_touch_selector_get_selected (HILDON_TOUCH_SELECTOR (selector),
                                            COLUMN_MINUTES, &iter)) {
      gtk_tree_model_get (selector->priv->minutes_model,
                          &iter, COLUMN_INT, minutes, -1);
    }
  }
}
