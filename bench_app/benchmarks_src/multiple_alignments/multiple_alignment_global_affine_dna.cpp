#include <iostream>
#include <seqan/align.h>
#include <seqan/graph_msa.h>

// #include <seqan/basic.h>
// #include <seqan/graph_msa.h>
// #include <seqan/modifier.h>
// #include <seqan/arg_parse.h>
#include <seqan/seq_io.h>
// #include <seqan/stream.h>


using namespace seqan;

int main(int argc, char **argv)
{
    typedef StringSet<Dna5String> TStringSet;
    typedef StringSet<Dna5String, Dependent<> > TDepStringSet;
    typedef Graph<Alignment<TDepStringSet> > TAlignGraph;

    StringSet<Dna5String> sequences;
    appendValue(sequences, "CGTACGTACGTACGTACGTACGTA");
    appendValue(sequences, "CGTACGT");
    appendValue(sequences, "CGTACGTCGTCCGTA");
    appendValue(sequences, "CATACGTCAAAAAAAAGTCCTTA");

    StringSet<String<char> > ids;
    appendValue(sequences, "id1");
    appendValue(sequences, "id2");
    appendValue(sequences, "id3");
    appendValue(sequences, "id4");

    TAlignGraph alignGraph(sequences);

    //more verbose:
    // TScore = decltype(Blosum62(-3, -1))
    // MsaOptions<AminoAcid, TScore> msaOpt;
    // msaOpt.sc = scoreObject;
    // msaOpt.outfile = "PATH";
    // appendValue(msaOpt.method, 0);  // Global pairwise
    // appendValue(msaOpt.method, 1);  // Local pairwise
    // globalMsaAlignment(alignGraph, Blosum62(-3, -1), ids, msaOpt);

    // Blosum62, gap_extend: -3, gap_open: -1
    globalMsaAlignment(alignGraph, Blosum62(-3, -1));
    std::cout << alignGraph << "\n";

    std::string outfile = "/home/mi/marehr/ClionProjects/build_bench/seqan_build/release/apps/bench/integrated_interface/multiple_alignment_output.fa";

    typedef VirtualStream<char, Output> TOutStream;
    TOutStream outStream;

    if (!open(outStream, toCString(outfile)))
    {
        std::cerr << "Can't open " << outfile << " for writing!" << std::endl;
        return 0;
    }
    if (guessFormatFromFilename(outfile, Fasta())) {
        std::cerr << "Fasta file!!" << std::endl;
        write(outStream, alignGraph, ids, FastaFormat());
    } else {
        std::cerr << "No fasta file?!" << std::endl;
    }

    return 0;
}
