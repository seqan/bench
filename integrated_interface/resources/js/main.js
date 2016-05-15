nw.Window.open('./resources/index.html', {}, function(new_win) {
    new_win.window.document.addEventListener("DOMContentLoaded", function(event) {
        const renderer = require('jsrender');
        var tmpl = renderer.templates('./resources/templates/gui/body.html');
        new_win.window.document.body.innerHTML = tmpl.render();
    });
});
