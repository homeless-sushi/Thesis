import argparse
import math
import sys
import random

def setup_args() :

    parser = argparse.ArgumentParser(
        prog='Data Generator',
        description=
            'This program produces a input of size N, from a refence file\n\n',
        usage=f'{sys.argv[0]} '
        'xMin yMin zMin X Y Z L N '
        '--output-file OUTPUT_URL'
    )

    parser.add_argument(
        'xMin',
        type=float,
        help='starting point in X'
    )

    parser.add_argument(
        'yMin',
        type=float,
        help='starting point in Y'
    )

    parser.add_argument(
        'zMin',
        type=float,
        help='starting point in Z'
    )

    parser.add_argument(
        'X',
        type=float,
        help='len of lattice in X'
    )

    parser.add_argument(
        'Y',
        type=float,
        help='len of lattice in Y'
    )

    parser.add_argument(
        'Z',
        type=float,
        help='len of lattice in Z'
    )

    parser.add_argument(
        'L',
        type=float,
        help='cube size'
    )

    parser.add_argument(
        'N',
        type=int,
        help='atoms per cube'
    )

    parser.add_argument(
        '--output-file',
        type=str,
        required=True,
        help='output file\'s url'
    )

    return parser

def main() :
    parser = setup_args()
    args = parser.parse_args()

    x_min = args.xMin
    y_min = args.yMin
    z_min = args.zMin
    x_size = args.X
    y_size = args.Y
    z_size = args.Z
    cube_size = args.L
    density = args.N
    output_url = args.output_file
    
    # Calculate the number of cubes along each dimension
    x_cubes = math.floor(x_size / cube_size + 1)
    y_cubes = math.floor(y_size / cube_size + 1)
    z_cubes = math.floor(z_size / cube_size + 1)
    
    atoms = []
    qs = [-0.834, 0.417]
    for i in range(x_cubes):
        for j in range(y_cubes):
            for k in range(z_cubes):
                # Generate N random points within each cube
                for _ in range(density):
                    random_x = random.uniform(x_min + i*cube_size, x_min + (i+1)*cube_size)
                    random_y = random.uniform(y_min + j*cube_size, y_min + (j+1)*cube_size)
                    random_z = random.uniform(z_min + k*cube_size, z_min + (k+1)*cube_size)
                    q = random.choice(qs)
                    atoms.append((random_x, random_y, random_z, q))

    with open(output_url, "w") as output_file :
        for atom in atoms :
            x,y,z,q = atom
            output_file.write(f"{x:.3f} {y:.3f} {z:.3f} {q:.3f}\n")

    return 0

if __name__ == '__main__' :
    main()