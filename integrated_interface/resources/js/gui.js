function slugify(text) {
    return text.toString().toLowerCase()
        .replace(/\s+/g, '-')           // Replace spaces with -
        .replace(/[^\w\-]+/g, '')       // Remove all non-word chars
        .replace(/\-\-+/g, '-')         // Replace multiple - with single -
        .replace(/^-+/, '')             // Trim - from start of text
        .replace(/-+$/, '');            // Trim - from end of text
}

;(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.Gui = factory();
    }

})(this, function() {
    "use strict";

    const path = require('path');
    const Configure = require('./modules/configure');
    const Exporter = require('./modules/exporter');
    const WebsiteGenerator = require('./modules/website_generator');
    const BenchmarkExecutor = require('./modules/benchmark_executor');
    var self = {};

    $.fn.save_as = function(on_save) {
        $(this).unbind('change')
        .change(function() {
            const filename = $(this).val();
            on_save(filename);
            $(this).val('');
        })
        .trigger('click');
    };

    self.show_system_infos = function() {
        const renderer = require('jsrender');
        const system = Configure.system_infos();

        var template = renderer.templates("./resources/templates/system/informations.html");
        var html = template.render(system);
        $("#system-panel-body").html(html);
    };

    self.show_help = function() {
        try{
            var helpFile = Configure.load_app_help_file();
        } catch(err) {
            console.error(err);
            return;
        }
        $("#HelpInfoHeader").html(helpFile.header);
        $("#HelpInfoBody").html(helpFile.body);
        $("#HelpInfoFooter").html(helpFile.footer);
    };

    var create_new_category = function(category) {
        const renderer = require('jsrender');

        var tabbox = {
            'tab_id': category.id + "Tab",
            'tab_panel_id': category.id + "TabContent",
            'tab_name': category.name,
            'tab_active': category.first ? 'active' : '',
        };

        var tabbox_tab = renderer.templates("./resources/templates/benchmarks/tabbox_tab.html");
        $("#benchmark-tabs").append(tabbox_tab.render(tabbox));

        var tabbox_panel = renderer.templates("./resources/templates/benchmarks/tabbox_panel.html");
        $("#benchmark-tabs-body").append(tabbox_panel.render(tabbox));
    };

    var create_new_panel = function(panel) {
        const renderer = require('jsrender');

        var tabbox_stripe = renderer.templates("./resources/templates/benchmarks/tabbox_stripe.html");
        $("#" + panel.id + "TabContent").append(tabbox_stripe.render({
            'color': panel.color,
            'stripe_id': panel.new_id + "TabContent"
        }));
    };

    self.benchmark_categories = function(benchmark_id) {
        const desc = Configure.benchmarks_description();
        const benchmark = desc['benchmarks'][benchmark_id];
        const category_id = benchmark['category'];
        const category = desc['categories'][category_id];

        var categories_ = [category.title, benchmark.title];
        if (benchmark.subtitle) {
            categories_.push(benchmark.subtitle);
        }
        return categories_;
    };

    self.create_categories = function() {
        const renderer = require('jsrender');
        const path = require('path');

        var colorBase=[ "default", "primary", "info", "danger", "warning", "success" ];
        var colorF=["#777", "#428bca","#5bc0de", "#d9534f","#f0ad4e","#5cb85c"];

        var tabbox = renderer.templates("./resources/templates/benchmarks/tabbox.html");
        $("#benchmark-panel-body").html(tabbox.render());

        var levelList = []
        var levelColorList = []
        var color = []
        var count = 0

        for (var benchmark_id in Configure.benchmarks()){
            const benchmark = Configure.benchmark(benchmark_id);

            const categories = self.benchmark_categories(benchmark_id);
            const ctg = categories;

            var maxk = 0
            var maxn = 0
            var levelBegins = 1
            var buffer = []
            var id=""
            count++
            for (var k = 0; k < levelList.length; k++){
                for (var n = 0; n < ctg.length; n++)
                    if (levelList[k][n] != ctg[n])
                        break

                if (maxn <= n){
                    if(maxn == n){
                        if(jQuery.inArray(levelList[k][n], buffer) == -1)
                            buffer.push(levelList[k][n])
                    }
                    else{
                        buffer=[]
                        buffer.push(levelList[k][n])
                    }
                    maxk = k
                    maxn = n
                }
            }

            levelList.push(ctg)
            levelColorList.push([-1])
            if (maxn == 0){
                create_new_category({
                    'id': slugify(ctg[0]),
                    'name': ctg[0],
                    'first': count == 1
                });
            }

            var panel_id = slugify(ctg[0]);
            color.push([0])
            for (var k = 1; k < ctg.length; k++){
                if (k < maxn)
                    color[color.length -1].push(color[maxk][k])
                if (k == maxn)
                    color[color.length -1].push((color[color.length - 1][k - 1] + buffer.length + 1) % colorBase.length)
                if (k > maxn)
                    color[color.length - 1].push((color[color.length - 1][k - 1] + 1) % colorBase.length)

                var new_panel_id = panel_id + slugify(ctg[k])
                if (k >= maxn){
                    create_new_panel({
                        'id': panel_id,
                        'new_id': new_panel_id,
                        'color': colorBase[color[color.length -1][k]]
                    });
                }
                panel_id = new_panel_id;
            }
            var text_color = colorF[color[color.length - 1][ctg.length - 1]];

            // Add ui elements that can modify the benchmark cmd, if a benchmark
            // should be run, etc..
            var tabbox_panel_content = renderer.templates("./resources/templates/benchmarks/tabbox_panel_content.html");
            $("#" + panel_id + "TabContent").append(tabbox_panel_content.render({
                'title': categories.slice(1).join(': '),
                'text_color': text_color,
                'tab_panel_id': panel_id,
                'benchmark': benchmark_id,
                'benchmark_command': benchmark.command
            }));

            // Add the help of a benchmark.
            var tabbox_panel_help_content = renderer.templates("./resources/templates/benchmarks/tabbox_panel_help_content.html");
            var help_content = "No Help Information available";

            try{
                const helpFile = Configure.load_benchmark_help_file(benchmark_id);
                help_content = helpFile.helpContent;
            } catch(err) {
                // indicate missing help
                $("#" + panel_id + "Help .glyphicon").css({color: 'DarkRed'});
            }

            $("#" + panel_id + "HelpContent").html(tabbox_panel_help_content.render({
                'text_color': text_color,
                'help_content': help_content
            }));

            self.category_panel_add_events(panel_id, benchmark_id);
        }
    };

    self.category_panel_add_events = function(panel_id, benchmark_id) {
        const benchmark = Configure.benchmark(benchmark_id);

        // Set the default value of the checkbox and toggle the state when
        // changed.
        $("#" + panel_id + "Checkbox")
        .prop('checked', !!benchmark.execute)
        .on("click", function(){
            var benchmark = Configure.benchmark(benchmark_id);
            benchmark.execute = !!$(this).prop('checked');
        });

        var save_btn = $("[name=saved][prg=" + benchmark_id + "]");
        var save_red = function() {
            save_btn
                .removeClass('glyphicon-floppy-saved')
                .addClass('glyphicon-floppy-disk')
                .css('color', '#d9534f');
        };
        var save_green = function() {
            save_btn
                .addClass('glyphicon-floppy-saved')
                .removeClass('glyphicon-floppy-disk')
                .css('color', 'green');
        };

        // If the benchmark cmd was modified, signal this by changing the color
        // of the save button.
        var input = $("#" + panel_id + "Input");
        input.bind("propertychange change click keyup input paste", function(event){
            const benchmark = Configure.benchmark(benchmark_id);

            var original_cmd = benchmark.command;
            if (original_cmd == input.val()) {
                save_green();
            } else {
                save_red();
            }
        });

        // The reset button that resets the save-button and benchmark cmd input
        // box.
        $("#" + panel_id + "Reset").click(function(){
            const original_cmd = Configure.reset_benchmark_command(benchmark_id);
            $("[name=gInput][prg=" + benchmark_id + "]").val(original_cmd);
            save_green();
        });

        // Pressing the save-button, changes it to green and saves the new
        // benchmark cmd into the data structure.
        $("#" + panel_id + "Saved").click(function(){
            var benchmark = Configure.benchmark(benchmark_id);
            var new_cmd = $("[name=gInput][prg=" + benchmark_id + "]").val();
            benchmark.command = new_cmd;
            save_green();
        });
    };

    var format_secs = function(time) {
        return (Math.floor(time * 10000) / 10000)  + ' Sec';
    };

    const scrollLock = function(fn) {
        return $.proxy(function() {
            const scrollTop = $("#panels").height() + $("#panels").scrollTop();
            const scrollHeight = $('#panels')[0].scrollHeight;
            const atBottom =  scrollTop == scrollHeight;

            // execute original function
            fn.apply(this, arguments)

            // scroll to bottom page, if we were previously at the bottom
            if (atBottom) {
                $("#panels").scrollTop($('#panels')[0].scrollHeight);
            }
        });
    };

    self.add_result = function(current_process, benchmark_queue){
        const renderer = require('jsrender');
        const result_table_id = benchmark_queue.result_table_id;

        const row_id = current_process.queue_id + 1;
        const benchmark_name = current_process.benchmark_name;

        if(current_process.state === 'SUCCESS'){
            const result_success = renderer.templates('./resources/templates/results/result_success.html');

            $('#' + result_table_id + '-body').append(result_success.render({
                row_id: row_id,
                benchmark_name: benchmark_name,
                message: 'complete',
                quality: current_process.validator.quality(),
                runtime: format_secs(current_process.runtime)
            }));
        } else if (current_process.state === 'CANCELED') {
            const result_aborted = renderer.templates('./resources/templates/results/result_aborted.html');
            $('#' + result_table_id + '-body').append(result_aborted.render());
        } else {
            const result_failure = renderer.templates('./resources/templates/results/result_failure.html');
            $('#' + result_table_id + '-body').append(result_failure.render({
                row_id: row_id,
                benchmark_name: benchmark_name,
                message: current_process.error.message,
                runtime: 'NULL'
            }));
        }
    };

    self.reenable_run_btn = function(){
        $("#run-btn").prop("disabled", false);
    };

    self.start_run = function() {
        const renderer = require('jsrender');

        $("#benchmark-panel-body").collapse('hide');
        $('#run-btn').prop({"disabled":"disabled"});

        // set the chosen threads
        Configure.system_infos({
            threads: parseInt($('#threads-input').val())
        });

        BenchmarkExecutor.on('canceled', self.reenable_run_btn);
        BenchmarkExecutor.on('done', self.reenable_run_btn);

        BenchmarkExecutor.run();
    };

    WebsiteGenerator.on('done', scrollLock(function(website_path) {
        const path = require('path');
        const fs = require('fs');

        try{
            fs.accessSync(website_path, fs.R_OK);
        } catch(err) {
            alert(err.message);
            return;
        }

        if (confirm("View the website?")) {
            const file = 'file://' + website_path + '/benchmark.html';
            nw.Shell.openExternal( file );
        }
    }));

    // benchmark starts
    BenchmarkExecutor.on('initialize', scrollLock(function(benchmark_queue){
        const result_table_id = "result-table-" + slugify(benchmark_queue.started_at.toISOString());
        benchmark_queue.result_table_id = result_table_id;

        var result_table = renderer.templates('./resources/templates/results/result_table.html');
        $('#result_tables').append(result_table.render({
            result_table_id: result_table_id
        }));

        var result_start_run = renderer.templates('./resources/templates/results/result_start_run.html');
        $('#' + result_table_id + '-head').append(result_start_run.render({
            result_table_id: result_table_id,
            project_title: benchmark_queue.project.title,
            start_date: benchmark_queue.started_at.toISOString()
        }));
    }));

    // add a summary of all executed benchmarks
    BenchmarkExecutor.on('done', scrollLock(function(benchmark_queue) {
        const renderer = require('jsrender');
        const result_table_id = benchmark_queue.result_table_id;

        $('#' + result_table_id + '-body').collapse('hide');

        var result_summary = renderer.templates('./resources/templates/results/result_summary.html');
        $('#' + result_table_id + '-foot').append(result_summary.render({
            benchmarks_count: benchmark_queue.length,
            runtime: format_secs(benchmark_queue.total_runtime())
        }));

        // prevent collapse if save buttons are clicked.
        $('#' + result_table_id + '-save-btns').click(function (e) {
            e.stopPropagation();
        });
    }));

    // on a result
    BenchmarkExecutor.on('result', scrollLock(self.add_result));

    // enable save results and save website buttons
    BenchmarkExecutor.on('done', function(benchmark_queue) {
        $('.btn-save-results:last')
        .data('benchmark_queue', benchmark_queue)
        .removeAttr('disabled')
        .click(function(){
            const benchmark_queue = $(this).data('benchmark_queue');

            $('#save-results-filechooser').save_as((filename) => {
                Exporter.save_results(filename, benchmark_queue);
            });
        });

        $('.btn-save-website:last')
        .data('benchmark_queue', benchmark_queue)
        .removeAttr('disabled')
        .click(function(){
            const strftime = require('strftime');

            const benchmark_queue = $(this).data('benchmark_queue');
            const filename = 'benchmark-' + strftime("%Y%m%d-%H%M%S", benchmark_queue.started_at);

            $('#save-website-filechooser')
            .attr('nwsaveas', filename)
            .save_as((website_path) => {
                WebsiteGenerator.generate(benchmark_queue, website_path);
            });
        });
    });

    // at least one listener for the error event, else quote
    // https://nodejs.org/api/events.html#events_error_events:
    //
    // > If an EventEmitter does not have at least one listener registered for
    // > the 'error' event, and an 'error' event is emitted, the error is
    // > thrown, a stack trace is printed, and the Node.js process exits.
    BenchmarkExecutor.on('error', function(error) {
        console.error("error_handler: " + error.message);
    });

    self.is_running = function() {
        return !!$("#run-btn").prop("disabled");
    };

    self.cancel_run = function() {
        if (self.is_running() && confirm("This will terminate running process")){
            BenchmarkExecutor.cancel();
            BenchmarkExecutor.removeListener('canceled', self.reenable_run_btn);
            BenchmarkExecutor.removeListener('done', self.reenable_run_btn);
            BenchmarkExecutor.clear_results();
            return true;
        }
        return false;
    };

    self.close_window = function() {
        if (self.is_running()){
            if(self.cancel_run()) {
                window.close();
            }
        } else {
            window.close();
        }
    };

    self.minimize_window = function() {
        var win = nw.Window.get();
        win.minimize();
    };

    self.on_resize_window = function(width, height) {
        const titlebar_height = $('#titlebar').height();
        const footer_height = $('#footer').height();

        const panels_height = height - titlebar_height - footer_height - 10;
        $('#panels').height(panels_height);
    };

    $(function() {
        const win = nw.Window.get();
        win.on('resize', self.on_resize_window);
        self.on_resize_window(win.width, win.height);
    });

    $(function() {
        // fill system informations section
        self.show_system_infos();

        // fill help section
        self.show_help();

        // add button events
        $('#run-btn').click(self.start_run);
        $('#cancel-btn').click(self.cancel_run);
        $("#close-btn").click(self.close_window);
        $("#minimize-btn").click(self.minimize_window);
        $("#configure-btn").click(function(){
            // show configuration window
            $("#app_confdiv").toggle("medium");
        });

        Gui.create_categories();
    });

    // config file events
    $(function(){
        $("#config-file").on('change', function(){
            const fs = require("fs");
            //fs.readFile($("#config-file").val(),'UTF-8',function(err,data){$("#chooseFileContent").html(data)})
        });

        $("#config-load-btn").click(function(){
            const success = Configure.load_benchmarks($("#config-file").val());
            if (!success)  {
                alert("ConfigFile Format Error");
                return false;
            } else{
                Gui.create_categories();
                $("#app_confdiv").toggle('fast');
            }
        });

        $("#config-reset-btn").click(function(){
            if (confirm("This will overwrite all the settings")){
                Configure.init();
                Gui.create_categories();
                $("#app_confdiv").toggle('fast');
            }
        });
    });

    return self;
});
