# iDirt Codebase - Modified to run on Ubuntu 18.04

This is the original iDirt codebase, modified to run on a modern operating system. 

A Dockerfile is included for generating a container image, in /abermud/Dockerfile

Instructions to build the MUD:

Clone the git repo.
Ensure you have gcc and make installed.
cd to /src

make depend

make

cd ../bin

./aberd

By default the MUD binds to port 6715, as is tradition. You can connect to it by telneting to your machine on this port.

This has been tested on an Ubuntu 18.04 VM, and on Ubuntu 18.04 on the Windows Subsystem for Linux in Windows 10.
This has also been tested running in the supplied container image.

There is absolutely no support provided for this codebase which is up to 30 years old in some places.
