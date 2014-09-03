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

#define APP_IBENCH_FILTER_CPP_

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
    double          filterTime;

    Stats() :
        filterTime()
    {}
};

// ----------------------------------------------------------------------------
// Class Options
// ----------------------------------------------------------------------------

struct Options : BaseOptions
{
    typedef std::string             TString;
    typedef std::vector<TString>    TList;

    enum AlgorithmType
    {
        ALGO_SEEDS_EXACT, ALGO_SEEDS_APPROXIMATE, ALGO_QGRAMS_CONTIGUOUS, ALGO_QGRAMS_GAPPED
    };

    CharString      queryFile;

    AlgorithmType   algorithmType;
    TList           algorithmTypeList;

    unsigned        errorRate;
    bool            edit;
    bool            verify;

    Options() :
        BaseOptions(),
        algorithmType(ALGO_SEEDS_EXACT),
        errorRate(0),
        edit(false),
        verify(false)
    {
        algorithmTypeList.push_back("seeds-exact");
        algorithmTypeList.push_back("seeds-approximate");
        algorithmTypeList.push_back("qgrams-contiguous");
        algorithmTypeList.push_back("qgrams-gapped");
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
    setAppName(parser, "iBench Filter");
    setShortDescription(parser, "Benchmark filtration efficiency");
    setCategory(parser, "Stringology");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fITEXT FILE\\fP> <\\fIQUERY FILE\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addOption(parser, ArgParseOption("v", "tsv", "Tab separated value output."));

    addSection(parser, "Main Options");
    setAlphabetType(parser, options);
    setIndexType(parser, options);
    setTextLimits(parser, options);

    addSection(parser, "Search Options");
    setAlgorithmType(parser, options);
    addOption(parser, ArgParseOption("e", "error-rate", "Error rate.", ArgParseOption::INTEGER));
    setMinValue(parser, "error-rate", "0");
    setMaxValue(parser, "error-rate", "10");
    setDefaultValue(parser, "error-rate", options.errorRate);
    addOption(parser, ArgParseOption("ed", "edit-distance", "Edit distance. Default: Hamming distance."));
    addOption(parser, ArgParseOption("vf", "verify", "Verify candidate locations. Default: filter only."));

    addSection(parser, "Seeds Filtering Options");

    addSection(parser, "q-Gram Filtering Options");

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

    getArgumentValue(options.textFile, parser, 0);
//    getArgumentValue(options.textIndexFile, parser, 0);
    getArgumentValue(options.queryFile, parser, 1);
    getOptionValue(options.tsv, parser, "tsv");

    getAlphabetType(options, parser);
    getIndexType(options, parser);
    getTextLimits(options, parser);
    getAlgorithmType(options, parser);
    getOptionValue(options.edit, parser, "edit-distance");
    getOptionValue(options.errorRate, parser, "error-rate");
    getOptionValue(options.verify, parser, "verify");

    return ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function verify()
// ----------------------------------------------------------------------------

//template <typename TIndex, typename TSpec>
//inline void verify(Iter<TIndex, TSpec> const & it, True)
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

// ----------------------------------------------------------------------------
// Function filter()
// ----------------------------------------------------------------------------
// Seeds filters.

//template <typename TIndex, typename TQueries, typename TLocate, typename TDistance, typename TSpec>
//inline unsigned long filter(Options const & options, Stats & stats, TIndex & index, TQueries & queries,
//                            TLocate, Backtracking<TDistance, TSpec>, Nothing)
//{
//    typedef Backtracking<TDistance>                         TAlgorithm;
//    typedef typename Iterator<TIndex, TopDown<> >::Type     TIndexIt;
//    typedef typename Iterator<TQueries const, Rooted>::Type TQueriesIt;
//
//    unsigned long count = 0;
//
//    double timer = sysTime();
//    find(index, queries, options.errors, [&](TIndexIt const & indexIt, TQueriesIt const &, unsigned)
//    {
//        count += filter(indexIt);
//        locateOccurrences(indexIt, TLocate());
//    },
//    TAlgorithm());
//    stats.countTime = sysTime() - timer;
//
//    return count;
//}

// ----------------------------------------------------------------------------
// Function filter()
// ----------------------------------------------------------------------------
// q-Gram filters.

template <typename TText, typename TQueries>
inline unsigned long filter(Options const & options, Stats & stats, TText & text, TQueries & queries, Nothing)
{
    typedef Shape<Dna, UngappedShape<11> >                  TShape;
//    typedef IndexQGram<TShape, OpenAddressing>              TIndexSpec;
    typedef IndexQGram<TShape>                              TIndexSpec;
    typedef Index<TQueries, TIndexSpec>                     TPatternIndex;
    typedef Pattern<TPatternIndex, Swift<SwiftSemiGlobal> > TPattern;

    typedef typename Value<TText>::Type const               THaystack;
    typedef typename Size<TText>::Type                      THaystackSize;
    typedef Finder<THaystack, Swift<SwiftSemiGlobal> >      TFinder;

    unsigned long count = 0;

    double timer = sysTime();

    TPatternIndex patternIndex(queries);
    TPattern pattern(patternIndex);

    unsigned threshold = 1;
    for (unsigned i = 0; i < length(queries); ++i)
        setMinThreshold(pattern, i, threshold);

    for (THaystackSize i = 0; i < length(text); ++i)
    {
        TFinder finder(text[i]);
        while (find(finder, pattern, (double)(length(front(queries)) * options.errorRate)))
        {
            count++;
            // Verify match.
//            Finder<TContigSeq> verifyFinder(fragStore.contigStore[i].seq);
//            setPosition(verifyFinder, beginPosition(finder));
//            Pattern<TReadSeq, HammingSimple> verifyPattern(fragStore.readSeqStore[position(pattern).i1]);
//            unsigned readLength = length(fragStore.readSeqStore[position(pattern).i1]);
//            int minScore = -static_cast<int>(EPSILON * readLength);
//            while (find(verifyFinder, verifyPattern, minScore) && position(verifyFinder) < endPosition(infix(finder))) {
//                TAlignedRead match(length(fragStore.alignedReadStore), position(pattern).i1, i,
//                                   beginPosition(verifyFinder), endPosition(verifyFinder));
//                appendValue(fragStore.alignedReadStore, match);
//            }
        }
    }
    stats.filterTime = sysTime() - timer;

    return count;
}

// ----------------------------------------------------------------------------
// Function filter()
// ----------------------------------------------------------------------------
// Dispatch locate, distance, and search algorithm.

template <typename TText, typename TQueries>
inline unsigned long filter(Options const & options, Stats & stats, TText & text, TQueries & queries)
{
    switch (options.algorithmType)
    {
//    case Options::ALGO_SEEDS_EXACT:
//        return ...;

//    case Options::ALGO_SEEDS_APPROXIMATE:
//        switch (options.seedsErrors)
//        {
//        case 1:
//            return filter(options, stats, index, queries, TLocate(), Backtracking<HammingDistance, Threshold<1> >(), BfsIterator());
//        case 2:
//            return filter(options, stats, index, queries, TLocate(), Backtracking<HammingDistance, Threshold<2> >(), BfsIterator());
//        }

    case Options::ALGO_QGRAMS_CONTIGUOUS:
        return filter(options, stats, text, queries, Nothing());

//    case Options::ALGO_QGRAMS_GAPPED:
//        return ...;

    default:
        throw RuntimeError("Unsupported filter");
    }
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

    TText text;

    if (!open(text, toCString(options.textFile)))
        throw RuntimeError("Error while loading text");

//    TIndex index;
//
//    if (!open(index, toCString(options.textIndexFile)))
//        throw RuntimeError("Error while loading full-text index");

    TQueries queries;

    if (!open(queries, toCString(options.queryFile)))
        throw RuntimeError("Error while loading queries");

    Stats stats;

    unsigned long hitsCount = filter(options, stats, text, queries);

    if (options.tsv)
    {
//        std::cout << hitsCount << '\t' << std::fixed << stats.countTime << '\t' << stats.preprocessingTime << std::endl;
    }
    else
    {
        std::cout << length(queries) << " queries" << std::endl;
        std::cout << lengthSum(queries) << " symbols" << std::endl;
        std::cout << hitsCount << " hits" << std::endl;
//        std::cout << std::fixed << stats.countTime << " + " << stats.preprocessingTime << " sec" << std::endl;
    }
}

int main(int argc, char const ** argv)
{
    return run<Options>(argc, argv);
}
