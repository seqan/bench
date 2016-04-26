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

    std::ofstream ofs(toCString(options.output), std::ofstream::out);

    Dna5String Base = "ACGT";
    Dna5String tmpSeq;
    SEQAN_OMP_PRAGMA(parallel for firstprivate(esaFinder, tmpSeq))
    for (unsigned k = 0; k < length(id); k++)
    {
        for (unsigned j1 = 0; j1 < length(seq[k]); j1++)
            for (unsigned j2 = j1 + 1; j2 < length(seq[k]); j2++)
                for (unsigned j3 = j2 + 1; j3 < length(seq[k]); j3++)
                    //for (unsigned j4 = j3 + 1; j4 < length(seq[k]); j4++)
                    {
                        tmpSeq = seq[k];
                        for (unsigned n1 = 0; n1 < length(Base); n1++)
                        for (unsigned n2 = 0; n2 < length(Base); n2++)
                        for (unsigned n3 = 0; n3 < length(Base); n3++)
                        //for (unsigned n4 = 0; n4 < length(Base); n4++)
                        {
                            unsigned count = 0;
                            tmpSeq[j1] = Base[n1];
                            tmpSeq[j2] = Base[n2];
                            tmpSeq[j3] = Base[n3];
                          //  tmpSeq[j4] = Base[n4];
                            if (Base[n1] == seq[k][j1])
                                count++;
                            if (Base[n2] == seq[k][j2])
                                count++;
                            if (Base[n3] == seq[k][j3])
                                count++;
                           // if (Base[n4] == seq[k][j4])
                                count++;

                            while(find(esaFinder, tmpSeq))
                                ofs << position(esaFinder) << std::endl;
                            clear(esaFinder);

                        }

                    }

    }

    //SEQAN_OMP_PRAGMA(parallel for firstprivate(ali))
    //for (unsigned m = 0; m < length(id); m++)
    //for (unsigned n = m + 1; n < length(id); n++)
    //{
    //    assignSource(row(ali, 0), seq[m]);
    //    assignSource(row(ali, 1), seq[n]);
    //
    //    score[m*length(id) + n] = globalAlignment(ali, Score<int>(2, -3, -2));
    //
    //}
    ////serial output
    //for (unsigned m = 0; m < length(id); m++)
    //for (unsigned n = m + 1; n < length(id); n++)

    //    ofs << m << " " << n << " " << score[m*length(id) + n] << std::endl;

    return 0;
}
