# Custom FTP Project

> ⚠️ **Work in progress** — currently has a lot of known bugs. Chat support is fairly functional; more features coming soon.

This is a custom FTP project that currently only supports **chatting**. Additional FTP functionality is still in development.

This software uses NimLib, Copyright (c) 2026 ArtikLamartik.
Licensed under NLL-2.0.0: https://github.com/ArtikLamartik/NimLib/blob/main/LICENSE

---

## Building

```bash
g++ -Iheaders ./build.cpp -L. ./nimlib.so -o build && ./build && rm ./build
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
