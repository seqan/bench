#include <iostream>

#include <seqan/seq_io.h>
#include <seqan/arg_parse.h>
#include <seqan/index.h>
#include <seqan/find.h>

using namespace seqan;

struct Options {
    std::string inputFile;
    std::string outputFile;
};

seqan::ArgumentParser::ParseResult parseCommandLine(Options & options, int argc, char** const argv)
{
    seqan::ArgumentParser parser("Global Pairwise Alignment benchmark demo");
    setCategory(parser, "Benchmark");
    setVersion(parser, "0.1");
    setDate(parser, "Apr 2016");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] \\fIIN\\fP \\fIOUT\\fP ");

    addDescription(parser,
    "\\fIIN\\fP is a .fa / fasta input file.  \\fIOUT\\fP is a collections of created indices.");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE, "IN"));
    addArgument(parser, ArgParseArgument(ArgParseArgument::OUTPUT_FILE, "OUT"));
    setValidValues(parser, 0, "FASTS, fa");
    // setValidValues(parser, 1, "inx");

    addSection(parser, "Settings");

    seqan::ArgumentParser::ParseResult res = seqan::parse(parser, argc, argv);
    if(res == seqan::ArgumentParser::PARSE_OK)
    {
        getArgumentValue(options.inputFile, parser, 0);
        getArgumentValue(options.outputFile, parser, 1);
    }
    return res;
}

template <typename TIndexSpec, typename TString>
void create_index(TString & genome, std::string filename)
{
    Index<TString, TIndexSpec > index(genome);
    Finder<Index<TString, TIndexSpec > > finder(index);

    TString needle = "TTTTATGGTAACGATACATTAAGGATAGTTTTTTAGAGTTCACGGTTGAA";
    find(finder, needle);

    save(index, toCString(filename));
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

    // create sa index
    create_index<IndexSa<> >(genome, options.outputFile + "_sa.inx");

    // create esa index
    create_index<IndexEsa<> >(genome, options.outputFile + "_esa.inx");

    // create fm index
    create_index<FMIndex<> >(genome, options.outputFile + "_fm.inx");

    // create wotd index
    create_index<IndexWotd<> >(genome, options.outputFile + "_wotd.inx");

    // create dfi index
    create_index<IndexWotd<Dfi<> > >(genome, options.outputFile + "_wotd_dfi.inx");

    // create qgram index
    create_index<IndexQGram<UngappedShape<50>, OpenAddressing > >(genome, options.outputFile + "_qgram.inx");
}
