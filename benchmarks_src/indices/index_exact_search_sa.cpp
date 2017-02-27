#include "index_exact_search_base.h"

using namespace seqan;

int main(int argc, char **argv)
{
    typedef IndexSa<> TIndex;
    return benchmark_index_main<TIndex>("Benchmark: Index based exact search using non-enhanced suffix arrays.", argc, argv);
}
