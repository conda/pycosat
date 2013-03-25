#include <Python.h>

#ifdef _MSC_VER
#define NGETRUSAGE
#define inline __inline
#endif

#include "picosat.h"
#ifndef DONT_INCLUDE_PICOSAT
#include "picosat.c"
#endif

/* when defined, picosat uses the Python memory manager */
#define WITH_PYMEM


#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#ifdef IS_PY3K
#define PyInt_FromLong  PyLong_FromLong
#define IS_INT(x)  (PyLong_Check(x))
#else
#define IS_INT(x)  (PyInt_Check(x) || PyLong_Check(x))
#endif


#ifdef WITH_PYMEM
static void *py_malloc(void *mmgr, size_t bytes) {
    return PyMem_Malloc(bytes);
}
static void *py_realloc(void *mmgr, void *ptr, size_t old, size_t new) {
    return PyMem_Realloc(ptr, new);
}
static void py_free(void *mmgr, void *ptr, size_t bytes) {
    PyMem_Free(ptr);
}
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
    PyObject *result = NULL;    /* return value */
    int res, val, max_idx, i, vars, verbose = 0;

    if (!PyArg_ParseTuple(args, "iO|i:verbose", &vars, &clauses, &verbose))
        return NULL;

#ifdef WITH_PYMEM
    picosat = picosat_minit(NULL, py_malloc, py_realloc, py_free);
#else
    picosat = picosat_init();
#endif
    picosat_set_verbosity(picosat, verbose);

    picosat_adjust(picosat, vars);
    if (add_clauses(picosat, clauses) < 0)
        return NULL;

    if (verbose >= 2)
        picosat_print(picosat, stdout);

    Py_BEGIN_ALLOW_THREADS  /* release GIL */
    res = picosat_sat(picosat, -1);
    Py_END_ALLOW_THREADS

    switch (res) {
    case PICOSAT_SATISFIABLE:
        max_idx = picosat_variables(picosat);
        result = PyList_New((Py_ssize_t) max_idx);
        if (result == NULL)
            return NULL;
        for (i = 1; i <= max_idx; i++) {
            val = picosat_deref(picosat, i);
            assert(val == -1 || val == 1);
            if (PyList_SetItem(result, (Py_ssize_t) (i - 1),
                               PyInt_FromLong((long) (val * i))))
                return NULL;
        }
        break;

    case PICOSAT_UNSATISFIABLE:
        result = PyUnicode_FromString("UNSAT");
        break;

    case PICOSAT_UNKNOWN:
        result = PyUnicode_FromString("UNKNOWN");
        break;

    default:
        PyErr_Format(PyExc_SystemError,
                     "unknown picosat return value: %d", res);
    }

    picosat_reset(picosat);
    return result;
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
    if (m == NULL)
        return NULL;
#else
    m = Py_InitModule3("pycosat", module_functions, 0);
    if (m == NULL)
        return;
#endif

#ifdef PYCOSAT_VERSION
    PyModule_AddObject(m, "__version__",
                       PyUnicode_FromString(PYCOSAT_VERSION));
#endif

#ifdef IS_PY3K
    return m;
#endif
}
