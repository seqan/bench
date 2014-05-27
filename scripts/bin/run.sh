#!/bin/bash

function vars_dna_common
{
    # text
    TEXT_COUNT_BIT=8
    TEXT_SUM_BIT=32
    TEXT_LENGTH_BIT=32

    # pattern
    PATTERN_COUNT_BIT=32
    PATTERN_SUM_BIT=32
    PATTERN_LENGTH_BIT=8
    PATTERN_LENGTHS=$(seq 5 5 50)
    PATTERN_COUNT=1000000

    # index
    INDEX_TYPE="sa esa qgram fm-tl fm-wt"

    # visit
    VISIT_DEPTH=$(seq 1 20) #30

    # query
#    QUERY_LOCATE=15
    QUERY_ERRORS="0 1" #2

    # multi-query
    MULTI_COUNTS="10000 100000 1000000 10000000"
    MULTI_LENGTHS="15 30"
}

function vars_dna_ecoli
{
    SRC=~/Datasets/ecoli

    vars_dna_common

    # text
    TEXT_INPUT="genome.fasta"
    TEXT_NAME=ecoli.txt

    # pattern
    PATTERN_INPUT="realworld_ecoli_illumina_ERR022075_10M_1.fastq"
    PATTERN_NAME=ecoli.pat #.$PATTERN_LENGTHS[i]

    # index
    INDEX_NAME=ecoli #.$INDEX_TYPE
}

function vars_dna_celegans
{
    SRC=~/Datasets/celegans

    vars_dna_common

    # text
    TEXT_INPUT="celegans.fasta"
    TEXT_NAME=celegans.txt

    # pattern
    PATTERN_INPUT="1M_1.fastq.gz"
    PATTERN_NAME=celegans.pat #.$PATTERN_LENGTHS[i]

    # index
    INDEX_NAME=celegans #.$INDEX_TYPE
}

function vars_protein_uniprot
{
    SRC=~/Datasets/uniprot

    # text
    TEXT_INPUT="uniprot_sprot.fasta.gz"
    TEXT_NAME=sprot.txt
    TEXT_COUNT_BIT=32
    TEXT_SUM_BIT=32
    TEXT_LENGTH_BIT=16

    # pattern
    PATTERN_INPUT="HUMAN.fasta.gz"
    PATTERN_NAME=sprot.pat #.$PATTERN_LENGTHS[i]
    PATTERN_COUNT_BIT=32
    PATTERN_SUM_BIT=32
    PATTERN_LENGTH_BIT=8
    PATTERN_LENGTHS=$(seq 5 5 30)
    PATTERN_COUNT=1000000

    # index
    INDEX_NAME=sprot #.$INDEX_TYPE
    INDEX_TYPE="sa esa qgram fm-wt"

    # visit
    VISIT_DEPTH=$(seq 1 10)

    # query
#    QUERY_LOCATE=15
    QUERY_ERRORS="0 1"
}

# ======================================================================================================================

