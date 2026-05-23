# Custom FTP Project

> ⚠️ **Work in progress** — currently has a lot of known bugs. Chat support is functional; more features coming.

This is a custom FTP project that currently supports **chatting**. Additional FTP functionality is still in development.

This software uses NimLib, Copyright (c) 2026 ArtikLamartik. Licensed under NLL-1.0.0

---

## Building

```bash
g++ ./build.cpp -L. ./nimlib.so -o build && ./build
```

---

## Usage

### Start a server

```bash
./ftp server
```

### Connect as a client

```bash
./ftp <ip-address> <port>
```

Replace `<ip-address>` and `<port>` with the values shown when the server starts.

---

## Debugging

Add the `-dbg` flag to enable debugging mode:

```bash
./ftp server -dbg
./ftp <ip-address> <port> -dbg
```
