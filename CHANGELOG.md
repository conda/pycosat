[//]: # (current developments)

## 0.6.6 (2023-10-03)

### Bug fixes

* Fix `PyMem_Calloc()` call signature. (#76)

### Contributors

* @dholth
* @kenodegard



## 0.6.5 (2023-10-02)

### Enhancements

* Update Python version support. (#56)

### Bug fixes

* Use `PyMem_Calloc()` to initialize memory to `0`. (#58)

### Contributors

* @conda-bot
* @dholth made their first contribution in https://github.com/conda/pycosat/pull/54
* @kenodegard



## 0.6.4 (2022-10-24)

* Fix memory leak in blocksol() when iterating over solutions


## 0.6.3 (2017-10-29)

* Revert 'always use Python memory manager'


## 0.6.2 (2017-03-16)

* update picosat to 965
* fixed __version__ attribute on Windows, #21
* fix some typos
* add official Python 3.5 and 3.6 support


## 0.6.1 (2014-04-28)

* fix the initialization of the soliterator type in Py3k, thanks bfroehle
* update picosat from 954 to 957
* add ability of solve and itersolve to take general iterables as clauses arguments, each containing general iterables


## 0.6.0 (2013-04-16)

* add ability to specify clauses as objects which support the iterator protocol
* added a Python 3 version of the 8 queens example which demonstrates the usefulness of the "yield from" statement


## 0.5.0 (2013-04-09)

* add Python 2.5 support
* add module docstring
* add 8 queens problem as an example


## 0.4.2 (2013-04-01)

* fixed types in readme
* add SAT-based Sudoku solver as an example
* add docstings to solve and itersolve
* add test for clauses containing type long literals on Python 2


## 0.4.1 (2013-03-28)

* add documentation
* added more tests
* add error handling for memory management


## 0.4.0 (2013-03-27)

* allow keyword arguments to pycosat.solve and pycosat.itersolve
* add ability to set propagation limit
* fixed some bugs which were causing segfaults


## 0.3.0 (2013-03-26)

* add solution iterator (pycosat.itersolve)
* make number of variables optional argument


## 0.2.0 (2013-03-25)

* fixed MSVC compiler problems
* use Python memory manager in picosat
* release GIL during main picosat computation


## 0.1.0 (2013-03-24)

* initial pre-alpha release
