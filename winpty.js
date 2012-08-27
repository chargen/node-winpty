var fs = require('fs'),
    path = require('path');

var bindings = path.resolve(__dirname+'/build/Release/winpty.node');
if (!fs.existsSync(bindings)) {
  bindings = path.resolve(__dirname+'/compiled/'+process.arch+'/winpty.node');
  if (!fs.existsSync(bindings)) {
    throw new Error('winpty_bindings.node node found');
  }
}
var bindings = require(bindings);
var WinPTY = bindings.WinPTY;



function wstring(string){
  var buff = new Buffer(string.length * 2 + 2);
  buff.write(string + '\0', 0, 'ucs2');
  return buff;
}


function EnvPair(k, v){
  this.key = k;
  this.val = v;
  var str = k+'='+v+'\0';
  this.bytes = Buffer.byteLength(str) * 2;
  this.buffer = new Buffer(this.bytes);
  this.buffer.write(str, 0, 'ucs2');
}

EnvPair.prototype = {
  copy: function copy(buffer){
    buffer.cursor || (buffer.cursor = 0);
    this.buffer.copy(buffer, buffer.cursor);
    buffer.cursor += this.bytes;
  }
};

bindings.formatEnv = function formatEnv(obj){
  var pairs = [],
      chars = 0;

  if (!obj) obj = process.env;

  for (var k in obj) {
    var pair = new EnvPair(k, obj[k]);
    pairs[pairs.length] = pair;
    chars += pair.bytes;
  }

  var out = new Buffer(chars + 2);
  pairs.forEach(function(pair){
    pair.copy(out);
  });

  out.writeUInt16LE(chars, 0);
  return this.otuput('env', out);
}



bindings.formatArgs = function formatArgs(args){
  if (Array.isArray(args)) args = args.join(' ');
  return this.otuput('args', wstring(args));
}

bindings.formatCwd = function formatCwd(cwd){
  if (!cwd) {
    return this.otuput('cwd', wstring(process.cwd()));
  }
  cwd += '';
  if (!fs.existsSync(cwd)) {
    return this.otuput('cwd', wstring(process.cwd()));
  }
}

bindings.formatFile = function formatFile(file){
  if (!file) {
    return this.otuput('file','');
  }
  file += '';
  if (!fs.existsSync(file)) {
    return this.otuput('file','');
  }
  return this.otuput('file', wstring(path.resolve(file)));
}

bindings.output = function(where, what){
  console.log(where, what);
  return what;
}

module.exports = WinPTY;

// function fork(file, args, env, cwd, cols, rows){
//   return {
//     fd: master,
//     pid: pid,
//     pty: filename
//   }
// }

// function open(cols, rows){
//   return {
//     master: fd,
//     slave: fd,
//     pty: name
//   }
// }

// function resize(fd, cols, rows){

// }

// function process(fd, tty) {
//   return name;
// }


var pty = new WinPTY;
console.log(pty.fork())
