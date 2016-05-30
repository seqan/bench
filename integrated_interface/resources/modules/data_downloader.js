/**
 * DataDownloader:
 *
 * Events:
 *
 *   'initialize': ()
 *       Will be called if the data must be downloaded.
 *
 *   'done': ()
 *       Will be called after everything is done.
 *
 *   'progress': (state)
 *       Will be called every 250ms with the current state.
 *
 *   'downloaded': (response)
 *       Will be called after the download finished.
 *
 *   'extracted': ()
 *       Will be called after the data.tar.gz was extracted.
 *
 *   'index_created': ()
 *       Will be called after the index was created.
 *
 *   'error': (error)
 *       Will be called if an error occurred during download / index creation.
 *
 *   'stdout': (chunk)
 *       Gives a chunk of stdout.
 *
 *   'stderr': (chunk)
 *       Gives a chunk of stderr.
 *
 *
 */
;(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.DataDownloader = factory();
    }

})(this, function() {
    "use strict";

    const fs = require('fs-extra');
    const request = require('request');
    const progress = require('request-progress');
    const targz = require('tar.gz');

    const EventEmitter = require('events');
    const Configure = require('./configure');

    const execCwd = Configure.execCwd;
    const downloadedTar = execCwd + '/data/data.tar.gz';
    const dataPath = execCwd + '/data/';
    const untarPath = dataPath;

    var self = new EventEmitter();

    /**
     * Downloads data if not available.
     * If data is available, but index is missing, create index.
     */
    self.rebuild = function() {
        fs.access(dataPath + '/genome.fa', fs.F_OK, (err) => {
            if (err) {
                return self.download();
            }

            fs.access(dataPath + '/indices/genome.index_esa.inx.lcp', fs.F_OK, (err) => {
                if (!err) {
                    self.emit('done');
                    return;
                }

                self.emit('initialize');
                self.emit('downloaded', {});
                self.emit('extracted');
                self.createIndex();
            });
        });
    };

    self.download = function() {
        const url = Configure.app().dataUrl;
        self.emit('initialize');

        const download = request(url, function (error, response, body) {
            if (error) {
                self.emit('error', error)
                return;
            }
            self.emit('downloaded', response);
            self.untar();
        });

        progress(download, {
            throttle: 250, // Throttle the progress event to 250ms, defaults to 1000ms
        })
        .on('progress', function (state) {
            self.emit('progress', state)
        })
        .on('error', function (err) {
            self.emit('error', err)
        })
        .pipe(fs.createWriteStream(downloadedTar));
    };

    self.untar = function() {
        targz().extract(downloadedTar, untarPath)
        .then(function(){
            self.emit('extracted');
            self.createIndex();
        })
        .catch(function(err){
            self.emit('error', err);
        });
    };

    self.createIndexProcess = function(options, closed) {
        const isWin = /^win/.test(process.platform);

        const spawn = require('child_process').spawn;
        const shell_parse = require('shell-quote').parse;

        const create_cmd = 'benchmarks/index_create' + (isWin ? '.exe' : '') +
                           ' data/' + options.data +
                           ' data/indices/' + options.index_name;
        const create_args = shell_parse(create_cmd);

        var indexCreate = spawn(create_args[0], create_args.slice(1), {
            cwd: execCwd,
            detached: true
        }).on('close', closed)
        .on('error', function(err) {
            self.emit('error', err);
        });

        indexCreate.stdout.on('data', (chunk) => {
            self.emit('stdout', chunk);
        });
        indexCreate.stderr.on('data', (chunk) => {
            self.emit('stderr', chunk);
        });
        return indexCreate;
    }

    self.createIndex = function() {

        fs.ensureDirSync(execCwd + '/data/indices');

        self.createIndexProcess({
            data: 'genome.fa',
            index_name: 'genome.index'
        }, function(code) {
            if (code != 0) {
                self.emit('error', new Error("index_create: exited with " + code));
            }

            self.emit('index_created');
            self.emit('done');
        });
    };

    return self;
});
