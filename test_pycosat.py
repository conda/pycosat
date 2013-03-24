import unittest

import pycosat


tests = []

class TestSolver(unittest.TestCase):

    def test_sat_1(self):
        res = pycosat.solve(5, [[1, -5, 4],
                                [-1, 5, 3, 4],
                                [-3, -4]])
        self.assertEqual(res, [True, False, False, False, True])

    def test_unsat_2(self):
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
