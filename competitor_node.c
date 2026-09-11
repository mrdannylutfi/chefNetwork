#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <hiredis/hiredis.h>

#define LOCK_KEY "lock:vegan_cheese"
#define SCOREBOARD_CHANNEL "scoreboard_updates"

void log_visual_state(redisContext *ctx, const char *chef, const char *step, const char *status) {
    // Asynchronous Pub/Sub visual message passing stream
    redisReply *reply = redisCommand(ctx, "PUBLISH %s [%s] -> Step: %s | Status: %s",
                                     SCOREBOARD_CHANNEL, chef, step, status);
    if (reply) freeReplyObject(reply);

    // Print local human-centric scannable trace logs
    printf("\033[0;36m[LOG]\033[0m %s: %s (%s)\n", chef, step, status);
}

int acquire_distributed_lock(redisContext *ctx, const char *chef) {
    // SET NX PX ensures atomic mutual exclusion across completely different cluster machines
    redisReply *reply = redisCommand(ctx, "SET %s %s NX PX 5000", LOCK_KEY, chef);
    if (reply == NULL) return 0;

    int acquired = 0;
    if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0) {
        acquired = 1;
    }
    freeReplyObject(reply);
    return acquired;
}

void release_distributed_lock(redisContext *ctx) {
    redisReply *reply = redisCommand(ctx, "DEL %s", LOCK_KEY);
    if (reply) freeReplyObject(reply);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <ChefName> <Redis_IP>\n", argv[0]);
        return 1;
    }

    const char *chef = argv[1];
    const char *redis_ip = argv[2];
    srand(time(NULL) ^ getpid());

    redisContext *ctx = redisConnect(redis_ip, 6379);
    if (ctx == NULL || ctx->err) {
        printf("Redis Connection Failed: %s\n", ctx ? ctx->errstr : "Allocation error");
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Step 1: Base Pasta Preparation
    log_visual_state(ctx, chef, "Pasta Preparation", "STARTING");
    usleep((rand() % 200 + 100) * 1000); // Unique processing delay
    log_visual_state(ctx, chef, "Pasta Preparation", "COMPLETED");

    // Step 2: Acquire Distributed Lock for Shared Vegan Cheese Machine
    log_visual_state(ctx, chef, "Vegan Cheese Module", "AQUIRING_LOCK_WAIT");
    while (!acquire_distributed_lock(ctx, chef)) {
        usleep(10000); // Polling constraint fallback
    }

    log_visual_state(ctx, chef, "Vegan Cheese Module", "LOCK_SECURED_COOKING");
    usleep(150000); // Shared compilation work block
    log_visual_state(ctx, chef, "Vegan Cheese Module", "COMPLETED");
    release_distributed_lock(ctx);

    // Step 3: Alfredo Final Fusion Assembly
    log_visual_state(ctx, chef, "Alfredo Fusion (Oil + Mushrooms)", "ASSEMBLING");
    usleep((rand() % 300 + 100) * 1000);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    log_visual_state(ctx, chef, "Alfredo Fusion", "FINISHED");

    // Push the final JSON structural metric directly into a Redis List for Storm Ingestion
    redisReply *push_reply = redisCommand(ctx, "LPUSH storm_metrics {\"chef\":\"%s\",\"latency\":%.3f}", chef, elapsed_ms);
    if (push_reply) freeReplyObject(push_reply);

    printf("\033[1;32m[SYSTEM SUCCESS]\033[0m %s successfully pushed stats. Total Latency: %.2f ms\n", chef, elapsed_ms);

    redisFree(ctx);
    return 0;
}
