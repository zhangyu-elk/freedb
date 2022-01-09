#include <stdio.h>
#include "src/server.h"

int main() {
    server_t  *server = server_new("");
    if (server == NULL) {
        return -1;
    }
    server_run(server);
    server_close(server);
    return 0;
}
