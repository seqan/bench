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
// This file contains the backtracking application.
// ==========================================================================

#include <seqan/basic.h>
#include <seqan/sequence.h>

#include "../../../../extras/apps/masai/options.h"
#include "../../../../extras/apps/masai/mapper.h"

using namespace seqan;

// ============================================================================
// Tags, Classes, Enums
// ============================================================================

// ----------------------------------------------------------------------------
// Class Options
// ----------------------------------------------------------------------------

struct Options : public MasaiOptions
{
    enum AlgorithmType
    {
        ALGO_SINGLE, ALGO_DFS, ALGO_BFS
    };

    CharString  genomeFile;
    CharString  genomeIndexFile;
    IndexType   genomeIndexType;
    CharString  readsFile;

    AlgorithmType algorithmType;
    unsigned    seedErrors;
    unsigned    seedLength;
    unsigned    seedCount;
    bool        mismatchesOnly;

    TList       algorithmTypeList;

    Options() :
        MasaiOptions(),
        genomeIndexType(INDEX_SA),
        algorithmType(ALGO_SINGLE),
        seedErrors(5),
        seedLength(33),
        seedCount(10000),
        mismatchesOnly(false)
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
// Function setAlgorithmType()
// ----------------------------------------------------------------------------

template <typename TOptions>
void setAlgorithmType(ArgumentParser & parser, TOptions const & options)
{
    addOption(parser, ArgParseOption("a", "algorithm", "Select the algorithm.", ArgParseOption::STRING));
    setValidValues(parser, "algorithm", options.algorithmTypeList);
    setDefaultValue(parser, "algorithm", options.algorithmTypeList[options.algorithmType]);
}

// ----------------------------------------------------------------------------
// Function getAlgorithmType()
// ----------------------------------------------------------------------------

template <typename TOptions>
void getAlgorithmType(TOptions & options, ArgumentParser const & parser)
{
    getOptionValue(options.algorithmType, parser, "algorithm", options.algorithmTypeList);
}

// ----------------------------------------------------------------------------
// Function setupArgumentParser()                              [ArgumentParser]
// ----------------------------------------------------------------------------

void setupArgumentParser(ArgumentParser & parser, Options const & options)
{
    setAppName(parser, "backtracking");
    setShortDescription(parser, "Backtracking");
    setCategory(parser, "Benchmarking");

    setDateAndVersion(parser);
    setDescription(parser);

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fIGENOME FILE\\fP> <\\fIREADS FILE\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUTFILE));
    setValidValues(parser, 0, "fasta fa");
    setValidValues(parser, 1, "fastq fasta fa");

    addSection(parser, "Genome Index Options");

    setIndexType(parser, options);
    setIndexPrefix(parser);

    addSection(parser, "Benchmarking Parameters");

    setAlgorithmType(parser, options);

    addOption(parser, ArgParseOption("e", "seed-errors", "Number of errors per seed.", ArgParseOption::INTEGER));
    setMinValue(parser, "seed-errors", "0");
    setMaxValue(parser, "seed-errors", "5");
    setDefaultValue(parser, "seed-errors", options.seedErrors);

    addOption(parser, ArgParseOption("sl", "seed-length", "Seed length.", ArgParseOption::INTEGER));
    setMinValue(parser, "seed-length", "5");
    setMaxValue(parser, "seed-length", "100");
    setDefaultValue(parser, "seed-length", options.seedLength);

    addOption(parser, ArgParseOption("sc", "seed-count", "Quantity of seeds.", ArgParseOption::INTEGER));
    setMinValue(parser, "seed-count", "10");
    setDefaultValue(parser, "seed-count", options.seedCount);

    addOption(parser, ArgParseOption("ng", "no-gaps", "Do not align seeds with gaps."));
}

// ----------------------------------------------------------------------------
// Function parseCommandLine()                                        [Options]
// ----------------------------------------------------------------------------

