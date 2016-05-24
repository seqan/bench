/**
 * WebsiteGenerator:
 *
 * Events:
 */
;(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.WebsiteGenerator = factory();
    }

})(this, function() {
    const EventEmitter = require('events');
    const Configure = require('./configure');
    const Exporter = require('./exporter');

    var self = new EventEmitter();

    self.on('done', (website_path, benchmark_queue) => {
        const file = website_path + '/files/benchmark.json';
        const results = Exporter.website_results(benchmark_queue);
        Configure.save_json(file, results);

        Exporter.save_results(website_path + "/benchmark_results.json", benchmark_queue);
    });

    self.generate = function(benchmark_queue, website_path) {
        Exporter.save_results("./results/benchmark_results.json", benchmark_queue);

        const spawn = require('child_process').spawn;
        try {
            var process = spawn('./website_generator/website_generate.py', ['-o', website_path], {detached: true});
        } catch(error) {
            console.log('website generator - error spawn');
            console.error(error);
        }
        process.stdout.on('data', function(data) {
            console.log("stdout: " + data);
        });
        process.stderr.on('data', function(data) {
            console.log("stderr: " + data);
        });

        process.on('close', function() {
            console.log('website generator closed');
            self.emit('done', website_path, benchmark_queue);
        });
        process.stdout.on('error', function(error) {
            console.log('website generator stdout error');
            console.error(error);
        });
        process.stderr.on('error', function(error) {
            console.log('website generator stderr error');
            console.error(error);
        });
        process.on('error', function(error) {
            console.log('website generator error');
            console.error(error);
        });
    }

    return self;
});
