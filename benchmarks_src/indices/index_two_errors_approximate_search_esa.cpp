#include "index_approximate_search_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef IndexEsa<> TIndex;
    return benchmark_index_approximate_search_main<TIndex, 2u>("Benchmark: Index based approximation search with two errors using enhanced suffix array.", argc, argv);
}
