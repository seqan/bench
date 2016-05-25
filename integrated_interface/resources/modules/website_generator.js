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

    self.generate = function(benchmark_queue, website_path) {
        const ncp = require('ncp');

        // copy website template to the given path
        ncp('./resources/website_template', website_path, function (err) {
            if (err) {
                console.error(err);
                self.emit('error', err);
            }

            Exporter.save_results(website_path + "/benchmark_results.json", benchmark_queue);
            Exporter.save_website_results(website_path + '/files/benchmark.json', benchmark_queue);
            self.emit('done', website_path, benchmark_queue);
        });
    }

    return self;
});
