#include <Python.h>

#include "picosat.h"
#ifndef DONT_INCLUDE_PICOSAT
#include "picosat.c"
#endif

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

#ifdef IS_PY3K
#define IS_INT(x)  (PyLong_Check(x))
#else
#define IS_INT(x)  (PyInt_Check(x) || PyLong_Check(x))
#endif


static int add_clause(PicoSAT *picosat, PyObject *list)
{
    PyObject *lit;     /* the literals are integers */
    Py_ssize_t n, i;

    if (!PyList_Check(list)) {
        PyErr_SetString(PyExc_TypeError, "list expected");
        return -1;
    }

    n = PyList_Size(list);
    for (i = 0; i < n; i++) {
        lit = PyList_GetItem(list, i);
        if (lit == NULL)
            return -1;
        if (!IS_INT(lit))  {
            PyErr_SetString(PyExc_TypeError, "interger expected");
            return -1;
        }
        picosat_add(picosat, PyLong_AsLong(lit));
    }
    picosat_add(picosat, 0);
    return 0;
}

static int add_clauses(PicoSAT *picosat, PyObject *list)
{
    PyObject *clause;  /* each clause is a list of intergers */
    Py_ssize_t n, i;

    /* printf("HERE>%s<\n", PyString_AS_STRING(PyObject_Repr(iter))); */
    if (!PyList_Check(list)) {
        PyErr_SetString(PyExc_TypeError, "list expected");
        return -1;
    }

    n = PyList_Size(list);
    for (i = 0; i < n; i++) {
        clause = PyList_GetItem(list, i);
        if (clause == NULL)
            return -1;
        if (add_clause(picosat, clause) < 0)
            return -1;
    }
    return 0;
}

static PyObject* solve(PyObject* self, PyObject* args)
{
    PicoSAT *picosat;
    PyObject *obj;
    int res, max_idx, i, val, vars, verbose = 0;

    if (!PyArg_ParseTuple(args, "iO|i:verbose", &vars, &obj, &verbose))
        return NULL;

    picosat = picosat_init();
    picosat_set_verbosity(picosat, verbose);

    picosat_adjust(picosat, vars);
    if (add_clauses(picosat, obj) < 0)
        return NULL;

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
