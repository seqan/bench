#ifndef BENCHMARK_HEADER_INDEX_ONE_ERROR_SEARCH_BASE_H
#define BENCHMARK_HEADER_INDEX_ONE_ERROR_SEARCH_BASE_H

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

template <typename TFinder>
inline void benchmark_index_one_error_search(TFinder && finder, StringSet<Dna5String> seqs, CharString outputFile)
{
    typedef Backtracking<> TBacktracking;
    typedef Pattern<Dna5String, TBacktracking> TPattern;

    std::ofstream ofs(toCString(outputFile), std::ofstream::out);

    // SEQAN_OMP_PRAGMA(parallel for firstprivate(finder))
    for (unsigned k = 0; k < length(seqs); k++)
    {
        TPattern pattern(seqs[k]);
        if(find(finder, pattern, 1u))
        {
            ofs << k << ": " << position(finder);

            while(find(finder, seqs[k], 1u))
            {
                ofs << "," << position(finder);
            }

            ofs << std::endl;
        }
        clear(finder);
    }
}

template <typename TIndexSpec>
inline int benchmark_index_one_error_search_main(std::string infoText, int argc, char **argv)
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
    typedef Finder<TIndex, TBacktracking> TFinder;
    typedef Pattern<Dna5String, TBacktracking> TPattern;

    // read in all 50-reads
    StringSet<CharString> ids;
    StringSet<Dna5String> seqs;
    SeqFileIn seqFileIn(toCString(options.readFile));
    readRecords(ids, seqs, seqFileIn);

    // read in the index of the genome
    Index<Dna5String, TIndexSpec > index;
    open(index, toCString(options.indexFile));
    TFinder finder(index);

    // search all reads in the index structure
    benchmark_index_one_error_search(finder, seqs, options.output);

    return 0;
}

#endif