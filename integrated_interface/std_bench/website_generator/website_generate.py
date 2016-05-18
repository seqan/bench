#!/usr/bin/env python3

import json
import shutil
import sys
import subprocess
import time
import os

root_path = './'
bench_app_path = '../'

template_path = root_path + "template/"
validator_path = bench_app_path + 'validators/'
validations_path = bench_app_path + 'data/validations/'
outputs_path = bench_app_path + 'results/'

def copy_template(website_path):
    """
    Copy template to the new location `website_path`
    """
    try:
        shutil.copytree(template_path, website_path)
    except Exception as err:
        # if path already exists, warn about that.
        print(err)


def validators():
    """
    The benchmarks grouped by validators.
    """

    validators_file = "./validators.json"
    with open(validators_file) as data_file:
        return json.load(data_file)


def descriptions_categories():
    """
    A dict of all categories with human-readable category titles.
    """

    descriptions_file = "./descriptions.json"
    with open(descriptions_file) as data_file:
        return json.load(data_file)['categories']


def descriptions_benchmarks():
    """
    A dict of benchmarks with their human-readable title, subtitle and
    associated category.
    """

    descriptions_file = "./descriptions.json"
    with open(descriptions_file) as data_file:
        return json.load(data_file)['benchmarks']


def execute_validator(validator, output_file, validation_file):
    """
    Execute `validator <output_file> <validation_file>` and return the float
    value of the validator
    """

    print ("execute:", validator, output_file, validation_file)
    validation = 0

    args = [validator, output_file, validation_file]
    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    try:
        for line in p.stdout.readlines():
            if not line.strip():
                continue
            print("\t->", line.strip())
            validation = float(line)
            break
    except:
        pass
    print ()

    p.wait()

    return validation


def validate_results(results):
    """
    Add the validators return values to .results.<benchmark>.single_core.quality
    and .results.<benchmark>.multi_core.quality
    """

    for validator, benchmarks in validators().items():
        validator = validator_path + validator

        for benchmark_id in benchmarks:
            if benchmark_id not in results['results']:
                print("skip: %s validation" % (benchmark_id))
                continue

            for thread_type in ['single_core', 'multi_core']:
                validation_file = validations_path + benchmark_id + ".validate.txt"
                output_file = outputs_path + benchmark_id + "." + thread_type + ".result.txt"

                # execute validator
                validation = execute_validator(validator, output_file, validation_file)

                results['results'][benchmark_id][thread_type]['quality'] = validation

    return results


def load_benchmark_results():
    """
    Load benchmark results from the benchmark app.
    """

    json_file_path = outputs_path + "benchmark_results.json"
    with open(json_file_path) as data_file:
        return json.load(data_file)


def reduce_repeat_measures(results):
    """
    The raw output file of the benchmark app can have repeat measurements in
    `.results.<benchmark>.single_core.time` and
    `.results.<benchmark>.multi_core.time`.

    Those times will be reduced to a single time. E.g. there repeated
    measurements [12.343, 21.124, 43.3132] would be reduced to a single
    measurements 12.343.
    """

    for benchmark_id in results['results'].keys():
        single_core = results['results'][benchmark_id]['single_core']
        multi_core = results['results'][benchmark_id]['multi_core']

        single_core['time'] = min(single_core['time'])
        multi_core['time'] = min(multi_core['time'])

    return results


def calculate_scores(results):
    """
    Calculate a score for each measured time based on a base_time of a
    benchmark.
    """

    descriptions = descriptions_benchmarks()

    for benchmark_id in results['results'].keys():
        single_core = results['results'][benchmark_id]['single_core']
        multi_core = results['results'][benchmark_id]['multi_core']

        base_time = descriptions[benchmark_id]['base_time']
        single_core['score'] = round(1000 * base_time / single_core['time'])
        multi_core['score'] = round(1000 * base_time / multi_core['time'])

    return results


def map_dict(item):
    """
    Deep-Map each element as an array if it is not a dict. I.e. map

    ```
    {
        'a': {
            'b': "Hello"
        },
        'c': 1
    }
    ```

    to

    ```
    {
        'a': {
            'b': ["Hello"]
        },
        'c': [1]
    }
    ```
    """

    if isinstance(item, dict):
        return {k: map_dict(v) for k, v in item.items()}
    else:
        return [item]


def generate_website_json(results):
    """
    Generate the content of benchmark.json by combining
    `descriptions_benchmarks()`, `descriptions_categories()` and `results`.
    """

    # map all elements which are non-dicts as [element]
    results = map_dict(results)

    descriptions = descriptions_benchmarks()
    categories = descriptions_categories()

    # add subcategories to all categories
    for category_id in categories.keys():
        categories[category_id]['subcategories'] = {}

    # merge benchmark results with the benchmark description and group them into
    # their category
    for benchmark_id, description in descriptions.items():
        if benchmark_id not in results['results']:
            continue

        category_id = description.pop('category', None)

        benchmark = results['results'][benchmark_id]
        benchmark.update(descriptions[benchmark_id])

        # extend category with subcategories (benchmarks)
        categories[category_id]['subcategories'][benchmark_id] = benchmark

    # filter empty categories out
    categories = { key: category for key, category in categories.items() if category['subcategories'] }

    # replace the initial benchmark results with the merged and group results
    results['results'] = categories

    return results


def save_website_json(json_file):
    """
    Generate and save benchmark.json.
    """

    results = load_benchmark_results()
    results = reduce_repeat_measures(results)
    results = calculate_scores(results)

    results = validate_results(results)

    website_json = generate_website_json(results)

    with open(json_file, 'w') as outfile:
        json.dump(website_json, outfile, sort_keys=True, indent=4, separators=(',', ': '))


def generate_website(args):
    """
    Generate the static results website out of the output file of the bench app.
    """
    website_path = args.website_path + '/'
    json_file = website_path + 'files/benchmark.json'

    print("COPY TEMPLATE -> " + website_path)
    copy_template(website_path)

    save_website_json(json_file)

    print("GENERATING WEBSITE - done")


if __name__ == "__main__":
    import argparse

    # all paths should be relative to this file
    os.chdir(os.path.dirname(sys.argv[0]))

    default = "%sbenchmark-%s" % (root_path, time.strftime("%Y%m%d-%H%M%S"))

    parser = argparse.ArgumentParser()
    parser.add_argument('-o', metavar='OUTPUT_DIR', default=default, dest='website_path',
                        help='output directory (default: benchmark-YYYYmmdd-HHMMSS)')
    args = parser.parse_args()

    generate_website(args)
