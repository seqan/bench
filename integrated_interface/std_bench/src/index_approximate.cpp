#include <iostream>
#include <seqan/seq_io.h>
#include <seqan/arg_parse.h>
#include <seqan/index.h>
#include <seqan/find.h>

using namespace seqan;

struct Options{
    unsigned threads;
    CharString readFile;
    CharString indexFile;
    CharString output;
};

seqan::ArgumentParser::ParseResult parseCommandLine(Options & options, int argc, char** const argv){
    seqan::ArgumentParser parser("Global Pairwise Alignment benchmark demo");
    setCategory(parser, "Demo");
    setVersion(parser, "0.1");
    setDate(parser, "Nov 2015");
    
    addUsageLine(parser, "[\\fIOPTIONS\\fP] \\fIIN\\fP \\fIOUT\\fP ");
    
    addDescription(parser,
    "\\fIIN\\fP is a .fa / fasta input file.  \\fIOUT\\fP is a txt output file."); 
    
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE, "IN"));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INPUT_FILE, "IN"));
    addArgument(parser, ArgParseArgument(ArgParseArgument::OUTPUT_FILE, "OUT"));
    setValidValues(parser, 0, "FASTS, fa");
    setValidValues(parser, 1, "inx");
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
        getArgumentValue(options.readFile, parser, 0);
        getArgumentValue(options.indexFile, parser, 1);
        getArgumentValue(options.output, parser, 2);
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
    SeqFileIn seqFileIn(toCString(options.readFile));
    readRecords(id, seq, seqFileIn);

    Index<String<Dna5>, IndexEsa<> > index;
    open(index, toCString(options.indexFile));
    Finder<Index<String<Dna5>, IndexEsa<> > > esaFinder(index);
    
    SEQAN_OMP_PRAGMA(parallel for firstprivate(esaFinder))   
    for (unsigned m = 0; m < length(id); m++)  
    {   
        
        while(find(esaFinder, seq[m]))
            std::cout << position(esaFinder) << std::endl;
        clear(esaFinder);
    }
}


