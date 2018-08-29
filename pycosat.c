/*
  Copyright (c) 2013-2017, Ilan Schnell, Continuum Analytics, Inc.
  Python bindings to picosat (http://fmv.jku.at/picosat/)
  This file is published under the same license as picosat itself, which
  uses an MIT style license.
*/
#define PYCOSAT_URL  "https://pypi.python.org/pypi/pycosat"
#define PYCOSAT_VERSION  "0.6.3"

#include <Python.h>

#ifdef _MSC_VER
#define NGETRUSAGE
#define inline __inline
#endif

#include "picosat.h"
#ifndef DONT_INCLUDE_PICOSAT
#include "picosat.c"
#endif

/* When defined, picosat uses the Python memory manager
   We cannot do this while we:
   "release GIL during main picosat computation"
   https://github.com/ContinuumIO/pycosat/commit/f50c89a10db2e87c3e6b896c43ce0549f92039d0
   I am assuming here that we would rather release the GIL than use the Python mem allocation
   though there is no comments to explain the trade-offs between the Python vs libc allocators
*/
/* #define WITH_PYMEM */


#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#ifdef IS_PY3K
#define PyInt_FromLong  PyLong_FromLong
#define IS_INT(x)  (PyLong_Check(x))
#else
#define IS_INT(x)  (PyInt_Check(x) || PyLong_Check(x))
#endif

#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION <= 5
#define PyUnicode_FromString  PyString_FromString
#endif

#if defined(WITH_PYMEM)
/* the following three adapter functions are used as arguments to
   picosat_minit, such that picosat used the Python memory manager */
inline static void *py_malloc(void *mmgr, size_t bytes)
{
    return PyMem_Malloc(bytes);
}

inline static void *py_realloc(void *mmgr, void *ptr, size_t old, size_t new)
{
    return PyMem_Realloc(ptr, new);
}

inline static void py_free(void *mmgr, void *ptr, size_t bytes)
{
    PyMem_Free(ptr);
}
#endif

/* Add the inverse of the (current) solution to the clauses.
   This function is essentially the same as the function blocksol in app.c
   in the picosat source. */
static int blocksol(PicoSAT *picosat, signed char *mem)
{
    int max_idx, i;

    max_idx = picosat_variables(picosat);
    if (mem == NULL) {
        mem = PyMem_Malloc(max_idx + 1);
        if (mem == NULL) {
            PyErr_NoMemory();
            return -1;
        }
    }
    for (i = 1; i <= max_idx; i++)
        mem[i] = (picosat_deref(picosat, i) > 0) ? 1 : -1;

    for (i = 1; i <= max_idx; i++)
        picosat_add(picosat, (mem[i] < 0) ? i : -i);

    picosat_add(picosat, 0);
    return 0;
}

static int add_clause(PicoSAT *picosat, PyObject *clause)
{
    PyObject *iterator;         /* each clause is an iterable of literals */
    PyObject *lit;              /* the literals are integers */
    int v;

    iterator = PyObject_GetIter(clause);
    if (iterator == NULL)
        return -1;

    while ((lit = PyIter_Next(iterator)) != NULL) {
        if (!IS_INT(lit))  {
            Py_DECREF(lit);
            Py_DECREF(iterator);
            PyErr_SetString(PyExc_TypeError, "integer expected");
            return -1;
        }
        v = PyLong_AsLong(lit);
        Py_DECREF(lit);
        if (v == 0) {
            Py_DECREF(iterator);
            PyErr_SetString(PyExc_ValueError, "non-zero integer expected");
            return -1;
        }
        picosat_add(picosat, v);
    }
    Py_DECREF(iterator);
    if (PyErr_Occurred())
        return -1;
    picosat_add(picosat, 0);
    return 0;
}

static int _add_clauses_from_array_int(
        PicoSAT *picosat, const size_t array_length, const int *array)
{
    size_t k;

    if (array_length == 0)
        return 0;
    if (array[array_length - 1] != 0) {
        PyErr_SetString(PyExc_ValueError, "last clause not terminated by zero");
        return -1;
    }
    for (k = 0; k < array_length; ++k)
        picosat_add(picosat, array[k]);
    return 0;
}

