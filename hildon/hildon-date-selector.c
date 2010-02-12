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
 * SECTION:hildon-date-selector
 * @short_description: A widget to select the current date.
 *
 * #HildonDateSelector is a date widget with multiple columns. Users
 * can choose a date by selecting values in the day, month and year
 * columns.
 *
 * The currently selected month and year can be altered with
 * hildon_date_selector_select_month(). The day can be selected from
 * the active month using hildon_date_selector_select_day().
 */

#define _GNU_SOURCE     /* needed for GNU nl_langinfo_l */
#define __USE_GNU       /* needed for locale */

#include <locale.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <string.h>
#include <stdlib.h>

#include <libintl.h>
#include <time.h>
#include <langinfo.h>

#include "hildon-date-selector.h"

#define HILDON_DATE_SELECTOR_GET_PRIVATE(obj)                           \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HILDON_TYPE_DATE_SELECTOR, HildonDateSelectorPrivate))

G_DEFINE_TYPE (HildonDateSelector, hildon_date_selector, HILDON_TYPE_TOUCH_SELECTOR)

#define INIT_YEAR 100
#define LAST_YEAR 50    /* since current year */

#define _(String) dgettext("hildon-libs", String)

/* #define _(String) "%A %e. %B %Y"  debug purposes */

enum
{
  COLUMN_STRING,
  COLUMN_INT,
  N_COLUMNS
};

enum
{
  DAY,
  MONTH,
  YEAR
};

struct _HildonDateSelectorPrivate
{
  GtkTreeModel *year_model;
  GtkTreeModel *month_model;
  GtkTreeModel *day_model;

  GSList *column_order;
  gint day_column;
  gint month_column;
  gint year_column;             /* it depends on the locale */

  gchar *format;                /* day/month/year format, depends on locale */

  gint creation_day;
  gint creation_month;
  gint creation_year;           /* date at creation time */

  gint current_num_days;

  gint min_year;
  gint max_year;
};

enum {
  PROP_MIN_YEAR = 1,
  PROP_MAX_YEAR,
};

static GObject * hildon_date_selector_constructor (GType                  type,
                                                   guint                  n_construct_properties,
                                                   GObjectConstructParam *construct_properties);
static void hildon_date_selector_finalize (GObject * object);

/* private functions */
static GtkTreeModel *_create_day_model (HildonDateSelector * selector);
static GtkTreeModel *_create_year_model (HildonDateSelector * selector);
static GtkTreeModel *_create_month_model (HildonDateSelector * selector);

static void _get_real_date (gint * year, gint * month, gint * day);
static void _locales_init (HildonDateSelectorPrivate * priv);

static void _manage_selector_change_cb (HildonTouchSelector * selector,
                                        gint num_column, gpointer data);

static GtkTreeModel *_update_day_model (HildonDateSelector * selector);

static gint _month_days (gint month, gint year);
static void _init_column_order (HildonDateSelector * selector);

static gchar *_custom_print_func (HildonTouchSelector * selector,
                                  gpointer user_data);

/***************************************************************************/
/* The following date routines are taken from the lib_date package.  Keep
 * them separate in case we want to update them if a newer lib_date comes
 * out with fixes.  */

typedef unsigned int N_int;

typedef unsigned long N_long;

typedef signed long Z_long;

typedef enum
{ false = FALSE, true = TRUE } boolean;

#define                                         and &&  /* logical (boolean) operators: lower case */

#define                                         or ||

