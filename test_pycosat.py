import unittest

import pycosat


tests = []

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

    def test_unsat_1(self):
        """
        p cnf 2 2
        -1 0
        1 0
        """
        res = pycosat.solve(2, [[-1],
                                [1]])
        self.assertEqual(res, False)

tests.append(TestSolver)

# ------------------------------------------------------------------------

def run(verbosity=1, repeat=1):
    print("pycosat version: %r" % pycosat.__version__)
    suite = unittest.TestSuite()
    for cls in tests:
        for _ in range(repeat):
            suite.addTest(unittest.makeSuite(cls))

    runner = unittest.TextTestRunner(verbosity=verbosity)
    return runner.run(suite)


if __name__ == '__main__':
    run()
    n_vars, clauses = read_cnf('uf20-0801.cnf')
    #n_vars, clauses = (5, [[1, -5, 4],
    #                       [-1, 5, 3, 4],
    #                       [-3, -4]])
    #sol = pycosat.solve(n_vars, clauses)
    #print sol
    #print verify(n_vars, clauses, sol)
