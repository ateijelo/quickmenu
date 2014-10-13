QuickMenu
=========

Intro
-----

https://github.com/ateijelo/quickmenu/

Create menus easily with a JSON file.

QuickMenu is a simple way to create menus for your notification area. You define
a menu in a JSON file and give it as argument to QuickMenu, who will show it as
an icon in your notification area or system tray.

A sample menu file is included.

Remote Activation
-----------------

If you want to have a global keyboard shortcut that brings up a QuickMenu, you
can give the menu a name with:

    quickmenu --name=menu1 menu1.json
  
and then create a keyboard combination in your desktop environment that calls
the following:

    quickmenu --show=menu1

License
-------

QuickMenu is distributed under the MIT License.
