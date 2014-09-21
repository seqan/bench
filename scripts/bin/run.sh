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

    # index
    INDEX_TYPE="sa esa qgram fm-tl fm-wt"

    # visit
    VISIT_DEPTH=$(seq 1 20) #30

    # query
    PATTERN_LENGTHS=$(seq 5 5 50)
    PATTERN_COUNT=1000000
    QUERY_ERRORS=$(seq 0 1) #2

    # multi-query
    MULTI_LENGTHS="15 30"
    MULTI_COUNTS="10000 100000 1000000 10000000"

    # pattern filter
    FILTER_LENGTHS="100"
    FILTER_COUNTS="100000"
    FILTER_ERRORS=$(seq 3 6)
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
    PATTERN_INPUT="1M_1.fastq"
#    PATTERN_INPUT="SRR065390_10M_1.fastq"
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

    # index
    INDEX_NAME=sprot #.$INDEX_TYPE
    INDEX_TYPE="sa qgram fm-wt" #esa

    # visit
    VISIT_DEPTH=$(seq 1 10)

    # query
    PATTERN_LENGTHS=$(seq 5 5 30)
    PATTERN_COUNT=1000000
    QUERY_ERRORS=$(seq 0 1)

    # multi-query
    MULTI_LENGTHS="10 20"
    MULTI_COUNTS="10000 100000 1000000"
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

# cmd_filter_seeds text pattern alphabet count sum length index plength errors distance verify rdup seederr
function cmd_filter_seeds
{
    CMD="$BIN/ibench_filter --tsv $1.$7 $2.$8 -a $3 -tc $4 -ts $5 -tl $6 -i $7 -e $9 -g seeds -se ${13}"
    if [[ ${10} = 'edit' ]]; then
        CMD+=" -ed"
    fi
    if [[ ${11} = 'true' ]]; then
        CMD+=" -vy"
    fi
    if [[ ${12} = 'true' ]]; then
        CMD+=" -rd"
    fi
}

# cmd_filter_qgrams text pattern alphabet count sum length NONE plength errors distance verify rdup weight threshold
function cmd_filter_qgrams
{
    CMD="$BIN/ibench_filter --tsv $1 $2.$8 -a $3 -tc $4 -ts $5 -tl $6 -e $9 -g qgrams -qw ${13} -qt ${14}" # -se ${13} -so
    if [[ ${10} = 'edit' ]]; then
        CMD+=" -ed"
    fi
    if [[ ${11} = 'true' ]]; then
        CMD+=" -vy"
    fi
    if [[ ${12} = 'true' ]]; then
        CMD+=" -rd"
    fi
}

# ======================================================================================================================

# param_filter_seeds plength errors distance
function param_filter_seeds
{
    plength=$1
    errors=$2
    distance=$3

    SEEDS_ERRORS="0"
    if [[ $distance = 'hamming' ]]; then
        if [[ $errors -ge 2 ]]; then
            SEEDS_ERRORS+=" 1"
        fi
        if [[ $errors -ge 5 ]]; then
            SEEDS_ERRORS+=" 2"
        fi
    fi
}

# param_filter_qgrams plength errors distance
function param_filter_qgrams
{
    plength=$1
    errors=$2
    distance=$3

    qgrams_weight=$(python -c "import math; print min(int(math.ceil((${plength})/(${errors}+1))),31)")
    qgrams_threshold=$(python -c "import math; print ${plength} - (${errors}+1) * ${qgrams_weight} + 1")
    QGRAMS_WEIGHT=$qgrams_weight
    QGRAMS_THRESHOLD=$qgrams_threshold

    if [[ $QGRAMS_THRESHOLD -le 2 ]]; then
        qgrams_weight=$(python -c "import math; print min(int(math.ceil((${plength} - 1)/(${errors}+1))),31)")
        qgrams_threshold=$(python -c "import math; print ${plength} - (${errors}+1) * ${qgrams_weight} + 1")
        QGRAMS_WEIGHT+=" ${qgrams_weight}"
        QGRAMS_THRESHOLD+=" ${qgrams_threshold}"
    fi

    if [[ $qgrams_threshold -le 4 ]]; then
        qgrams_weight=$(python -c "import math; print min(int(math.ceil((${plength} - 3)/(${errors}+1))),31)")
        qgrams_threshold=$(python -c "import math; print ${plength} - (${errors}+1) * ${qgrams_weight} + 1")
        QGRAMS_WEIGHT+=" ${qgrams_weight}"
        QGRAMS_THRESHOLD+=" ${qgrams_threshold}"
    fi
}

