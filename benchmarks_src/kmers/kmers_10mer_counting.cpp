#include "kmers_kmer_counting_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef IndexQGram<UngappedShape<10u> > TIndexSpec;
    return benchmark_kmers_kmer_counting_main<TIndexSpec, 10u>("Benchmark: Counting many 10mers in one genome.", argc, argv);
}
