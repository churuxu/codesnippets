var stun = require("stun");

stun.request('stun.l.google.com:19302', (err, res) => {
  if (err) {
    console.error(err);
	progress.exit(1);
  } else {    
    console.log(res.getXorAddress().address);
  }
});


