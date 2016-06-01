#include "kmers_kmer_counting_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef IndexQGram<UngappedShape<50u>, OpenAddressing> TIndexSpec;
    return benchmark_kmers_kmer_counting_main<TIndexSpec, 50u>("Benchmark: Counting many 50mers in one genome.", argc, argv);
}
