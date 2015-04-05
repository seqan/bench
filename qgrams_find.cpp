// ==========================================================================
//                             SeqAn Benchmark
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

#define APP_BENCH_QGRAMS_FIND_CPP_

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
// Class Stats
// ----------------------------------------------------------------------------

struct Stats
{
    static double   preprocessingTime;
    static double   countTime;
    static double   locateTime;
};

double Stats::preprocessingTime = 0;
double Stats::countTime = 0;
double Stats::locateTime = 0;

// ----------------------------------------------------------------------------
// Class Options
// ----------------------------------------------------------------------------

struct Options : QGramOptions
{
    CharString      queryFile;

    unsigned        errors;
    bool            locate;

    Options() :
        QGramOptions(),
        errors(0),
        locate(false)
    {}
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
    setAppName(parser, "SeqAn Benchmark - q-Gram Query");
    setShortDescription(parser, "Benchmark query speed of full-text indices");
    setCategory(parser, "Benchmarking");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fIINDEX FILE\\fP> <\\fIQUERY FILE\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE));
    addOption(parser, ArgParseOption("v", "tsv", "Tab separated value output."));

    addSection(parser, "Main Options");
    setAlphabetType(parser, options);
    setIndexType(parser, options);
    setTextLimits(parser, options);

    addSection(parser, "q-Gram Options");
    addOption(parser, ArgParseOption("q", "qgram", "Fix the q-gram weight.", ArgParseOption::INTEGER));
    setDefaultValue(parser, "q", options.q);

//    addSection(parser, "Query Options");
//    addOption(parser, ArgParseOption("e", "errors", "Number of errors.", ArgParseOption::INTEGER));
//    setMinValue(parser, "errors", "0");
//    setMaxValue(parser, "errors", "5");
//    setDefaultValue(parser, "errors", options.errors);
//    addOption(parser, ArgParseOption("l", "locate", "Locate occurrences. Default: count only."));
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

    getOptionValue(options.q, parser, "q");

//    getOptionValue(options.errors, parser, "errors");
//    getOptionValue(options.locate, parser, "locate");

    return ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function locateOccurrences()
// ----------------------------------------------------------------------------

//template <typename TIndex, typename TSpec>
//inline void locateOccurrences(Iter<TIndex, TSpec> const & it, True)
//{
//    typedef typename Fibre<TIndex, FibreSA>::Type   TSA;
//    typedef typename Infix<TSA const>::Type         TOccurrences;
//    typedef typename Value<TOccurrences>::Type      TOccurrence;
//    typedef typename Size<TIndex>::Type             TSize;
//
//    TOccurrences const & occs = getOccurrences(it);
//
//    forEach(occs, [&](TOccurrence const & occ)
//    {
//        TSize volatile offset = getSeqOffset(occ);
//        ignoreUnusedVariableWarning(offset);
//    });
//}
//
//template <typename TIndex, typename TSpec>
//inline void locateOccurrences(Iter<TIndex, TSpec> const & /* it */, False) {}

// ----------------------------------------------------------------------------
// Function findOccurrences()
// ----------------------------------------------------------------------------
// Exact search.

template <typename TIndex, typename TQueries, typename TLocate>
inline unsigned long findOccurrences(Options const & /* options */, TIndex & index, TQueries & queries, TLocate, Exact)
{
    typedef typename Value<TQueries const>::Type      TQuery;
//    typedef typename Fibre<TIndex, FibreSA>::Type     TSA;
//    typedef typename Infix<TSA const>::Type           TOccurrences;

    unsigned long count = 0;

    double timer = sysTime();

    forEach(queries, [&](TQuery const & query)
    {
        hash(indexShape(index), begin(query, Rooted()));
        count += countOccurrences(index, indexShape(index));
//        TOccurrences const & occs = getOccurrences(index, indexShape(index));
//        count += length(occs);
    });

    Stats::countTime = sysTime() - timer;

    return count;
}

// ----------------------------------------------------------------------------
// Function countOccurrences()
// ----------------------------------------------------------------------------
// Dispatch locate and distance.

template <typename TIndex, typename TQueries, typename TLocate>
inline unsigned long countOccurrences(Options const & options, TIndex & index, TQueries & queries, TLocate)
{
//    if (options.errors)
//        return findOccurrences(options, index, queries, TLocate(), HammingDistance());
//    else
        return findOccurrences(options, index, queries, TLocate(), Exact());
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

    unsigned long occurrencesCount = countOccurrences(options, index, queries);

    if (options.tsv)
    {
        std::cout << occurrencesCount << '\t' << std::fixed << Stats::countTime << '\t' << Stats::preprocessingTime << std::endl;
    }
    else
    {
        std::cout << length(queries) << " queries" << std::endl;
        std::cout << lengthSum(queries) << " symbols" << std::endl;
        std::cout << occurrencesCount << " occurrences" << std::endl;
        std::cout << std::fixed << Stats::countTime << " + " << Stats::preprocessingTime << " sec" << std::endl;
    }
}

int main(int argc, char const ** argv)
{
    return run<Options>(argc, argv);
}
