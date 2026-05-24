#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../nimlib.cpp"

#define PORT 8000
#define MAX_BODY 4096
#define HDR_SIZE 5

#define TYPE_MESSAGE 0
#define TYPE_BROADCAST 101
#define CTL_NONE 0
#define CTL_END 8

static pthread_mutex_t g_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cv = PTHREAD_COND_INITIALIZER;
static int g_connfd = -1;
static int g_done = 0;

struct Packet {
    uint8_t type;
    uint32_t len;
    char body[MAX_BODY];
    uint8_t ctl;
};

static int send_packet(int fd, uint8_t type, uint8_t ctl, const char *body) {
    if (fd < 0) {
        return 0;
    }
    uint8_t buf[HDR_SIZE + MAX_BODY + 1];
    uint32_t blen = (uint32_t)strlen(body);
    uint32_t nl = htonl(blen);
    buf[0] = type;
    memcpy(buf + 1, &nl, 4);
    memcpy(buf + 5, body, blen);
    buf[5 + blen] = ctl;
    return send(fd, buf, HDR_SIZE + blen + 1, 0) > 0;
}

static int recv_packet(int fd, struct Packet *p) {
    uint8_t hdr[HDR_SIZE];
    int n = 0;
    while (n < HDR_SIZE) {
        int r = recv(fd, hdr + n, HDR_SIZE - n, 0);
        if (r <= 0) {
            return -1;
        }
        n += r;
    }
    uint32_t nlen;
    memcpy(&nlen, hdr + 1, 4);
    uint32_t blen = ntohl(nlen);
    if (blen >= MAX_BODY) {
        blen = MAX_BODY - 1;
    }
    uint32_t need = blen + 1;
    uint8_t  tmp[MAX_BODY + 1];
    n = 0;
    while (n < (int)need) {
        int r = recv(fd, tmp + n, need - n, 0);
        if (r <= 0) {
            return -1;
        }
        n += r;
    }
    p->type = hdr[0];
    p->len  = blen;
    memcpy(p->body, tmp, blen);
    p->body[blen] = '\0';
    p->ctl = tmp[blen];
    return 0;
}

static int make_socket(void) {
    int fd  = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    return fd;
}

static const char *lan_ip(void) {
    static char ip[INET_ADDRSTRLEN];
    struct ifaddrs *ifa, *cur;
    if (getifaddrs(&ifa) != 0) {
        return "?.?.?.?";
    }
    for (cur = ifa; cur; cur = cur->ifa_next) {
        if (!cur->ifa_addr || cur->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        struct sockaddr_in *sin = (struct sockaddr_in *)cur->ifa_addr;
        inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));
        if (strncmp(ip, "127.", 4) != 0) {
            freeifaddrs(ifa);
            return ip;
        }
    }
    freeifaddrs(ifa);
    return "127.0.0.1";
}

static void *server_recv_thread(void *arg) {
    (void)arg;
    struct Packet p;
    while (1) {
        pthread_mutex_lock(&g_mu);
        int fd = g_connfd;
        pthread_mutex_unlock(&g_mu);
        if (recv_packet(fd, &p) < 0) {
            stdo("[INFO] Client disconnected.\n");
            break;
        }
        if (p.type == TYPE_BROADCAST) {
            stdo(strformat(2, "[BROADCAST] ", p.body));
        } else {
            stdo(strformat(3, "[CLIENT] ", p.body, "\n"));
        }
        if (p.ctl == CTL_END) {
            stdo("[ACTION] CLIENT LEFT.\n");
            break;
        }
    }
    pthread_mutex_lock(&g_mu);
    g_done = 1;
    pthread_cond_signal(&g_cv);
    pthread_mutex_unlock(&g_mu);
    return NULL;
}

static void *server_send_thread(void *arg) {
    (void)arg;
    while (1) {
        const char *line = stdi(1);
        if (!line) break;
        if (strcomp(line, "!exit") == 1) {
            pthread_mutex_lock(&g_mu);
            int fd = g_connfd;
            pthread_mutex_unlock(&g_mu);
            if (fd != -1) {
                send_packet(fd, TYPE_BROADCAST, CTL_END, "Server is shutting down.\n");
                sleep(1);
                shutdown(fd, SHUT_RDWR);
                close(fd);
            }
            stdo("Server stopped.\n");
            halt(0);
        }
        if (strcomp(line, "!kick") == 1) {
            pthread_mutex_lock(&g_mu);
            int fd = g_connfd;
            pthread_mutex_unlock(&g_mu);
            if (fd == -1) {
                stdo("[INFO] No client connected.\n");
            } else {
                send_packet(fd, TYPE_BROADCAST, CTL_END, "You have been kicked by the server.\n");
                sleep(1);
                shutdown(fd, SHUT_RDWR);
                close(fd);
                pthread_mutex_lock(&g_mu);
                g_connfd = -1;
                g_done = 1;
                pthread_cond_signal(&g_cv);
                pthread_mutex_unlock(&g_mu);
                stdo("[ACTION] Client kicked.\n");
            }
            continue;
        }
        if (line[0] == '\0') {
            continue;
        }
        pthread_mutex_lock(&g_mu);
        int fd = g_connfd;
        pthread_mutex_unlock(&g_mu);
        if (fd == -1) {
            stdo("[INFO] No client connected, message dropped.\n");
            continue;
        }
        send_packet(fd, TYPE_MESSAGE, CTL_NONE, line);
        stdo(strformat(3, "[YOU]: ", line, "\n"));
    }
    return NULL;
}

