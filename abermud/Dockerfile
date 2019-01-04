FROM ubuntu:18.04
 
MAINTAINER Kenny Lowe <kenny@kennylowe.org>
 
RUN apt-get update
RUN apt-get upgrade -y
RUN mkdir /mud
 
EXPOSE 6715
 
ENTRYPOINT /mud/bin/aberd
