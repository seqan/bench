function g_showSysInfo(){
    os=require("os")    
    var mem=Math.floor(os.totalmem()/1024/1024/1024*10)/10
    $("#sysInfoBody").html("<table class='table table-hover' style='width:100%;margin-bottom:3px; padding:0; box-shadow:1px 5px 10px #888888'></tr><tr><td><h6>Opearting system:</h6</td><td><h6>"+os.platform()+" "+os.arch()+" "+os.release()+"</h6></td></tr><tr><td><h6>Memory:</h6></td><td><h6>" + mem + " GB</h6></td></tr><tr><td><h6>CPU:</h6></td><td><h6>"+os.cpus()[0].model+"<h6></td></tr><tr><td><h6>Threads:</h6></td><td><h6>" + os.cpus().length + "</h6></td></tr></tbody></table>");
}

function g_createCategory(){
    var colorBase=[ "default", "primary", "info", "danger", "warning", "success" ]
    var colorF=["#777", "#428bca","#5bc0de", "#d9534f","#f0ad4e","#5cb85c"]
    $("#ConfigForm").html($("<ul id = 'ConfigFormTabs' class='nav nav-tabs' role='tablist'></ul>"))
    $("#ConfigForm").append($("<div id = 'ConfigFormTabContent' class='tab-content' style='height:400px'><div>"))
    var levelList = []
    var levelColorList = []
    var color = []
    var count = 0
    for (prg in cmp.getConfig()){
        var ctg = cmp.getCategory(prg)
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
            var tabId = ctg[0] + "Tab"
            var tabContentId = ctg[0] + "TabContent"
            if (count == 1){
                $("#ConfigFormTabs").append($("<li role='presentation' id =" + tabId + " class='active'><a href='#" + tabContentId + "' aria-controls=" + tabContentId + " role='tab' data-toggle='tab'><h5 style='margin: 0px'>" + ctg[0] + "</h5></a></li>"))
                $("#ConfigFormTabContent").append($("<div role='tabpanel' class='tab-pane in active' id=" + tabContentId + " style='overflow: auto; margin-top:0px; height:400px;'></div>"))

            }
            else{
                $("#ConfigFormTabs").append($("<li role='presentation' id =" + tabId + "><a href='#" + tabContentId + "' aria-controls=" + tabContentId + " role='tab' data-toggle='tab'><h5 style='margin: 0px'>" + ctg[0] + "</h5></a></li>"))
                $("#ConfigFormTabContent").append($("<div role='tabpanel' class='tab-pane' id=" + tabContentId + " style='overflow: auto; margin-top:0px; height:400px'></div>"))
            }
        }
        
        id = ctg[0]
        color.push([0])
        for (var k = 1; k < ctg.length; k++){ 
            if (k < maxn)
                color[color.length -1].push(color[maxk][k])
            if (k == maxn)
                color[color.length -1].push((color[color.length - 1][k - 1] + buffer.length + 1) % colorBase.length)
            if (k > maxn)
                color[color.length - 1].push((color[color.length - 1][k - 1] + 1) % colorBase.length)
            if (k >= maxn){
            $("#" + id + "TabContent").append($("<div class='bs-callout bs-callout-" + colorBase[color[color.length -1][k]] + "' id=" + id + ctg[k] + "TabContent style='padding-left:0; margin:0'></div>"))
            }
            id += ctg[k]
        }
        var textColor = colorF[color[color.length - 1][ctg.length - 1]]
        var title = ""
        for (k = 1; k < cmp.getCategory(prg).length; k++)
            title += cmp.getCategory(prg)[k] + " "
        $("#" + id + "TabContent").append($("<h6 style='color:" + textColor + "'>&nbsp&nbsp&nbsp <b>" + title + "</b></h6><div class='input-group input-group-sm' style='margin:10px; width:700px; border-radius:0px'><span class='input-group-addon' style='border-radius:0px'><input id='" + id + "Checkbox' prg='" + prg + "' type='checkbox' style='border-radius:0px'></span><span class='input-group-addon'>" + "cmd" + "</span><input type='text' class='form-control' name='gInput' style = 'border-right-style:none' placeholder='>_' id='" + id + "Input' prg='" + prg + "' value='" + cmp.getRunCmd(prg) +"'><span class='input-group-addon' style='border-left-style:none; background-color: white'><button type='button' id='" + id + "Reset' class='btn btn-default btn-xs' prg='" + prg + "' style='height:17px; padding-top:0px; padding-bottom:0px'>Reset</button></span><span class='input-group-btn'><button id='" + id + "Help' class='btn btn-default' data-toggle='collapse' data-target='#" + id + "HelpContent' aria-expanded='false' aria-controls='"+ id +"HelpContent'><span class='glyphicon glyphicon-book' aria-hidden='true' style='color:gray'></span></button><button class='btn btn-default' data-toggle='tooltip' data-placement:'left' title='save' style='border-radius: 0px'><span id='" + id + "Saved' name='saved' prg=" + prg + " class='glyphicon glyphicon-floppy-saved' aria-hidden='true' style='color:green'></span></button></span></div> <div class='collapse' id='" + id +"HelpContent' style='padding-left:15px; margin: 0px; overflow: auto'></div>"))   
        try{
            var helpFile = require(path.resolve(cmp.getInfoFile(prg)))
            $("#" + id + "HelpContent").html("<h6 style='color:" + textColor + "'>" + helpFile.helpContent + "</h6>")
        }
        catch(err){
            $("#" + id + "HelpContent").html("<h6 style='color:" + textColor + "'>No Help Information available</h6>")    
        }
        if (cmp.getChecked(prg))
            $("#" + id + "Checkbox").prop('checked', true)
        var checkCallback = function(){
            if($(this).prop('checked'))
            { 
                cmp.setChecked($(this).attr("prg"), true)
            }
            else
            {
                cmp.setChecked($(this).attr("prg"), false)
            }  
        }
        $("#" + id + "Checkbox").on("click", checkCallback)
        $("#" + id + "Input").change(function(){
            $("[name=saved][prg=" + $(this).attr("prg") + "]").css('color', '#d9534f')  
        })
        $("#" + id + "Reset").click(function(){
            cmp.setRunCmd($(this).attr("prg"), cmpConfig.getRunCmd($(this).attr("prg")))
            $("[name=saved][prg=" + $(this).attr("prg") + "]").css('color', 'green')
            $("[name=gInput][prg=" + $(this).attr("prg") + "]").val(cmp.getRunCmd($(this).attr("prg")))
        })
        $("#" + id + "Saved").click(function(){
            $("[name=saved][prg=" + $(this).attr("prg") + "]").css('color', 'green')  
            //cmp.setRunCmd($(this).attr("prg"), cmp.getRunCmdPrg($(this).attr("prg")) + " " + $("[name=gInput][prg=" + $(this).attr("prg") + "]").val())
            cmp.setRunCmd($(this).attr("prg"), $("[name=gInput][prg=" + $(this).attr("prg") + "]").val())
        })
    }
}

