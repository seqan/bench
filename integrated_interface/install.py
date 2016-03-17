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
benchBuildDir = 'apps/bench/integrated_interface/'
binDir = 'bin/SeqanBench' + platform.system()
App = 'app'
if platform.system() == "Windows":
    print 1
if platform.system() == "Linux":
    if args.n is None:
        print 'Please specify path to nw.js'
        sys.exit()
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

        if (not os.path.exists(_exeDir)): 
            os.mkdir(_exeDir)
        else:
            print 'please remove ' + _exeDir 
        #shutil.rmtree(_exeDir)
    shutil.rmtree(_exeDir)
    shutil.copytree(os.path.join(sys.path[0],'config'), os.path.join(_exeDir,'config'))
    shutil.copytree(os.path.join(sys.path[0],'resources'), os.path.join(_exeDir,'resources'))
    shutil.copytree(os.path.join(sys.path[0], 'std_bench', 'data'), os.path.join(_exeDir, 'std_bench', 'data'))
    shutil.copytree(os.path.join(sys.path[0],'Info'), os.path.join(_exeDir, 'Info'))
    shutil.copyfile(os.path.join(sys.path[0],'package.json'), os.path.join(_exeDir, 'package.json'))

    #compile benchmark programs
    print "Compiling benchmark programs..."
    _tmpCmd = ' '.join(['make --silent -C', os.path.join(_seqBuildDir, benchBuildDir), 'all'])
    os.system(_tmpCmd)
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'global_alignment_dna'), os.path.join(_exeDir, 'std_bench'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'local_alignment_dna'), os.path.join(_exeDir, 'std_bench'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'global_alignment_protein'), os.path.join(_exeDir, 'std_bench'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'local_alignment_protein'), os.path.join(_exeDir, 'std_bench'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'index_create'), os.path.join(_exeDir, 'std_bench'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'index_approximate'), os.path.join(_exeDir, 'std_bench'))

    #package source
    def zipdir(path, ziph):
        for root, dirs, files in os.walk(path):
            for file in files:
                abs_path = os.path.join(os.path.join(root, file))
                rel_path = os.path.relpath(abs_path, path)
                ziph.write(os.path.join(root, file),rel_path)
               
    if __name__ == '__main__':
        _zipSrc = _exeDir
        _zipFile = os.path.abspath(os.path.join(_exeDir, 'app.nw'))        
        zipf = zipfile.ZipFile(_zipFile, 'w', zipfile.ZIP_DEFLATED)
        zipdir(_zipSrc, zipf)
        zipf.close()
    shutil.copytree(os.path.join(_nwjsDir, localesDir), os.path.join(_exeDir, localesDir))
    shutil.copyfile(os.path.join(_nwjsDir, icudtlFile), os.path.join(_exeDir, icudtlFile))
    shutil.copyfile(os.path.join(_nwjsDir, 'nw.pak'), os.path.join(_exeDir, 'nw.pak'))
    _tmpCmd = ' '.join(['cat', os.path.join(_nwjsDir, 'nw'), 
                       _zipFile, '>', os.path.join(_exeDir, App)])
    os.system(_tmpCmd)
    _tmpCmd = ' '.join(['chmod a+x', os.path.join(_exeDir, App)])
    os.system(_tmpCmd)
    print "Deleting files..."
    os.remove(os.path.join(_exeDir, 'package.json'))
    shutil.rmtree(os.path.join(_exeDir, 'Info'))
    shutil.rmtree(os.path.join(_exeDir, 'resources'))
    shutil.rmtree(os.path.join(_exeDir, 'config'))
    shutil.rmtree(os.path.join(_exeDir, 'std_bench'))
    print "Complete."
if platform.system() == "Darwin":
    if args.n is None:
        print 'Please specify path to nw.js'
        sys.exit()
    else:
        if os.path.isdir(os.path.join(args.n, 'nwjs.app')): 
            _nwjsDir = args.n      
        else:
            print 'can\'t find nwjs.app'
            sys.exit()
    if args.s is None:
        print 'Please specify path to SeqAn build directory'
        sys.exit()
    else:
        if os.path.isdir(os.path.join(args.s, benchBuildDir)):
            _exeDir = os.path.join(args.s, binDir + '.app')
            _seqBuildDir = args.s
        else:
            print 'can\'t find SeqAn Bench build directory'
        if (os.path.exists(_exeDir)): 
            shutil.rmtree(_exeDir) 
    print 'Compiling benchmarks'
    shutil.copytree(os.path.join(sys.path[0], 'resources', 'icons', 'osx', 'SeqanBench.app'), _exeDir)
    shutil.copytree(os.path.join(_nwjsDir,'nwjs.app', 'Contents'), os.path.join(_exeDir, 'Contents'))
    _appDir = os.path.join(_exeDir, 'Contents', 'Resources', 'app.nw')
    os.mkdir(_appDir)
    shutil.copytree(os.path.join(sys.path[0],'config'), os.path.join(_appDir,'config'))
    shutil.copytree(os.path.join(sys.path[0],'resources'), os.path.join(_appDir,'resources'))
    shutil.copytree(os.path.join(sys.path[0], 'std_bench', 'data'), os.path.join(_appDir, 'std_bench', 'data'))
    shutil.copytree(os.path.join(sys.path[0],'Info'), os.path.join(_appDir, 'Info'))
    shutil.copyfile(os.path.join(sys.path[0],'package.json'), os.path.join(_appDir, 'package.json'))
    _iconPath = os.path.join(_exeDir, 'Contents', 'Resources', 'nw.icns')
    os.remove(_iconPath)
    shutil.copyfile(os.path.join(_appDir, 'resources', 'icons', 'nw.icns'), _iconPath)

    _tmpCmd = ' '.join(['make --silent -C', os.path.join(_seqBuildDir, benchBuildDir), 'all'])
    os.system(_tmpCmd)
    _appStdProDir = os.path.join(_appDir, 'std_bench')
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'global_alignment_dna'), os.path.join(_appStdProDir, 'global_alignment_dna'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'local_alignment_dna'), os.path.join(_appStdProDir, 'local_alignment_dna'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'global_alignment_protein'), os.path.join(_appStdProDir, 'global_alignment_protein'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'local_alignment_protein'), os.path.join(_appStdProDir, 'local_alignment_protein'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'index_create'), os.path.join(_appStdProDir, 'index_create'))
    shutil.move(os.path.join(_seqBuildDir, 'bin', 'index_approximate'), os.path.join(_appStdProDir, 'index_approximate'))
    _installPath = os.path.join('/','Applications', 'SeqanBench.app')
    if os.path.isdir(_installPath):
        shutil.rmtree(_installPath)
    shutil.copytree(_exeDir, _installPath)
    shutil.rmtree(_exeDir)
    print 'Complete'
    print 'Check SeqanBench in /Applications folder'