static int _add_clauses_from_array_long(
        PicoSAT *picosat, const size_t array_length, const long *array)
{
    size_t k;

    if (array_length == 0)
        return 0;
    if (array[array_length - 1] != 0) {
        PyErr_SetString(PyExc_ValueError, "last clause not terminated by zero");
        return -1;
    }
    for (k = 0; k < array_length; ++k)
        picosat_add(picosat, array[k]);
    return 0;
}

static int _add_clauses_from_array_long_long(
        PicoSAT *picosat, const size_t array_length, const long long *array)
{
    size_t k;

    if (array_length == 0)
        return 0;
    if (array[array_length - 1] != 0) {
        PyErr_SetString(PyExc_ValueError, "last clause not terminated by zero");
        return -1;
    }
    for (k = 0; k < array_length; ++k)
        picosat_add(picosat, array[k]);
    return 0;
}

static int _add_clauses_from_buffer_info(
        PicoSAT *picosat, PyObject *buffer_info, const size_t itemsize)
{
    PyObject *py_array_length;
    long array_length;
    PyObject *py_array_address;
    const void *array;

    py_array_length = PyTuple_GetItem(buffer_info, 1);
    if (py_array_length == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: could not get array length");
        return -1;
    }
    array_length = PyLong_AsLong(py_array_length);
    if (array_length < 0) {
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: could not get array length");
        return -1;
    }
    py_array_address = PyTuple_GetItem(buffer_info, 0);
    if (py_array_address == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: could not get array address");
        return -1;
    }
    array = PyLong_AsVoidPtr(py_array_address);
    if (py_array_address == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: could not get array address");
        return -1;
    }
    if (itemsize == sizeof(int))
        return _add_clauses_from_array_int(
                picosat, array_length, (const int *) array);
    if (itemsize == sizeof(long))
        return _add_clauses_from_array_long(
                picosat, array_length, (const long *) array);
    if (itemsize == sizeof(long long))
        return _add_clauses_from_array_long_long(
                picosat, array_length, (const long long *) array);
    PyErr_Format(PyExc_ValueError,
                 "invalid clause array: invalid itemsize '%ld'", itemsize);
    return -1;
}

static int _check_array_typecode(PyObject *clauses) {
    PyObject *py_typecode;
    PyObject *typecode_bytes;
    const char *typecode_cstr;
    char typecode;

    py_typecode = PyObject_GetAttrString(clauses, "typecode");
    if (py_typecode == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: typecode is NULL");
        return -1;
    }
#ifdef IS_PY3K
    typecode_bytes = PyUnicode_AsASCIIString(py_typecode);
    Py_DECREF(py_typecode);
    if (typecode_bytes == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: could not get typecode bytes");
        return -1;
    }
#else
    typecode_bytes = py_typecode;
#endif
    typecode_cstr = PyBytes_AsString(typecode_bytes);
    if (typecode_cstr == NULL) {
        Py_DECREF(typecode_bytes);
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: could not get typecode cstring");
        return -1;
    }
    typecode = typecode_cstr[0];
    if (typecode == '\0' || typecode_cstr[1] != '\0') {
        PyErr_Format(PyExc_ValueError,
                     "invalid clause array: invalid typecode '%s'",
                     typecode_cstr);
        Py_DECREF(typecode_bytes);
        return -1;
    }
    Py_DECREF(typecode_bytes);
    if (typecode != 'i' && typecode != 'l' && typecode != 'q') {
        PyErr_Format(PyExc_ValueError,
                     "invalid clause array: invalid typecode '%c'", typecode);
        return -1;
    }
    return 0;
}

static int add_clauses_array(PicoSAT *picosat, PyObject *clauses)
{
    long itemsize;
    int ret;

    if (_check_array_typecode(clauses) == -1)
        return -1;
    PyObject *py_itemsize = PyObject_GetAttrString(clauses, "itemsize");
    if (py_itemsize == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: itemsize is NULL");
        return -1;
    }
    itemsize = PyLong_AsLong(py_itemsize);
    Py_DECREF(py_itemsize);
    if (itemsize < 0) {
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: could not get itemsize");
        return -1;
    }
    PyObject *buffer_info = PyObject_CallMethod(clauses, "buffer_info", NULL);
    if (buffer_info == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "invalid clause array: buffer_info is NULL");
        return -1;
    }
    ret = _add_clauses_from_buffer_info(picosat, buffer_info, itemsize);
    Py_DECREF(buffer_info);
    return ret;
}

