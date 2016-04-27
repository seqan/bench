#include "pairwise_alignment_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef String<AminoAcid> TSequence;
    typedef Align<TSequence> TAlign;

    auto compute = [] (TAlign & align) {
        // Blosum62, gap_extend: -3, gap_open: -1
        return globalAlignment(align, Blosum62(-3, -1));
    };
    return benchmark_pairwise_alignment_main<TSequence>("Benchmark: Global Pairwise Alignment with Affine Gap Model for Protein.", compute, argc, argv);
}
