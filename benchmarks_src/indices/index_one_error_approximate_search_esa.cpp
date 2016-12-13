#include "index_approximate_search_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef IndexEsa<> TIndex;
    return benchmark_index_approximate_search_main<TIndex, 1u>("Benchmark: Index based approximation search with one error using enhanced suffix array.", argc, argv);
}
