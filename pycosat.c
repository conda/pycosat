#include <Python.h>

#include "picosat.h"
#ifndef DONT_INCLUDE_PICOSAT
#include "picosat.c"
#endif

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#ifdef IS_PY3K
#define IS_INT(x)  (PyLong_Check(x))
#else
#define IS_INT(x)  (PyInt_Check(x) || PyLong_Check(x))
#endif


static int add_clause(PicoSAT *picosat, PyObject *clause)
{
    PyObject *lit;     /* the literals are integers */
    Py_ssize_t n, i;

    if (!PyList_Check(clause)) {
        PyErr_SetString(PyExc_TypeError, "list expected");
        return -1;
    }

    n = PyList_Size(clause);
    for (i = 0; i < n; i++) {
        lit = PyList_GetItem(clause, i);
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

static int add_clauses(PicoSAT *picosat, PyObject *clauses)
{
    PyObject *item;             /* each clause is a list of intergers */
    Py_ssize_t n, i;

    /* printf("HERE>%s<\n", PyString_AS_STRING(PyObject_Repr(iter))); */
    if (!PyList_Check(clauses)) {
        PyErr_SetString(PyExc_TypeError, "list expected");
        return -1;
    }

    n = PyList_Size(clauses);
    for (i = 0; i < n; i++) {
        item = PyList_GetItem(clauses, i);
        if (item == NULL)
            return -1;
        if (add_clause(picosat, item) < 0)
            return -1;
    }
    return 0;
}

static PyObject* solve(PyObject* self, PyObject* args)
{
    PicoSAT *picosat;
    PyObject *clauses;          /* list of clauses */
    PyObject *list;             /* return value if satisfiable */
    Py_ssize_t max_idx, i;
    int res, val, vars, verbose = 0;

    if (!PyArg_ParseTuple(args, "iO|i:verbose", &vars, &clauses, &verbose))
        return NULL;

    picosat = picosat_init();
    picosat_set_verbosity(picosat, verbose);

    picosat_adjust(picosat, vars);
    if (add_clauses(picosat, clauses) < 0)
        return NULL;

    if (verbose >= 2)
        picosat_print(picosat, stdout);

    res = picosat_sat(picosat, -1);

    switch (res) {
    case PICOSAT_SATISFIABLE:
        max_idx = picosat_variables(picosat);
        list = PyList_New(max_idx);
        if (list == NULL)
            return NULL;
        for (i = 1; i <= max_idx; i++) {
            val = picosat_deref(picosat, i);
            assert(val == -1 || val == 1);
            if (PyList_SetItem(list, i - 1,
                               PyBool_FromLong(val < 0 ? 0 : 1)) < 0)
                return NULL;
        }
        return list;

    case PICOSAT_UNSATISFIABLE:
        Py_RETURN_FALSE;

    case PICOSAT_UNKNOWN:
        Py_RETURN_NONE;

    default:
        PyErr_Format(PyExc_SystemError,
                     "unknown picosat return value: %d", res);
        return NULL;
    }
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
PyMODINIT_FUNC PyInit_pycosat(void)
#else
PyMODINIT_FUNC initpycosat(void)
#endif
{
    PyObject *m;

#ifdef IS_PY3K
    m = PyModule_Create(&moduledef);
#else
    m = Py_InitModule3("pycosat", module_functions, 0);
#endif
    if (m == NULL)
        return NULL;

#ifdef IS_PY3K
    return m;
#endif
}
