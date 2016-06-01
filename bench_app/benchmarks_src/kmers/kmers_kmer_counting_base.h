#ifndef BENCHMARK_HEADER_KMERS_KMER_COUNTING_BASE_H
#define BENCHMARK_HEADER_KMERS_KMER_COUNTING_BASE_H

#include <iostream>
#include <seqan/align.h>
#include <seqan/seq_io.h>
#include <seqan/arg_parse.h>
#include <seqan/index.h>
#include <seqan/find.h>

using namespace seqan;

struct Options {
    unsigned threads;
    CharString input;
    CharString output;
};

seqan::ArgumentParser::ParseResult parseCommandLine(std::string infoText, Options & options, int argc, char** const argv)
{
    seqan::ArgumentParser parser(infoText);
    setCategory(parser, "Benchmark");
    setVersion(parser, "0.1");
    setDate(parser, "April 2016");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] \\fIIN\\fP \\fIOUT\\fP ");

    addDescription(parser,
    "\\fIIN\\fP is a .fa / fasta input file containing a part of a genome. \\fIOUT\\fP is a txt output file.");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE, "READS"));
    addArgument(parser, ArgParseArgument(ArgParseArgument::OUTPUT_FILE, "OUT"));
    setValidValues(parser, 0, "FASTS, fa");
    setValidValues(parser, 1, "txt");

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
        getArgumentValue(options.input, parser, 0);
        getArgumentValue(options.output, parser, 1);
    }
    return res;
}

template <unsigned kmerLength, typename TIndex, typename TString, typename TOutputStream>
inline void kmer_counting (TIndex & index, TString & genome, TOutputStream & cout)
{
    unsigned i = 0;
    auto it = begin(genome);
    auto itEnd = end(genome);
    for(; it != itEnd; ++it)
    {
        hash(indexShape(index), it);
        auto count = countOccurrences(index, indexShape(index));
        auto firstPosition = getOccurrence(index, indexShape(index));

        if (i <= firstPosition && count >= 5u) {
            cout << i << ": " <<  count << std::endl;
        }

        i++;
    }
}

template <typename TIndexSpec, unsigned kmerLength>
inline int benchmark_kmers_kmer_counting_main(std::string infoText, int argc, char **argv)
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

    // read in the genome
    CharString id;
    Dna5String genome;
    SeqFileIn seqFileIn(toCString(options.input));
    readRecord(id, genome, seqFileIn);

    // read in the index of the genome
    Index<Dna5String, TIndexSpec > index(genome);

    // prepare write steam
    std::ofstream ofs(toCString(options.output), std::ofstream::out);

    // search all reads in the index structure
    kmer_counting<kmerLength>(index, genome, ofs);

    return 0;
}

#endif