# cmd_prepare input output alphabet count sum length [plength] [pcount]
function cmd_prepare
{
    CMD="$BIN/ibench_dump $1 $2 -a $3 -tc $4 -ts $5 -tl $6"
    if [ $# -ge 7 ]
    then
        CMD+=" -ll $7"
    fi
    if [ $# -ge 8 ]
    then
        CMD+=" -lc $8"
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

# ======================================================================================================================

# prepare text
function exec_prepare_text
{
    cmd_prepare $SRC/$TEXT_INPUT $DIR/$TEXT_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT
    echo $CMD
    $CMD
}

# construct text index
function exec_construct_text
{
    if [[ ! -e $DIR/construct.tsv ]]; then
        echo -e "alphabet\tindex\tsymbols\ttime" > $DIR/construct.tsv
    fi
    for index_type in $INDEX_TYPE;
    do
        cmd_construct $DIR/$TEXT_NAME $DIR/$INDEX_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT $index_type
        echo $CMD
        output=$($CMD)
        if [ $? -eq 0 ]
        then
            echo -e "$ALPHABET\t$index_type\t$output" >> $DIR/construct.tsv
        fi
    done
}

# visit text index
function exec_visit_text
{
    if [[ ! -e $DIR/visit.tsv ]]; then
        echo -e "alphabet\tindex\tdepth\tnodes\ttime" > $DIR/visit.tsv
    fi
    for index_type in $INDEX_TYPE;
    do
        for depth in $VISIT_DEPTH;
        do
            cmd_visit $depth $DIR/$INDEX_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT $index_type
            echo $CMD
            output=$($CMD)
            if [ $? -eq 0 ]
            then
                echo -e "$ALPHABET\t$index_type\t$depth\t$output" >> $DIR/visit.tsv
            fi
        done
    done
}

# prepare patterns
function exec_prepare_patterns
{
    for pattern_length in $PATTERN_LENGTHS;
    do
        cmd_prepare $SRC/$PATTERN_INPUT $DIR/$PATTERN_NAME.$pattern_length $ALPHABET $PATTERN_COUNT_BIT $PATTERN_SUM_BIT $PATTERN_LENGTH_BIT $pattern_length $PATTERN_COUNT
        echo $CMD
        $CMD
    done
}

# query patterns
function exec_query
{
    if [[ ! -e $DIR/query.tsv ]]; then
        echo -e "alphabet\tindex\terrors\tplength\toccurrences\ttime\tpreprocessing" > $DIR/query.tsv
    fi
    for index_type in $INDEX_TYPE;
    do
        for errors in $QUERY_ERRORS;
        do
            for pattern_length in $PATTERN_LENGTHS;
            do
                cmd_query $DIR/$INDEX_NAME $DIR/$PATTERN_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT $index_type $pattern_length $errors single
                echo $CMD
                output=$($CMD)
                if [ $? -eq 0 ]
                then
                    echo -e "$ALPHABET\t$index_type\t$errors\t$pattern_length\t$output" >> $DIR/query.tsv
                fi
            done
        done
    done
}

# prepare multi patterns
function exec_prepare_patterns_multi
{
    for multi_count in $MULTI_COUNTS;
    do
        for multi_length in $MULTI_LENGTHS;
        do
            cmd_prepare $SRC/$PATTERN_INPUT $DIR/$PATTERN_NAME.$multi_length.$multi_count $ALPHABET $PATTERN_COUNT_BIT $PATTERN_SUM_BIT $PATTERN_LENGTH_BIT $multi_length $multi_count
            echo $CMD
            $CMD
        done
    done
}

# multi-query patterns
function exec_query_multi
{
    if [[ ! -e $DIR/query.tsv ]]; then
        echo -e "alphabet\tindex\terrors\tplength\tpcount\talgorithm\toccurrences\ttime\tpreprocessing" > $DIR/multi.tsv
    fi
    for index_type in $INDEX_TYPE;
    do
        for errors in $QUERY_ERRORS;
        do
            for multi_count in $MULTI_COUNTS;
            do
#                for multi_length in $MULTI_LENGTHS;
#                do
                    multi_lengths=($MULTI_LENGTHS)
                    multi_length=${multi_lengths[$errors]}

                    for algo in single sort dfs;
                    do
                        cmd_query $DIR/$INDEX_NAME $DIR/$PATTERN_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT $index_type $multi_length.$multi_count $errors $algo
                        echo $CMD
#                        output=$($CMD)
#                        if [ $? -eq 0 ]
#                        then
#                            echo -e "$ALPHABET\t$index_type\t$errors\t$multi_length\t$multi_count\t$algo\t$output" >> $DIR/multi.tsv
#                        fi
                    done
#                done
            done
        done
    done
}

# ======================================================================================================================

if [ ! $# -eq 2 ]
then
    exit
fi

#SRC=~/Code/seqan/core/apps/ibench/scripts/resources
BIN=~/Code/seqan-builds/Release-Gcc/bin
DIR=~/Datasets/ibench
ALPHABET=$1
DATASET=$2

# configure vars
vars_$ALPHABET\_$DATASET

# ======================================================================================================================

#exec_prepare_text
#exec_construct_text
#exec_visit_text
#exec_prepare_patterns
#exec_query
#exec_prepare_patterns_multi
exec_query_multi

