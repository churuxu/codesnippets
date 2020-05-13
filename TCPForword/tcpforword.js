var net = require("net");
var fs = require("fs");


var configdata = fs.readFileSync("config.json");
var config = JSON.parse(configdata.toString());

var logfile = fs.createWriteStream("log.txt");

var id = 0;

Date.prototype.format = function (fmt) {  
    var o = {
        "M+": this.getMonth() + 1, 
        "d+": this.getDate(), 
        "h+": this.getHours(),  
        "m+": this.getMinutes(),  
        "s+": this.getSeconds(), 
        "q+": Math.floor((this.getMonth() + 3) / 3), 
        "S": this.getMilliseconds()  
    };
    if (/(y+)/.test(fmt)) fmt = fmt.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
    for (var k in o)
    if (new RegExp("(" + k + ")").test(fmt)) fmt = fmt.replace(RegExp.$1, (RegExp.$1.length == 1) ? (o[k]) : (("00" + o[k]).substr(("" + o[k]).length)));
    return fmt;
}


function log(id, msg, data){
	var datestr = new Date().format("hh:mm:ss");
	var datastr = "";
	if(data)datastr = data.toString('hex');
	var logstr = `${datestr} #${id} ${msg} ${datastr}`;
	console.log(logstr);
	logfile.write(logstr);
}


function on_accept(in_con){
	id ++;
	in_con.id = id;
	log(id, "ACCEPT");
	var out_con = net.createConnection(config.forward);
	out_con.on('end', () => {
		log(in_con.id, "CLOSE");
		in_con.end();
	});
	in_con.on('end', () => {
		log(in_con.id, "CLOSE");
		out_con.end();
	});	
	out_con.on('error', (e) => {
		log(in_con.id, "ERROR");
		in_con.end();
	});	
	in_con.on('error', (e) => {
		log(in_con.id, "ERROR");
		out_con.end();
	});	
	in_con.on('data', (data) => {
		log(in_con.id, "RECV", data);
		out_con.write(data);
	});	
	out_con.on('data', (data) => {
		log(in_con.id, "SEND", data);
		in_con.write(data);
	});
}



var server = net.createServer(on_accept);

server.listen(config.listen);

console.log("tcp listen " + config.listen.port + " foword " + config.forward.host + ":"+ config.forward.port);







