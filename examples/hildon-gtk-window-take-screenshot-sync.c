#include <stdlib.h>
#include <dirent.h>
#include <hildon/hildon.h>

static gboolean Block;

static unsigned cntfiles(char const *path)
{
  DIR *dir;
  struct dirent const *dent;
  unsigned n;

  if (!(dir = opendir(path)))
    return 0;
  for (n = 0; (dent = readdir(dir)) != NULL; n++)
    ;
  closedir(dir);

  return n;
}

static gboolean cb(gpointer win)
{
  extern void hildon_gtk_window_take_screenshot_sync (
                                      GtkWindow *window, gboolean take);
  void (*fun)(GtkWindow *, gboolean) = Block
    ? hildon_gtk_window_take_screenshot_sync
    : hildon_gtk_window_take_screenshot;

  g_warning("CREATE: %u", cntfiles("/home/user/.cache/launch"));
  fun(win, TRUE);
  g_warning("        %u", cntfiles("/home/user/.cache/launch"));

  g_warning("DELETE: %u", cntfiles("/home/user/.cache/launch"));
  fun(win, FALSE);
  g_warning("        %u", cntfiles("/home/user/.cache/launch"));

  return FALSE;
}

int main(int argc, char const *argv[])
{
  GtkWidget *win, *text;

  Block = !argv[1];
  gtk_init(NULL, NULL);

  text = gtk_label_new("Moi!");
  gtk_widget_modify_font(text, pango_font_description_from_string("300"));

  win = hildon_window_new();
  gtk_window_set_wmclass(GTK_WINDOW(win), "osso_calculator", "Osso_calculator");
  g_signal_connect(win, "destroy", G_CALLBACK(exit), NULL);
  g_signal_connect(win, "button_press_event", G_CALLBACK(cb), NULL);
  gtk_container_add(GTK_CONTAINER(win), text);

  gtk_widget_realize(win);
  gdk_window_set_events(win->window,
            gdk_window_get_events(win->window) | GDK_BUTTON_PRESS_MASK);
  gtk_widget_show_all(win);

  gtk_main();
  return 0;
}
