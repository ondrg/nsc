FROM gcc:8.2.0

COPY . /usr/src/nsc

WORKDIR /usr/src/nsc

RUN make

CMD ["./nsc"]
