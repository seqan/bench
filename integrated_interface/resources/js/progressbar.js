;(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.ProgressBar = factory();
    }

})(this, function() {
    const BenchmarkExecutor = require('./modules/benchmark_executor');
    var self = {};

    const _percent_max = 1;
    const _delta_time = 100;
    const _dots_str = [".", "..", "..."];
    var _k = 0;
    var _total_expected_time = 0;
    var _current_benchmark_name = undefined;
    var timeout_id;
    var _progress_percent = 0;
    var _progress_percent_max = 1;
    var _progress_maxs = [];

    self._initialize = function (benchmark_queue) {
        _total_expected_time = benchmark_queue.total_expected_runtime();

        _progress_maxs[0] = 0;
        for (var k = 1; k < benchmark_queue.length; k++){
            _progress_maxs[k] = _progress_maxs[k - 1] + benchmark_queue[k - 1].expected_runtime / _total_expected_time;
        }
        _progress_maxs[benchmark_queue.length] = 1;
    };

    self.start = function(benchmark_queue) {
        self.clear();
        self._initialize(benchmark_queue);
        self.update_process(benchmark_queue.current_process());
        NProgress.start();

        NProgress.configure({showSpinner: false, parent:'#progressbar'});
        self._show_progress();
    };

    self.clear = function() {
        clearTimeout(timeout_id);
        _current_benchmark_name = undefined;
        _progress_percent = 0;
        _progress_percent_max = 1;
        _k = 0;
        _total_expected_time = 0;
        _progress_maxs = [];
    };

    self.update_process = function(current_process) {
        var queue_id = current_process.queue_id;
        _current_benchmark_name = current_process.benchmark_name;
        _progress_percent_max = _progress_maxs[queue_id+1];

        _progress_percent = _progress_maxs[queue_id];
        NProgress.set(_progress_percent);
    };

    self.stop = function() {
        self.clear();
    };

    self._show_progress = function() {
        timeout_id = setTimeout(function(){
            if(_progress_percent < _progress_percent_max){
                const delta = _progress_percent + _delta_time / _total_expected_time / 1000;
                _progress_percent = Math.min(delta, _progress_percent_max)
                NProgress.set(_progress_percent);
            }

            var benchmark_name = _current_benchmark_name;
            var progress_percent = Math.floor(_progress_percent * 100 * 10) / 10;
            var progress_percent_max = Math.floor(_progress_percent_max * 100 * 10)/10;
            var dots = _dots_str[Math.floor((_k++ * _delta_time / 1000)) % _dots_str.length];
            $('#progressbar_text').html("<p><h6 style='color: #428bca '><b> running " + benchmark_name + " " + progress_percent + "% / " + progress_percent_max + "% " + dots + "</b></h6></p>" );
            self._show_progress();
        }, _delta_time);
    };

    // before the benchmarks will be run
    BenchmarkExecutor.on('initialize', function(benchmark_queue) {
        $('#progressbar_text').html("<p><b><h6 style='color: #428bca; margin: 0px; padding: 0px'>Initializing...</h6></b></p>");

        ProgressBar.start(benchmark_queue);
    });

    // after all benchmarks run
    BenchmarkExecutor.on('done', function() {
        ProgressBar.stop();

        NProgress.set(_percent_max);
        $('#progressbar_text').html("<p><h6 style='color: #428bca '><b>complete</b></h6></p>");
    });

    // after benchmark was canceled
    BenchmarkExecutor.on('canceled', function() {
        ProgressBar.stop();

        NProgress.set(_percent_max);
        $('#progressbar_text').html("<p><h6>&nbsp;</h6></p>");
    });

    // before a benchmark will be run
    BenchmarkExecutor.on('setup', function(current_process) {
        // update progress bar
        ProgressBar.update_process(current_process);
    });

    return self;
});
