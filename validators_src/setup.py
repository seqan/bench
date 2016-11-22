from distutils.core import setup
import os
import platform
import glob
import argparse
import sys
if platform.system() == 'Windows':
    import py2exe

# consume argument --build-dir
parser = argparse.ArgumentParser(description='Build-setup of validators.')
parser.add_argument('--build-dir', default='../validators/',
                    help='the build dir')
parser.add_argument('argv', nargs=argparse.REMAINDER, help=argparse.SUPPRESS)
args = parser.parse_args()

# remove consumed arguments
sys.argv = [sys.argv[0]] + args.argv

files = [fn for fn in glob.glob('*.py')
         if not os.path.basename(fn) == 'setup.py']

if platform.system() == 'Windows':
    # call with `python setup.py py2exe`
    setup(console=files, options={
        'py2exe': {
            'dist_dir': args.build_dir
        }
    })
else:
    # call with `python setup.py build`
    setup(scripts=files, options={
        'build_scripts': {
            'build_dir': args.build_dir
        }
    })
