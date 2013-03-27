import unittest

import pycosat
from pycosat import solve, itersolve

# -------------------------- utility functions ---------------------------

def read_cnf(path):
    clauses = []
    for line in open(path):
        parts = line.split()
        if not parts or parts[0] == 'c':
            continue
        if parts[0] == 'p':
            assert len(parts) == 4
            assert parts[1] == 'cnf'
            n_vars, n_clauses = [int(n) for n in parts[2:4]]
            continue
        if parts[0] == '%':
            break
        assert parts[-1] == '0'
        clauses.append([int(lit) for lit in parts[:-1]])
    assert len(clauses) == n_clauses
    return clauses, n_vars

def verify(clauses, n_vars, sol):
    sol_vars = {} # variable number -> bool
    for i in sol:
        sol_vars[abs(i)] = bool(i > 0)
    return all(any(sol_vars[abs(i)] ^ bool(i < 0) for i in clause)
               for clause in clauses)

def py_itersolve(clauses, n_vars):
    while True:
        sol = pycosat.solve(clauses, n_vars)
        if isinstance(sol, list):
            yield sol
            clauses.append([-x for x in sol])
        else:
            return

def process_cnf_file(path):
    sys.stdout.write('%30s:  ' % basename(path))
    sys.stdout.flush()

    clauses, n_vars = read_cnf(path)
    sys.stdout.write('vars: %6d   cls: %6d   ' % (n_vars, len(clauses)))
    sys.stdout.flush()
    n_sol = 0
    for sol in itersolve(clauses, n_vars):
        sys.stdout.write('.')
        sys.stdout.flush()
        assert verify(clauses, n_vars, sol)
        n_sol += 1
    sys.stdout.write("%d\n" % n_sol)
    sys.stdout.flush()
    return n_sol

# -------------------------- test clauses --------------------------------

# p cnf 5 3
# 1 -5 4 0
# -1 5 3 4 0
# -3 -4 0
nvars1, clauses1 = 5, [[1, -5, 4], [-1, 5, 3, 4], [-3, -4]]

# p cnf 2 2
# -1 0
# 1 0
nvars2, clauses2 = 2, [[-1], [1]]

# -------------------------- actual unit tests ---------------------------

tests = []

class TestSolve(unittest.TestCase):

    def test_wrong_args(self):
        self.assertRaises(TypeError, solve, [[1, 2], [-3]], 'A')
        self.assertRaises(TypeError, solve, {})
        self.assertRaises(TypeError, solve, ['a'])
        self.assertRaises(TypeError, solve, [[1, 2], [3, None]], 5)
        self.assertRaises(ValueError, solve, [[1, 2], [3, 0]])

    def test_cnf1(self):
        res = solve(clauses1)
        self.assertEqual(res, [1, -2, -3, -4, 5])

    def test_cnf2(self):
        res = solve(clauses2)
        self.assertEqual(res, "UNSAT")

    def test_cnf1_prop_limit(self):
        res = solve(clauses1, prop_limit=2)
        self.assertEqual(res, "UNKNOWN")

    def test_cnf1_vars(self):
        res = solve(clauses1, vars=7)
        self.assertEqual(res, [1, -2, -3, -4, 5, -6, -7])

tests.append(TestSolve)

# -----

class TestIterSolve(unittest.TestCase):

    def test_cnf1(self):
        for sol in itersolve(clauses1, nvars1):
            #sys.stderr.write('%r\n' % repr(sol))
            self.assertTrue(verify(clauses1, nvars1, sol))

        sols = list(itersolve(clauses1, vars=nvars1))
        self.assertEqual(len(sols), 18)
        # ensure solutions are unique
        self.assertEqual(len(set(tuple(sol) for sol in sols)), 18)

    def test_cnf2(self):
        sols = list(itersolve(clauses2, nvars2))
        self.assertEqual(sols, [])

    def test_cnf1_prop_limit(self):
        res = list(itersolve(clauses1, prop_limit=2))
        self.assertEqual(res, [])

tests.append(TestIterSolve)

# ------------------------------------------------------------------------

def run(verbosity=1, repeat=1):
    try:
        print("pycosat version: %r" % pycosat.__version__)
    except AttributeError:
        pass
    suite = unittest.TestSuite()
    for cls in tests:
        for _ in range(repeat):
            suite.addTest(unittest.makeSuite(cls))

    runner = unittest.TextTestRunner(verbosity=verbosity)
    return runner.run(suite)


if __name__ == '__main__':
    import sys
    from os.path import basename

    if len(sys.argv) == 1:
        run()
    else:
        for path in sys.argv[1:]:
            process_cnf_file(path)
