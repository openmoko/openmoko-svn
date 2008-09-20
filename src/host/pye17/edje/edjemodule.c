#include <pygobject.h>
 
void edje_register_classes (PyObject *d); 
extern PyMethodDef edje_functions[];
  
DL_EXPORT(void)
initedje(void)
{
  PyObject *m, *d;
       
  init_pygobject ();
            
  m = Py_InitModule ("edje", edje_functions);
  d = PyModule_GetDict (m);
                     
  edje_register_classes (d);
                          
  if (PyErr_Occurred ()) {
    Py_FatalError ("can't initialise module edje");
  }
}
                                          