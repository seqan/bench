#!/usr/bin/env python3

import json
import shutil
import sys
import subprocess
import time
import os

root_path = './'

template_path = root_path + "template/"


def copy_template(website_path):
    """
    Copy template to the new location `website_path`
    """
    try:
        shutil.copytree(template_path, website_path)
    except Exception as err:
        # if path already exists, warn about that.
        print(err)


def generate_website(args):
    """
    Generate the static results website out of the output file of the bench
    app.
    """
    website_path = args.website_path + '/'

    print("COPY TEMPLATE -> " + website_path)
    copy_template(website_path)

    print("GENERATING WEBSITE - done")


if __name__ == "__main__":
    import argparse

    # all paths should be relative to this file
    os.chdir(os.path.dirname(sys.argv[0]))

    default = "%sbenchmark-%s" % (root_path, time.strftime("%Y%m%d-%H%M%S"))

    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-o',
        metavar='OUTPUT_DIR',
        default=default,
        dest='website_path',
        help='output directory (default: benchmark-YYYYmmdd-HHMMSS)'
    )
    args = parser.parse_args()

    generate_website(args)
