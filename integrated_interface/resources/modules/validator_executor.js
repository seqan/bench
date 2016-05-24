/**
 * ValidatorExecutor:
 *
 * Events:
 *   'setup': (validator, benchmark_process)
 *       Will be called before a validator will be called.
 *
 *   'result': (validator)
 *       Will be called after a benchmark terminated. Independent of whether the
 *       validator was successful, failed or canceled.
 *
 *   'error': (error)
 *       Will be called if an error occured executing the validator.
 */
;(function(root, factory) {

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.ValidatorExecutor = factory();
    }

})(this, function() {
    const EventEmitter = require('events');
    const Configure = require('./configure');

    var self = new EventEmitter();

    self.validators = Configure.load_validators();

    self.validator_id = (benchmark_id) => {
        const validators = self.validators;

        for (const validator_id in validators) {
            if(validators[validator_id].indexOf(benchmark_id) > -1) {
                return validator_id;
            }
        }
        return undefined;
    }

    var error_handler = (error) => {
        self.emit('error', error);
    };

    self.validate = (current_process) => {
        const spawn = require('child_process').spawn;

        const benchmark_id = current_process.benchmark_id;
        const validator_id = self.validator_id(benchmark_id);

        var validator = {
            id: validator_id,
            benchmark_id: benchmark_id,
            cmd: "./validators/"+validator_id,
            args: [
                current_process.result_file,
                "./data/validations/" + benchmark_id + ".validate.txt"
            ],
            stdout: "",
            stderr: "",

            quality: function() {
                return parseFloat(this.stdout);
            }
        };

        self.emit('setup', validator, current_process);

        if(!validator_id) {
            self.emit('result', validator);
            return false;
        }

        var child_process;
        try {
            var child_process = spawn(validator.cmd, validator.args, {detached: true});
        } catch(err){
            return error_handler({
                message: err
            });
        }

        // handles child_process termination
        child_process.on('close', () => {
            self.emit('result', validator);
        });
        child_process.stdout.on('data', (chunk) => {
            validator.stdout += chunk;
        });
        child_process.stderr.on('data', (chunk) => {
            validator.stderr += chunk;
        });
        child_process.stdout.on('error', error_handler);
        child_process.stderr.on('error', error_handler);
        child_process.on('error', error_handler);
    };

    return self;
});
