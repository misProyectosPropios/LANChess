# LANChess

Minimal C scaffold for a LAN chess client and server.

This project intentionally does not implement chess rules, networking, or concurrency yet. It only provides separate client and server programs, a simple build, and Docker support.

## Layout

```text
src/
  client/
    main.c
  server/
    main.c
  common/
    app_info.c
    app_info.h
tests/
  smoke_test.c
Dockerfile
Makefile
```

## Build

```sh
make
```

## Run Locally

```sh
make run-server
make run-client
```

## Test

```sh
make test
```

## Docker

Build the image:

```sh
docker build -t lanchess .
```

Run the server:

```sh
docker run --rm lanchess server
```

Run the client:

```sh
docker run --rm lanchess client
```
