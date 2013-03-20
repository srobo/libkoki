var fs = require("fs");
var koki = require("./build/Release/koki");

console.log("Reading " + process.argv[2]);
var f = fs.openSync(process.argv[2], 'r');
var file_size = fs.fstatSync(f).size;
var b = Buffer(file_size);
fs.readSync(f, b, 0, file_size, 0);

var markers = koki.findMarkers(b);
console.log(markers);
