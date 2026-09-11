#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>

int main(int argc, char *argv[]) {
    const char *redis_ip = (argc > 1) ? argv[1] : "127.0.0.1";
    redisContext *ctx = redisConnect(redis_ip, 6379);

    // Subscribe to the shared pipeline event bus
    redisReply *reply = redisCommand(ctx, "SUBSCRIBE scoreboard_updates");
    freeReplyObject(reply);

    printf("\033[2J\033[H"); // Clear terminal interface screen
    printf("==================================================\n");
    printf("   REAL-TIME DISTRIBUTED COOKING SCOREBOARD VISUALIZER \n");
    printf("==================================================\n\n");

    while (redisGetReply(ctx, (void **)&reply) == REDIS_OK) {
        if (reply->type == REDIS_REPLY_ARRAY && reply->element[2] != NULL) {
            // Asynchronous message pass received
            printf("\033[1;33m[EVENT TIME]\033[0m %s\n", reply->element[2]->str);
        }
        freeReplyObject(reply);
    }

    redisFree(ctx);
    return 0;
}
