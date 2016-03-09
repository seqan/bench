import os
import platform
import sys
import zipfile
import argparse
import shutil

parser = argparse.ArgumentParser(description='In')
parser.add_argument('-n', help='path to nw.js directory')
parser.add_argument('-s', help='path to SeqAn build directory')
args = parser.parse_args()

_nwjsDir = ''
_seqDir = ''
_exeDir = ''
nwFile = 'nw'
localesDir = 'locales'
icudtlFile = 'icudtl.dat'
benchBuildDir = 'apps/bench/integrated_interface/src/'
binDir = 'bin/BenchMark_interface_' + platform.system()
App = 'app'


if args.n is None:
    print 'Please specify path to nw.js'
    #sys.exit()
else:
    if os.path.isfile(os.path.join(args.n, nwFile)) \
       and os.path.isdir(os.path.join(args.n, localesDir)) \
       and os.path.isfile(os.path.join(args.n, icudtlFile)):
        _nwjsDir = args.n      
    else:
        print 'can\'t find nw.js'
        sys.exit()
if args.s is None:
    print 'Please specify path to SeqAn build directory'
    sys.exit()
else:
    if os.path.isdir(os.path.join(args.s, benchBuildDir)):
        _exeDir = os.path.join(args.s, binDir)
        _seqBuildDir = args.s
    else:
        print 'can\'t find SeqAn Bench build directory'


if platform.system() == "Windows":
    print 1
elif platform.system() == "Linux":
    #if (not os.path.exists(_exeDir)): 
    #    os.mkdir(_exeDir)
    shutil.copytree(os.path.join(sys.path[0], 'src'), _exeDir)
    #compile benchmark programs
    _tmpCmd = ' '.join(['make -C', os.path.join(_seqBuildDir, benchBuildDir), 'all'])
    os.system(_tmpCmd)
    shutil.copytree
    os.remove('')
    #package source
    def zipdir(path, ziph):
        for root, dirs, files in os.walk(path):
            for file in files:
                abs_path = os.path.join(os.path.join(root, file))
                rel_path = os.path.relpath(abs_path, path)
                ziph.write(os.path.join(root, file),rel_path)
               
    if __name__ == '__main__':
        _zipSrc = os.path.join(sys.path[0], 'src')
        _zipFile = os.path.abspath(os.path.join(_exeDir, 'app.nw'))        
        zipf = zipfile.ZipFile(_zipFile, 'w', zipfile.ZIP_DEFLATED)
        zipdir(_zipSrc, zipf)
        zipf.close()
 
    _tmpCmd = ' '.join(['cat', os.path.join(_nwjsDir, 'nw'), 
                       _zipFile, '>', os.path.join(_exeDir, App)])
    os.system(_tmpCmd)
    _tmpCmd = ' '.join(['chmod a+x', os.path.join(_exeDir, App)])
    os.system(_tmpCmd)
    os.remove(os.path.join(_exeDir, 'app.nw'))
else:
    print 3


 
