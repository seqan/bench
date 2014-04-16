#!/usr/bin/python
"""Memory watchdog scripts.

USAGE: watch-memory.py PID
       watch-memory.py NAME

You can either give a PID to watch out for or a process name (as returned
by "ps -e").  The program will use "ps -e" to retrieve the PID if necessary.
Then it will look into the file /proc/${PID}/status and retrieve the
information about the memory usage.

When there is no PID left for the program name or the process with the
given PID is no longer there, the program will quit (also if the user stops
the program with a keyboard interrupt).

Finally, the largest "VMPeak" and "VMRSS" values seen so far are printed as
tab delimited values.
"""

import argparse
import sys
import time
import subprocess
import traceback

def peekPid(pid):
    """Look into /proc status file for the given pid.

    Return current VMPeak and VmRSS values as pair of integers.
    """
    with open('/proc/' + str(pid) + '/status', 'r') as status:
        lines = status.readlines()
        newVmPeak = lines[10].split()[1]
        newVmRSS = lines[14].split()[1]
        return int(newVmPeak), int(newVmRSS)


def watchPid(pid):
    """Watch a given PID until it tops or a keyboard interrupt."""
    maxVmPeak = 0
    maxVmRSS = 0
    while True:
        try:
            newVmPeak, newVmRSS = peekPid(pid)
            maxVmPeak = max(maxVmPeak, newVmPeak)
            maxVmRSS = max(maxVmRSS, newVmRSS)
            #print 'WATCHING\t%d\t%d' % (maxVmPeak, maxVmRSS)
            time.sleep(5)
        except (IOError, KeyboardInterrupt):
            break
        except Exception, e:
            print 'UNEXPECTED EXCEPTION: %s' % e
            traceback.print_exc(file=sys.stdout)
            break
    print "\t".join([str(maxVmPeak),str(maxVmRSS)])


def watchName(name):
    """Watch a process with a given name."""
    maxVmPeak = 0
    maxVmRSS = 0
    while True:
        pids = []
        try:
            # Get pids of program.
            handle = subprocess.Popen(['ps', '-e'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            handle.wait()
            stdout = handle.communicate()[0]
            for line in stdout.splitlines():
                arr = line.split()
                if len(arr) != 4:
                  continue
                pid, tty, time_, cmd = arr
                if cmd == name:
                  pids.append(int(pid))
            if not pids:
                break  # No more running programs.
            # Now, read data from pids.
            for pid in pids:
                newVmPeak, newVmRSS = peekPid(pid)
                maxVmPeak = max(maxVmPeak, newVmPeak)
                maxVmRSS = max(maxVmRSS, newVmRSS)
                time.sleep(5)
        except (IOError, KeyboardInterrupt):
            break
        except Exception, e:
            print 'UNEXPECTED EXCEPTION: %s' % e
            traceback.print_exc(file=sys.stdout)
            break
    print "\t".join([str(maxVmPeak),str(maxVmRSS)])


def main():
  parser = argparse.ArgumentParser(description='Watch memory usage of a process.')
  parser.add_argument('pid', metavar='PID_OR_NAME', type=str,
                      help='A PID or program name to watch for.')
  args = parser.parse_args()

  # Check whether this is a PID or a name.
  try:
      pid = int(args.pid)
      watchPid(pid)
  except ValueError:
      watchName(args.pid)


if __name__ == '__main__':
    main()
