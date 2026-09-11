#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <hiredis/hiredis.h>

#define LOCK_KEY "lock:vegan_cheese"

void update_status(redisContext *ctx, const char *chef, const char *status) {
    char key[64];
    snprintf(key, sizeof(key), "status:%s", chef);
    redisReply *r = redisCommand(ctx, "SET %s %s", key, status);
    if (r) freeReplyObject(r);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ChefName>\n", argv[0]);
        return 1;
    }
    const char *chef = argv[1];
    srand(time(NULL) ^ getpid());

    redisContext *ctx = redisConnect("127.0.0.1", 6379);
    if (!ctx || ctx->err) return 1;

    update_status(ctx, chef, "READY");

    for (int round = 1; round <= 3; round++) {
        sleep(2);
        
        update_status(ctx, chef, "COOKING_PASTA");
        usleep((rand() % 400 + 200) * 1000);

        update_status(ctx, chef, "WAITING_LOCK");
        while (1) {
            redisReply *lock_r = redisCommand(ctx, "SET %s %s NX PX 2000", LOCK_KEY, chef);
            if (lock_r && lock_r->type == REDIS_REPLY_STATUS && strcmp(lock_r->str, "OK") == 0) {
                freeReplyObject(lock_r);
                break;
            }
            if (lock_r) freeReplyObject(lock_r);
            usleep(10000);
        }

        update_status(ctx, chef, "USING_CHEESE_MACHINE");
        usleep(300000);

        redisReply *del_r = redisCommand(ctx, "DEL %s", LOCK_KEY);
        if (del_r) freeReplyObject(del_r);

        update_status(ctx, chef, "FUSING_ALFREDO");
        usleep((rand() % 300 + 100) * 1000);
        update_status(ctx, chef, "ROUND_DONE");

        redisReply *win_r = redisCommand(ctx, "SET round:%d:winner %s NX", round, chef);
        if (win_r && win_r->type == REDIS_REPLY_STATUS && strcmp(win_r->str, "OK") == 0) {
            redisCommand(ctx, "INCR score:%s", chef);
        }
        if (win_r) freeReplyObject(win_r);

        redisReply *sa = redisCommand(ctx, "GET score:Alice");
        redisReply *sb = redisCommand(ctx, "GET score:Bob");
        int score_a = sa && sa->str ? atoi(sa->str) : 0;
        int score_b = sb && sb->str ? atoi(sb->str) : 0;
        
        redisCommand(ctx, "SET tournament:score Alice: %d | Bob: %d", score_a, score_b);

        if (score_a >= 2) redisCommand(ctx, "SET tournament:winner MATCH_WINNER: ALICE!");
        if (score_b >= 2) redisCommand(ctx, "SET tournament:winner MATCH_WINNER: BOB!");

        if (sa) freeReplyObject(sa);
        if (sb) freeReplyObject(sb);
        
        sleep(1);
    }

    redisFree(ctx);
    return 0;
}
