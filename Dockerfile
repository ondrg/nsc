FROM gcc:8.2.0 as build

COPY . /usr/src/nsc
WORKDIR /usr/src/nsc
RUN make static

FROM scratch

COPY --from=build /usr/src/nsc/nsc /bin/nsc
CMD ["/bin/nsc"]
