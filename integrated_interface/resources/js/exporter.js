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
    var self = {};

    self.generate_results = function(benchmark_queue) {
        // TODO benchmark_queue.started_at
        var benchmark_results = {
            informations: {
                system: Configure.system_infos(),
                project: {
                    title: 'seqan-2.1-gcc'
                }
            },
            results: {
            }
        };

        // prepare benchmark_results.results
        for(const benchmark of benchmark_queue) {
            const id = benchmark.benchmark_id;
            benchmark_results.results[id] = {
                "single_core": {
                    "time": []
                },
                "multi_core": {
                    "time": []
                }
            };
        }

        // fill results
        for(const benchmark of benchmark_queue) {
            const id = benchmark.benchmark_id;
            var result = benchmark_results.results[id];

            if (benchmark.threads == 1) {
                result.single_core.time.push(benchmark.runtime);
            } else {
                result.multi_core.time.push(benchmark.runtime);
            }
        }

        return benchmark_results;
    };

    self.save_results = function(path, benchmark_queue) {
        const results = self.generate_results(benchmark_queue);
        Configure.save_json(path, results);
    };

    return self;
});
