#include "index_exact_search_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef IndexQGram<UngappedShape<50>, OpenAddressing > TIndex;
    return benchmark_index_main<TIndex>("Benchmark: Index based exact search using qgram index (kmers).", argc, argv);
}
