FROM gcc:14-bookworm AS build

WORKDIR /app
COPY Makefile ./
COPY src ./src
COPY tests ./tests

RUN make

FROM debian:bookworm-slim

WORKDIR /app
COPY --from=build /app/build/lanchess-client /usr/local/bin/lanchess-client
COPY --from=build /app/build/lanchess-server /usr/local/bin/lanchess-server
COPY docker-entrypoint.sh /usr/local/bin/lanchess

RUN chmod +x /usr/local/bin/lanchess

ENTRYPOINT ["lanchess"]
CMD ["server"]
