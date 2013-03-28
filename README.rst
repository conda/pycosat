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
(x\ :sub:`1`  or not x\ :sub:`5` or x\ :sub:`4`).
Note that the variable x\ :sub:`2` is not used in any of the clauses,
which means that for each solution with x\ :sub:`2` = True, we must
also have a solution with x\ :sub:`2` = False.  In Python, each clause is
most conveniently represented as a list of integers.  Naturally, it makes
sense to represent each solution also as list of integers, where the sign
corresponds to the boolean value (+ for True and - for False) and the
absolute value corresponds to i\ :sup:`th` variable::

   >>> import pycosat
   >>> cnf = [[1, -5, 4], [-1, 5, 3, 4], [-3, -4]]
   >>> pycosat.solve(cnf)
   [1, -2, -3, -4, 5]

This solution translates to: x\ :sub:`1` = x\ :sub:`5` = True,
x\ :sub:`2` = x\ :sub:`3` = x\ :sub:`4` = False

To find all solution, use ``itersolve``:

   >>> for sol in pycosat.itersolve(cnf):
   ...     print sol
   ...
   [1, -2, -3, -4, 5]
   [1, -2, -3, 4, -5]
   [1, -2, -3, 4, 5]
   ...
   >>> len(list(pycosat.itersolve(cnf)))
   18

In this example, there are a total of 18 possible solutions, which had to
be an even number because x\ :sub:`2` was left unspecified in the clauses.

The fact that ``itersolve`` returns an iterator, makes it many types
of operations very elegant and efficient.  For example, using
the ``itertools`` module from the standard library, here is how one
would construct a list of (up to) 3 solutions::

   >>> import itertools
   >>> list(itertools.islice(pycosat.itersolve(cnf), 3))
   [[1, -2, -3, -4, 5], [1, -2, -3, 4, -5], [1, -2, -3, 4, 5]]


Implementation of itersolve:
----------------------------

How does one go from having found one solution to another solution?
The answer is surprisingly simple.  One adds the *inverse* of the already
found solution as a new clause.  This new clause ensures that another
solution (if any) is searched for.  Here is basically a pure Python
implementation of ``itersolve`` in terms of ``solve``::

   def py_itersolve(clauses): # don't use this function!
       while True:            # (it is only here to explain things)
           sol = pycosat.solve(clauses)
           if isinstance(sol, list):
               yield sol
               clauses.append([-x for x in sol])
           else: # no more solutions -- stop iteration
               return

This implementation has several problems.  Firstly, it is quite slow as
``pycosat.solve`` has to convert the list of clauses over and over and over
again.  Secondly, after calling ``py_itersolve`` the list of clauses will
be modified.  In pycosat, ``itersolve`` is implemented on the C level,
making use of the picosat C interface (which makes it orders of magnitude
faster than the Python implementation above).
