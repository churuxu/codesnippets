const cp = require("child_process");
const fs = require("fs");



//生成私钥文件
function generate_key(file){
	console.log(`generate key ${file} ...`);
	cp.execSync(`openssl genrsa -out ${file} 2048`);
}

//生成证书请求文件
function generate_csr(key, file, subject){
	console.log(`generate csr ${file} ...`);
	cp.execSync(`openssl req -new -key ${key} -out ${file} -subj ${subject}`);
}

//生成根证书文件
function generate_ca_cert(key, csr, file){
	console.log(`generate ca cert ${file} ...`);
	cp.execSync(`openssl x509 -req -days 365 -signkey ${key} -in ${csr} -out ${file}`);
}

//生成证书文件
function generate_cert(csr, file, parentkey, parentcert){
	console.log(`generate cert ${file} ...`);
	cp.execSync(`openssl x509 -req -days 365 -in ${csr} -CA ${parentcert} -CAkey ${parentkey} -CAcreateserial -out ${file}`);	
}

//生成证书文件
function generate_pki(key, cert, file, pwd){
	console.log(`generate pki cert ${file} ...`);
	var opt = {};
	opt.input = pwd + "\n" + pwd + "\n";
	cp.execSync(`openssl pkcs12 -export -inkey ${key} -in ${cert} -out ${file} `, opt);	
}


//单步生成证书
function make_cert(parentname, name, subject){
	generate_key(`${name}.pem`);
	generate_csr(`${name}.pem`, `${name}.csr`, subject);
	generate_cert(`${name}.csr`, `${name}.crt`, `${parentname}.pem`, `${parentname}.crt`);
	generate_pki(`${name}.pem`, `${name}.crt`, `${name}.p12`, "123456");
}

//单步生成根证书
function make_ca_cert(name, subject){
	generate_key(`${name}.pem`);
	generate_csr(`${name}.pem`, `${name}.csr`, subject);
	generate_ca_cert(`${name}.pem`,`${name}.csr`, `${name}.crt`);
    //fs.writeFileSync(`${name}.srl`, "");
}



var outdir_ = "certs";
if(!fs.existsSync(outdir_))fs.mkdirSync(outdir_);

if(!fs.existsSync(`${outdir_}/ca.crt`)){
	make_ca_cert(`${outdir_}/ca`, "/C=CN/CN=myCa");
}
make_cert(`${outdir_}/ca`, `${outdir_}/server`, "/C=CN/CN=localhost");
make_cert(`${outdir_}/ca`, `${outdir_}/client`, "/C=CN/CN=client");



