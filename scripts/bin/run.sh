#!/bin/bash

#PBS -l mem=70gb
#PBS -l nodes=1:ppn=24:Intel.X5650
#PBS -l walltime=2:00:00
#PBS -d /data/scratch/NO_BACKUP/esiragusa/backtracking
#PBS -N backtracking
#PBS -j oe
#PBS -q batch

SELECT_JOBID=${PBS_ARRAYID}
umask 002

NETDIR=/data/scratch/NO_BACKUP/esiragusa/backtracking
TMPDIR=/data/scratch.local/$USER/backtracking
BINDIR=$NETDIR/bin
RUNDIR=$NETDIR/run
EXE=exec.sh
BIN=backtracking

RESDIR=resources
RESEXT=tsv

LOGDIR=logs
LOGEXT=txt

mkdir -p $TMPDIR
mkdir -p $RESDIR
mkdir -p $LOGDIR

JOBID=0

for ALGO in single dfs # bfs
do
    for DISTANCE in hamming # edit
    do
        for ORGA in ecoli celegans dmel hg18
        do
            GENOMEFILE="$ORGA.fasta"
            INDEXFILE="$ORGA.masai_sa"
            READSFILE="$ORGA.fastq"

            for SEEDLEN in 15 30
            do
                for SEEDERR in 0 1 2
                do
                    for SEEDCNT in 10000 100000 1000000 10000000
                    do
                        ((JOBID++))

                        if [ -n "${SELECT_JOBID}" ]
                        then
                            if [ $JOBID != $SELECT_JOBID ]
                            then
                                continue
                            fi
                        fi

                        rsync -aL $RUNDIR/* $BINDIR/$BIN $NETDIR/references/$GENOMEFILE $NETDIR/indices/$INDEXFILE.* $NETDIR/reads/$READSFILE $TMPDIR/

                        RESFILE=$RESDIR/$ALGO.$DISTANCE.$ORGA.$SEEDLEN.$SEEDERR.$SEEDCNT.$RESEXT
                        LOGFILE=$LOGDIR/$ALGO.$DISTANCE.$ORGA.$SEEDLEN.$SEEDERR.$SEEDCNT.$LOGEXT

                        rm -f $RESFILE
                        rm -f $LOGFILE

                        hostname >> $LOGFILE
                        date >> $LOGFILE
                        env >> $LOGFILE
                        df >> $LOGFILE
                        echo >> $LOGFILE

                        ARGS=""
                        if [ "${DISTANCE}" == "hamming" ]
                        then
                            ARGS+=" --no-gaps "
                        fi

                        CMD="$TMPDIR/$BIN --algorithm $ALGO $ARGS --seed-length $SEEDLEN --seed-errors $SEEDERR --seed-count $SEEDCNT -xp $TMPDIR/$INDEXFILE $TMPDIR/$GENOMEFILE $TMPDIR/$READSFILE"

                        echo "${PBS_JOBID}:${PBS_ARRAYID}"
                        echo $CMD
                        echo "$CMD" >> $LOGFILE
                        RESOURCES=$RESFILE EXEC_COLNAMES="\talgo\tdistance\torga\tseedlen\tseederr\tseedcnt\thits\ttotaltime" EXEC_VALUES="\t$ALGO\t$DISTANCE\t$ORGA\t$SEEDLEN\t$SEEDERR\t$SEEDCNT\tHITS\tTIME" $TMPDIR/$EXE $CMD >> $LOGFILE 2>&1

                        HITS=$(grep Hits $LOGFILE | tail -1 | cut -f 5)
                        TIME=$(grep Backtracking $LOGFILE |tail -1|cut -f 3|cut -f 1 -d " ")
                        sed -i "s/HITS/${HITS}/g" $RESFILE
                        sed -i "s/TIME/${TIME}/g" $RESFILE
                    done
                done
            done
        done
    done
done

if [ -n "${SELECT_JOBID}" ]
then
	rm -rf $TMPDIR
fi
