/**
 * WebsiteGenerator:
 *
 * Events:
 *
 *   'done': (website_path, benchmark_queue)
 *       Will be called after the website was generated.
 *
 *   'error': (error)
 *       Will be called if an error occured during generation.
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
    "use strict";

    const EventEmitter = require('events');
    const Configure = require('./configure');
    const Exporter = require('./exporter');

    var self = new EventEmitter();

    self.generate = function(benchmark_queue_or_results, website_path) {
        const ncp = require('ncp');

        // copy website template to the given path
        ncp('./resources/website_template', website_path, function (err) {
            if (err) {
                console.error(err);
                self.emit('error', err);
            }

            // save raw results, if we have it.
            if (!Exporter.is_website_result(benchmark_queue_or_results)) {
                Exporter.save_results(website_path + "/benchmark_results.json", benchmark_queue_or_results);
            }

            Exporter.save_website_results_jsonp(
                website_path + '/files/benchmark.jsonp',
                benchmark_queue_or_results,
                'loadWebsite'
            );
            Exporter.save_website_results(website_path + '/files/benchmark.json', benchmark_queue_or_results);
            self.emit('done', website_path, benchmark_queue_or_results);
        });
    }

    return self;
});
