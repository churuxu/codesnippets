

exports.requestListener = function(req, res){
	res.write("path:  " + req.pathname);	
	res.write("\nquery: " + JSON.stringify(req.query));
	res.end();
}
