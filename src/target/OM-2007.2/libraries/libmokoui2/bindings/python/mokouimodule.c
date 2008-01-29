#include <pygobject.h>
 
void moko_register_classes (PyObject *d); 
extern PyMethodDef moko_functions[];
 
DL_EXPORT(void)
initmoko(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    m = Py_InitModule ("moko", moko_functions);
    d = PyModule_GetDict (m);
 
    moko_register_classes (d);
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module trayicon");
    }
}
