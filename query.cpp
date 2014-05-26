// ==========================================================================
//                 iBench - Full-text indexing benchmarks
// ==========================================================================
// Copyright (c) 2012-2014, Enrico Siragusa, FU Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Knut Reinert or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ENRICO SIRAGUSA OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ==========================================================================
// Author: Enrico Siragusa <enrico.siragusa@fu-berlin.de>
// ==========================================================================

// ============================================================================
// Prerequisites
// ============================================================================

// ----------------------------------------------------------------------------
// SeqAn headers
// ----------------------------------------------------------------------------

#include <seqan/basic.h>
#include <seqan/sequence.h>
#include <seqan/index.h>
#include <seqan/arg_parse.h>

#include "options.h"
#include "types.h"
#include "run.h"
#include "misc.h"

using namespace seqan;

// ============================================================================
// Classes
// ============================================================================

// ----------------------------------------------------------------------------
// Class Options
// ----------------------------------------------------------------------------

struct Options : BaseOptions
{
    typedef std::string             TString;
    typedef std::vector<TString>    TList;

    enum AlgorithmType
    {
        ALGO_SINGLE, ALGO_DFS, ALGO_BFS
    };

    CharString      queryFile;

    AlgorithmType   algorithmType;
    TList           algorithmTypeList;

    bool            edit;
    unsigned        errors;
    bool            locate;

    Options() :
        BaseOptions(),
        algorithmType(ALGO_SINGLE),
        edit(false),
        errors(0),
        locate(false)
    {
        algorithmTypeList.push_back("single");
        algorithmTypeList.push_back("dfs");
        algorithmTypeList.push_back("bfs");
    }
};

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function setupArgumentParser()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline void setupArgumentParser(ArgumentParser & parser, TOptions const & options)
{
    setAppName(parser, "iBench Query");
    setShortDescription(parser, "Benchmark full-text index query time");
    setCategory(parser, "Stringology");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fIINDEX FILE\\fP> <\\fIQUERY FILE\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addOption(parser, ArgParseOption("v", "tsv", "Tab separated value output."));

    addSection(parser, "Main Options");
    setAlphabetType(parser, options);
    setIndexType(parser, options);
    setTextLimits(parser, options);

    addSection(parser, "Query Options");
    setAlgorithmType(parser, options);
    addOption(parser, ArgParseOption("ed", "edit-distance", "Edit distance. Default: Hamming distance."));
    addOption(parser, ArgParseOption("e", "errors", "Number of errors.", ArgParseOption::INTEGER));
    setMinValue(parser, "errors", "0");
    setMaxValue(parser, "errors", "5");
    setDefaultValue(parser, "errors", options.errors);
    addOption(parser, ArgParseOption("l", "locate", "Locate occurrences. Default: count only."));
}

// ----------------------------------------------------------------------------
// Function parseCommandLine()
// ----------------------------------------------------------------------------

