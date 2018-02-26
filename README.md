# fuzzle
An 'uzzle' framework that allows your to download a process dump from a GDB instance and replay it under the unicorn framework.

Once complete it will be much easier to attack embedded systems using binary instrumented fuzzing, taint analysis and concolic execution.

## Tools
The frame work and tools are split like so.
- fuzzle is the over arching framework
- duzzle remotely dumps a process via GDB's MI --- machine interface.
- buzzle converts the process dump to a fully working binary, execution will resume where ever the state was saved.
- uuzzle wraps the binary in the unicorn frame work in a way that's ready for fuzzing.
