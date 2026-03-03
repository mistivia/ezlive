#include "srtserver.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#if defined(_WIN32)
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <sys/poll.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <arpa/inet.h>
#endif

#include <srt/srt.h>

#include "ezlive_config.h"

#define BUFFER_SIZE 1500

int handshake_callback(void* opaq, SRTSOCKET ns, int hs_version, const struct sockaddr* peeraddr, const char* streamid) {
    char addr_str[INET_ADDRSTRLEN] = {0};
    struct sockaddr_in* sin = (struct sockaddr_in*)peeraddr;
    inet_ntop(AF_INET, &(sin->sin_addr), addr_str, INET_ADDRSTRLEN);

    printf("[Callback] Incoming connection from: %s:%d\n", addr_str, ntohs(sin->sin_port));
    printf("[Callback] Client Stream ID: %s\n", streamid);
    
    if (streamid == NULL || strlen(streamid) == 0) {
        printf("[Callback] Rejected: No Stream ID provided.\n");
        return -1;
    }

	if (strlen(ezlive_config->key) == 0) {
		printf("[Callback] No key. Skip auth.\n");
		return 0;
	}
    if (strcmp(streamid, ezlive_config->key) != 0) {
        printf("[Callback] Rejected: Invalid Key. Expected '%s', got '%s'\n", ezlive_config->key, streamid);
        return -1;
    }

    printf("[Callback] Key validated. Connection Accepted.\n");
    return 0;
}

void setsock(SRTSOCKET *sock) {
    int recv_latency = 8000;
    srt_setsockopt(*sock, 0, SRTO_RCVLATENCY, &recv_latency, sizeof recv_latency);
    
    int peer_latency = 8000;
    srt_setsockopt(*sock, 0, SRTO_PEERLATENCY, &peer_latency, sizeof peer_latency);
    
    int snd_buf = 60000;
    srt_setsockopt(*sock, 0, SRTO_SNDBUF, &snd_buf, sizeof snd_buf);

    int latency = 8000;
    srt_setsockopt(*sock, 0, SRTO_LATENCY, &latency, sizeof(latency));
}

void start_srt_server(SrtCallbacks srtcb, void *ctx) {
    if (srt_startup() != 0) {
        fprintf(stderr, "SRT startup failed.\n");
        return;
    }

    SRTSOCKET bind_sock = srt_create_socket();
    if (bind_sock == SRT_INVALID_SOCK) {
        fprintf(stderr, "Error creating socket: %s\n", srt_getlasterror_str());
        return;
    }

    int yes = 1;
    srt_setsockopt(bind_sock, 0, SRTO_RCVSYN, &yes, sizeof(yes));
    setsock(&bind_sock);

    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(ezlive_config->listening_port);
	if (inet_pton(AF_INET, ezlive_config->listening_addr, &sa.sin_addr.s_addr) <= 0) {
		fprintf(stderr, "Invalid IP address\n");
		exit(-1);
	}

    if (srt_bind(bind_sock, (struct sockaddr*)&sa, sizeof(sa)) == SRT_ERROR) {
        fprintf(stderr, "Bind error: %s\n", srt_getlasterror_str());
        srt_close(bind_sock);
        srt_cleanup();
        return;
    }
	srt_listen_callback(bind_sock, handshake_callback, NULL);
    printf("SRT Server listening on port %d...\n", ezlive_config->listening_port);
    if (srt_listen(bind_sock, 5) == SRT_ERROR) {
        fprintf(stderr, "Listen error: %s\n", srt_getlasterror_str());
        return;
    }

    while (1) {
        struct sockaddr_storage client_sa = {0};
        int sa_len = sizeof(client_sa);

        printf("Waiting for client to connect...\n");

        SRTSOCKET client_sock = srt_accept(bind_sock, (struct sockaddr*)&client_sa, &sa_len);
        if (client_sock == SRT_INVALID_SOCK) {
            fprintf(stderr, "Accept error: %s\n", srt_getlasterror_str());
            continue;
        }
        setsock(&client_sock);

        printf("Client connected! Starting to receive data.\n");
		srtcb.on_start(ctx);

        char buffer[BUFFER_SIZE];

        while (1) {
            int n = srt_recvmsg(client_sock, buffer, sizeof(buffer));

            if (n == SRT_ERROR) {
                fprintf(stderr, "Connection lost or error: %s\n", srt_getlasterror_str());
				srtcb.on_stop(ctx);
                break;
            }

            if (n == 0) {
                printf("Client closed connection.\n");
				srtcb.on_stop(ctx);
                break;
            }
			srtcb.on_data(ctx, buffer, n);
        }
        printf("Closing client socket.\n");
        srt_close(client_sock);
    }

    srt_close(bind_sock);
    srt_cleanup();
    return;
}
