#include <iostream>

#include <seqan/seq_io.h>
#include <seqan/arg_parse.h>
#include <seqan/index.h>
#include <seqan/find.h>

using namespace seqan;

struct Options {
    std::string inputFile;
    std::string outputFile;
    int createAll;
};

seqan::ArgumentParser::ParseResult parseCommandLine(Options & options, int argc, char** const argv)
{
    seqan::ArgumentParser parser("index_create");
    setCategory(parser, "Benchmark");
    setVersion(parser, "0.1");
    setDate(parser, "Apr 2016");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] \\fIIN\\fP \\fIOUT\\fP ");

    addDescription(parser,
    "Preprocess data and create an index. \\fIIN\\fP is a .fa / fasta input file.  \\fIOUT\\fP is a collections of created indices.");

    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE, "IN"));
    addArgument(parser, ArgParseArgument(ArgParseArgument::OUTPUT_PREFIX, "OUT"));
    setValidValues(parser, 0, "FASTS, fa");
    // setValidValues(parser, 1, "inx");

    addSection(parser, "Settings");
    addOption(parser, seqan::ArgParseOption("a", "all", "Build every available index in seqan", seqan::ArgParseArgument::INTEGER, "[0,1]"));
    setDefaultValue(parser, "all", 0);

    seqan::ArgumentParser::ParseResult res = seqan::parse(parser, argc, argv);
    if(res == seqan::ArgumentParser::PARSE_OK)
    {
        getArgumentValue(options.inputFile, parser, 0);
        getArgumentValue(options.outputFile, parser, 1);
        getOptionValue(options.createAll, parser, "all");
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

    // create esa index
    std::cout << "Create Esa-Index" << std::endl;
    create_index<IndexEsa<> >(genome, options.outputFile + "_esa.inx");

    if (options.createAll) {
        // create sa index
        std::cout << "Create Sa-Index" << std::endl;
        create_index<IndexSa<> >(genome, options.outputFile + "_sa.inx");

        // create fm index
        std::cout << "Create FM-Index" << std::endl;
        create_index<FMIndex<> >(genome, options.outputFile + "_fm.inx");

        // create wotd index
        std::cout << "Create Wotd-Index" << std::endl;
        create_index<IndexWotd<> >(genome, options.outputFile + "_wotd.inx");

        // create dfi index
        std::cout << "Create Wotd-dfi-Index" << std::endl;
        create_index<IndexWotd<Dfi<> > >(genome, options.outputFile + "_wotd_dfi.inx");

        // create 10gram index
        // std::cout << "Create 10gram-Index" << std::endl;
        // create_index<IndexQGram<UngappedShape<10> > >(genome, options.outputFile + "_10gram.inx");

        // create 15gram index
        // std::cout << "Create 15gram-Index" << std::endl;
        // create_index<IndexQGram<UngappedShape<15>, OpenAddressing> >(genome, options.outputFile + "_15gram.inx");

        // create 50gram index
        std::cout << "Create 50gram-Index" << std::endl;
        create_index<IndexQGram<UngappedShape<50>, OpenAddressing > >(genome, options.outputFile + "_50gram.inx");
    }
}