static int run_server(void) {
    int srv = make_socket();
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        stdo("Error binding socket.\n");
        return -1;
    }
    listen(srv, 5);
    stdo(strformat(7,
        "\n========Server Info========\nLocal: 127.0.0.1:",
        tostrint(PORT),
        "\nLAN:   ",
        lan_ip(),
        ":",
        tostrint(PORT),
        "\n===========================\n\n"
    ));
    pthread_t stid;
    pthread_create(&stid, NULL, server_send_thread, NULL);
    pthread_detach(stid);
    while (1) {
        struct sockaddr_in cli;
        socklen_t clen = sizeof(cli);
        int fd = accept(srv, (struct sockaddr *)&cli, &clen);
        if (fd < 0) {
            continue;
        }
        pthread_mutex_lock(&g_mu);
        g_connfd = fd;
        g_done = 0;
        pthread_mutex_unlock(&g_mu);
        stdo("Client connected.\n");
        send_packet(fd, TYPE_BROADCAST, CTL_NONE, "Connected to server!\n");
        pthread_t rid;
        pthread_create(&rid, NULL, server_recv_thread, NULL);
        pthread_detach(rid);
        pthread_mutex_lock(&g_mu);
        while (!g_done) {
            pthread_cond_wait(&g_cv, &g_mu);
        }
        shutdown(g_connfd, SHUT_RDWR);
        close(g_connfd);
        g_connfd = -1;
        pthread_mutex_unlock(&g_mu);
    }
    close(srv);
    return 0;
}

static void *client_recv_thread(void *arg) {
    (void)arg;
    struct Packet p;
    while (1) {
        if (recv_packet(g_connfd, &p) < 0) {
            stdo("[INFO] Server disconnected.\n");
            halt(0);
        }
        if (p.type == TYPE_BROADCAST) {
            stdo(strformat(2, "[BROADCAST] Server: ", p.body));
        } else {
            stdo(strformat(3, "[SERVER]: ", p.body, "\n"));
        }
        if (p.ctl == CTL_END) {
            stdo("[INFO] Server closed the connection.\n");
            halt(0);
        }
    }
    return NULL;
}

static int run_client(const char *ip, uint16_t port) {
    stdo(strformat(5, "Connecting to ", ip, ":", tostrint(port), "...\n"));
    g_connfd = make_socket();
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        stdo("Error converting IP address.\n");
        return -1;
    }
    if (connect(g_connfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        stdo("Error connecting to server.\n");
        return -1;
    }
    stdo("Connected.\n");
    pthread_t tid;
    pthread_create(&tid, NULL, client_recv_thread, NULL);
    pthread_detach(tid);
    while (1) {
        const char *line = stdi(1);
        if (!line) { break; }
        if (strcomp(line, "/~end~/") == 1) { send_packet(g_connfd, TYPE_MESSAGE, CTL_END, line); break; }
        if (strcomp(line, "!exit") == 1) {
            send_packet(g_connfd, TYPE_BROADCAST, CTL_NONE, "Client is exiting.\n");
            break;
        }
        if (line[0] != '\0') {
            send_packet(g_connfd, TYPE_MESSAGE, CTL_NONE, line);
            stdo(strformat(3, "[YOU]: ", line, "\n"));
        }
    }
    shutdown(g_connfd, SHUT_RDWR);
    close(g_connfd);
    return 0;
}

int main() {
    Getargs args = getargs();
    if (args.length < 1) {
        stdo("Usage: ./ftp server | ./ftp client <ip> <port>\n");
        return 1;
    }
    if (strcomp(args.args[0], "server") == 1) {
        return run_server();
    }
    if (strcomp(args.args[0], "client") == 1) {
        if (args.length < 3) {
            stdo("Usage: ./ftp client <ip> <port>\n");
            return 1;
        }
        return run_client(args.args[1], (uint16_t)tointstr(args.args[2]));
    }
    stdo("Usage: ./ftp server | ./ftp client <ip> <port>\n");
    return 1;
}
