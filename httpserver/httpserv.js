/*
Node http 服务模块

httpserv.js  -- 主模块
api   -- 业务处理模块
其它则按静态文件返回

业务处理模块接口：
exports.requestListener = function(req, resp){}


*/


const http = require('http');
const https = require('https');
const url = require('url');
const fs = require('fs');
const util = require('util');
const path = require('path');
const querystring = require('querystring');

const getFileStat = util.promisify(fs.stat);

const mimeType = {
	"bin": "application/octet-stream",
    "css": "text/css",
    "gif": "image/gif",
    "html": "text/html",
    "ico": "image/x-icon",
    "jpeg": "image/jpeg",
    "jpg": "image/jpeg",
    "js": "text/javascript",
    "json": "application/json",
    "pdf": "application/pdf",
    "png": "image/png",
    "svg": "image/svg+xml",    
    "tiff": "image/tiff",
    "txt": "text/plain",
    "wav": "audio/x-wav",
    "wma": "audio/x-ms-wma",
    "wmv": "video/x-ms-wmv",
    "xml": "text/xml"	
};


const httpErrorMessage = {
	404: "Not Found",
	500: "Internal Server Error"
};


var port_ = 80;
var root_ = __dirname;

process.argv.forEach((val, index) => {
	if(index==2)port_ = val;
 });

var httpProvider = http;

//回复http错误
function sendErrorResponse(res, code, detail){
	var msg = httpErrorMessage[code];
	if(!msg){
		code = 500;
		msg = "Internal Server Error";
	}
	var body = {};
	body.ok = false;
	body.error = code;
	body.msg = msg;
	if(detail)body.detail = detail;
	res.statusCode = code;
	res.statusMessage = msg;
	res.end(JSON.stringify(body));	
}



//字符串是否以xxx开头
String.prototype.startWith = function(b){
	var a = this;
	if(!a || !b || a.length ==0 || b.length==0)return false;
	if(a.substr(0, b.length) == b){  
		return true;
	}
	return false;   
}


//目录是否可访问
function isPathAccessable(pathname){
	var lcpath = pathname.toLowerCase();	
	var lcpathr = lcpath.replace('\\', '/');
	if(lcpathr.startWith("/api/"))return false;  //防止获取业务代码js文件	
	if(lcpathr.startWith("/./"))return false;  //防止 /./api/xxxx.js 获取业务代码js文件	
	if(lcpathr.startWith("/node_module/"))return false; //防止获取node_module
	if(lcpathr.indexOf("/../") >0 )return false; //防止获取父目录
	return true;
}

//处理api调用请求
function handleApiRequest(req, res){
	var subpath = req.pathname;	
	var jsname = "./" + subpath;
	var mod = null;
	try{
		mod = require(jsname);		
	}catch(e){
		sendErrorResponse(res, 404);		
		return ;		
	}
	try{
		mod.requestListener(req, res);	
	}catch(e){
		sendErrorResponse(res, 500, e.stack);		
		return ;		
	}
}


//处理文件请求
async function handleFileRequest(req, res){
	let pathname = req.pathname;
	try{
		//文件路径
		let fullpath = root_ + pathname;

		//获取文件状态		
		let stats = await getFileStat(fullpath);
		if(stats.isDirectory()){
			fullpath = fullpath + "/index.html";
			stats = await getFileStat(fullpath);
		}

		//获取mimeType
    	let ext = path.extname(fullpath);
		ext = ext ? ext.slice(1) : 'bin';	
		let mime = mimeType[ext]; 
		if(!mime){
			mime = "application/octet-stream";
		}		

		//设置http resp
		res.statusCode = 200;
		res.setHeader('Content-Type', mime);
		res.setHeader('Content-Length', stats.size);
		
		//读取文件
		let stream = fs.createReadStream(fullpath);
		stream.on('error', function(e){
			sendErrorResponse(res, 404);				
		});		
		stream.pipe(res);
		
	}catch(e){
		sendErrorResponse(res, 404);	
	}
}


//创建服务器入口
var server = httpProvider.createServer((req, res) => {	   
    var obj = url.parse(req.url);
	var pathname = obj.pathname;
	req.pathname = pathname;
	req.query =  querystring.parse(obj.query);
	if(pathname.startWith("/api/")){
		handleApiRequest(req, res);
		return;
	}
	if(!isPathAccessable(pathname)){
		sendErrorResponse(res, 404);
		return ;
	}	
	handleFileRequest(req, res);	
});


server.listen(port_);
console.log("http server started, port=" + port_ + " root=" + root_);







