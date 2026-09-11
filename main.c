#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "AlfredoPastaAlice.c"
#include "AlfredoPastaBob.c"

#define LOCK_FILE "/tmp/vegan_cheese.lock"
#define PORT 8888

typedef struct {
    char name[10];
    double latency;
    int rank;
} Result;

Result results[2];
pthread_mutex_t report_mutex = PTHREAD_MUTEX_INITIALIZER;
int completed_count = 0;

// Shared file locking mechanism
void acquire_cheese_lock() {
    int fd = open(LOCK_FILE, O_CREAT | O_WRONLY, 0666);
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fcntl(fd, F_SETLKW, &fl); // Blocks until resource is acquired

    // Simulating time spent implementing local machine vegan cheese layer
    usleep(100000);

    fl.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &fl);
    close(fd);
}

// Background network thread to broadast results to network
void *network_broadcast_server(void *arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    char buffer[512];
    Result *r1 = &results[0];
    Result *r2 = &results[1];

    snprintf(buffer, sizeof(buffer),
             "\n=== GAME OVER RESULTS ===\nRank 1: %s (Latency: %.3f ms)\nRank 2: %s (Latency: %.3f ms)\n",
             r1->name, r1->latency, r2->name, r2->latency);

    send(new_socket, buffer, strlen(buffer), 0);
    close(new_socket);
    close(server_fd);
    return NULL;
}

void *run_alice(void *arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    Pasta p = make_pasta();
    acquire_cheese_lock();
    VeganCheese c = make_vegan_cheese();

    AlfredoPasta dish = cook_alfredo_alice(p, c);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    pthread_mutex_lock(&report_mutex);
    int my_rank = ++completed_count;
    results[my_rank - 1] = (Result){"Alice", elapsed, my_rank};
    pthread_mutex_unlock(&report_mutex);
    return NULL;
}

void *run_bob(void *arg) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    Pasta p = make_pasta();
    acquire_cheese_lock();
    VeganCheese c = make_vegan_cheese();

    AlfredoPasta dish = cook_alfredo_bob(p, c);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    pthread_mutex_lock(&report_mutex);
    int my_rank = ++completed_count;
    results[my_rank - 1] = (Result){"Bob", elapsed, my_rank};
    pthread_mutex_unlock(&report_mutex);
    return NULL;
}

int main() {
    srand(time(NULL));
    printf("[Game Engine] Starting Pasta cooking competition...\n");

    pthread_t server_thread, t1, t2;
    pthread_create(&server_thread, NULL, network_broadcast_server, NULL);

    // Allow server socket setup time
    usleep(50000);

    pthread_create(&t1, NULL, run_alice, NULL);
    pthread_create(&t2, NULL, run_bob, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("[Game Engine] Competition completed. Winner announced! Fetching network logs...\n");

    // Simulating system network receiver execution client loop
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
        char recv_buf[512] = {0};
        read(sock, recv_buf, sizeof(recv_buf));
        printf("%s", recv_buf);
        close(sock);
    }

    pthread_join(server_thread, NULL);
    return 0;
}
