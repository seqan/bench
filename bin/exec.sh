#!/bin/bash

#
# ./exec.sh CMD ARG1 ARG2 ...
#
# This script executes a command, measures its resource consumption, and
# appends a line to the tsv file ${RESOURCES} (default: resources.tsv).
# Additional columns and values can be added via the variables EXEC_COLNAMES
# and EXEC_VALUES.
#

TIME_PID=0
PID=0
control_c()
{
    if [ ${PID} != 0 ]; then
        kill ${PID}
    fi
    if [ ${TIME_PID} != 0 ]; then
        kill ${TIME_PID}
    fi
    exit $?
}

# trap keyboard interrupt (control-c)
trap control_c SIGINT

# create new resource file if absent
if [ -z "${RESOURCES}" ]; then
    RESOURCES=resources.tsv
fi

# remove existing resource file
if [[ "${EXEC_MODE}" == **new_file** ]]; then
	rm -f ${RESOURCES}
fi

if [ ! -f ${RESOURCES} ] && [[ "${EXEC_MODE}" != **no_header** ]]; then
    echo -e "date\ttime\texec\tstatus\twc_time\tcpu_time\tsys_time\tperc_cpu\tres_mem_peak\tvirt_mem_peak\trss_mem_peak${EXEC_COLNAMES}" > ${RESOURCES}
fi

# time stamp
date +"%Y%m%d%t%H%M%S%t" >> ${RESOURCES}

# executable basename
echo -en "$(basename $1)\t" >> ${RESOURCES}

# running time
/usr/bin/time --quiet -f "%x\t%e\t%U\t%S\t%P\t%M\t" -ao ${RESOURCES} $@ &
TIME_PID=$!

# get process id of time's child process
sleep 0.5
PID=$(ps --ppid ${TIME_PID} -o pid= || echo 0)

# Watch memory consumption.  If the variable WATCH_NAME is set then we use
# it instead of the PID of the program we just executed.
if [ -z "${WATCH_NAME}" ];
then
    MEM=$($(dirname $0)/watch-memory.py ${PID})
else
    MEM=$($(dirname $0)/watch-memory.py ${WATCH_NAME})
fi
wait ${TIME_PID}
echo -e "${MEM}${EXEC_VALUES}" >> ${RESOURCES}

# join lines
sed -i ':a;N;$!ba;s/\t\n/\t/g' ${RESOURCES}

if [ -f ${RESOURCES} ] && [ -f ${RESOURCES}_ ]; then
    rm ${RESOURCES}_
fi