static int add_clauses(PicoSAT *picosat, PyObject *clauses)
{
    PyObject *iterator;       /* clauses can be any iterable */
    PyObject *item;           /* each clause is an iterable of integers */
    int ret;

    if (
        PyObject_HasAttr(clauses, PyUnicode_FromString("buffer_info")) &&
        PyObject_HasAttr(clauses, PyUnicode_FromString("typecode")) &&
        PyObject_HasAttr(clauses, PyUnicode_FromString("itemsize"))
    ) {
        ret = add_clauses_array(picosat, clauses);
        if (PyErr_Occurred())
            return -1;
        return ret;
    }

    iterator = PyObject_GetIter(clauses);
    if (iterator == NULL)
        return -1;

    while ((item = PyIter_Next(iterator)) != NULL) {
        if (add_clause(picosat, item) < 0) {
            Py_DECREF(item);
            Py_DECREF(iterator);
            return -1;
        }
        Py_DECREF(item);
    }
    Py_DECREF(iterator);
    if (PyErr_Occurred())
        return -1;
    return 0;
}

static PicoSAT* setup_picosat(PyObject *args, PyObject *kwds)
{
    PicoSAT *picosat;
    PyObject *clauses;          /* iterable of clauses */
    int vars = -1, verbose = 0;
    unsigned long long prop_limit = 0;
    static char* kwlist[] = {"clauses",
                             "vars", "verbose", "prop_limit", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|iiK:(iter)solve", kwlist,
                                     &clauses,
                                     &vars, &verbose, &prop_limit))
        return NULL;

#if defined(WITH_PYMEM)
    picosat = picosat_minit(NULL, py_malloc, py_realloc, py_free);
#else
    picosat = picosat_init();
#endif
    picosat_set_verbosity(picosat, verbose);
    if (vars != -1)
        picosat_adjust(picosat, vars);

    if (prop_limit)
        picosat_set_propagation_limit(picosat, prop_limit);

    if (add_clauses(picosat, clauses) < 0) {
        picosat_reset(picosat);
        return NULL;
    }

    if (verbose >= 2)
        picosat_print(picosat, stdout);

    return picosat;
}

/* read the solution from the picosat object and return a Python list */
static PyObject* get_solution(PicoSAT *picosat)
{
    PyObject *list;
    int max_idx, i, v;

    max_idx = picosat_variables(picosat);
    list = PyList_New((Py_ssize_t) max_idx);
    if (list == NULL) {
        picosat_reset(picosat);
        return NULL;
    }
    for (i = 1; i <= max_idx; i++) {
        v = picosat_deref(picosat, i);
        assert(v == -1 || v == 1);
        if (PyList_SetItem(list, (Py_ssize_t) (i - 1),
                           PyInt_FromLong((long) (v * i))) < 0) {
            Py_DECREF(list);
            picosat_reset(picosat);
            return NULL;
        }
    }
    return list;
}

static PyObject* solve(PyObject *self, PyObject *args, PyObject *kwds)
{
    PicoSAT *picosat;
    PyObject *result = NULL;    /* return value */
    int res;

    picosat = setup_picosat(args, kwds);
    if (picosat == NULL)
        return NULL;

    Py_BEGIN_ALLOW_THREADS      /* release GIL */
    res = picosat_sat(picosat, -1);
    Py_END_ALLOW_THREADS

    switch (res) {
    case PICOSAT_SATISFIABLE:
        result = get_solution(picosat);
        break;

    case PICOSAT_UNSATISFIABLE:
        result = PyUnicode_FromString("UNSAT");
        break;

    case PICOSAT_UNKNOWN:
        result = PyUnicode_FromString("UNKNOWN");
        break;

    default:
        PyErr_Format(PyExc_SystemError, "picosat return value: %d", res);
    }

    picosat_reset(picosat);
    return result;
}

PyDoc_STRVAR(solve_doc,
"solve(clauses [, kwargs]) -> list\n\
\n\
Solve the SAT problem for the clauses, and return a solution as a\n\
list of integers, or one of the strings \"UNSAT\", \"UNKNOWN\".\n\
Please see " PYCOSAT_URL " for more details.");

/*********************** Solution Iterator *********************/

typedef struct {
    PyObject_HEAD
    PicoSAT *picosat;
    signed char *mem;           /* temporary storage */
} soliterobject;

static PyTypeObject SolIter_Type;

