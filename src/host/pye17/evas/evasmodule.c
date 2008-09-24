#include <pygobject.h>

void evas_register_classes(PyObject *d);
extern PyMethodDef evas_functions[];

DL_EXPORT(void)
initevas(void)
{
  PyObject *m, *d;

  init_pygobject();

  m = Py_InitModule("evas", evas_functions);

  d = PyModule_GetDict(m);

  evas_register_classes(d);

  if (PyErr_Occurred()) {
    Py_FatalError ("can't initialise module evas");
  }
}
