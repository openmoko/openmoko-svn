#include <pygobject.h>

void ecore_register_classes(PyObject *d);
extern PyMethodDef ecore_functions[];

DL_EXPORT(void)
initecore(void)
{
  PyObject *m, *d;

  init_pygobject();

  m = Py_InitModule("ecore", ecore_functions);

  d = PyModule_GetDict(m);

  ecore_register_classes(d);

  if (PyErr_Occurred()) {
    Py_FatalError ("can't initialise module ecore");
  }
}