template <typename TOptions>
ArgumentParser::ParseResult
inline parseCommandLine(TOptions & options, ArgumentParser & parser, int argc, char const ** argv)
{
    ArgumentParser::ParseResult res = parse(parser, argc, argv);

    if (res != ArgumentParser::PARSE_OK)
        return res;

    getArgumentValue(options.textIndexFile, parser, 0);
    getArgumentValue(options.queryFile, parser, 1);
    getOptionValue(options.tsv, parser, "tsv");

    getAlphabetType(options, parser);
    getIndexType(options, parser);
    getTextLimits(options, parser);
    getAlgorithmType(options, parser);
    getOptionValue(options.edit, parser, "edit-distance");
    getOptionValue(options.errors, parser, "errors");
    getOptionValue(options.locate, parser, "locate");

    return ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function locateOccurrences()
// ----------------------------------------------------------------------------

template <typename TIndex, typename TSpec>
inline void locateOccurrences(Iter<TIndex, TSpec> const & it, True)
{
    typedef typename Fibre<TIndex, FibreSA>::Type   TSA;
    typedef typename Infix<TSA const>::Type         TOccurrences;
    typedef typename Value<TOccurrences>::Type      TOccurrence;
    typedef typename Size<TIndex>::Type             TSize;

    TOccurrences const & occs = getOccurrences(it);

    forEach(occs, [&](TOccurrence const & occ)
    {
        TSize volatile offset = getSeqOffset(occ);
        ignoreUnusedVariableWarning(offset);
    });
}

template <typename TIndex, typename TSpec>
inline void locateOccurrences(Iter<TIndex, TSpec> const & /* it */, False) {}

// ----------------------------------------------------------------------------
// Function findOccurrences()
// ----------------------------------------------------------------------------
// Single search.

template <typename TIndex, typename TQueries, typename TLocate, typename TDistance>
inline unsigned long findOccurrences(Options const & options, TIndex & index, TQueries const & queries, TLocate, Backtracking<TDistance>, Nothing)
{
    typedef Backtracking<TDistance>                         TAlgorithm;
    typedef typename Iterator<TIndex, TopDown<> >::Type     TIndexIt;
    typedef typename Iterator<TQueries const, Rooted>::Type TQueriesIt;

    unsigned long count = 0;

    find(index, queries, options.errors, [&](TIndexIt const & indexIt, TQueriesIt const &, unsigned)
    {
        count += countOccurrences(indexIt);
        locateOccurrences(indexIt, TLocate());
    },
    TAlgorithm());

    return count;
}

// ----------------------------------------------------------------------------
// Function findOccurrences()
// ----------------------------------------------------------------------------
// Multiple search (DFS).

template <typename TIndex, typename TQueries, typename TLocate, typename TDistance>
inline unsigned long findOccurrences(Options const & options, TIndex & index, TQueries & queries, TLocate, Backtracking<TDistance>, DfsPreorder)
{
    typedef Index<TQueries, IndexWotd<> >                   TPattern;
    typedef Backtracking<TDistance>                         TAlgorithm;
    typedef typename Iterator<TIndex, TopDown<> >::Type     TIndexIt;
    typedef typename Iterator<TPattern, TopDown<> >::Type   TQueriesIt;

    TPattern pattern(queries);
    indexCreate(pattern, FibreSA(), Trie());
    indexBuild(pattern);

    unsigned long count = 0;

    find(index, pattern, options.errors, [&](TIndexIt const & indexIt, TQueriesIt const & queriesIt, unsigned)
    {
        count += countOccurrences(indexIt) * countOccurrences(queriesIt);
        locateOccurrences(indexIt, TLocate());
    },
    TAlgorithm());

    return count;
}

// ----------------------------------------------------------------------------
// Function findOccurrences()
// ----------------------------------------------------------------------------
// Disable HammingDistance search on tree indices lacking trie-like iterator.

template <typename TText, typename TSpec, typename TQueries, typename TLocate>
inline unsigned long findOccurrences(Options const &, Index<TText, IndexEsa<TSpec> > &, TQueries &, TLocate, Backtracking<HammingDistance>, Nothing)
{
    throw RuntimeError("Unsupported index type");
    return 0;
}

template <typename TText, typename TSpec, typename TQueries, typename TLocate>
inline unsigned long findOccurrences(Options const &, Index<TText, IndexEsa<TSpec> > &, TQueries &, TLocate, Backtracking<EditDistance>, Nothing)
{
    throw RuntimeError("Unsupported index type");
    return 0;
}

template <typename TText, typename TSpec, typename TQueries, typename TLocate, typename TDistance>
inline unsigned long findOccurrences(Options const &, Index<TText, IndexEsa<TSpec> > &, TQueries &, TLocate, Backtracking<TDistance>, DfsPreorder)
{
    throw RuntimeError("Unsupported index type");
    return 0;
}

// ----------------------------------------------------------------------------
// Function countOccurrences()
// ----------------------------------------------------------------------------
// Dispatch locate, distance, and search algorithm.

template <typename TIndex, typename TQueries, typename TLocate, typename TDistance>
inline unsigned long countOccurrences(Options const & options, TIndex & index, TQueries & queries, TLocate, TDistance)
{
    switch (options.algorithmType)
    {
    case Options::ALGO_SINGLE:
        return findOccurrences(options, index, queries, TLocate(), Backtracking<TDistance>(), Nothing());

    case Options::ALGO_DFS:
        return findOccurrences(options, index, queries, TLocate(), Backtracking<HammingDistance>(), DfsPreorder());

//    case Options::ALGO_BFS:
//        return findOccurrences(options, index, queries, TLocate(), Backtracking<TDistance, Bfs>());

    default:
        throw RuntimeError("Unsupported search algorithm");
    }
}

template <typename TIndex, typename TQueries, typename TLocate>
inline unsigned long countOccurrences(Options const & options, TIndex & index, TQueries & queries, TLocate)
{
    if (options.errors)
    {
//        if (options.edit)
//            return countOccurrences(options, index, queries, TLocate(), EditDistance());
//        else
            return countOccurrences(options, index, queries, TLocate(), HammingDistance());
    }
    else
    {
        return countOccurrences(options, index, queries, TLocate(), Exact());
    }
}

template <typename TIndex, typename TQueries>
inline unsigned long countOccurrences(Options const & options, TIndex & index, TQueries & queries)
{
//    if (options.locate)
//        return countOccurrences(options, index, queries, True());
//    else
        return countOccurrences(options, index, queries, False());
}

// ----------------------------------------------------------------------------
// Function run()
// ----------------------------------------------------------------------------

template <typename TAlphabet, typename TLimits, typename TSetLimits, typename TIndexSpec>
inline void run(Options const & options)
{
    typedef typename TextCollection<TAlphabet, Limits<__uint8> >::Type      TQueries;
    typedef typename TextCollection<TAlphabet, TLimits, TSetLimits>::Type   TText;
    typedef Index<TText, TIndexSpec>                                        TIndex;

    TIndex index;

    if (!open(index, toCString(options.textIndexFile)))
        throw RuntimeError("Error while loading full-text index");

    TQueries queries;

    if (!open(queries, toCString(options.queryFile)))
        throw RuntimeError("Error while loading queries");

    double start = sysTime();
    unsigned long occurrences = countOccurrences(options, index, queries);
    double finish = sysTime();

    if (options.tsv)
    {
        std::cout << occurrences << '\t' << std::fixed << finish - start << std::endl;
    }
    else
    {
        std::cout << length(queries) << " queries" << std::endl;
        std::cout << lengthSum(queries) << " symbols" << std::endl;
        std::cout << occurrences << " occurrences" << std::endl;
        std::cout << std::fixed << finish - start << " sec" << std::endl;
    }
}

int main(int argc, char const ** argv)
{
    return run<Options>(argc, argv);
}
