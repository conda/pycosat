import pycosat

print pycosat.solve(
    5, [
    [1, -5, 4],
    [-1, 5, 3, 4],
    [-3, -4],
    ],
#    True
)

print pycosat.solve(
    2, [
    [-1],
    [1],
    ],
#    True
)
