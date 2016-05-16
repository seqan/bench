#include "pairwise_alignment_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef Dna5String TSequence;
    typedef Align<TSequence> TAlign;

    auto compute = [] (TAlign & align) {
        // match: 2, mismatch: -3, gap_extend: -3, gap_open: -1
        return globalAlignment(align, Score<int>(2, -3, -3, -1));
    };
    return benchmark_pairwise_alignment_main<TSequence>("Benchmark: Global Pairwise Alignment with Affine Gap Model for DNA.", compute, argc, argv);
}
