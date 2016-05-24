;(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.Exporter = factory();
    }

})(this, function() {
    const Configure = require('./configure');
    const is_object = require('isobject');
    var self = {};

    self.results = function(benchmark_queue) {
        var benchmark_results = {
            informations: {
                system: benchmark_queue.system,
                project: benchmark_queue.project
            },
            results: {
            }
        };

        // prepare benchmark_results.results
        for(const benchmark of benchmark_queue) {
            const id = benchmark.benchmark_id;
            benchmark_results.results[id] = {
                "single_core": {
                    "time": [],
                    "quality": []
                },
                "multi_core": {
                    "time": [],
                    "quality": []
                }
            };
        }

        // fill results
        for(const benchmark of benchmark_queue) {
            const id = benchmark.benchmark_id;
            var result = benchmark_results.results[id];
            var core = benchmark.threads == 1 ? result.single_core : result.multi_core;

            core.quality.push(benchmark.validator.quality());
            core.time.push(benchmark.runtime);
        }

        return benchmark_results;
    };

    /**
     * The raw output file of the benchmark app can have repeat measurements in
     * `.results.<benchmark>.single_core.time`,
     * `.results.<benchmark>.single_core.quality`,
     * `.results.<benchmark>.multi_core.time` and
     * `.results.<benchmark>.multi_core.quality`.
     *
     * Those times will be reduced to a single time. E.g. these repeated
     * measurements [12.343, 21.124, 43.3132] would be reduced to a single
     * measurements 12.343.
     */
    self.reduce_repeat_measures = function(results) {
        const min_array = function(array) {
            return array.slice().sort()[0];
        };
        const median_array = function(array) {
            const middle = Math.floor(array.length / 2);
            return array.slice().sort()[middle];
        };

        for (const benchmark_id in results['results']) {
            const benchmark = results['results'][benchmark_id];

            benchmark['single_core']['time'] = median_array(benchmark['single_core']['time']);
            benchmark['multi_core']['time'] = median_array(benchmark['multi_core']['time']);
            benchmark['single_core']['quality'] = min_array(benchmark['single_core']['quality']);
            benchmark['multi_core']['quality'] = min_array(benchmark['multi_core']['quality']);
        }
        return results;
    };

    /**
     * Calculate a score for each measured time based on a base_time of a
     * benchmark.
     */
    self.calculate_scores = function (results) {
        const Configure = require('./configure');
        const descriptions = Configure.benchmarks_description()['benchmarks'];

        for (const benchmark_id in results['results']) {
            const benchmark = results['results'][benchmark_id];

            var single_core = benchmark['single_core'];
            var multi_core = benchmark['multi_core'];

            const base_time = descriptions[benchmark_id]['base_time'];
            single_core['score'] = Math.round(1000 * base_time / single_core['time'])
            multi_core['score'] = Math.round(1000 * base_time / multi_core['time'])
        }
        return results;
    };

    /**
     * Deep-Map each element as an array if it is not a dict. I.e. map
     * {
     *     'a': {
     *         'b': "Hello"
     *     },
     *     'c': 1
     * } as
     * {
     *     'a': {
     *         'b': ["Hello"]
     *     },
     *     'c': [1]
     * }
     */
    self.deep_array_map = function(item) {
        if (!is_object(item)) {
            return [item];
        }

        Object.keys(item).map(function(key) {
           item[key] = self.deep_array_map(item[key]);
        });
        return item;
    };

    self.categorize_results = function(results) {
        const Configure = require('./configure');
        const extend = require('extend');

        const config = Configure.benchmarks_description();
        const descriptions = config['benchmarks'];
        const categories = config['categories'];

        // default category
        categories['undefined'] = {subcategories: {}};

        // add subcategories to all categories
        for (const category_id in categories) {
            categories[category_id]['subcategories'] = {};
        }

        const old_results = results['results'];
        const new_results = {};

        // merge benchmark results with the benchmark description and group them
        // into their category
        for (const benchmark_id in old_results) {
            const benchmark = old_results[benchmark_id];
            const description = descriptions[benchmark_id] ? descriptions[benchmark_id] : {
                category: 'undefined',
                title: benchmark_id,
                subtitle: ''
            };
            const category_id = description['category'];

            // add title, subtitle and category to the benchmark
            extend(true, benchmark, description);

            // extend category with subcategories (benchmarks)
            categories[category_id]['subcategories'][benchmark_id] = benchmark;
        }

        // filter empty categories out
        for (const category_id in categories) {
            const category = categories[category_id];

            // is empty category
            if (!Object.keys(category.subcategories).length) {
                continue;
            }

            new_results[category_id] = category;
        }

        results['results'] = new_results;
        return results;
    };

    self.website_results = function(benchmark_queue) {
        var results = self.results(benchmark_queue);
        results = self.reduce_repeat_measures(results);
        results = self.calculate_scores(results);
        results = self.deep_array_map(results);
        results = self.categorize_results(results);
        return results;
    };

    self.save_website_results = function(path, benchmark_queue) {
        const results = self.website_results(benchmark_queue);
        Configure.save_json(path, results);
    };

    self.save_results = function(path, benchmark_queue) {
        const results = self.results(benchmark_queue);
        Configure.save_json(path, results);
    };

    return self;
});
