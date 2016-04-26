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

seqan::ArgumentParser::ParseResult parseCommandLine(Options & options, int argc, char** const argv)
{
    seqan::ArgumentParser parser("Global Pairwise Alignment benchmark demo");
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

int main(int argc, char **argv)
{
    Options options;
    ArgumentParser::ParseResult res = parseCommandLine(options, argc, argv);
    if (res != seqan::ArgumentParser::PARSE_OK)
        return res == seqan::ArgumentParser::PARSE_ERROR;
    #ifdef _OPENMP
    omp_set_num_threads(options.threads);
    #endif

    StringSet<CharString> id;
    StringSet<Dna5String> seq;
    SeqFileIn seqFileIn(toCString(options.input));
    readRecords(id, seq, seqFileIn);

    Align<Dna5String> ali;
    resize(rows(ali), 2);
    String<int> score;
    resize(score, length(id)*length(id));
    std::ofstream ofs(toCString(options.output), std::ofstream::out);

    SEQAN_OMP_PRAGMA(parallel for firstprivate(ali))
    for (unsigned m = 0; m < length(id); m++)
    for (unsigned n = m + 1; n < length(id); n++)
    {
        assignSource(row(ali, 0), seq[m]);
        assignSource(row(ali, 1), seq[n]);

        score[m * length(id) + n] = globalAlignment(ali, Score<int>(2, -3, -2));
    }

    //serial output score
    for (unsigned m = 0; m < length(id); m++)
    for (unsigned n = m + 1; n < length(id); n++)
        ofs << m << " " << n << " " << score[m*length(id) + n] << std::endl;

    return 0;
}
