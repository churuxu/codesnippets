var https = require('https')
var fs = require("fs");

var options = {
    key: fs.readFileSync(__dirname + '/certs/server.pem'),
    cert: fs.readFileSync(__dirname + '/certs/server.crt')
};


function handle_req(req, resp){
	resp.write("hello world");
	resp.end();
}


var https_serv = https.createServer(options, handle_req);

https_serv.listen(443, function () {
    console.log('Https server started');
});

