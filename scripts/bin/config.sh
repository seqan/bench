
function vars_dna
{
    SRC=~/Datasets/ecoli

    # text
    TEXT_INPUT="genome.fasta"
    TEXT_NAME=ecoli.txt
    TEXT_COUNT=8
    TEXT_SUM=32
    TEXT_LENGTH=32

    # pattern
    PATTERN_INPUT="ecoli_1M.fastq"
    PATTERN_NAME=ecoli.pat #.$PATTERN_LENGTHS[i]
    PATTERN_COUNT=32
    PATTERN_SUM=32
    PATTERN_LENGTH=8
    PATTERN_LENGTHS="5 10 15 20"
#    PATTERN_COUNTS="10000 100000 1000000"

    # index
    INDEX_NAME=ecoli #.$INDEX_TYPE
    INDEX_TYPE="sa esa fm-tl fm-wt"

    # visit
    VISIT_DEPTH=$(seq 1 30)

    # query
#    QUERY_LOCATE=15
    QUERY_ERRORS="0 1"
    QUERY_ALGORITHM="single dfs"
}

function vars_dna_celegans
{
    SRC=~/Datasets/celegans

    # text
    TEXT_INPUT="celegans.fasta"
    TEXT_NAME=celegans.txt
    TEXT_COUNT=8
    TEXT_SUM=32
    TEXT_LENGTH=32

    # pattern
    PATTERN_INPUT="1M_1.fastq.gz"
    PATTERN_NAME=celegans.pat #.$PATTERN_LENGTHS[i]
    PATTERN_COUNT=32
    PATTERN_SUM=32
    PATTERN_LENGTH=8
    PATTERN_LENGTHS="5 10 15 20 25 30 40 50 60"
#    PATTERN_COUNTS="10000 100000 1000000"

    # index
    INDEX_NAME=celegans #.$INDEX_TYPE
    INDEX_TYPE="sa esa fm-tl fm-wt"

    # visit
    VISIT_DEPTH=$(seq 1 30)

    # query
#    QUERY_LOCATE=15
    QUERY_ERRORS="0 1"
    QUERY_ALGORITHM="single dfs"
}

function vars_protein
{
    SRC=~/Datasets/uniprot

    # text
    TEXT_INPUT="uniprot_sprot.fasta.gz"
    TEXT_NAME=sprot.txt
    TEXT_COUNT=32
    TEXT_SUM=32
    TEXT_LENGTH=16

    # pattern
    PATTERN_INPUT="HUMAN.fasta.gz"
    PATTERN_NAME=sprot.pat #.$PATTERN_LENGTHS[i]
    PATTERN_COUNT=32
    PATTERN_SUM=32
    PATTERN_LENGTH=8
    PATTERN_LENGTHS="5 10 15 20 25 30"
#    PATTERN_COUNTS="10000 100000 1000000"

    # index
    INDEX_NAME=sprot #.$INDEX_TYPE
    INDEX_TYPE="sa esa fm-wt"

    # visit
    VISIT_DEPTH=$(seq 1 10)

    # query
#    QUERY_LOCATE=15
    QUERY_ERRORS="0 1"
    QUERY_ALGORITHM="single dfs"
}


# cmd_prepare input output alphabet count sum length [plength]
function cmd_prepare
{
    CMD="$BIN/ibench_dump $1 $2 -a $3 -tc $4 -ts $5 -tl $6"
    if [ $# -eq 7 ]
    then
        CMD+=" -pl $7"
    fi
}

# cmd_construct input output alphabet count sum length index
function cmd_construct
{
    CMD="$BIN/ibench_construct --tsv $1 $2.$7 -a $3 -tc $4 -ts $5 -tl $6 -i $7"
}

# cmd_visit depth input alphabet count sum length index
function cmd_visit
{
    CMD="$BIN/ibench_visit --tsv $2.$7 -a $3 -tc $4 -ts $5 -tl $6 -i $7 -d $1"
}

# cmd_query text pattern alphabet count sum length index plength errors algo
function cmd_query
{
    CMD="$BIN/ibench_query --tsv $1.$7 $2.$8 -a $3 -tc $4 -ts $5 -tl $6 -i $7 -e $9 -g ${10}"
}


BIN=~/Code/seqan-builds/Release-Clang/bin
DIR=~/Datasets/ibench
ALPHABET=$1

vars_$ALPHABET


# prepare text
cmd_prepare $SRC/$TEXT_INPUT $DIR/$TEXT_NAME $ALPHABET $TEXT_COUNT $TEXT_SUM $TEXT_LENGTH
echo $CMD
$CMD

# construct text index
echo -e "alphabet\tindex\ttime" > $DIR/construct.tsv
for index_type in $INDEX_TYPE;
do
    cmd_construct $DIR/$TEXT_NAME $DIR/$INDEX_NAME $ALPHABET $TEXT_COUNT $TEXT_SUM $TEXT_LENGTH $index_type
    echo $CMD
    output=$($CMD)
    if [ $? -eq 0 ]
    then
        echo -e "$ALPHABET\t$index_type\t$output" >> $DIR/construct.tsv
    fi
done

# visit text index
echo -e "alphabet\tindex\tdepth\tnodes\ttime" > $DIR/visit.tsv
for index_type in $INDEX_TYPE;
do
    for depth in $VISIT_DEPTH;
    do
        cmd_visit $depth $DIR/$INDEX_NAME $ALPHABET $TEXT_COUNT $TEXT_SUM $TEXT_LENGTH $index_type
        echo $CMD
        output=$($CMD)
        if [ $? -eq 0 ]
        then
            echo -e "$ALPHABET\t$index_type\t$depth\t$output" >> $DIR/visit.tsv
        fi
    done
done

# prepare patterns
for pattern_length in $PATTERN_LENGTHS;
do
    cmd_prepare $SRC/$PATTERN_INPUT $DIR/$PATTERN_NAME.$pattern_length $ALPHABET $PATTERN_COUNT $PATTERN_SUM $PATTERN_LENGTH $pattern_length
    echo $CMD
    $CMD
done

# query patterns
echo -e "alphabet\tindex\terrors\tplength\toccurrences\ttime" > $DIR/query.tsv
for index_type in $INDEX_TYPE;
do
    for errors in $QUERY_ERRORS;
    do
        for pattern_length in $PATTERN_LENGTHS;
        do
            cmd_query $DIR/$INDEX_NAME $DIR/$PATTERN_NAME $ALPHABET $TEXT_COUNT $TEXT_SUM $TEXT_LENGTH $index_type $pattern_length $errors single
            echo $CMD
            output=$($CMD)
            if [ $? -eq 0 ]
            then
                echo -e "$ALPHABET\t$index_type\t$errors\t$pattern_length\t$output" >> $DIR/query.tsv
            fi
        done
    done
done
