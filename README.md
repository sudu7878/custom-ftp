<h1>WireSend</h1>

> ⚠️ **Work in progress** — currently has a lot of known bugs. Chat support is fairly functional; more features coming soon.

A FTP project that currently only supports **chatting**. Additional FTP functionality is still in development.

This software uses NimLib, Copyright (c) 2026 ArtikLamartik.
Licensed under NLL-2.0.0: https://github.com/ArtikLamartik/NimLib/blob/main/LICENSE

---

## Building

Just run the makefile.
```bash
make all
```

---

## Usage

### Start a server

```bash
./wires server
```

### Connect as a client

```bash
./wires <ip-address> <port>
```

Replace `<ip-address>` and `<port>` with the values shown when the server starts.

---

## Debugging

Add the `-dbg` flag to enable debugging mode:

```bash
./wires server -dbg
./wires <ip-address> <port> -dbg
```
