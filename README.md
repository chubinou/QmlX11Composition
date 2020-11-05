How to build
============

to build this sample you need:

  - meson
  - xcb (render/damage/composite)
  - Qt (>= 5.11) with Core, Widgets, X11extra, QtDeclarative, and QtQuickControls2
  - libvlc

on ubuntu/debian something like this should work

```
  sudo apt install \
      meson \
      qtbase5-dev qtdeclarative5-dev libqt5x11extras5-dev qml-module-qtquick-controls2 \
      libxcb-composite0-dev libxcb-damage0-dev  libxcb-render0-dev \
      libvlc-dev
```

What to report
==============

* Is there only one window (Or 3)

* Is the video visible

* Is the interface visible (translucent red with spinning text)

* Is the video playing smoothly

* press the rotate button. Is the video still playing smoothly

* can you edit the input field on the top left

----

* what is your OS (cat /etc/os-release)

* what window manager are you using

* Do you use a compositor

  * if so which one

  * if you know it, what backend is it using (xrender, opengl)

* what is your X11 version  

* Do you have a 3D controler. 

please report your results here: https://lite.framacalc.org/9jwy-qmlx11composition

Known issues
============

* multi screen is not handled at the moment

* there are some visual glitches when resizing

* closing the window won't close the program properly, do a Ctrl-C
