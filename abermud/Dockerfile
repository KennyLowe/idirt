FROM ubuntu:18.04
 
MAINTAINER Kenny Lowe <kenny@kennylowe.org>
 
RUN apt-get update
RUN apt-get upgrade -y
RUN mkdir /mud
RUN apt-get install git -y
RUN apt-get install gcc -y
RUN apt-get install make -y
RUN git clone https://www.github.com/kennylowe/idirt.git /mud
RUN make -C /mud/src

RUN chmod +x /mud/bin/test.sh

WORKDIR /mud/bin

EXPOSE 6715

ENTRYPOINT /mud/bin/aberd -f

#CMD ["/mud/bin/test.sh"]
