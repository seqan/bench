#include "kmers_kmer_counting_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef IndexQGram<UngappedShape<15u>, OpenAddressing> TIndexSpec;
    return benchmark_kmers_kmer_counting_main<TIndexSpec, 15u>("Benchmark: Counting many 15mers in one genome.", argc, argv);
}
