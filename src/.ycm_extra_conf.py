from os import path

src_dir = path.dirname( path.realpath( __file__ ) )

flags_gtk3 = [

'-x', 'c',

'-I', src_dir,
'-I', path.join(src_dir, 'proto'),
'-I', '/usr/include/gtk-3.0',
'-I', '/usr/include/at-spi2-atk/2.0',
'-I', '/usr/include/at-spi-2.0',
'-I', '/usr/include/dbus-1.0',
'-I', '/usr/lib/x86_64-linux-gnu/dbus-1.0/include',
'-I', '/usr/include/gtk-3.0',
'-I', '/usr/include/gio-unix-2.0/',
'-I', '/usr/include/mirclient',
'-I', '/usr/include/mircommon',
'-I', '/usr/include/mircookie',
'-I', '/usr/include/cairo',
'-I', '/usr/include/pango-1.0',
'-I', '/usr/include/harfbuzz',
'-I', '/usr/include/pango-1.0',
'-I', '/usr/include/atk-1.0',
'-I', '/usr/include/cairo',
'-I', '/usr/include/pixman-1',
'-I', '/usr/include/freetype2',
'-I', '/usr/include/libpng12',
'-I', '/usr/include/gdk-pixbuf-2.0',
'-I', '/usr/include/libpng12',
'-I', '/usr/include/glib-2.0',
'-I', '/usr/lib/x86_64-linux-gnu/glib-2.0/include',
'-DHAVE_CONFIG_H',
'-DFEAT_GUI_GTK',
'-U_FORTIFY_SOURCE',
'-D_FORTIFY_SOURCE=1'

]

flags_gtk2 = [

'-x', 'c',

'-I', src_dir,
'-I', path.join(src_dir, 'proto'),
'-I', '/usr/include/gtk-2.0',
'-I', '/usr/lib/x86_64-linux-gnu/gtk-2.0/include',
'-I', '/usr/include/gio-unix-2.0/',
'-I', '/usr/include/cairo',
'-I', '/usr/include/pango-1.0',
'-I', '/usr/include/atk-1.0',
'-I', '/usr/include/cairo',
'-I', '/usr/include/pixman-1',
'-I', '/usr/include/libpng12',
'-I', '/usr/include/gdk-pixbuf-2.0',
'-I', '/usr/include/libpng12',
'-I', '/usr/include/pango-1.0',
'-I', '/usr/include/harfbuzz',
'-I', '/usr/include/pango-1.0',
'-I', '/usr/include/glib-2.0',
'-I', '/usr/lib/x86_64-linux-gnu/glib-2.0/include',
'-I', '/usr/include/freetype2',
'-DHAVE_CONFIG_H',
'-DFEAT_GUI_GTK',
'-U_FORTIFY_SOURCE',
'-D_FORTIFY_SOURCE=1'

]

def FlagsForFile( filename, **kwargs ):
  return { 'flags': flags_gtk2 }
