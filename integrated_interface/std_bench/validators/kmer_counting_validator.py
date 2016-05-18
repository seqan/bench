#!/usr/bin/env python

import sys


def parse_file(file_path):
    """
    Let s_0 s_1 ... s_n be the text and k the kmer length.
    The file format is:
        <position_0>: <occurences of s_0 ... s_{k-1}>
        <position_1>: <occurences of s_1 ... s_{k}>
        ...
        <position_i>: <occurences of s_i ... s_{i+k-1}>
        ...

    The positions will be sorted.
    """

    result = dict()
    with open(file_path) as infile:
        for line in infile:
            # skip empty lines
            if not line.strip():
                continue

            position, occurences = line.split(':')
            result[position.strip()] = occurences.strip()
    return result


def validate(benchmark, validation):
    """
    The validation is simple, because both files should have the same content.

    Because, each position has a unique number of occurences.
    """

    if benchmark == validation:
        return 1
    else:
        return 0


def main(argv):
    """
    This validates kmer counting benchmarks.

    Like:
        kmers_10mer_counting,
        kmers_15mer_counting,
        kmers_50mer_counting
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
