#include <Python.h>

#include "picosat.h"
#include "picosat.c"


#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#ifdef IS_PY3K
#include "bytesobject.h"
#define PyString_FromStringAndSize  PyBytes_FromStringAndSize
#define PyString_Check  PyBytes_Check
#define PyString_Size  PyBytes_Size
#define PyString_AsString  PyBytes_AsString
#endif


static PyObject* solve(PyObject* self, PyObject* args)
{
    PicoSAT *picosat;
    PyObject *x;
    int res, max_idx, i, val, vars, verbose = 0;
    
    if (!PyArg_ParseTuple(args, "iO|i:verbose", &vars, &x, &verbose))
        return NULL;

    printf("Here\n");

    picosat = picosat_init();
    picosat_set_verbosity(picosat, 0);

    picosat_adjust(picosat, 5);
    picosat_add(picosat,  1);
    picosat_add(picosat, -5);
    picosat_add(picosat,  4);
    picosat_add(picosat,  0);

    picosat_add(picosat, -1);
    picosat_add(picosat,  5);
    picosat_add(picosat,  3);
    picosat_add(picosat,  4);
    picosat_add(picosat,  0);

    picosat_add(picosat, -3);
    picosat_add(picosat, -4);
    picosat_add(picosat,  0);

    picosat_print(picosat, stdout);

    res = picosat_sat(picosat, -1);

    printf("res=%d\n", res);

    max_idx = picosat_variables(picosat);
    for (i = 1; i <= max_idx; i++) {
        val = picosat_deref (picosat, i);
        printf("i=%d lit=%d\n", i, val);
    }

    Py_RETURN_NONE;
}


/* declaration of methods supported by this module */
static PyMethodDef module_functions[] = {
    {"solve", (PyCFunction) solve, METH_VARARGS},
    {NULL,    NULL}  /* sentinel */
};

/* initialization routine for the shared libary */
#ifdef IS_PY3K
static PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT, "pycosat", 0, -1, module_functions,
};

PyMODINIT_FUNC
PyInit_pycosat(void)
{
    PyObject *m;

    m = PyModule_Create(&moduledef);
    if (m == NULL)
        return NULL;
    return m;
}
#else
PyMODINIT_FUNC
initpycosat(void)
{
    Py_InitModule("pycosat", module_functions);
}
#endif
