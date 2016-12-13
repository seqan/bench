#include "index_approximate_search_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef IndexEsa<> TIndex;
    return benchmark_index_approximate_search_main<TIndex, 3u>("Benchmark: Index based approximation search with three errors using enhanced suffix array.", argc, argv);
}
