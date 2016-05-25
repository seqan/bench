(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.ProjectGui = factory();
    }

})(this, function() {
    "use strict";

    const Configure = require('./modules/configure');

    var self = {};

    self.show_project_infos = function() {
        const renderer = require('jsrender');
        const project = Configure.project_infos();

        var template = renderer.templates("./resources/templates/project/informations.html");
        var html = template.render(project);
        $("#project-panel-body").html(html);
    };

    self.on_save = function(new_title) {
        const project = Configure.project_infos();
        project.title = new_title;
    };

    self.add_events = function() {

        var save_btn = $("#project-title-save-btn");
        var save_icon = $("#project-title-save-btn span");

        // change button to unsaved
        var save_red = function() {
            save_icon
                .removeClass('glyphicon-floppy-saved')
                .addClass('glyphicon-floppy-disk')
                .css('color', '#d9534f');
        };

        // change button to saved
        var save_green = function() {
            save_icon
                .addClass('glyphicon-floppy-saved')
                .removeClass('glyphicon-floppy-disk')
                .css('color', 'green');
        };

        // Content modified?
        var input = $("#project-title");
        input.data('input-default', input.val());
        input.bind("propertychange change click keyup input paste", function(event){
            var input_default = $(this).data('input-default');
            if (input_default == input.val()) {
                save_green();
            } else {
                save_red();
            }
        });

        // Pressing the save-button, changes it to green.
        save_btn.click(function(){
            var new_value = input.val();
            self.on_save(new_value);
            input.data('input-default', new_value);
            save_green();
        });
    };

    $(function() {
        // fill project informations section
        self.show_project_infos();
        self.add_events();
    })

    return self;
});
