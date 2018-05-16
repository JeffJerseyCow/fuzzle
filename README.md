# fuzzle
An 'uzzle' framework that allows your to download a process dump from a GDB instance and replay it under the unicorn framework.

Once complete it will be much easier to attack embedded systems using binary instrumented fuzzing, taint analysis and concolic execution.

## Tools
The framework and tools are split like so.
- fuzzle is the over arching framework
- duzzle remotely dumps a process via GDB's MI --- machine interface.
- uuzzle wraps the binary in the unicorn frame work in a way that's ready for fuzzing.

## Requirements
- gdb
- cmake
- python3
- python3-pip
- unicorn engine
- capstone engine

I’d advise you use a python virtual environment and install everything from there – I’m not going to support the project in any other form apart from an eventual Docker image.

## Docker File
```docker run -ti jeffjerseycow/fuzzle:latest```

All files are found within /root/tools.

## Usage
For now please watch the following videos as I've not had time to write proper documentation. 

https://www.youtube.com/watch?v=exoa4GfhqVI (hello_world)

https://www.youtube.com/watch?v=HECIJ-rTLws (http_server) 

The ```--follow_child / -f``` switch changes the behaviour of intercepted syscalls and directs the emulator to follow the child process path &mdash; this behaviour can be seen in the http_server video.

## Caveats
By default Linux operates on the principle of late binding/lazy loading. This means that when symbols are resolved for the first time the process calls to the PLT, jumps to the GOT and into the dynamic loader. After it’s finished doing its magic subsequent calls will automatically jump to the correct library at the correct offset.

To prevent clobbering registers the loader saves everything before performing the dlopen/dlsym routines &mdash; part of which involves the AVX instruction set. Unfortunately unicorn runs on top of an older version of QEMU and does not offer support for the vector extensions.

Because of this it is preferable to either resolve symbols at start-up with the ```LD_BIND_NOW=1``` environment variable, attack binaries without AVX &mdash common in the embedded world &mdash or attack functions that have previously had their symbols resolved.

Until unicorn is integrated with a later version of QEMU I cannot resolve this problem.
