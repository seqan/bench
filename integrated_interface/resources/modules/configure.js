;(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.Configure = factory();
    }

})(this, function() {
    var self = {
        config: {
            files: {
                app_config: "./config/app_config.json"
            },
            app: {},
            system: {},
            benchmarks: {},
            benchmarks_default: {}
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

    self.load_app = function() {
        try{
            self.config.app = self.load_json(self.config.files.app_config);
            return true;
        } catch(err){
            console.error(err);
            return false;
        }
    };

    self.load_benchmarks = function(filepath) {
        try{
            self.config.benchmarks = self.load_json(filepath);
            self.config.benchmarks_default = self.load_json(filepath);
            return true;
        } catch(err){
            console.error(err);
            return false;
        }
    };

    self.load_benchmark_help_file = function(benchmark_id) {
        const path = "./resources/help/benchmarks/";
        const file = path + benchmark_id + ".help.json";
        return self.load_json(file);
    };

    self.system_infos = function(extend_infos) {
        const os = require("os");
        const strftime = require('strftime');
        const extend = require('extend');

        const infos = {
            "os": os.platform() + " " + os.arch() + " " + os.release(),
            "cpu_frequency": os.cpus()[0].speed,
            "cpu_name": os.cpus()[0].model,
            "memory": Math.floor(os.totalmem()/1024/1024/1024*10)/10,
            "date": strftime('%F %T'), // "2016-04-13 14:44:35",
            "threads": os.cpus().length,
            "max_threads": os.cpus().length
        };

        return self.config.system = extend(infos, extend_infos);
    };

    self.app = function() {
        return self.config.app;
    };

    self.benchmarks = function() {
        return self.config.benchmarks;
    };

    self.benchmark = function(benchmark_id) {
        return self.config.benchmarks[benchmark_id];
    };

    self.reset_benchmark_command = function(benchmark_id) {
        const default_command = self.config.benchmarks_default[benchmark_id].command;
        self.config.benchmarks[benchmark_id].command = default_command;
        return default_command;
    };

    self.init = function() {
        self.load_app();
        self.load_benchmarks(self.app().ConfFilePath);
    };

    self.init();

    return self;
});
