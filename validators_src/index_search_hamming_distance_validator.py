#!/usr/bin/env python

import sys


def parse_file(file_path):
    """
    The file format is:
        <pattern_1>: <pos_1_1>, <pos_2_1>, <pos_3_1>, ..., <pos_k1_1>
        <pattern_2>: <pos_1_2>, ..., <pos_k2_2>
        <pattern_3>: <pos_1_3>, <pos_2_3>, ..., <pos_k3_3>

    The pattern ids will be sorted and the positions within a pattern.
    """

    result = dict()
    with open(file_path) as infile:
        for line in infile:
            # skip empty lines
            if not line.strip():
                continue

            pattern_id, line = line.split(':')
            positions = line.split(',')
            positions = [s.strip() for s in positions]
            result[pattern_id.strip()] = sorted(positions)
    return result


def validate(benchmark, validation):
    """
    The validation is simple, because both files should have the same content.

    If one file has a missing pattern id or a missing position, that means that
    either the benchmark found to many or to few matches. This clearly indicates
    a wrong algorithm.
    """

    if benchmark == validation:
        return 1
    else:
        return 0


def main(argv):
    """
    This validates exact and approximate search benchmarks with hamming distance
    based on a index.

    Like:
        index_exact_search,
        index_one_error_approximate_search,
        index_two_error_approximate_search,
        index_three_error_approximate_search
    """

    if len(argv) < 3:
        print ('Usage:', argv[0], '<benchmark output> <validation file>')
        sys.exit()

    benchmark = parse_file(argv[1])
    validation = parse_file(argv[2])

    # import pprint;
    # pprint.pprint(benchmark)
    # pprint.pprint(validation)

    print(validate(benchmark, validation))


if __name__ == "__main__":
    main(sys.argv)
