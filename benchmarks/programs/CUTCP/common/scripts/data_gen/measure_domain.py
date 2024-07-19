import argparse
import os
import math
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
REFERENCE_URL = os.path.join(SCRIPT_DIR,"reference","reference.txt")

class Atom:
    def __init__(self, x, y, z, q):
        self.x = x
        self.y = y
        self.z = z
        self.q = q

def setup_args() :

    parser = argparse.ArgumentParser(
        prog='Data Generator',
        description=
            'This program computes domain information from a refence file\n\n',
        usage=f'{sys.argv[0]} '
        '--input-file INPUT_URL'
    )

    parser.add_argument(
        '--input-file',
        type=str,
        required=True,
        help='input file\'s url'
    )

    return parser

def main():
    parser = setup_args()
    args = parser.parse_args()

    input_file = args.input_file


    atoms = []
    with open(input_file, "r") as input_file :
        
        for data_line in input_file:
            x, y, z, q = map(float, data_line.strip().split())
            atoms.append(Atom(x, y, z, q))

    xMin = yMin = zMin = float('inf')
    xMax = yMax = zMax = float('-inf')

    for atom in atoms:
        xMin = min(xMin, atom.x)
        yMin = min(yMin, atom.y)
        zMin = min(zMin, atom.z)
        xMax = max(xMax, atom.x)
        yMax = max(yMax, atom.y)
        zMax = max(zMax, atom.z)

    xDelta = xMax - xMin
    yDelta = yMax - yMin
    zDelta = zMax - zMin

    print(f"xMin: {xMin}, yMin: {yMin}, zMin: {zMin}")
    print(f"xMax: {xMax}, yMax: {yMax}, zMax: {zMax}")
    print(f"xDelta: {xDelta}, yDelta: {yDelta}, zDelta: {zDelta}")

if __name__ == '__main__' :
    main()