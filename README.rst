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

Let us consider the following clauses, represented using by
the DIMACS `cnf <http://en.wikipedia.org/wiki/Conjunctive_normal_form>`_
format::

   p cnf 5 3
   1 -5 4 0
   -1 5 3 4 0
   -3 -4 0

Here, we have 5 variables and 3 clauses, the first clause being
(x_1 or not x_5 or x_4).  Note that the variable x_2 is not used in any
of the clauses, which means that for each solution with x_2 = True, we must
also have a solution with x_2 = False.  In Python, each clause is most
conveniently represented as a list of integers.  Naturally, it makes
sense to represent each solution also as list of integers, where the sign
corresponds to the boolean value (+ for True and - for False) and the
absolute value corresponds to i^th variable::

   >>> import pycosat
   >>> cnf = [[1, -5, 4], [-1, 5, 3, 4], [-3, -4]]
   >>> pycosat.solve(cnf)
   [1, -2, -3, -4, 5]

This solution translates to: x_1 = x_5 = True, x_2 = x_3 = x_4 = False

To find all solution to the problems::

   >>> for sol in pycosat.itersolve(cnf):
   ...     print sol
   ...
   [1, -2, -3, -4, 5]
   [1, -2, -3, 4, -5]
   [1, -2, -3, 4, 5]
   ...

In this example, there are a total of 18 possible solutions.
