;(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.Configure = factory();
    }

})(this, function() {
    "use strict";

    // the benchmarks, config and validators are not bundled in the app and
    // reside next to the app executable.
    const execCwd = function() {
        const path = require('path');
        const fs = require('fs');
        try {
            const cwd_path = process.cwd();
            fs.accessSync(cwd_path + '/benchmarks');
            return cwd_path;
        } catch (e) {
        }
        const macCwd = process.execPath.split('.app/Contents/', 2);
        // the bundled version on mac has the following execPath:
        // <execCwd>/seqan-bench-app.app/Contents/Versions/[...]/nwjs Helper.app/Contents/MacOS/nwjs Helper
        // Thus, we split at the first .app/Contents/ point
        return path.dirname(macCwd[0]);
    }();

    var self = {
        execCwd: execCwd,
        config: {
            files: {
                app_config: "./resources/config/app_config.json",
                app_help: "./resources/help/app/helpinfo.json",
                benchmark_help: "./resources/help/benchmarks/%s.help.json",
                benchmarks_config: execCwd + "/config/config.json",
                benchmarks_description: "./resources/config/descriptions.json",
                validators: "./resources/config/validators.json"
            },
            app: {},
            system: {},
            project: {},
            benchmarks: {},
            benchmarks_default: {},
            benchmarks_description: {}
        }
    };

    self.load_json = function(json_file) {
        const fs = require('fs');
        return JSON.parse(fs.readFileSync(json_file, 'utf8'));
    };

    self.save_json = function(json_file, json) {
        const fs = require('fs');
        const serialize = JSON.stringify(json, null, 2);
        return fs.writeFileSync(json_file, serialize, 'utf8');
    };

    self.save_jsonp = function(json_file, json, callback_name) {
        const fs = require('fs');
        const serialize = JSON.stringify(json, null, 2);
        const jsonp = callback_name + "("+ serialize +");"
        return fs.writeFileSync(json_file, jsonp, 'utf8');
    };

    self.load_app = function(filepath) {
        try{
            self.config.app = self.load_json(filepath);
            return true;
        } catch(err){
            console.error(err);
            return false;
        }
    };

    self.load_benchmarks = function(filepath) {
        try{
            var json = self.load_json(filepath);
            self.config.project = json['project'];
            self.config.benchmarks = json['benchmarks'];

            var json = self.load_json(filepath);
            self.config.benchmarks_default = json['benchmarks'];
            return true;
        } catch(err){
            console.error(err);
            return false;
        }
    };

    self.load_app_help_file = function() {
        const file = self.config.files.app_help;
        return self.load_json(file);
    };

    self.load_benchmark_help_file = function(benchmark_id) {
        const sprintf = require("sprintf-js").sprintf;
        const file = self.config.files.benchmark_help;
        return self.load_json(sprintf(file, benchmark_id));
    };

    self.load_benchmarks_description = function(filepath) {
        return self.config.benchmarks_description = self.load_json(filepath);
    };

    self.load_validators = function() {
        const file = self.config.files.validators;
        return self.load_json(file);
    };

    self.load_system_infos = function() {
        const os = require("os");
        self.config.system = {};

        self.system_infos({
            "os": os.platform() + " " + os.arch() + " " + os.release(),
            "cpu_frequency": os.cpus()[0].speed,
            "cpu_name": os.cpus()[0].model,
            "memory": Math.floor(os.totalmem()/1024/1024/1024*10)/10,
            "threads": os.cpus().length,
            "max_threads": os.cpus().length
        });
    };

    self.system_infos = function(extend_infos) {
        const os = require("os");
        const strftime = require('strftime');
        const extend = require('extend');

        const infos = {
            "cpu_frequency": os.cpus()[0].speed,
            "date": strftime('%F %T'), // "2016-04-13 14:44:35",
        };

        return extend(true, self.config.system, infos, extend_infos);
    };

    self.app = function() {
        return self.config.app;
    };

    self.project_infos = function() {
        return self.config.project;
    };

    self.benchmarks = function() {
        return self.config.benchmarks;
    };

    self.benchmark = function(benchmark_id) {
        return self.config.benchmarks[benchmark_id];
    };

    self.benchmarks_description = function() {
        return self.config.benchmarks_description;
    };

    self.reset_benchmark_command = function(benchmark_id) {
        const default_command = self.config.benchmarks_default[benchmark_id].command;
        self.config.benchmarks[benchmark_id].command = default_command;
        return default_command;
    };

    self.init = function(config) {
        const extend = require('extend');

        const config_ = extend({
            app: self.config.files.app_config,
            benchmarks: self.config.files.benchmarks_config,
            benchmarks_description: self.config.files.benchmarks_description
        }, config);

        self.load_system_infos();
        self.load_app(config_.app);
        self.load_benchmarks(config_.benchmarks);
        self.load_benchmarks_description(config_.benchmarks_description);
    };

    self.init();

    return self;
});
