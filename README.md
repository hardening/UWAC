UWAC
====

UWAC stands for Using Wayland As Client, it tries to provide a library to help porting existing 
Xlib application without using heavy toolkits that are already wayland compatible (SDL, Qt5, Gtk, ...).

UWAC tries to keep the Xlib spirit to minimize changes needed to port the application to wayland.


# Supported features

UWAC is in heavy development so it's far from beeing feature full, anyway it supports the following:

* share graphic content with the wayland compositor using the wl_shm protocol. OpenGl
is not supported yet;
* triple buffering of window content: so that you can draw in a buffer, have previously
submitted one and be waiting for the last one to be released by the compositor;
* handle seats with keyboard, pointer (mouse) and touch devices. UWAC is multi-seat aware (even if
most application may don't care);
* an event system that looks like the X11 one. With extra bonus that you will be notified when it's a good idea to 
update graphical content for a frame;

# Dependencies

We have tried to minimize the libraries required to compile UWAC, today the dependencies are:

* wayland client
* pixman
* XKB common
