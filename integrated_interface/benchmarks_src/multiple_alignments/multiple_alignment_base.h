#ifndef BENCHMARK_HEADER_MULTIPLE_ALIGNMENT_BASE_H
#define BENCHMARK_HEADER_MULTIPLE_ALIGNMENT_BASE_H

#include <iostream>
#include <seqan/align.h>
#include <seqan/seq_io.h>
#include <seqan/arg_parse.h>

using namespace seqan;

struct Options {
    unsigned threads;
    CharString input;
    CharString output;
};

seqan::ArgumentParser::ParseResult parseCommandLine(std::string infoText, Options & options, int argc, char** const argv)
{
    seqan::ArgumentParser parser(infoText);
    setCategory(parser, "Demo");
    setVersion(parser, "0.1");
    setDate(parser, "Nov 2015");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] \\fIIN\\fP \\fIOUT\\fP ");

    addDescription(parser,
    "\\fIIN\\fP is a .fa / fasta input file.  \\fIOUT\\fP is a txt output file.");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE, "IN"));
    addArgument(parser, ArgParseArgument(ArgParseArgument::OUTPUT_FILE, "OUT"));
    setValidValues(parser, 0, "fasta, fa");
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

template <typename TSequence, typename TScore>
inline int benchmark_multiple_alignment_main(std::string infoText, TScore computeAlignment, int argc, char **argv)
{
    Options options;
    ArgumentParser::ParseResult res = parseCommandLine(infoText, options, argc, argv);
    if (res != seqan::ArgumentParser::PARSE_OK)
        return res == seqan::ArgumentParser::PARSE_ERROR;
    #ifdef _OPENMP
    omp_set_num_threads(options.threads);
    #endif

    StringSet<CharString> ids;
    StringSet<TSequence> sequences;
    SeqFileIn seqFileIn(toCString(options.input));
    readRecords(ids, sequences, seqFileIn);

    typedef StringSet<Dna5String> TStringSet;
    typedef StringSet<Dna5String, Dependent<> > TDepStringSet;
    typedef Graph<Alignment<TDepStringSet> > TAlignGraph;

    TAlignGraph alignGraph(sequences);

    return 0;
}

#endif