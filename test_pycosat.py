import copy
import unittest

import pycosat

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
    return n_vars, clauses


def verify(n_vars, clauses, sol):
    sol_vars = {} # variable number -> bool
    for i in sol:
        sol_vars[abs(i)] = bool(i > 0)
    return all(any(sol_vars[abs(i)] ^ bool(i < 0) for i in clause)
               for clause in clauses)


def solveall(n_vars, clauses):
    while True:
        sol = pycosat.solve(n_vars, clauses)
        if isinstance(sol, list):
            yield sol
            clauses.append([-x for x in sol])
        else:
            return

# -------------------------- actual unit tests ---------------------------

tests = []

class TestSolver(unittest.TestCase):

    def test_sat_1(self):
        """
        p cnf 5 3
        1 -5 4 0
        -1 5 3 4 0
        -3 -4 0
        """
        res = pycosat.solve(5, [[1, -5, 4],
                                [-1, 5, 3, 4],
                                [-3, -4]])
        self.assertEqual(res, [1, -2, -3, -4, 5])

    def test_sat_all_1(self):
        clauses = [[1, -5, 4],
                   [-1, 5, 3, 4],
                   [-3, -4]]
        res = pycosat.solveall(5, copy.deepcopy(clauses))
        self.assertEqual(len(res), 18)
        self.assertEqual(len(set(tuple(sol) for sol in res)), 18)
        for sol in res:
            #sys.stderr.write('%r\n' % repr(sol))
            self.assertTrue(verify(5, clauses, sol))

    def test_unsat_1(self):
        """
        p cnf 2 2
        -1 0
        1 0
        """
        res = pycosat.solve(2, [[-1],
                                [1]])
        self.assertEqual(res, "UNSAT")

tests.append(TestSolver)

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
            sys.stdout.write('%30s:  ' % basename(path))
            sys.stdout.flush()

            n_vars, clauses = read_cnf(path)
            sys.stdout.write('vars: %6d   cls: %6d   ' %
                             (n_vars, len(clauses)))
            sys.stdout.flush()

            sol = pycosat.solve(n_vars, clauses)
            if isinstance(sol, list):
                sys.stdout.write("SAT\n")
                assert verify(n_vars, clauses, sol)
            else:
                sys.stdout.write("%s\n" % sol)
            sys.stdout.flush()
