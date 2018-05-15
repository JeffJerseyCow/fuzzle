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

The --follow_child / -f switch will change the behaviour of the intercepted syscalls and follow either the child process path or remain in the parent. This behaviour can be seen in the http_server video.
