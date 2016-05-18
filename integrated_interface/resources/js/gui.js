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
    var self = {};

    self.show_system_infos = function() {
        const renderer = require('jsrender');
        const system = Configure.system_infos();

        var template = renderer.templates("./resources/templates/system/informations.html");
        var html = template.render(system);
        $("#sysInfoBody").html(html);
    };

    self.show_help = function() {
        try{
            var helpFile = Configure.load_json(Configure.app().HelpFilePath);
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
        $("#ConfigFormTabs").append(tabbox_tab.render(tabbox));

        var tabbox_panel = renderer.templates("./resources/templates/benchmarks/tabbox_panel.html");
        $("#ConfigFormTabContent").append(tabbox_panel.render(tabbox));
    };

    var create_new_panel = function(panel) {
        const renderer = require('jsrender');

        var tabbox_stripe = renderer.templates("./resources/templates/benchmarks/tabbox_stripe.html");
        $("#" + panel.id + "TabContent").append(tabbox_stripe.render({
            'color': panel.color,
            'stripe_id': panel.new_id + "TabContent"
        }));
    };

    self.create_categories = function() {
        const renderer = require('jsrender');
        const path = require('path');

        var colorBase=[ "default", "primary", "info", "danger", "warning", "success" ];
        var colorF=["#777", "#428bca","#5bc0de", "#d9534f","#f0ad4e","#5cb85c"];

        var tabbox = renderer.templates("./resources/templates/benchmarks/tabbox.html");
        $("#ConfigForm").html(tabbox.render());

        var levelList = []
        var levelColorList = []
        var color = []
        var count = 0

        for (var benchmark_id in Configure.benchmarks()){
            const benchmark = Configure.benchmark(benchmark_id);

            var ctg = benchmark.categories
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
                'title': benchmark.categories.slice(1).join(': '),
                'text_color': text_color,
                'tab_panel_id': panel_id,
                'benchmark': benchmark_id,
                'benchmark_command': benchmark.command
            }));

            // Add the help of a benchmark.
            var tabbox_panel_help_content = renderer.templates("./resources/templates/benchmarks/tabbox_panel_help_content.html");
            var help_content = "No Help Information available";
            try{
                const help_file = benchmark.help_file;
                const helpFile = require(path.resolve(help_file));
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

    self.add_result = function(current_process){
        const renderer = require('jsrender');

        const row_id = current_process.queue_id + 1;
        const benchmark_name = current_process.benchmark_name;

        if(current_process.state === 'SUCCESS'){
            const result_success = renderer.templates('./resources/templates/results/result_success.html');
            $('#result_table').append(result_success.render({
                row_id: row_id,
                benchmark_name: benchmark_name,
                message: 'complete',
                runtime: format_secs(current_process.runtime)
            }));
        } else if (current_process.state === 'CANCELED') {
            const result_aborted = renderer.templates('./resources/templates/results/result_aborted.html');
            $('#result_table').append(result_aborted.render());
        } else {
            const result_failure = renderer.templates('./resources/templates/results/result_failure.html');
            $('#result_table').append(result_failure.render({
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

        $("#ConfigForm").collapse('hide');
        $('#progressbar_text').html("<p><b><h6 style='color: #428bca; margin: 0px; padding: 0px'>Initializing...</h6></b></p>");
        $('#run-btn').prop({"disabled":"disabled"});

        eventEmitter.on(_EVENTS.CANCEL, self.reenable_run_btn);
        eventEmitter.on(_EVENTS.DONE, self.reenable_run_btn);

        var date = new Date();

        // disable old website save buttons
        $('.btn-save-website').attr('disabled', 'disabled');

        var result_start_run = renderer.templates('./resources/templates/results/result_start_run.html');
        $('#result_table').append(result_start_run.render({
            start_date: date.toISOString()
        }));

        $('.btn-save-results:last').click(function(){
            const benchmark_queue = $(this).data('benchmark_queue');

            $('#save-results-filechooser')
                .unbind('change')
                .change(function() {
                    const filename = $(this).val();
                    Exporter.save_results(filename, benchmark_queue);
                })
                .trigger('click');
        });

        eventEmitter.once(_EVENTS.DONE, function() {
            const benchmark_queue = BenchmarkExecutor.benchmark_queue();
            $('.btn-save-results:last')
                .data('benchmark_queue', benchmark_queue)
                .removeAttr('disabled');
        });

        BenchmarkExecutor.run(Gui.add_result);
    };

    eventEmitter.on(_EVENTS.DONE, function() {
        const renderer = require('jsrender');
        const queue = BenchmarkExecutor.benchmark_queue();

        var result_summary = renderer.templates('./resources/templates/results/result_summary.html');
        $('#result_table').append(result_summary.render({
            runtime: format_secs(queue.total_runtime())
        }));
    });

    self.cancel_run = function() {
        if (_SIGNAL.RUN && confirm("This will terminate running process")){
            BenchmarkExecutor.cancel();
            removeEventListener(_EVENTS.CANCEL, self.reenable_run_btn);
            removeEventListener(_EVENTS.DONE, self.reenable_run_btn);
            BenchmarkExecutor.clear_results();
            return true;
        }
        return false;
    };

    self.close_window = function() {
        if (_SIGNAL.RUN){
            if(self.cancel_run()) {
                window.close();
            }
        } else {
            if (confirm("Quit the program?")) {
                window.close();
            }
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
