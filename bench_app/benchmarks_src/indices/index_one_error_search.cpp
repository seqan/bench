#include <iostream>
#include <seqan/align.h>
#include <seqan/seq_io.h>
#include <seqan/arg_parse.h>
#include <seqan/index.h>
#include <seqan/find.h>

// #include "index_one_error_search_base.h"

using namespace seqan;

template <unsigned errors, typename TIndex, typename TStringSet, typename TOutputStream>
void approximate_search(TIndex & index, TStringSet & patterns, TOutputStream & cout)
{
    typedef typename Iterator<TStringSet const, Rooted>::Type TPatternsIt;
    typedef typename Iterator<TIndex,TopDown<> >::Type TIndexIt;

    std::vector<std::vector<unsigned> > endPostions;
    resize(endPostions, length(patterns));

    auto delegate = [&endPostions] (TIndexIt const & it, TPatternsIt const & patternsIt, unsigned /*score*/)
    {
        auto i = position(patternsIt);
        auto pattern_len = length(*patternsIt);

        for(auto occ: getOccurrences(it))
        {
            // calculate end positions
            unsigned end = getSeqOffset(occ) + pattern_len;
            appendValue(endPostions[i], end);
        }
    };

    find(index, patterns, errors, delegate, Backtracking<>());

    for(unsigned i = 0; i < length(endPostions); ++i)
    {
        cout << patterns[i] << ": ";
        for(auto endPos: endPostions[i])
        {
            cout << endPos << ", ";
        }
        cout << std::endl;
    }
}

int main(int argc, char **argv)
{
    // typedef IndexEsa<> TIndex;
    // return benchmark_index_one_error_search_main<TIndex>("Benchmark: Index based search with one error using suffix array.", argc, argv);

    // * position: 0    5    0    5    0
    // * Genome  : GACCGCAGCGATCGTAACGAGAGA
    Dna5String genome = "GACCGCAGCGATCGTAACGAGAGA";
    // Dna5String _needle = "GCGA";

    typedef IndexEsa<> TIndexSpec;

    typedef Backtracking<> TBacktracking;
    typedef Index<Dna5String, TIndexSpec> TIndex;
    // typedef Finder<TIndex, TBacktracking> TFinder;
    // typedef Pattern<Dna5String, TBacktracking> TPattern;

    // typedef Finder<Index<Dna5String, TIndexSpec >> TFinder;
    // typedef Pattern<Dna5String> TPattern;

    Dna5String pattern_raw("GAGA");
    // TPattern pattern("GCGA");

    TIndex index(genome);
    // TFinder finder(index);

    // std::cout << "genome: " << genome << std::endl;
    // std::cout << "pattern: " << needle(pattern) << std::endl;
    // while(find(finder, pattern, 3u))
    // {
    //     std::cout << endPosition(finder) << ", " << std::endl;
    // }
    // clear(finder);

    typedef typename Iterator<decltype(index),TopDown<> >::Type TIndexIt;

    size_t pattern_len = length(pattern_raw);
    auto delegate = [pattern_len] (TIndexIt const & it, int /*score*/)
    {
        for(auto occ: getOccurrences(it))
        {
            unsigned end = getSeqOffset(occ) + pattern_len;
            std::cout << end << ", ";
        }
    };

    find(index, pattern_raw, 1u, delegate, Backtracking<>());
    std::cout << std::endl;


    StringSet<Dna5String> patterns;
    appendValue(patterns, "GCGA");
    appendValue(patterns, "GAGA");
    appendValue(patterns, "GCGAT");

    approximate_search<1u>(index, patterns, std::cout);

    return 0;
}

