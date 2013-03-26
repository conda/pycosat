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

    def test_itersolve(self):
        clauses = [[1, -5, 4],
                   [-1, 5, 3, 4],
                   [-3, -4]]
        for sol in pycosat.itersolve(5, clauses):
            #sys.stderr.write('%r\n' % repr(sol))
            self.assertTrue(verify(5, clauses, sol))

        sols = list(pycosat.itersolve(5, clauses))
        self.assertEqual(len(sols), 18)
        self.assertEqual(len(set(tuple(sol) for sol in sols)), 18)

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
            n_sol = 0
            for sol in pycosat.itersolve(n_vars, clauses):
                sys.stdout.write('.')
                sys.stdout.flush()
                assert verify(n_vars, clauses, sol)
                n_sol += 1
            sys.stdout.write("%d\n" % n_sol)
            sys.stdout.flush()