# ======================================================================================================================

# exec_prepare_text
function exec_prepare_text
{
    cmd_prepare $SRC/$TEXT_INPUT $DIR/$TEXT_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT
    echo $CMD
    $CMD
}

# exec_prepare_patterns lengths counts
function exec_prepare_patterns
{
    for patterns_count in $2;
    do
        for patterns_length in $1;
        do
            cmd_prepare $SRC/$PATTERN_INPUT $DIR/$PATTERN_NAME.$patterns_length.$patterns_count $ALPHABET $PATTERN_COUNT_BIT $PATTERN_SUM_BIT $PATTERN_LENGTH_BIT $patterns_length $patterns_count
            echo $CMD
            $CMD
        done
    done
}

# exec_construct_text filename.tsv
function exec_construct_text
{
    filename=$1

    if [[ ! -e $filename ]]; then
        echo -e "alphabet\tdataset\tindex\tsymbols\ttime" > $filename
    fi
    for index_type in $INDEX_TYPE;
    do
        cmd_construct $DIR/$TEXT_NAME $DIR/$INDEX_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT $index_type
        echo $CMD
        output=$($CMD)
        if [ $? -eq 0 ]; then
            echo -e "$ALPHABET\t$DATASET\t$index_type\t$output" >> $filename
        fi
    done
}

# exec_visit_text filename.tsv
function exec_visit_text
{
    filename=$1

    if [[ ! -e $filename ]]; then
        echo -e "alphabet\tdataset\tindex\tdepth\tnodes\ttime" > $filename
    fi
    for index_type in $INDEX_TYPE;
    do
        for depth in $VISIT_DEPTH;
        do
            cmd_visit $depth $DIR/$INDEX_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT $index_type
            echo $CMD
            output=$($CMD)
            if [ $? -eq 0 ]; then
                echo -e "$ALPHABET\t$DATASET\t$index_type\t$depth\t$output" >> $filename
            fi
        done
    done
}

# exec_query filename.tsv
function exec_query
{
    filename=$1

    if [[ ! -e $filename ]]; then
        echo -e "alphabet\tdataset\tindex\terrors\tplength\toccurrences\ttime\tpreprocessing" > $filename
    fi
    for index_type in $INDEX_TYPE;
    do
        for errors in $QUERY_ERRORS;
        do
            for pattern_length in $PATTERN_LENGTHS;
            do
                cmd_query $DIR/$INDEX_NAME $DIR/$PATTERN_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT $index_type $pattern_length.$PATTERN_COUNT $errors single
                echo $CMD
                output=$($CMD)
                if [ $? -eq 0 ]; then
                    echo -e "$ALPHABET\t$DATASET\t$index_type\t$errors\t$pattern_length\t$output" >> $filename
                fi
            done
        done
    done
}

# exec_query_multi filename.tsv
function exec_query_multi
{
    filename=$1

    if [[ ! -e $filename ]]; then
        echo -e "alphabet\tdataset\tindex\terrors\tplength\tpcount\talgorithm\toccurrences\ttime\tpreprocessing" > $filename
    fi
    for index_type in $INDEX_TYPE;
    do
        for errors in $QUERY_ERRORS;
        do
            multi_lengths=($MULTI_LENGTHS)
            for multi_count in $MULTI_COUNTS;
            do
                multi_length=${multi_lengths[$errors]}

                for algo in single sort dfs bfs;
                do
                    cmd_query $DIR/$INDEX_NAME $DIR/$PATTERN_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT $index_type $multi_length.$multi_count $errors $algo
                    echo $CMD
                    output=$($CMD)
                    if [ $? -eq 0 ]; then
                        echo -e "$ALPHABET\t$DATASET\t$index_type\t$errors\t$multi_length\t$multi_count\t$algo\t$output" >> $filename
                    fi
                done
            done
        done
    done
}

