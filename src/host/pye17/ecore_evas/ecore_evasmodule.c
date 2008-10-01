#include <pygobject.h>

void ecore_evas_register_classes(PyObject *d);
extern PyMethodDef ecore_evas_functions[];

DL_EXPORT(void)
initecore_evas(void)
{
  PyObject *m, *d;

  init_pygobject();

  m = Py_InitModule("ecore_evas", ecore_evas_functions);

  d = PyModule_GetDict(m);

  ecore_evas_register_classes(d);

  if (PyErr_Occurred()) {
    Py_FatalError("can't register ecore_evas functions");
  }

  if (ecore_evas_init() != 1) {
    Py_FatalError("can't initialise module ecore_evas");
  }
}
