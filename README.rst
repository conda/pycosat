===========================================
pycosat: bindings to picosat (a SAT solver)
===========================================

Python bindings to picosat: http://fmv.jku.at/picosat/

For ease if deployment, the picosat source (namely picosat.c and picosat.h)
is included in this project.  These files have been extracted from the
picosat source (picosat-954.tar.gz), which can be downloaded from the
URL above.


Usage
-----

The ``pycosat`` module has two functions ``solve`` and ``itersolve``,
with both take a list of clauses as an argument.  Each clause is itself
is represented as a list of (non-zero) integers.

The function ``solve`` returns one of the following:
  * one solution (a list of integers)
  * the string "UNSAT" (when the clauses are unsatisfiable)
  * the string "UNKNOWN" (when a solution could not be determined within the
    propagation limit)

The function ``itersolve`` returns an iterator over solutions.  When the
propagation limit is specified, exhausting the iterator may not yield all
possible solution.

Both functions take the following keyword arguments:
  * ``prop_limit``: the propagation limit (integer)
  * ``vars``: number of variables (integer)
  * ``verbose``: the verbosity level (integer)


Example
-------

Let us consider the following clauses (represented using by
the `CNF <http://en.wikipedia.org/wiki/Conjunctive_normal_form>`_)::

   p cnf 5 3
   1 -5 4 0
   -1 5 3 4 0
   -3 -4 0

Now::

   >>> import pycosat
   >>> cnf = [[1, -5, 4], [-1, 5, 3, 4], [-3, -4]]
   >>> pycosat.solve(cnf)
   [1, -2, -3, -4, 5]
   >>> for sol in pycosat.itersolve(cnf):
   ...     print sol
   ...
   [1, -2, -3, -4, 5]
   [1, -2, -3, 4, -5]
   [1, -2, -3, 4, 5]
   ...

In this example, there are a total of 18 possible solutions.
