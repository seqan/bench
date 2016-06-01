#!/usr/bin/env python

import sys


def parse_file(file_path):
    """
    The file format is:
        <sequence_1>, <sequence_2>: <alignmentScore(sequence_1, sequence_2)>
        <sequence_1>, <sequence_3>: <alignmentScore(sequence_1, sequence_3)>
        ...
        <sequence_1>, <sequence_m>: <alignmentScore(sequence_1, sequence_m)>
        <sequence_2>, <sequence_3>: <alignmentScore(sequence_2, sequence_3)>
        <sequence_2>, <sequence_4>: <alignmentScore(sequence_2, sequence_4)>
        ...
        <sequence_2>, <sequence_m>: <alignmentScore(sequence_2, sequence_m)>
        ...
        <sequence_n>, <sequence_n+1>: <alignmentScore(sequence_n, sequence_n+1)>
        <sequence_n>, <sequence_n+2>: <alignmentScore(sequence_n, sequence_n+2)>
        ...
        <sequence_n>, <sequence_m>: <alignmentScore(sequence_n, sequence_m)>

    The sequence tuples will be sorted.
    """

    result = dict()
    with open(file_path) as infile:
        for line in infile:
            # skip empty lines
            if not line.strip():
                continue

            sequences, score = line.split(':')
            sequences = [s.strip() for s in sequences.split(',')]
            sequences = tuple(sorted(sequences))
            result[sequences] = score.strip()
    return result


def validate(benchmark, validation):
    """
    The validation is simple, because both files should have the same content.

    Because, each sequence pair has a unique alignment score.
    """

    if benchmark == validation:
        return 1
    else:
        return 0


def main(argv):
    """
    This validates pairwises alignment benchmarks.

    Like:
        pairwise_alignment_global_linear_dna,
        pairwise_alignment_global_linear_protein,
        pairwise_alignment_global_affine_dna,
        pairwise_alignment_global_affine_protein,
        pairwise_alignment_local_linear_dna,
        pairwise_alignment_local_linear_protein,
        pairwise_alignment_local_affine_dna,
        pairwise_alignment_local_affine_protein
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
