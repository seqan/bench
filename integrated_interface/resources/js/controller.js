const path = require('path')
const pro = require('process')
const events = require('events')
const fs = require('fs')
const os = require('os')

var eventEmitter = new events.EventEmitter()
var _GENERAL_CONFIG_FILE = path.resolve("./config/gconfig.json")
var _RESULT_FILE = path.resolve("/Users/xp/result.json")

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
    "infoFile",
    "runCmdRemark" 
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
    "DONE": false,
    "ERROR": false
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

var _system = {
    "os": os.platform() + " " + os.arch() + " " + os.release(),
    "cpu_name": os.cpus()[0].model,
    "memory": Math.floor(os.totalmem()/1024/1024/1024*100)/100,
    "date": "",
    "threads": os.cpus().length
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
        _SIGNAL.ERROR = false
        try{
            var jqxhr = $.getJSON(path.resolve(_path), function(data){
                $.each(data, function(key,val){
                   options[key] = val
                })
                         
                //_SIGNAL.INIT = true 
            })           
           .fail(function(){
                _SIGNAL.ERROR = true
                return false
            })
           .success(function(){
               for (var prg in options.benchmarks){
                   if (typeof options.benchmarks[prg] == 'object'){
                       if (! _KEY.checkPrimary(options.benchmarks[prg])){
                            _SIGNAL.ERROR = true     
                            return false
                       } 
                   }
                   else{
                       _SIGNAL.ERROR = true 
                       return false            
                    }
               }
            })
           .always(function(){
                _SIGNAL.INIT = true
            })
        }
        catch(err){
            return false 
        }
         
        var funcs = {}
        funcs.getName = function(prg){
            if ("name" in options.benchmarks[prg])
                return options.benchmarks[prg].name
            else
                return false
        }
        funcs.getCategory = function(prg){
            if ("category" in options.benchmarks[prg])
                return options.benchmarks[prg].category
            else
                return false
        }   
        funcs.getInfoFile = function(prg){
            if ("infoFile" in options.benchmarks[prg])
                return options.benchmarks[prg].infoFile
            else
                return false
        }
        funcs.getChecked = function(prg){
            if ("checked" in options.benchmarks[prg])
                return options.benchmarks[prg].checked
            else 
                return false
        }
        funcs.getRunCmd = function(prg){
            if ("runCmd" in options.benchmarks[prg])
                return options.benchmarks[prg].runCmd  
            else
                return false
        } 
        funcs.getRunCmdPrg = function(prg){
            if ("runCmd" in options.benchmarks[prg])
                return options.benchmarks[prg].runCmd.split(" ")[0]
            else
                return false
        }
        funcs.getRunCmdArgv = function(prg){
            if ("runCmd" in options.benchmarks[prg]){
                var runCmdArgv = ""
                var tmp = options.benchmarks[prg].runCmd.split(" ")
                for (k = 1; k < tmp.length; k++)
                    runCmdArgv += tmp[k] + " "
                return runCmdArgv
            }
            else
                return false
        }
        funcs.getTime = function(prg){
            if ("time" in options.benchmarks[prg])
                return options.benchmarks[prg].time
            else
                return false
        }
        funcs.getRunCmdRemark = function(prg){
            if ("runCmdRemark" in options.benchmarks[prg])
                return options.benchmarks[prg].runCmdRemark
            else 
                return false
        }
        funcs.getConfig = function(){
            try{
                return options.benchmarks
            }
            catch(err){
                return err
            }
        }
        funcs.getProjectTitle = function(){
            try{
                return options.project.title
            }
            catch(err){
                return err
            }
        }

        funcs.setName = function(prg, name){ //string
            options.benchmarks[prg].name = name
        }
        funcs.setCategory = function(prg, category){ //[]
            options.benchmarks[prg].category = category
        }
        funcs.setInfoFile = function(prg, infoFile){ //string
            options.benchmarks[prg].infoFile = infoFile
        }
        funcs.setChecked = function(prg, checked){ //bool
            options.benchmarks[prg].checked = checked
        }
        funcs.setRunCmd = function(prg, runCmd){ //string
            options.benchmarks[prg].runCmd = runCmd 
        }
        funcs.setTime = function(prg, time){ //float second
            options.benchmarks[prg].time = time
        }
        funcs.setProjectTitle = function(title){
            options.project.title = title
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
            cmpConfig = Options.init(GCmp.getConfFile())
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
    var spawnCmd = [] 
    var _prgFlag = true
    var count = 0, m = 0

    spawnCmd[0] = {"prg":'', "arg":[]}
    for (var k = 0; k < tmp.length; k++){
        if (tmp[k] == '&&'){
            _prgFlag = true
            spawnCmd[++count] = {"prg":'', "arg":[]}
            m = 0
            continue 
        }
        else
            if (_prgFlag) {
                _prgFlag = false
                spawnCmd[count].prg = tmp[k]
            }
            else
                spawnCmd[count].arg[m++] = tmp[k]
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

var _2Time = function(hrtime, tag){
     if (tag == "sec")
         return hrtime[0] + hrtime[1] * 1e-9
     else
         return hrtime[0] * 1000 + hrtime[1] * 1e-6
 }

function clearResult(file){
    file.forEach(function(filename){
        fs.exists(filename, function(exists){
            if (exists)
                fs.unlink(path.resolve(filename))
        })
    })
}

//function _runEach(opts,funcs){
//    var path = require("path")
//    var spawn = require('child_process').spawn
//    var time
//    var free
//    var errMsg
//    
//    var _onCloseFunc = function(){
//        opts.runtime = pro.hrtime(time)
//        funcs(errMsg)   
//        if(++opts.count < opts.list.length){
//            _run(opts,funcs)
//        }
//        else{
//            _SIGNAL.DONE = true
//            eventEmitter.emit(_EVENTS.DONE)
//            _SIGNAL.RUN = false
//            clearResult(opts.resultFiles)
//        }
//    }
//    var _run = function(opts, funcs){
//        if (_SIGNAL.CANCEL)
//            return _EVENTS.CANCEL
//        var spawnCmd = _2SpawnCmd(opts.cmd[opts.count])
//        var spawnArgv = _2SpawnArgv(opts.cmd[opts.count]) 
//        if (require('os').platform() == 'linux')
//        try{
//            require('fs').chmodSync(spawnCmd.prg, 764)
//        }
//        catch(err){
//            _SIGNAL.NORM = false
//            errMsg = err
//        } 
//        time=pro.hrtime()
//        try{
//            free = spawn(spawnCmd[0].prg, spawnCmd[0].arg, {detached: true})
//        }
//        catch(err){
//            alert(err)
//        }
//        proc.setPid(free.pid)
//        _SIGNAL.NORM = true    
//        free.on('close', _onCloseFunc)
//        free.stderr.on('error', function(){
//            _SIGNAL.NORM = false
//            errMsg = error 
//            opts.st[opts.count] = false
//            eventEmitter.emit(_EVENTS.FAILED)
//        })
//        free.on('error', function(){
//            _SIGNAL.NORM = false 
//            errMsg = "Starting program failed"
//            eventEmitter.emit(_EVENTS.FAILED)
//        })
//    } 
//    
//    _run(opts, funcs)
//}

function _runEach(opts,funcs){
    var path = require("path")
    var spawn = require('child_process').spawn
    var time
    var free
    var errMsg
    var _NEXT = true
    var spawnCmd = []
    var k = 0
    var _onCloseFunc = function(){
        opts.runtime[opts.count][opts.cmdCount] = pro.hrtime(time)
        funcs(errMsg)   
        if (++opts.cmdCount < spawnCmd.length){
            _NEXT = false 
        }
        else{
            opts.cmdCount = 0
            _NEXT = true
            opts.count++ 
        }
        if(opts.count < opts.list.length){
            _run(opts,funcs)
        }
        else{
            _SIGNAL.DONE = true
            eventEmitter.emit(_EVENTS.DONE)
            _SIGNAL.RUN = false
            clearResult(opts.resultFiles)
        }
    }
    
    var _run = function(opts, funcs){
        if (_SIGNAL.CANCEL)
            return _EVENTS.CANCEL
        if (_NEXT){
            spawnCmd = _2SpawnCmd(opts.cmd[opts.count])
            
            //var spawnArgv = _2SpawnArgv(opts.cmd[opts.count]) 
        if (require('os').platform() == 'linux')
            try{
                require('fs').chmodSync(spawnCmd.prg, 764)
            }
            catch(err){
                _SIGNAL.NORM = false
                errMsg = err
            } 
        }
        time=pro.hrtime()
        try{
            free = spawn(spawnCmd[opts.cmdCount].prg, spawnCmd[opts.cmdCount].arg, {detached: true})
            //free = spawn('ls')
        }
        catch(err){
            errMsg += '\n spawnError: ' + err
            alert(err)
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

function run(opts, funcs){
    proc = Process.init()
    _Total_Time = 0;
    _SIGNAL.CANCEL = false
    _SIGNAL.RUN = true
    _SIGNAL.DONE = false
    for (var prg in cmp.getConfig()){
        if(cmp.getChecked(prg))
        {
            opts.list.push(prg);
            opts.st.push(true);
            opts.cmd.push(cmp.getRunCmd(prg))
            opts.time.push(cmp.getTime(prg))
            opts.cmdRemark.push(cmp.getRunCmdRemark(prg))
            opts.runtime.push([])
            _Total_Time += cmp.getTime(prg)
        }
    }
    opts.weight[0] = opts.time[0] / _Total_Time;
    for (var k = 1; k < opts.time.length; k++){
        opts.weight[k] = opts.time[k] / _Total_Time + opts.weight[k - 1]
    }
    
            
    var results = {
            "information":{
                "system": _system,
                "project": {
                    "title": "seqan-2.1-gcc"
                }
            }, 
            'results':{}
        }
    eventEmitter.on(_EVENTS.DONE, function(){
        var _date = new Date()
        results.information.system.date = _date.toISOString()
        for (var k = 0; k < opts.cmd.length; k++ ){
            results.results[opts.list[k]] = {}
            for (var j = 0; j < opts.cmdRemark[k].length; j++)
            {
                results.results[opts.list[k]][opts.cmdRemark[k][j]] = {'time': [_2Time(opts.runtime[k][j], "sec")]}
            }
        }
    fs.writeFile(_RESULT_FILE, JSON.stringify(results))
    })
    eventEmitter.on(_EVENTS.CANCEL, function(){
    })

    _runEach(opts, funcs)

}

function cancel(){
    try{
        pro.kill("-" + proc.getPid())
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
