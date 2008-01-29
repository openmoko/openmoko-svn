#include <pygobject.h>
 
void mokoui_register_classes (PyObject *d); 
extern PyMethodDef mokoui_functions[];
 
DL_EXPORT(void)
initmokoui(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    m = Py_InitModule ("mokoui", mokoui_functions);
    d = PyModule_GetDict (m);
 
    mokoui_register_classes (d);
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module mokoui");
    }
}