ArgumentParser::ParseResult
parseCommandLine(Options & options, ArgumentParser & parser, int argc, char const ** argv)
{
    ArgumentParser::ParseResult res = parse(parser, argc, argv);

    if (res != seqan::ArgumentParser::PARSE_OK)
        return res;

    // Parse genome input file.
    getArgumentValue(options.genomeFile, parser, 0);

    // Parse reads input file.
    getArgumentValue(options.readsFile, parser, 1);

    // Parse genome index prefix.
    getIndexPrefix(options, parser);

    // Parse genome index type.
    getIndexType(options, parser);

    // Parse algorithm type.
    getAlgorithmType(options, parser);

    // Parse benchmarking parameters.
    getOptionValue(options.seedErrors, parser, "seed-errors");
    getOptionValue(options.seedLength, parser, "seed-length");
    getOptionValue(options.seedCount, parser, "seed-count");
    options.mismatchesOnly = isSet(parser, "no-gaps");

    return seqan::ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function isDisabled()
// ----------------------------------------------------------------------------

namespace seqan {
template <typename TReadId>
inline bool isDisabled(Nothing const & /* manager */, TReadId /* readId */)
{
    return false;
}
}

// ----------------------------------------------------------------------------
// Function findSeedsExact()                                           [Seeder]
// ----------------------------------------------------------------------------

template <typename TSpec, typename TGenomeIndex, typename TDistance>
void findSeedsExact(Seeder<Nothing, Nothing, TSpec> & seeder,
                    TGenomeIndex & genomeIndex,
                    TReadSeqSize seedsLength,
                    TReadSeqSize /* errorsPerSeed */,
                    TDistance const & /* tag */)
{
    typedef Backtracking<TDistance, Stretched<> >           TBacktracking;
//	typedef Backtracking<TDistance>                         TBacktracking;
    typedef Finder<TGenomeIndex, TBacktracking>             TFinder;
    typedef Pattern<TReadsQGram, TBacktracking>             TPattern;

    TFinder finder(genomeIndex);
    TPattern pattern(seeder.readsQGram, seedsLength);

    while (_resume(finder, pattern, 0u))
        seeder.hitsCount += (finder.index_range.i2 - finder.index_range.i1) * (pattern.index_range.i2 - pattern.index_range.i1);
//        seeder.hitsCount += countOccurrences(finder.index_iterator) * countOccurrences(pattern.index_iterator);
}

template <typename TGenomeIndex, typename TDistance>
void findSeedsExact(Seeder<Nothing, Nothing, SingleBacktracking> & seeder,
                    TGenomeIndex & genomeIndex,
                    TReadSeqSize seedsLength,
                    TReadSeqSize firstSeed,
                    TReadSeqSize lastSeed,
                    TDistance const & /* tag */)
{
    typedef typename Iterator<TGenomeIndex, TopDown<> >::Type   TIterator;

    TIterator it(genomeIndex);

    TReadSeqStoreSize readsCount = seeder.readsCount * 2;

    for (TReadSeqStoreSize readId = 0; readId < readsCount; ++readId)
    {
        TReadSeq & read = seeder.store.readSeqStore[readId];

        for (TReadSeqSize seed = firstSeed; seed < lastSeed; ++seed)
        {
            goRoot(it);

            if (goDown(it, infix(read, seedsLength * seed, seedsLength * (seed + 1))))
                seeder.hitsCount += countOccurrences(it);
        }
    }
}

// ----------------------------------------------------------------------------
// Function findSeedsApproximate()                                     [Seeder]
// ----------------------------------------------------------------------------

template <typename TSpec, typename TGenomeIndex, typename TDistance>
void findSeedsApproximate(Seeder<Nothing, Nothing, TSpec> & seeder,
                          TGenomeIndex & genomeIndex,
                          TReadSeqSize seedsLength,
                          TReadSeqSize errorsPerSeed,
                          TDistance const & /* tag */)
{
    typedef Backtracking<TDistance>                         TBacktracking;
    typedef Finder<TGenomeIndex, TBacktracking>             TFinder;
    typedef Pattern<TReadsWotd, TBacktracking>              TPattern;

    TFinder finder(genomeIndex);
    TPattern pattern(seeder.readsWotd, seedsLength);

    while (_resume(finder, pattern, errorsPerSeed))
        seeder.hitsCount += (finder.index_range.i2 - finder.index_range.i1) * (pattern.index_range.i2 - pattern.index_range.i1);
//        seeder.hitsCount += countOccurrences(finder.index_iterator) * countOccurrences(pattern.index_iterator);
}

template <typename TGenomeIndex, typename TDistance>
void findSeedsApproximate(Seeder<Nothing, Nothing, SingleBacktracking> & seeder,
                          TGenomeIndex & genomeIndex,
                          TReadSeqSize seedsLength,
                          TReadSeqSize errorsPerSeed,
                          TReadSeqSize firstSeed,
                          TReadSeqSize lastSeed,
                          TDistance const & /* tag */)
{
    typedef Backtracking<TDistance>                 TBacktracking;
    typedef Finder<TGenomeIndex, TBacktracking>     TFinder;
    typedef Pattern<TReadSeq, TBacktracking>        TPattern;

    TFinder finder(genomeIndex);
    TPattern pattern;

    TReadSeqStoreSize readsCount = seeder.readsCount * 2;

    for (TReadSeqStoreSize readId = 0; readId < readsCount; ++readId)
    {
        TReadSeq & read = seeder.store.readSeqStore[readId];

        for (TReadSeqSize seed = firstSeed; seed < lastSeed; ++seed)
        {
            clear(finder);
            setHost(pattern, infix(read, seedsLength * seed, seedsLength * (seed + 1)));

            while (_resume(finder, pattern) && _backtrack(finder, pattern, errorsPerSeed))
                seeder.hitsCount += finder.index_range.i2 - finder.index_range.i1;
        }
    }
}

// ----------------------------------------------------------------------------
// Function runBenchmark()
// ----------------------------------------------------------------------------

template <typename TIndex, typename TAlgorithm, typename TDistance>
int runBenchmark(Options & options)
{
    typedef Mapper<TIndex>                                  TMapper;
    typedef Seeder<Nothing, Nothing, TAlgorithm>            TSeeder;

    TMapper mapper(options.seedLength, false, false, false);

    double start, finish;

    // Loading genome.
    std::cout << "Loading genome:\t\t\t" << std::flush;
    start = sysTime();
    if (!loadGenome(mapper.indexer, options.genomeFile))
    {
        std::cerr << "Error while loading genome" << std::endl;
        return 1;
    }
    finish = sysTime();
    std::cout << finish - start << " sec" << std::endl;

    // Loading genome index.
    std::cout << "Loading genome index:\t\t" << std::flush;
    start = sysTime();
    if (!loadGenomeIndex(mapper.indexer, options.genomeIndexFile))
    {
        std::cout << "Error while loading genome index" << std::endl;
        return 1;
    }
    finish = sysTime();
    std::cout << finish - start << " sec" << std::endl;

    // Loading reads.
    std::cout << "Loading reads:\t\t\t" << std::flush;
    start = sysTime();
    if (!loadReads(mapper, options.readsFile, Raw()))
    {
        std::cerr << "Error while loading reads" << std::endl;
        return 1;
    }
    finish = sysTime();
    std::cout << finish - start << " sec" << std::endl;
    std::cout << "Reads count:\t\t\t" << mapper.readsCount << std::endl;

    // Init seeder.
    Nothing nothing;
    TSeeder seeder(mapper.store, nothing, nothing);
    seeder.readsCount = options.seedCount / 2;

    std::cout << "Seeds count:\t\t\t" << seeder.readsCount * 2 << std::endl;
    if (seeder.readsCount > mapper.readsCount)
    {
        std::cerr << "Not enough seeds" << std::endl;
        return 1;
    }

    // Benchmark backtracking.
    start = sysTime();
    find(seeder, mapper.indexer.genomeIndex, options.seedLength, options.seedErrors, 0, 1, TDistance());
    finish = sysTime();
    std::cout << "Backtracking time:\t\t" << std::flush;
    std::cout << finish - start << " sec" << std::endl;

    std::cout << "Hits:\t\t\t\t" << seeder.hitsCount << std::endl;

    return 0;
}

template <typename TIndex, typename TAlgorithm>
int configureDistance(Options & options)
{
//    if (options.mismatchesOnly)
        return runBenchmark<TIndex, TAlgorithm, HammingDistance>(options);
//    else
//        return runBenchmark<TIndex, TAlgorithm, EditDistance>(options);
}

template <typename TIndex>
int configureAlgorithm(Options & options)
{
    switch (options.algorithmType)
    {
    case Options::ALGO_SINGLE:
        return configureDistance<TIndex, SingleBacktracking>(options);

    case Options::ALGO_DFS:
        return configureDistance<TIndex, MultipleBacktracking>(options);

    default:
        return 1;
    }
}

int mainWithOptions(Options & options)
{
    switch (options.genomeIndexType)
    {
    case Options::INDEX_ESA:
        return configureAlgorithm<TGenomeEsa>(options);

    case Options::INDEX_SA:
        return configureAlgorithm<TGenomeSa>(options);

//    case Options::INDEX_QGRAM:
//        return configureAlgorithm<TGenomeQGram>(options);

        case Options::INDEX_FM:
            return configureAlgorithm<TGenomeFM>(options);

    default:
        return 1;
    }
}

int main(int argc, char const ** argv)
{
    ArgumentParser parser;
    Options options;
    setupArgumentParser(parser, options);

    ArgumentParser::ParseResult res = parseCommandLine(options, parser, argc, argv);

    if (res == seqan::ArgumentParser::PARSE_OK)
        return mainWithOptions(options);
    else
        return res;
}
