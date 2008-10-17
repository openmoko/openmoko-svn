#!/usr/bin/python

import ecore
import ecore_evas

try:
  window = ecore_evas.software_x11_16_new(500,500)
except:
  window = ecore_evas.software_x11_new(500,500)

window.title_set("TestWindow")
window.show()

ecore.ecore_main_loop_begin()
