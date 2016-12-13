#ifndef BENCHMARK_HEADER_INDEX_APPROXIMATE_SEARCH_BASE_H
#define BENCHMARK_HEADER_INDEX_APPROXIMATE_SEARCH_BASE_H

#include <iostream>
#include <seqan/align.h>
#include <seqan/seq_io.h>
#include <seqan/arg_parse.h>
#include <seqan/index.h>
#include <seqan/find.h>

using namespace seqan;

struct Options {
    unsigned threads;
    CharString readFile;
    CharString indexFile;
    CharString output;
};

seqan::ArgumentParser::ParseResult parseCommandLine(std::string infoText, Options & options, int argc, char** const argv)
{
    seqan::ArgumentParser parser(infoText);
    setCategory(parser, "Benchmark");
    setVersion(parser, "0.1");
    setDate(parser, "April 2016");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] \\fIINDEX\\fP \\fIREADS\\fP \\fIOUT\\fP ");

    addDescription(parser,
    "\\fIINDEX\\fP is the index file. \\fIREADS\\fP is a .fa / fasta input file containing the reads. \\fIOUT\\fP is a txt output file.");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE, "INDEX"));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE, "READS"));
    addArgument(parser, ArgParseArgument(ArgParseArgument::OUTPUT_FILE, "OUT"));
    setValidValues(parser, 0, "inx");
    setValidValues(parser, 1, "FASTS, fa");
    setValidValues(parser, 2, "txt");

    addSection(parser, "Settings");
    addOption(parser, seqan::ArgParseOption("tc", "threads", "Number of threads", seqan::ArgParseArgument::INTEGER, "INT"));

    #ifdef _OPENMP
    setDefaultValue(parser, "threads", omp_get_max_threads());
    #else
    setDefaultValue(parser, "threads", 1);
    #endif

    seqan::ArgumentParser::ParseResult res = seqan::parse(parser, argc, argv);
    if(res == seqan::ArgumentParser::PARSE_OK)
    {
        getOptionValue(options.threads, parser, "threads");
        getArgumentValue(options.indexFile, parser, 0);
        getArgumentValue(options.readFile, parser, 1);
        getArgumentValue(options.output, parser, 2);
    }
    return res;
}

/**
 * Example with one error (hamming distance):
 * position: 0    5    0    5    0
 * Genome  : GACCGCAGCGATCGTAACGAGAGA
 *
 * reads:
 * >1
 * GCGA
 * >2
 * GAGA
 * ....
 *
 * Output:
 * GCGA: 20, 24, 22, 11
 * GAGA: 24, 22, 11
 * .......
 */
template <unsigned errors, typename TIndex, typename TStringSet, typename TOutputStream>
inline void approximate_search(TIndex & index, TStringSet & patterns, TOutputStream & cout)
{
    typedef typename Iterator<TStringSet const, Rooted>::Type TPatternsIt;
    typedef typename Iterator<TIndex,TopDown<> >::Type TIndexIt;

    std::vector<std::vector<unsigned> > endPostions;
    resize(endPostions, length(patterns));

    auto delegate = [&endPostions] (TIndexIt const & it, TPatternsIt const & patternsIt, unsigned /*score*/)
    {
        auto i = position(patternsIt);
        auto pattern_len = length(*patternsIt);

        for(auto & occ: getOccurrences(it))
        {
            // calculate end positions
            unsigned end = getSeqOffset(occ) + pattern_len;
            appendValue(endPostions[i], end);
        }
    };

    find(index, patterns, errors, delegate, Backtracking<>());

    for(unsigned i = 0; i < length(endPostions); ++i)
    {
        if(endPostions[i].empty())
        {
            continue;
        }

        cout << i << ": ";
        auto it = endPostions[i].begin();
        auto const end = endPostions[i].end();

        cout << *it;
        for (++it; it!=end; ++it)
        {
            cout << ", " << *it;
        }

        cout << std::endl;
    }
}

template <typename TIndexSpec, unsigned errors>
inline int benchmark_index_approximate_search_main(std::string infoText, int argc, char **argv)
{
    Options options;
    ArgumentParser::ParseResult res = parseCommandLine(infoText, options, argc, argv);
    if (res != seqan::ArgumentParser::PARSE_OK)
        return res == seqan::ArgumentParser::PARSE_ERROR;

    #ifdef _OPENMP
    omp_set_num_threads(options.threads);
    #endif

    typedef Backtracking<> TBacktracking;
    typedef Index<Dna5String, TIndexSpec > TIndex;

    // read in all 50-reads
    StringSet<CharString> ids;
    StringSet<Dna5String> seqs;
    SeqFileIn seqFileIn(toCString(options.readFile));
    readRecords(ids, seqs, seqFileIn);

    // read in the index of the genome
    Index<Dna5String, TIndexSpec > index;
    open(index, toCString(options.indexFile));

    // prepare write steam
    std::ofstream ofs(toCString(options.output), std::ofstream::out);

    // search all reads in the index structure
    approximate_search<errors>(index, seqs, ofs);

    return 0;
}

#endif
