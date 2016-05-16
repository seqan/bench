#include <iostream>
#include <seqan/align.h>
#include <seqan/seq_io.h>
#include <seqan/arg_parse.h>
#include <seqan/index.h>
#include <seqan/find.h>

using namespace seqan;

void example1()
{
///First, we create a @Class.StringSet@ of 4 @Class.String.Strings@.
    StringSet<String<char> > mySet;
    resize(mySet, 4);
    mySet[0] = "tobeornottobe";
    mySet[1] = "thebeeonthecomb";
    mySet[2] = "hellobebe";
    mySet[3] = "beingjohnmalkovich";

///Then we create an @Class.Index@ of our @Class.StringSet@ and
///a @Class.Finder@ of the @Class.Index@.
    typedef Index<StringSet<String<char> >, IndexQGram<UngappedShape<2> > > TIndex;
    typedef Infix<Fibre<TIndex, QGramCounts>::Type const>::Type TCounts;

    TIndex myIndex(mySet);

/// Now we output how often $"be"$ occurs in each sequence.
    std::cout << "Number of sequences: " << countSequences(myIndex) << std::endl;
    hash(indexShape(myIndex), "be");
    TCounts cnts = countOccurrencesMultiple(myIndex, indexShape(myIndex));
    for (unsigned i = 0; i < length(cnts); ++i)
        std::cout << cnts[i].i2 << " occurrences in sequence " << cnts[i].i1  << std::endl;
}

void example2()
{
///First, we create a @Class.StringSet@ of 4 @Class.String.Strings@.
    typedef String<char> TString;
    TString genome = "tobeornottobe";

///Then we create an @Class.Index@ of our @Class.StringSet@ and
///a @Class.Finder@ of the @Class.Index@.
    typedef IndexQGram<UngappedShape<2> > TIndexSpec;
    typedef Index<TString, TIndexSpec> TIndex;

    TIndex myIndex(genome);

/// Now we output how often $"be"$ occurs in each sequence.
    std::cout << "Number of sequences: " << countSequences(myIndex) << std::endl;
    {
        TString pattern = "be";
        hash(indexShape(myIndex), begin(genome));
        auto count = countOccurrences(myIndex, indexShape(myIndex));
        std::cout << count << " occurrences in sequence " << 0  << std::endl;
    }
    {
        hash(indexShape(myIndex), "to");
        auto count = countOccurrences(myIndex, indexShape(myIndex));
        std::cout << count << " occurrences in sequence " << 0  << std::endl;
    }
    {
        hash(indexShape(myIndex), "ot");
        auto count = countOccurrences(myIndex, indexShape(myIndex));
        std::cout << count << " occurrences in sequence " << 0  << std::endl;
    }
}

template <typename TIndex, typename TStringSet, typename TOutputStream>
inline void kmer_counting (TIndex & index, TStringSet & patterns, TOutputStream & cout)
{
    unsigned i = 0;
    for (auto & pattern: patterns)
    {
        hash(indexShape(index), begin(pattern));
        auto count = countOccurrences(index, indexShape(index));
        cout << i++ << ": " <<  count << std::endl;
    }
}

int main(int argc, char **argv)
{
    example1();
    example2();

    typedef IndexQGram<UngappedShape<4> > TIndexSpec;
    typedef Index<Dna5String, TIndexSpec> TIndex;

    Dna5String genome = "GACCGCAGCGATCGGACCTAACGAGAGA";
    TIndex index(genome);

    StringSet<Dna5String> patterns;
    resize(patterns, 4);
    patterns[0] = "GACC";
    patterns[1] = "GAGA";
    patterns[2] = "AGCG";
    patterns[3] = "CGCT";

    kmer_counting(index, patterns, std::cout);

    return 0;
}
