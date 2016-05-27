"use strict";

const Promise = require('promise');

const nwbuild_ = require('nwjs-builder').commands.nwbuild;
const nwbuild = Promise.denodeify(nwbuild_)

const fs = require('fs-extra');
const os = require('os');
const Configure = require('./resources/modules/configure');
const shell_parse = require('shell-quote').parse;
const shell_quote = require('shell-quote').quote;
const spawnSync = require('child_process').spawnSync;
const execSync = require('child_process').execSync;
const targz = require('tar.gz');

const isWin = /^win/.test(process.platform);
const isMac = /^darwin/.test(process.platform);
const isLinux = /^linux/.test(process.platform);

const nwVersion = '0.14.5-sdk';

if (!isWin && !isMac && !isLinux) {
  console.warn("platform: " + process.platform + " unsupported.");
  process.exit(-1);
}

if (process.arch != 'x64') {
  console.warn("arch: " + process.arch + " unsupported, only package x64.");
  process.exit(-1);
}

var options = {
  compiler: undefined,
  compilerFlags: undefined
};

// use g++ 4.9 under linux
if (isLinux) {
  options.compiler = '$(which g++-4.9)';
}

// use parallel builds under linux and mac
if (isLinux || isMac) {
  const maxThreads = os.cpus().length;
  options.compilerFlags = '-j'+ maxThreads;
}

const buildPlatform = isWin ? 'win' : (isMac ? 'osx' : 'linux');
const buildName = 'seqan-bench-app-'+buildPlatform+'-x64';
const buildDir = './build/' + buildName + '/';
const platform = buildPlatform + '64';

// empty build dir if it exists
fs.mkdirsSync('./build/');
fs.removeSync(buildDir);

const cloneSeqan = function() {
  // clone seqan repository
  try {
    fs.accessSync('./build/seqan', fs.F_OK);
  } catch (e) {
    execSync('git clone https://github.com/seqan/seqan.git', {
      cwd: './build/',
      stdio: 'inherit'
    });
  }
};

const compileBenchmarks = function() {
  // generate makefiles for benchmarks
  fs.copySync('./benchmarks_src/', './build/benchmarks_src/', {
    preserveTimestamps: true
  });
  fs.mkdirsSync('./build/benchmarks/');
  fs.mkdirsSync('./build/benchmarks_cmake/');

  const compiler = (options.compiler ? '-DCMAKE_CXX_COMPILER='+ options.compiler + ' ' : '');
  execSync('cmake ../benchmarks_src ' +
     compiler +
    '-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=../benchmarks/ ' +
    '-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=../benchmarks/ ' +
    '-DCMAKE_MODULE_PATH=../seqan/util/cmake/ '+
    '-DCMAKE_INCLUDE_PATH=../seqan/include/ ', {
    cwd: './build/benchmarks_cmake/',
    stdio: 'inherit'
  });

  // create binaries for benchmarks
  const compilerFlags = (options.compilerFlags) ? ' -- ' + options.compilerFlags : '';
  execSync('cmake --build . --config Release' + compilerFlags, {
    cwd: './build/benchmarks_cmake/',
    stdio: 'inherit'
  });
};

const compileValidators = function() {
  fs.copySync('./validators_src/', './build/validators_src/', {
    preserveTimestamps: true
  });
  fs.mkdirsSync('./build/validators/');
  fs.mkdirsSync('./build/validators_cmake/');

  const compiler = (options.compiler ? '-DCMAKE_CXX_COMPILER='+ options.compiler + ' ' : '');
  execSync('cmake ../validators_src ' +
     compiler +
    '-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=../validators/ ' +
    '-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=../validators/ ' +
    '-DCMAKE_MODULE_PATH=../seqan/util/cmake/ '+
    '-DCMAKE_INCLUDE_PATH=../seqan/include/ ', {
    cwd: './build/validators_cmake/',
    stdio: 'inherit'
  });

  // create binaries for validators
  const compilerFlags = (options.compilerFlags) ? ' -- ' + options.compilerFlags : '';
  execSync('cmake --build . --config Release' + compilerFlags, {
    cwd: './build/validators_cmake/',
    stdio: 'inherit'
  });
};

const preparePackagingBenchApp = function () {
  // prepare building nwjs binaries
  fs.mkdirsSync('./build/src');
  fs.emptyDirSync('./build/src/');
  fs.copySync('./package.json', './build/src/package.json');
  fs.copySync('./resources', './build/src/resources');

  modifyPackagedExecutables();

  // install only needed node_modules
  execSync('npm install --production', {
    cwd: './build/src/',
    stdio: 'inherit'
  });
};

const toExe = function(file) {
  const Path = require('path');
  const dirname = Path.dirname(file);
  const ext = Path.extname(file);
  const new_file = Path.basename(file, ext) + '.exe';

  return (dirname == '.' ? '' : dirname + '/') + new_file;
};

const modifyPackagedExecutables = function () {
  if (!isWin) return;

  // add [.exe] suffix on windows in resources/config/validators.json
  const validators_json = "./build/src/resources/config/validators.json";
  const validators_ = Configure.load_json(validators_json);
  const validators = {};
  for (const id in validators_) {
    validators[toExe(id)] = validators_[id];
  }
  Configure.save_json(validators_json, validators);
};

const modifyConfigExecutables = function () {
  if (!isWin) return;

  // add [.exe] suffix on windows in config/config.json
  const config_json = buildDir + "/config/config.json";
  const config = Configure.load_json(config_json);
  for (const id in config['benchmarks']) {
    const benchmark = config['benchmarks'][id];
    const cmd_args = shell_parse(benchmark['command']);
    cmd_args[0] = toExe(cmd_args[0]);
    benchmark['command'] = shell_quote(cmd_args);
  }
  // add platform information, e.g. seqan-2.1-win if it's a windows build
  config['project']['title'] += '-' + buildPlatform;
  Configure.save_json(config_json, config);
};

const packageBenchApp = function() {
  return new Promise(function(fulfill, reject) {
    nwbuild('./build/src', {
        version: nwVersion,
        platforms: platform,
        outputDir: './build/',
        macIcns: './resources/icons/nw.icns',
        winIco: './resources/icons/nw.ico'
    }, (err) => {
      if(err) {
        console.log(err);
        throw err;
      } else {
        fulfill();
      }
    });
  });
};

const finalizeBenchApp = function() {
  fs.ensureDirSync(buildDir + '/results/');
  fs.ensureDirSync(buildDir + '/data/');

  fs.copySync('./build/benchmarks/', buildDir + '/benchmarks/');
  fs.copySync('./build/validators/', buildDir + '/validators/');
  fs.copySync('./config/config.json', buildDir + '/config/config.json');

  modifyConfigExecutables();
};

const compressBenchApp = function() {
  const tarFile = buildName + '.tar.gz';

  console.log("Create " + tarFile);
  var read = targz({
    level: 9, // Maximum compression
    memLevel: 9
  }).createReadStream(buildDir);
  var write = fs.createWriteStream(tarFile);
  read.pipe(write);
};

Promise.resolve()
  .then(cloneSeqan)
  .then(compileBenchmarks)
  .then(compileValidators)
  .then(preparePackagingBenchApp)
  .then(packageBenchApp)
  .then(finalizeBenchApp)
  .then(compressBenchApp)
  .catch(function(error) {
    console.error(error);
  });
