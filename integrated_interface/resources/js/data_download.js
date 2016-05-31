;(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.DataDownload = factory();
    }

})(this, function() {
    "use strict";

    const DataDownloader = require('./modules/data_downloader');
    var self = {};

    const renderer = require('jsrender');
    const download_modal = renderer.templates("./resources/templates/data_download/modal.html");
    $("body").append(download_modal.render());

    var total_size;

    const progressStyle = function(current, next) {
        $(current + ' .media-left span')
            .removeClass('glyphicon-minus')
            .addClass('glyphicon-ok');
        $(current + ' .media-heading')
            .removeClass('alert-info')
            .addClass('alert-success');

        if (!next) return;

        $(next + ' .media-heading')
            .addClass('alert-info');
    };

    DataDownloader.on('initialize', () => {
        $('#data-download-close-btn').hide();
        $('#data-download').modal({
            backdrop: 'static',
            keyboard: false
        }).show();
    }).on('progress', (state) => {
        const pretty = require('prettysize');
        total_size = state.size.total;

        $('#data-download .progress-bar').css({
            width: (state.percentage*100) + "%"
        });
        $('#data-download .progress-bar span').html(
            pretty(state.size.transferred) + ' / ' + pretty(state.size.total)
        );
        $('#data-download-speed').html(pretty(state.speed) + '/s');
        $('#data-download-rem-time').html(state.time.remaining + 's');
    }).on('downloaded', () => {
        const pretty = require('prettysize');
        progressStyle('#data-download-download', '#data-download-extract');

        $('#data-download .progress-bar')
            .css({width: (100) + "%"})
            .parent()
            .removeClass('active');

        $('#data-download .progress-bar span').html(
            pretty(total_size) + ' / ' + pretty(total_size)
        );

        $('#data-download-speed').html('');
        $('#data-download-rem-time').html('');
    }).on('extracted', () => {
        progressStyle('#data-download-extract', '#data-download-preprocess');
    }).on('index_created', () => {
        progressStyle('#data-download-preprocess');
    }).on('done', () => {
        $('#data-download-close-btn').show();
    }).on('stdout', (chunk) => {
        $('#data-download-stdout').show();
        $('#data-download-stdout pre')[0].innerHTML += "" + chunk;
    }).on('stderr', (chunk) => {
        $('#data-download-stderr').show();
        $('#data-download-stderr pre')[0].innerHTML += "" + chunk;
    }).on('error', (error) => {
        alert(error);
    });

    DataDownloader.rebuild();

    return self;
});