#define SolIter_Check(op)  PyObject_TypeCheck(op, &SolIter_Type)

static PyObject* itersolve(PyObject *self, PyObject *args, PyObject *kwds)
{
    soliterobject *it;          /* iterator to be returned */

    it = PyObject_GC_New(soliterobject, &SolIter_Type);
    if (it == NULL)
        return NULL;

    it->picosat = setup_picosat(args, kwds);
    if (it->picosat == NULL)
        return NULL;

    it->mem = NULL;
    PyObject_GC_Track(it);
    return (PyObject *) it;
}

PyDoc_STRVAR(itersolve_doc,
"itersolve(clauses [, kwargs]) -> iterator\n\
\n\
Solve the SAT problem for the clauses, and return an iterator over\n\
the solutions (which are lists of integers).\n\
Please see " PYCOSAT_URL " for more details.");

static PyObject* soliter_next(soliterobject *it)
{
    PyObject *result = NULL;    /* return value */
    int res;

    assert(SolIter_Check(it));

    Py_BEGIN_ALLOW_THREADS      /* release GIL */
    res = picosat_sat(it->picosat, -1);
    Py_END_ALLOW_THREADS

    switch (res) {
    case PICOSAT_SATISFIABLE:
        result = get_solution(it->picosat);
        if (result == NULL) {
            PyErr_SetString(PyExc_SystemError, "failed to create list");
            return NULL;
        }
        /* add inverse solution to the clauses, for next iteration */
        if (blocksol(it->picosat, it->mem) < 0)
            return NULL;
        break;

    case PICOSAT_UNSATISFIABLE:
    case PICOSAT_UNKNOWN:
        /* no more solutions -- stop iteration */
        break;

    default:
        PyErr_Format(PyExc_SystemError, "picosat return value: %d", res);
    }
    return result;
}

static void soliter_dealloc(soliterobject *it)
{
    PyObject_GC_UnTrack(it);
    if (it->mem)
        PyMem_Free(it->mem);
    picosat_reset(it->picosat);
    PyObject_GC_Del(it);
}

static int soliter_traverse(soliterobject *it, visitproc visit, void *arg)
{
    return 0;
}

static PyTypeObject SolIter_Type = {
#ifdef IS_PY3K
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                                        /* ob_size */
#endif
    "soliterator",                            /* tp_name */
    sizeof(soliterobject),                    /* tp_basicsize */
    0,                                        /* tp_itemsize */
    /* methods */
    (destructor) soliter_dealloc,             /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    PyObject_GenericGetAttr,                  /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,  /* tp_flags */
    0,                                        /* tp_doc */
    (traverseproc) soliter_traverse,          /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    PyObject_SelfIter,                        /* tp_iter */
    (iternextfunc) soliter_next,              /* tp_iternext */
    0,                                        /* tp_methods */
};

/*************************** Method definitions *************************/

/* declaration of methods supported by this module */
static PyMethodDef module_functions[] = {
    {"solve",     (PyCFunction) solve,     METH_VARARGS | METH_KEYWORDS,
      solve_doc},
    {"itersolve", (PyCFunction) itersolve, METH_VARARGS | METH_KEYWORDS,
      itersolve_doc},
    {NULL,        NULL}  /* sentinel */
};

PyDoc_STRVAR(module_doc, "\
pycosat: bindings to PicoSAT\n\
============================\n\n\
There are two functions in this module, solve and itersolve.\n\
Please see " PYCOSAT_URL " for more details.");

/* initialization routine for the shared library */
#ifdef IS_PY3K
static PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT, "pycosat", module_doc, -1, module_functions,
};
PyMODINIT_FUNC PyInit_pycosat(void)
#else
PyMODINIT_FUNC initpycosat(void)
#endif
{
    PyObject *m;

#ifdef IS_PY3K
    if (PyType_Ready(&SolIter_Type) < 0)
        return NULL;
    m = PyModule_Create(&moduledef);
    if (m == NULL)
        return NULL;
#else
    if (PyType_Ready(&SolIter_Type) < 0)
        return;
    m = Py_InitModule3("pycosat", module_functions, module_doc);
    if (m == NULL)
        return;
#endif

    PyModule_AddObject(m, "__version__",
                       PyUnicode_FromString(PYCOSAT_VERSION));

#ifdef IS_PY3K
    return m;
#endif
}