# exec_filter_seeds filename.tsv distance verify remove-duplicates
function exec_filter_seeds
{
    filename=$1
    distance=$2
    verify=$3
    rdup=$4
    seeds_errors=$5
    patterns_length=$FILTER_LENGTHS
    index_type='qgram'

    if [[ ! -e $filename ]]; then
        echo -e "alphabet\tdataset\tpcount\tplength\terrors\tdistance\tfilter\tverifications\tduplicates\toccurrences\ttime" > $filename
    fi

    for errors in $FILTER_ERRORS;
    do
        for patterns_count in $FILTER_COUNTS;
        do
            param_filter_seeds $patterns_length $errors $distance
            for seeds_errors in $SEEDS_ERRORS;
            do
                cmd_filter_seeds $DIR/$INDEX_NAME $DIR/$PATTERN_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT $index_type $patterns_length.$patterns_count $errors $distance $verify $rdup $seeds_errors
                filter_name="seeds_${seeds_errors}"

                echo $CMD
                output=$($CMD)
                if [ $? -eq 0 ]; then
                    echo -e "${ALPHABET}\t${DATASET}\t${patterns_count}\t${patterns_length}\t${errors}\t${distance}\t${filter_name}\t${output}" >> $filename
                fi
            done
        done
    done
}

# exec_filter_qgrams filename.tsv distance verify remove-duplicates
function exec_filter_qgrams
{
    filename=$1
    distance=$2
    verify=$3
    rdup=$4
    patterns_length=$FILTER_LENGTHS

    if [[ ! -e $filename ]]; then
        echo -e "alphabet\tdataset\tpcount\tplength\terrors\tdistance\tfilter\tverifications\tduplicates\toccurrences\ttime" > $filename
    fi

    for errors in $FILTER_ERRORS;
    do
        for patterns_count in $FILTER_COUNTS;
        do
            param_filter_qgrams $patterns_length $errors $distance
            qgrams_thresholds=($QGRAMS_THRESHOLD)
            param_idx=0
            for qgrams_weight in $QGRAMS_WEIGHT;
            do
                qgrams_threshold=${qgrams_thresholds[$param_idx]}
                param_idx+=1

                cmd_filter_qgrams $DIR/$TEXT_NAME $DIR/$PATTERN_NAME $ALPHABET $TEXT_COUNT_BIT $TEXT_SUM_BIT $TEXT_LENGTH_BIT 'NONE' $patterns_length.$patterns_count $errors $distance $verify $rdup $qgrams_weight $qgrams_threshold
                filter_name="qgrams_${param_idx}"

                echo $CMD
                output=$($CMD)
                if [ $? -eq 0 ]; then
                    echo -e "${ALPHABET}\t${DATASET}\t${patterns_count}\t${patterns_length}\t${errors}\t${distance}\t${filter_name}\t${output}" >> $filename
                fi
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
#exec_construct_text $DIR/construct.tsv
#exec_visit_text $DIR/visit.tsv
#exec_prepare_patterns $PATTERN_LENGTHS $PATTERN_COUNT
#exec_query $DIR/query.tsv
#exec_prepare_patterns $MULTI_LENGTHS $MULTI_COUNTS
#exec_query_multi $DIR/multi.tsv

#exec_prepare_patterns $FILTER_LENGTHS $FILTER_COUNTS
#exec_filter_seeds $DIR/filter_occurrences.tsv hamming true true
#exec_filter_qgrams $DIR/filter_occurrences.tsv hamming true true
exec_filter_seeds $DIR/filter_occurrences.tsv edit true true
exec_filter_qgrams $DIR/filter_occurrences.tsv edit true true

#exec_filter_seeds $DIR/filter_verify.tsv hamming true false
#exec_filter_qgrams $DIR/filter_verify.tsv hamming true false
#exec_filter_seeds $DIR/filter_only.tsv hamming false false
#exec_filter_qgrams $DIR/filter_only.tsv hamming false false