static const N_int month_length[2][13] = {
  {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static const N_int days_in_months[2][14] = {
  {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

static Z_long _calc_days (N_int year, N_int mm, N_int dd);

static N_int _day_of_week (N_int year, N_int mm, N_int dd);

static boolean _leap (N_int year);


static boolean
_leap (N_int year)
{
  return ((((year % 4) == 0) and ((year % 100) != 0)) or ((year % 400) == 0));
}

static N_int
_day_of_week (N_int year, N_int mm, N_int dd)
{
  Z_long days;

  days = _calc_days (year, mm, dd);
  if (days > 0L) {
    days--;
    days %= 7L;
    days++;
  }
  return ((N_int) days);
}

static Z_long
_year_to_days (N_int year)
{
  return (year * 365L + (year / 4) - (year / 100) + (year / 400));
}

static Z_long
_calc_days (N_int year, N_int mm, N_int dd)
{
  boolean lp;

  if (year < 1)
    return (0L);
  if ((mm < 1) or (mm > 12))
    return (0L);
  if ((dd < 1) or (dd > month_length[(lp = _leap (year))][mm]))
    return (0L);
  return (_year_to_days (--year) + days_in_months[lp][mm] + dd);
}

static void
hildon_date_selector_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  HildonDateSelectorPrivate *priv = HILDON_DATE_SELECTOR (object)->priv;

  switch (prop_id)
  {
  case PROP_MIN_YEAR:
    priv->min_year = g_value_get_int (value);
    break;
  case PROP_MAX_YEAR:
    priv->max_year = g_value_get_int (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hildon_date_selector_get_property (GObject      *object,
                                   guint         prop_id,
                                   GValue       *value,
                                   GParamSpec   *pspec)
{
  HildonDateSelectorPrivate *priv = HILDON_DATE_SELECTOR (object)->priv;

  switch (prop_id)
  {
  case PROP_MIN_YEAR:
    g_value_set_int (value, priv->min_year);
    break;
  case PROP_MAX_YEAR:
    g_value_set_int (value, priv->max_year);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hildon_date_selector_class_init (HildonDateSelectorClass * class)
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
  gobject_class->finalize = hildon_date_selector_finalize;
  gobject_class->get_property = hildon_date_selector_get_property;
  gobject_class->set_property = hildon_date_selector_set_property;
  gobject_class->constructor = hildon_date_selector_constructor;

  /* GtkWidget */

  /* GtkContainer */

  /* properties */

  g_object_class_install_property (
    gobject_class,
    PROP_MIN_YEAR,
    g_param_spec_int (
      "min-year",
      "Minimum year",
      "The minimum available year in the selector",
      1900,
      2100,
      1970,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  g_object_class_install_property (
    gobject_class,
    PROP_MAX_YEAR,
    g_param_spec_int (
      "max-year",
      "Maximum year",
      "The maximum available year in the selector",
      1900,
      2100,
      2037,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  /* signals */

  g_type_class_add_private (object_class, sizeof (HildonDateSelectorPrivate));
}

static void
hildon_date_selector_construct_ui (HildonDateSelector *selector)
{
  GSList *iter = NULL;
  gint current_item = 0;
  HildonTouchSelectorColumn *column = NULL;

  selector->priv->year_model = _create_year_model (selector);
  selector->priv->month_model = _create_month_model (selector);
  selector->priv->day_model = _create_day_model (selector);

  /* We add the columns, checking the locale order */
  iter = selector->priv->column_order;
  for (iter = selector->priv->column_order; iter; iter = g_slist_next (iter)) {
    current_item = GPOINTER_TO_INT (iter->data);

    switch (current_item) {
    case DAY:
      column = hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector),
                                                         selector->priv->day_model, TRUE);
      hildon_touch_selector_column_set_text_column (column, 0);
      break;
    case MONTH:
      column = hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector),
                                                         selector->priv->month_model, TRUE);
      hildon_touch_selector_column_set_text_column (column, 0);
      break;
    case YEAR:
      column = hildon_touch_selector_append_text_column (HILDON_TOUCH_SELECTOR (selector),
                                                         selector->priv->year_model, TRUE);
      hildon_touch_selector_column_set_text_column (column, 0);
      break;
    default:
      g_error ("Current column order incorrect");
      break;
    }
  }
}

static GObject *
hildon_date_selector_constructor (GType                  type,
                                  guint                  n_construct_properties,
                                  GObjectConstructParam *construct_properties)
{
  GObject *object;
  HildonDateSelector *selector;

  object = G_OBJECT_CLASS (hildon_date_selector_parent_class)->constructor
    (type, n_construct_properties, construct_properties);

  selector = HILDON_DATE_SELECTOR (object);

  g_object_set (object, "live-search", FALSE, NULL);

  hildon_date_selector_construct_ui (selector);

  g_signal_connect (object, "changed", G_CALLBACK (_manage_selector_change_cb), NULL);

  /* By default we should select the current day */
  hildon_date_selector_select_current_date (selector, selector->priv->creation_year,
                                            selector->priv->creation_month,
                                            selector->priv->creation_day);

  return object;
}

static void
hildon_date_selector_init (HildonDateSelector * selector)
{
  selector->priv = HILDON_DATE_SELECTOR_GET_PRIVATE (selector);

  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (selector), GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (selector), FALSE);

  hildon_touch_selector_set_print_func (HILDON_TOUCH_SELECTOR (selector),
                                        _custom_print_func);

  _locales_init (selector->priv);

  _init_column_order (selector);

  _get_real_date (&selector->priv->creation_year,
                  &selector->priv->creation_month, &selector->priv->creation_day);
  selector->priv->current_num_days = 31;
}

static void
hildon_date_selector_finalize (GObject * object)
{
  HildonDateSelector *selector = NULL;

  selector = HILDON_DATE_SELECTOR (object);

  g_slist_free (selector->priv->column_order);
  g_free (selector->priv->format);

  if (selector->priv->year_model) {
      g_object_unref (selector->priv->year_model);
      selector->priv->year_model = NULL;
  }

  if (selector->priv->month_model) {
      g_object_unref (selector->priv->month_model);
      selector->priv->month_model = NULL;
  }

  if (selector->priv->day_model) {
      g_object_unref (selector->priv->day_model);
      selector->priv->day_model = NULL;
  }

  (*G_OBJECT_CLASS (hildon_date_selector_parent_class)->finalize) (object);
}

/* ------------------------------ PRIVATE METHODS ---------------------------- */
static gchar *
_custom_print_func (HildonTouchSelector * touch_selector, gpointer user_data)
{
  HildonDateSelector *selector = NULL;
  gchar *result = NULL;
  guint year, month, day;
  gint day_of_week = 0;
  static gchar string[255];
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  selector = HILDON_DATE_SELECTOR (touch_selector);

  hildon_date_selector_get_date (selector, &year, &month, &day);
  day_of_week = _day_of_week (year, month + 1, day) % 7;

  tm.tm_mday = day;
  tm.tm_mon = month;
  tm.tm_year = year - 1900;
  tm.tm_wday = day_of_week;

  strftime (string, 255, _("wdgt_va_date_long"), &tm);

  result = g_strdup (string);

  return result;
}

/* This was copied from hildon-calendar */
static void
_locales_init (HildonDateSelectorPrivate * priv)
{
  /* Hildon: This is not exactly portable, see
   * http://bugzilla.gnome.org/show_bug.cgi?id=343415
   * The labels need to be instance variables as the startup wizard changes
   * locale on runtime.
   */
  locale_t l;

  l = newlocale (LC_TIME_MASK, setlocale (LC_MESSAGES, NULL), NULL);

  priv->format = g_locale_to_utf8 (nl_langinfo_l (D_FMT, l),
                                   -1, NULL, NULL, NULL);

  freelocale (l);
}

static void
_init_column_order (HildonDateSelector * selector)
{
  gchar *current_order[3] = { NULL, NULL, NULL };
  gchar *day_pos = NULL;
  gchar *month_pos = NULL;
  gchar *year_pos = NULL;
  gint i, c;
  gchar *aux = NULL;

  g_debug ("Current format: %s", selector->priv->format);

  /* search each token on the format */
  day_pos = g_strrstr (selector->priv->format, "%d");

  month_pos = g_strrstr (selector->priv->format, "%m");
  year_pos = g_strrstr (selector->priv->format, "%y");
  if (year_pos == NULL) {
    year_pos = g_strrstr (selector->priv->format, "%Y");
  }


  if ((day_pos == NULL) || (month_pos == NULL) || (year_pos == NULL)) {
    g_warning ("Wrong date format");      /* use default values */

    selector->priv->day_column = 0;
    selector->priv->month_column = 1;
    selector->priv->year_column = 2;
    selector->priv->column_order = g_slist_append (NULL, GINT_TO_POINTER (DAY));
    selector->priv->column_order =
      g_slist_append (selector->priv->column_order, GINT_TO_POINTER (MONTH));
    selector->priv->column_order =
      g_slist_append (selector->priv->column_order, GINT_TO_POINTER (YEAR));
  }

  /* sort current_order with this values (bubble sort) */
  current_order[0] = day_pos;
  current_order[1] = month_pos;
  current_order[2] = year_pos;

  for (c = 1; c <= 2; c++) {
    for (i = 0; i < 3 - c; i++) {
      if (current_order[i] > current_order[i + 1]) {
        aux = current_order[i];
        current_order[i] = current_order[i + 1];
        current_order[i + 1] = aux;
      }
    }
  }

  /* fill the column positions */
  selector->priv->column_order = NULL;
  c = 0;
  for (i = 0; i < 3; i++) {
    if (current_order[i] == day_pos) {
      selector->priv->column_order =
        g_slist_append (selector->priv->column_order, GINT_TO_POINTER (DAY));
      selector->priv->day_column = c++;
    }
    if (current_order[i] == month_pos) {
      selector->priv->column_order =
        g_slist_append (selector->priv->column_order, GINT_TO_POINTER (MONTH));
      selector->priv->month_column = c++;
    }
    if (current_order[i] == year_pos) {
      selector->priv->column_order =
        g_slist_append (selector->priv->column_order, GINT_TO_POINTER (YEAR));
      selector->priv->year_column = c++;
    }
  }
}


static GtkTreeModel *
_create_day_model (HildonDateSelector * selector)
{
  GtkListStore *store_days = NULL;
  gint i = 0;
  gchar label[255];
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  GtkTreeIter iter;

  store_days = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  for (i = 1; i < 32; i++) {
    tm.tm_mday = i;
    strftime (label, 255, _("wdgt_va_day_numeric"), &tm);

    gtk_list_store_append (store_days, &iter);
    gtk_list_store_set (store_days, &iter,
                        COLUMN_STRING, label, COLUMN_INT, i, -1);
  }

  return GTK_TREE_MODEL (store_days);
}

static GtkTreeModel *
_create_year_model (HildonDateSelector * selector)
{
  GtkListStore *store_years = NULL;
  gint real_year = 0;
  gint i = 0;
  static gchar label[255];
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  GtkTreeIter iter;

  real_year = selector->priv->creation_year;

  store_years = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  for (i = selector->priv->min_year; i < selector->priv->max_year + 1; i++) {
    tm.tm_year = i - 1900;
    strftime (label, 255, _("wdgt_va_year"), &tm);

    gtk_list_store_append (store_years, &iter);
    gtk_list_store_set (store_years, &iter,
                        COLUMN_STRING, label, COLUMN_INT, i, -1);
  }

  return GTK_TREE_MODEL (store_years);
}

static GtkTreeModel *
_create_month_model (HildonDateSelector * selector)
{
  GtkTreeIter iter;
  gint i = 0;
  GtkListStore *store_months = NULL;
  static gchar label[255];
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  store_months = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  for (i = 0; i < 12; i++) {
    tm.tm_mon = i;
    strftime (label, 255, _("wdgt_va_month"), &tm);

    gtk_list_store_append (store_months, &iter);
    gtk_list_store_set (store_months, &iter, COLUMN_STRING, label,
                        COLUMN_INT, i,
                        -1);
  }

  return GTK_TREE_MODEL (store_months);
}

static GtkTreeModel *
_update_day_model (HildonDateSelector * selector)
{
  GtkListStore *store_days = NULL;
  GtkTreePath *path = NULL;
  gint i = 0;
  GtkTreeIter iter;
  static gchar label[255];
  struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  guint current_day = 0;
  guint current_year = 0;
  guint current_month = 0;
  gint num_days = 31;

  hildon_date_selector_get_date (selector, &current_year, &current_month,
                                 &current_day);

  num_days = _month_days (current_month, current_year);
  store_days = GTK_LIST_STORE (selector->priv->day_model);

  if (num_days == selector->priv->current_num_days) {
    return GTK_TREE_MODEL (store_days);
  }

  if (num_days > selector->priv->current_num_days) {
    for (i = selector->priv->current_num_days + 1; i <= num_days; i++) {
      tm.tm_mday = i;
      strftime (label, 255, _("wdgt_va_day_numeric"), &tm);

      gtk_list_store_append (store_days, &iter);
      gtk_list_store_set (store_days, &iter,
                          COLUMN_STRING, label, COLUMN_INT, i, -1);
    }
  } else {
    path = gtk_tree_path_new_from_indices (num_days,
                                           -1);
    gtk_tree_model_get_iter (GTK_TREE_MODEL (store_days), &iter, path);
    do {
    }while (gtk_list_store_remove (store_days, &iter));

    gtk_tree_path_free (path);
  }


  selector->priv->current_num_days = num_days;

  /* now we select a day */
  if (current_day >= num_days) {
    current_day = num_days;
  }

  hildon_date_selector_select_day (selector, current_day);

  return GTK_TREE_MODEL (store_days);
}


static void
_get_real_date (gint * year, gint * month, gint * day)
{
  time_t secs;
  struct tm *tm = NULL;

  secs = time (NULL);
  tm = localtime (&secs);

  if (year != NULL) {
    *year = 1900 + tm->tm_year;
  }

  if (month != NULL) {
    *month = tm->tm_mon;
  }

  if (day != NULL) {
    *day = tm->tm_mday;
  }
}


static void
_manage_selector_change_cb (HildonTouchSelector * touch_selector,
                            gint num_column, gpointer data)
{
  HildonDateSelector *selector = NULL;

  g_return_if_fail (HILDON_IS_DATE_SELECTOR (touch_selector));
  selector = HILDON_DATE_SELECTOR (touch_selector);

  if ((num_column == selector->priv->month_column) ||
      (num_column == selector->priv->year_column)) /* it is required to check that with
                                                    * the years too,remember: leap years
                                                    * update_day_model will check if
                                                    * the number of days is different
                                                    */
  {
    _update_day_model (selector);
  }
}

static gint
_month_days (gint month, gint year)
{
  gint month_days[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
  };

  g_return_val_if_fail (month >= 0 && month < 12, -1);

  return month_days[_leap (year)][month];
}


/* ------------------------------ PUBLIC METHODS ---------------------------- */

/**
 * hildon_date_selector_new:
 *
 * Creates a new #HildonDateSelector
 *
 * Returns: a new #HildonDateSelector
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_date_selector_new ()
{
  return g_object_new (HILDON_TYPE_DATE_SELECTOR, NULL);
}

/**
 * hildon_date_selector_new_with_year_range:
 * @min_year: the minimum available year or -1 to ignore
 * @max_year: the maximum available year or -1 to ignore
 *
 * Creates a new #HildonDateSelector with a specific year range.
 * If @min_year or @max_year are set to -1, then the default
 * upper or lower bound will be used, respectively.
 *
 * Returns: a new #HildonDateSelector
 *
 * Since: 2.2
 **/
GtkWidget *
hildon_date_selector_new_with_year_range (gint min_year,
                                          gint max_year)
{
  GtkWidget *selector;

  g_return_val_if_fail (min_year <= max_year, NULL);

  if (min_year == -1 && min_year == -1) {
    selector = g_object_new (HILDON_TYPE_DATE_SELECTOR,
                             NULL);
  } else if (min_year == -1) {
    selector = g_object_new (HILDON_TYPE_DATE_SELECTOR,
                             "max-year", max_year,
                             NULL);
  } else if (max_year == -1) {
    selector = g_object_new (HILDON_TYPE_DATE_SELECTOR,
                             "min-year", min_year,
                             NULL);
  } else {
    selector = g_object_new (HILDON_TYPE_DATE_SELECTOR,
                             "min-year", min_year,
                             "max-year", max_year,
                             NULL);
  }

  return selector;
}
/**
 * hildon_date_selector_select_current_date:
 * @selector: the #HildonDateSelector
 * @year:  the current year
 * @month: the current month (between 0 and 11)
 * @day:   the current day (between 1 and 31)
 *
 * Sets the current active date on the #HildonDateSelector widget
 *
 * Since: 2.2
 *
 * Returns: %TRUE on success, %FALSE otherwise
 **/
gboolean
hildon_date_selector_select_current_date (HildonDateSelector * selector,
                                          guint year, guint month, guint day)
{
  GtkTreeIter iter;
  gint min_year = 0;
  gint max_year = 0;
  gint num_days = 0;

  min_year = selector->priv->min_year;
  max_year = selector->priv->max_year;

  g_return_val_if_fail (min_year <= year && year <= max_year, FALSE);
  g_return_val_if_fail (month < 12, FALSE);

  num_days = _month_days (month, year);
  g_return_val_if_fail (day > 0 && day <= num_days, FALSE);


  gtk_tree_model_iter_nth_child (selector->priv->year_model, &iter, NULL,
                                 year - min_year);
  hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector),
                                     selector->priv->year_column, &iter,
                                     FALSE);

  gtk_tree_model_iter_nth_child (selector->priv->month_model, &iter, NULL,
                                 month);
  hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector),
                                     selector->priv->month_column, &iter,
                                     FALSE);

  gtk_tree_model_iter_nth_child (selector->priv->day_model, &iter, NULL,
                                 day - 1);
  hildon_touch_selector_select_iter (HILDON_TOUCH_SELECTOR (selector),
                                     selector->priv->day_column, &iter,
                                     FALSE);

  return TRUE;
}


/**
 * hildon_date_selector_get_date:
 * @selector: the #HildonDateSelector
 * @year:  location of the current year, or NULL
 * @month: location of the current month (between 0 and 11), or NULL
 * @day:   location of the current day (between 1 and 31), or NULL
 *
 * Gets the current active date on the #HildonDateSelector widget
 *
 * Since: 2.2
 **/
void
hildon_date_selector_get_date (HildonDateSelector * selector,
                               guint * year, guint * month, guint * day)
{
  GtkTreeIter iter;

  if (year != NULL) {
    if (hildon_touch_selector_get_selected (HILDON_TOUCH_SELECTOR (selector),
                                            selector->priv->year_column, &iter))
      gtk_tree_model_get (selector->priv->year_model,
                          &iter, COLUMN_INT, year, -1);
  }

  if (month != NULL) {
    if (hildon_touch_selector_get_selected (HILDON_TOUCH_SELECTOR (selector),
                                            selector->priv->month_column, &iter))
      gtk_tree_model_get (selector->priv->month_model,
                          &iter, COLUMN_INT, month, -1);
  }

  if (day != NULL) {
    if (hildon_touch_selector_get_selected (HILDON_TOUCH_SELECTOR (selector),
                                            selector->priv->day_column, &iter))
    {
      gtk_tree_model_get (selector->priv->day_model,
                          &iter, COLUMN_INT, day, -1);
    }
/*       *day = *day - 1;  */
  }

}


/**
 * hildon_date_selector_select_month:
 * @selector: the #HildonDateSelector
 * @month: the current month (0-11)
 * @year:  the current year
 *
 * Modify the current month and year on the current active date
 *
 * Utility function to keep this API similar to the previously
 * existing #HildonCalendar widget.
 *
 * Since: 2.2
 *
 * Returns: %TRUE on success, %FALSE otherwise
 **/
gboolean hildon_date_selector_select_month (HildonDateSelector *selector,
                                            guint month, guint year)
{
  guint day = 0;

  hildon_date_selector_get_date (selector, NULL, NULL, &day);

  return hildon_date_selector_select_current_date (selector, year, month, day);
}

/**
 * hildon_date_selector_select_day:
 * @selector: the #HildonDateSelector
 * @day:   the current day (between 1 and 31)
 *
 * Modify the current day on the current active date
 *
 * Utility function to keep this API similar to the previously
 * existing #HildonCalendar widget.
 *
 * Since: 2.2
 **/
void
hildon_date_selector_select_day (HildonDateSelector *selector, guint day)
{
  guint month = 0;
  guint year = 0;

  hildon_date_selector_get_date (selector, &year, &month, NULL);

  hildon_date_selector_select_current_date (selector, year, month, day);
}
