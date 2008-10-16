#!/usr/bin/python

import ecore                                                      
import ecore_evas                                                 

window = ecore_evas.software_x11_new(500,500)                     
window.show()                                                     
ecore.ecore_main_loop_begin()
