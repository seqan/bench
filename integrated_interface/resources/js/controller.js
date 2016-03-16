var path = require("path")
var process = require('process')
var events = require('events')

var eventEmitter = new events.EventEmitter()
var _GENERAL_CONFIG_FILE = path.resolve("./config/gconfig.json")

var _Total_Time = 0
var Process = {
    init: function(){
        var TotalTime = 0
        var RunTime = 0
        var Pid = ""
        var funcs = {}
        funcs.getTotalTime = function(){
            return TotalTime
        }
        funcs.setTotalTime = function(time){
            TotalTime = time
        }
        funcs.getRunTime = function(){
            return RunTime
        }
        funcs.setRunTime = function(time){
            RunTime = time
        }
        funcs.getPid = function(){
            return Pid
        }
        funcs.setPid = function(pid){
            Pid = pid
        }
        return funcs
    }
}

var _KEY = {
    _primary:[1,2,4,5],
    _key:[
    "name",
    "runCmd",
    "category",
    "programName",
    "checked",
    "time",
    "argv",
    "infoFile"    
    ],
    checkPrimary: function(opt){
        if (typeof opt == 'object'){
            for (var k = 0; k < this._primary.length; k++){
                if (!(this._key[this._primary[k]] in opt)){
                    return false
                }                    
            }
        }
        else
            return false
        return true   
    }   
}

var _SIGNAL = {
    "CONF_CHANGE":false,
    "RUN":false,
    "INIT": false,
    "_LOCK": 0,
    "CANCEL": false,
    "NORM": true,
    "DONE": false
}

var _EVENTS = {
    "INIT": 'INIT',
    "BLOCKED": 'BLOCKED',
    "CANCEL":'CANCEL',
    "FAILED": 'FAILED'
}

var GenericOptions = {
    init: function(){
        try{
            var options = require(_GENERAL_CONFIG_FILE)
        }
        catch(err){
            return false
        }
        if (! ("ConfFilePath" in options))
            return false
            
        var funcs = {}
        funcs.getConfFile = function(){
            if ("ConfFilePath" in options)
                return options.ConfFilePath
            else
                return false
        }
        funcs.setConfFile = function(ConfFilPath){
            options.ConfFilePath = ConfFilePath
        }
        funcs.getServer = function(){
            if ("Server" in options)
                return options.Server
            else
                return false
        }
        funcs.setServer = function(server){
            options.Server = server
        }
        funcs.getHelpFilePath = function(){
            if ("HelpFilePath" in options)
                return path.resolve(options.HelpFilePath)
            else
                return false
        }
        funcs.setHelpFilePath = function(path){
            options.HelpFilePath = path 
        }
        return funcs
    }  
}

function waitExec(signal, callback){
    if (_SIGNAL[signal])
        callback()
    else{
        setTimeout(function(){waitExec(signal, callback)}, 5)
    }
}

function _loadJson(file, TAG){
    if (TAG == "JQuery")
    {
        var obj = {}
        $.getJSON(file, function(data){
            $.each(data, function(key,val){
                obj[key] = val
            })
        })
    }
    else
        obj = require(file)
    return obj        
} 

var Options = {
    init: function(_path){
        var options = {}
        try{
            $.getJSON(path.resolve(_path), function(data){
                $.each(data, function(key,val){
                   options[key] = val
                })
                for (var prg in options){
                    if (typeof options[prg] == 'object'){
                        if (! _KEY.checkPrimary(options[prg]))
                            return false
                    }
                    else
                    return false            
                }         
                _SIGNAL.INIT = true 
            })           
        }
        catch(err){
            return err
        }
         
        var funcs = {}
        funcs.getName = function(prg){
            if ("name" in options[prg])
                return options[prg].name
            else
                return false
        }
        funcs.getCategory = function(prg){
            if ("category" in options[prg])
                return options[prg].category
            else
                return false
        }   
        funcs.getInfoFile = function(prg){
            if ("infoFile" in options[prg])
                return options[prg].infoFile
            else
                return false
        }
        funcs.getChecked = function(prg){
            if ("checked" in options[prg])
                return options[prg].checked
            else 
                return false
        }
        funcs.getRunCmd = function(prg){
            if ("runCmd" in options[prg])
                return options[prg].runCmd  
            else
                return false
        } 
        funcs.getRunCmdPrg = function(prg){
            if ("runCmd" in options[prg])
                return options[prg].runCmd.split(" ")[0]
            else
                return false
        }
        funcs.getRunCmdArgv = function(prg){
            if ("runCmd" in options[prg]){
                var runCmdArgv = ""
                var tmp = options[prg].runCmd.split(" ")
                for (k = 1; k < tmp.length; k++)
                    runCmdArgv += tmp[k] + " "
                return runCmdArgv
            }
            else
                return false
        }
        funcs.getTime = function(prg){
            if ("time" in options[prg])
                return options[prg].time
            else
                return false
        }
        funcs.getConfig = function(){
            try{
                return options
            }
            catch(err){
                return err
            }
        }
        funcs.setName = function(prg, name){ //string
            options[prg].name = name
        }
        funcs.setCategory = function(prg, category){ //[]
            options[prg].category = category
        }
        funcs.setInfoFile = function(prg, infoFile){ //string
            options[prg].infoFile = infoFile
        }
        funcs.setChecked = function(prg, checked){ //bool
            options[prg].checked = checked
        }
        funcs.setRunCmd = function(prg, runCmd){ //string
            options[prg].runCmd = runCmd 
        }
        funcs.setTime = function(prg, time){ //float second
            options[prg].time = time
        }
        return funcs
    }
}