function g_run(){
    $("#ConfigForm").collapse()
    $('#progressbar_text').html("<p><b><h6 style='color: #428bca; margin: 0px; padding: 0px'>Initing...</h6></b></p>")
    $('#run-btn').prop({"disabled":"disabled"})
    enableRunBtn = function(){$("#run-btn").prop("disabled", false)}
    eventEmitter.on(_EVENTS.CANCEL, enableRunBtn)
    eventEmitter.on(_EVENTS.DONE, enableRunBtn)
    var _END = 1
 
    NProgress.start()
    var date = new Date()
    $('#result_table').append("<tr><td colspan='3'><h6><b>" + date.toISOString() + "</b><h6><td></td></tr>")
    var opts={
        'list': [],                   //list of running bench programs
        'st': [],                     //state of running result
        'cmd': [],
        'time': [],
        'count': 0,                   //current running bench program
        'weight': [],      //weight for each program
        'runtime': [],            //running time
        'results': {'time': []}
    }
    var timeSum = 0
    var timePercent = 0
    var statMax = 0
    var stat = 0
    var _P = 1
    var _2Time = function(hrtime, tag){
        if (tag == "sec")
            return hrtime[0] + hrtime[1] * 1e-9
        else
            return hrtime[0] * 1000 + hrtime[1] * 1e-6
    }
       
    var _showResults = function(data){
        timeSum += opts.time[opts.count]
        timePercent = timeSum / _Total_Time
        stat = opts.weight[opts.count]

        NProgress.set(stat)
        if(_SIGNAL.NORM){
       	    if(opts.st[opts.count])
                $('#result_table').append("<tr><td><h6>" + (opts.count+1) + "</h6></td><td><h6>"+opts.list[opts.count]+"</h6></td><td><h6><span class='glyphicon glyphicon-ok' style='color:#5cb85c' aria-hidden='true'></span>&nbsp complete</h6></td><td><h6>" + Math.floor( _2Time(opts.runtime, "sec") * 10000 ) / 10000 + " Sec</h6></td></tr>")
        } 
        else{
            if (_SIGNAL.CANCEL)
                $('#result_table').append("<tr><td colspan='4'><h6>Canceled by users</h6></td></tr>")
            else
                $('#result_table').append("<tr><td><h6>"+(opts.count+1)+"</h6></td><td><h6>"+opts.list[opts.count]+"</h6></td><td><h6><span class='glyphicon glyphicon-remove' style='color:#d9534f' aria-hidden='true'></span>&nbsp " + data + "</h6></td><td><h6>NULL</h6></td></tr>")
        }
        statMax += opts.weight[opts.count]
        NProgress.set(statMax)
    }

    run(opts,_showResults)
    var _deltaTime = 100
    var _str = [".", "..", "..."]
    var _k = 0
    function _showProgress(){
        NProgress.configure({showSpinner: false, parent:'#progressbar'})
        setTimeout(function(){
            if(_SIGNAL.CANCEL){
                NProgress.set(_END)
                $('#progressbar_text').html("<p><h6>&nbsp</h6></p>")
                return _EVENTS.CANCEL
            }
            if (_SIGNAL.DONE){
                NProgress.set(_END);
                $('#progressbar_text').html("<p><h6 style='color: #428bca '><b>complete</b></h6></p>")
                return _EVENTS.DONE
            }
            
            if(stat < opts.weight[opts.count] * _P){
                stat += _deltaTime / _Total_Time / 1000  
                NProgress.set(stat)
            }
            $('#progressbar_text').html("<p><h6 style='color: #428bca '><b> running " + opts.list[opts.count] + " " + Math.floor(stat * 100) + "% " + _str[Math.floor((_k++ * _deltaTime / 1000)) % _str.length]+ "</b></h6></p>" )
            _showProgress()
        }, _deltaTime)
    }
    _showProgress()    
} 

function g_cancel(){
    if(_SIGNAL.RUN){
        if (confirm("This will terminate running process")){
            cancel()
            removeEventListener(_EVENTS.CANCEL, enableRunBtn)
            removeEventListener(_EVENTS.DONE, enableRunBtn)
    
        }
    }
    var fileList = ['~/1.txt','~/2.txt','~/3.txt']
    clearResult(fileList)
}
