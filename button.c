
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/input.h>
#include <signal.h>

int running = 1;

void quit(int sig)
{
    printf("Closing.\n");
    running = 0;
}

int main(int argc, char *argv[])
{
    int fid;
    int rc;
    struct input_event evt[8];
    int nevts;
    int e;


    signal(SIGINT, quit);

    fid = open(
        "/dev/input/by-path/platform-keys-event",
        O_RDONLY | O_NONBLOCK);


    running = 1;

    while(running) {
        rc = read(fid, evt, sizeof(struct input_event) * 8);
        if(rc != -1) {
            nevts = rc / sizeof(struct input_event);
            for(e = 0; e < nevts; e++) {
                if(evt[e].type) {
                    fprintf(stdout,
                            "%d %d\n",
                            evt[e].code,
                            evt[e].value);
                    fflush(stdout);
                }
            }
        }
        usleep(10);
    }

    close(fid);
    return 0;
}
