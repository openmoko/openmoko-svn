#!/usr/bin/python

import ecore
import ecore_evas


def on_resize():
  print "resizing window"

try:
  window = ecore_evas.software_x11_16_new(500,500)
except:
  window = ecore_evas.software_x11_new(500,500)

window.title_set("TestWindow")
window.cb_resize_set(on_resize)
window.show()

ecore.ecore_main_loop_begin()
