#include "pairwise_alignment_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef Dna5String TSequence;
    typedef Align<TSequence> TAlign;

    auto compute = [] (TAlign & align) {
        // match: 2, mismatch: -3, gap: -2
        return globalAlignment(align, Score<int>(2, -3, -2));
    };
    return benchmark_pairwise_alignment_main<TSequence>("Benchmark: Global Pairwise Alignment with Linear Gap Model for DNA.", compute, argc, argv);
}