function Init(){
    _SIGNAL.INIT = false
    try{
        GCmp = GenericOptions.init()
    }
    catch(err){
        return err
    }
    if (GCmp != false){
        try{
            cmp = Options.init(GCmp.getConfFile())
        }
        catch(err){
            return err
        }
    }
    waitExec("INIT", function(){
        if (cmp != false)
            return true
    })    
}

function _formatCmd(prgName, flag){
    var path = require("path")
    var argv = cmp.getArgv(prgName);
    var argvSpawn = []; 
    for (var i in argv)
    {
        if(argv[i].cmdKey != "")
            argvSpawn.push(argv[i].cmdKey);
        for (var j in argv[i].value)       
            if(argv[i].type == "path")
                argvSpawn.push(argv[i].value[j])
            else 
                argvSpawn.push(argv[i].value[j]);
    }
    return argvSpawn
}

function _2SpawnCmd(cmd){   
    var tmp = cmd.split(" ")
    var spawnCmd = {"prg":tmp[0], "arg":[]}
    for (var k = 1; k < tmp.length; k++){
        spawnCmd.arg[k - 1] = tmp[k]
    }    
    return spawnCmd    
}

function _2SpawnArgv(argv){   
    var tmp = argv.split(" ")
    var spawnArgv=[]
    for (var k = 0; k < tmp.length; k++)
        spawnArgv[k] = tmp[k]    
    return spawnArgv    
}

function _runEach(opts,funcs){
    var path = require("path")
    var spawn = require('child_process').spawn
    var time
    var free
    var errMsg
    
    var _onCloseFunc = function(){
        opts.runtime = process.hrtime(time)
        funcs(errMsg)   
        if(++opts.count < opts.list.length){
            _run(opts,funcs)
        }
        else{
            _SIGNAL.DONE = true
            eventEmitter.emit(_EVENTS.DONE)
            _SIGNAL.RUN = false
        }
    }
    var _run = function(opts, funcs){
        if (_SIGNAL.CANCEL)
            return _EVENTS.CANCEL
        var spawnCmd = _2SpawnCmd(opts.cmd[opts.count])
        var spawnArgv = _2SpawnArgv(opts.cmd[opts.count]) 
        if (require('os').platform() == 'linux')
        try{
            require('fs').chmodSync(spawnCmd.prg, 764)
        }
        catch(err){
            _SIGNAL.NORM = false
            errMsg = err
        } 
        time=process.hrtime()
        try{
        free = spawn(spawnCmd.prg, spawnCmd.arg, {detached: true})
        }
        catch(err){
            alert("error")
        }
        proc.setPid(free.pid)
        _SIGNAL.NORM = true    
        free.on('close', _onCloseFunc)
        free.stderr.on('error', function(){
            _SIGNAL.NORM = false
            errMsg = error 
            opts.st[opts.count] = false
            eventEmitter.emit(_EVENTS.FAILED)
        })
        free.on('error', function(){
            _SIGNAL.NORM = false 
            errMsg = "Starting program failed"
            eventEmitter.emit(_EVENTS.FAILED)
        })
    } 
    
    _run(opts, funcs)
}

function run(opts,funcs){
    proc = Process.init()
    _Total_Time = 0;
    _SIGNAL.CANCEL = false
    _SIGNAL.RUN = true
    _SIGNAL.DONE = false
    for (prg in cmp.getConfig()){
        if(cmp.getChecked(prg))
        {
            opts.list.push(prg);
            opts.st.push(true);
            opts.cmd.push(cmp.getRunCmd(prg));
            opts.time.push(cmp.getTime(prg));
            _Total_Time += cmp.getTime(prg)
        }
    }
    opts.weight[0] = opts.time[0] / _Total_Time;
    for (var k = 1; k < opts.time.length; k++){
        opts.weight[k] = opts.time[k] / _Total_Time + opts.weight[k - 1]
    }
    
    _runEach(opts, funcs)
}

function cancel(){
    try{
        process.kill("-" + proc.getPid())
    }
    catch(err){
        return err
    }
    eventEmitter.emit(_EVENTS.CANCEL)
    _SIGNAL.CANCEL = true
    _SIGNAL.RUN = false
    _SIGNAL.NORM = false
    return _EVENTS.CANCEL
}

function sendResult(_url, _data){
    $.ajax({
        method: "POST",
        url: _url,
        data: _data
    })
        .done(function(msg) {
        alert(mst)
    })
}
