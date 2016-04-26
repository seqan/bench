#include <iostream>
#include <seqan/seq_io.h>
#include <seqan/arg_parse.h>
#include <seqan/index.h>
#include <seqan/find.h>

using namespace seqan;

struct Options {
    CharString inputFile;
    CharString outputFile;
};

seqan::ArgumentParser::ParseResult parseCommandLine(Options & options, int argc, char** const argv)
{
    seqan::ArgumentParser parser("Global Pairwise Alignment benchmark demo");
    setCategory(parser, "Demo");
    setVersion(parser, "0.1");
    setDate(parser, "Nov 2015");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] \\fIIN\\fP \\fIOUT\\fP ");

    addDescription(parser,
    "\\fIIN\\fP is a .fa / fasta input file.  \\fIOUT\\fP is a inx output file.");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE, "IN"));
    addArgument(parser, ArgParseArgument(ArgParseArgument::OUTPUT_FILE, "OUT"));
    setValidValues(parser, 0, "FASTS, fa");
    setValidValues(parser, 1, "inx");

    addSection(parser, "Settings");

    seqan::ArgumentParser::ParseResult res = seqan::parse(parser, argc, argv);
    if(res == seqan::ArgumentParser::PARSE_OK)
    {
        getArgumentValue(options.inputFile, parser, 0);
        getArgumentValue(options.outputFile, parser, 1);
    }
    return res;
}

int main(int argc, char **argv)
{
    Options options;
    ArgumentParser::ParseResult res = parseCommandLine(options, argc, argv);
    if (res != seqan::ArgumentParser::PARSE_OK)
        return res == seqan::ArgumentParser::PARSE_ERROR;

    CharString idGenome;
    Dna5String genome;
    SeqFileIn seqFileIn(toCString(options.inputFile));
    readRecord(idGenome, genome, seqFileIn);

    Index<String<Dna5>, IndexEsa<> > index(genome), openIndex;
    Finder<Index<String<Dna5>, IndexEsa<> > > esaFinder(index);

    indexRequire(index, EsaSA());
    indexRequire(index, EsaLcp());
    indexRequire(index, EsaChildtab());  // for TopDown iterators
    indexRequire(index, EsaBwt());       // for (Super-)MaxRepeats iterators

    save(index, toCString(options.outputFile));
    open(openIndex, toCString(options.outputFile));
    std::cout << length(openIndex) << std::endl;
}
