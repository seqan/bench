#!/usr/bin/env node

var started_at = new Date();

const sprintf = require('sprintf-js').sprintf;
const strftime = require('strftime');
const quote = require('shell-quote').quote;
const yargs = require('yargs');
const os = require('os');

const Configure = require('./resources/modules/configure');
const BenchmarkExecutor = require('./resources/modules/benchmark_executor');
const ValidatorExecutor = require('./resources/modules/validator_executor');
const WebsiteGenerator = require('./resources/modules/website_generator');
const DataDownloader = require('./resources/modules/data_downloader');
const Exporter = require('./resources/modules/exporter');
const max_threads = os.cpus().length;

const argv = yargs
    .usage('Usage: $0 -tc [THREADS] -c [CONFIG] -o [OUTPUT] --html [HTML]')
    .example('$0 -c config.json -o results.json', 'Run benchmarks defined in config.json and write results to results.json')
    .wrap(yargs.terminalWidth())
    .version()
    .option('v', {
      alias: 'verbose',
      describe: 'be verbose',
      boolean: true
    })
    .option('t', {
      alias: 'threads',
      describe: 'The number of threads the benchmark should use in the multi-core run.',
      default: max_threads,
      number: true
    })
    .check(function(argv) {
      const threads = parseInt(argv['threads']);

      if (1 <= threads && threads <= max_threads) {
        argv['t'] = threads;
        argv['threads'] = threads;
        return true;
      }

      throw new Error("Threads must be between 1 and " + max_threads + ", " + argv['threads'] + " given.");
    })
    .option('c', {
      alias: 'config',
      describe: 'CONFIG file that defines the benchmarks.',
      default: 'config/config.json',
      normalize: true
    })
    .option('o', {
      alias: 'output',
      describe: 'Write results to OUTPUT.',
      default: 'results-' + strftime("%Y%m%d-%H%M%S", started_at) + '.json',
      normalize: true
    })
    .option('html', {
      describe: 'Generate static website to HTML, that shows the results.',
      normalize: true
    })
    .help('h')
    .alias('h', 'help')
    .argv;

console.log("load benchmarks: " + argv['config']);
console.log();

Configure.init({
  benchmarks: argv['config']
});

Configure.system_infos({
  threads: argv['threads']
});

BenchmarkExecutor.on('setup', (benchmark_process, benchmark_queue) => {
  const cmd = benchmark_process.shell_command + " " + quote(benchmark_process.shell_args);
  console.log(benchmark_process.benchmark_id + ":");

  if (argv['verbose']) {
    console.log("\tcmd: " + cmd);
  }
  console.log("\trepeat: " + benchmark_process.repeat + '/' + benchmark_process.repeats);
  console.log("\tthreads: " + benchmark_process.threads);
});

BenchmarkExecutor.on('error', (error, benchmark_process, benchmark_queue) => {
  console.warn("\terror: " + error.message);
});

ValidatorExecutor.on('error', (error) => {
  console.warn("\terror (validator): " + error.message);
});

BenchmarkExecutor.on('result', (benchmark_process, benchmark_queue) => {
  console.log("\tstatus: " + benchmark_process.state);
  console.log("\toutput: " + benchmark_process.validator.quality());
  console.log("\ttime: " + benchmark_process.runtime);
  console.log();
});

BenchmarkExecutor.on('done', (benchmark_queue) => {
  console.log("---------------------------------------");
  console.log("total time: " + benchmark_queue.total_runtime());

  console.log();
  console.log("write results to: " + argv['output']);
  Exporter.save_results(argv['output'], benchmark_queue);

  if (argv['html']) {
    if (argv['html'] === true) {
      argv['html'] = 'benchmark-' + strftime("%Y%m%d-%H%M%S", started_at);
    }
    console.log("write website to: " + argv['html']);
    WebsiteGenerator.generate(benchmark_queue, argv['html']);
  }
});

DataDownloader.on('done', () => {
  BenchmarkExecutor.run();
}).on('initialize', () => {
  console.log('download data.tar.gz');
}).on('downloaded', () => {
  console.log('data.tar.gz downloaded');
}).on('extracted', () => {
  console.log('data.tar.gz extracted');
}).on('index_created', () => {
  console.log('data/indices/ rebuild');
}).on('stdout', (chunk) => {
  console.log("stdout", "" + chunk);
}).on('stderr', (chunk) => {
  console.log("stderr", "" + chunk);
}).on('error', (error) => {
  console.error(error);
}).rebuild();
