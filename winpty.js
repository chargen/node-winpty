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
  var buff = new Buffer(string.length * 2);
  buff.write(string, 0, 'ucs2');
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
  return out;
}

bindings.formatArgs = function formatArgs(args){
  var out = [];
  for (var i=0; i < args.length; i++) {
    i && out.push(' ');
    var quoted = !!(~args[i].indexOf(/\s/) || args[i][0] === '\0');
    quoted && out.push('"');
    var slashes = 0;
    for (var j=0; j < args[i].length && args[i][j] !== '\0'; j++) {
      if (args[i][j] === '\\') {
        slashes++;
      } else if (args[i][j] === '"') {
        out.push(new Array(slashes*2+1).join('\\'));
        out.push('"');
        slashes = 0;
      } else {
        out.push(new Array(slashes).join('\\'));
        slashes = 0;
        out.push(args[i][j]);
      }
    }
    if (quoted) {
      out.push(new Array(slashes*2).join('\\'));
      out.push('"');
    } else {
      out.push(new Array(slashes).join('\\'));
    }
  }
  return out.join('');
}

bindings.formatCwd = function formatCwd(cwd){
  if (!cwd) {
    return wstring(process.cwd());
  }
  cwd += '';
  if (!fs.existsSync(cwd)) {
    return wstring(process.cwd());
  }
  return wstring(path.resolve(cwd));
}

bindings.formatFile = function formatFile(file){
  if (!file) {
    return '';
  }
  file += '';
  if (!fs.existsSync(file)) {
    return '';
  }
  return wstring(path.resolve(file));
}

var pty = new WinPTY({
  columns: 80,
  rows: 30,
  file: 'cmd.exe',
  args: [],
  cwd: process.cwd(),
  env: process.env
});

module.exports = PTY;
