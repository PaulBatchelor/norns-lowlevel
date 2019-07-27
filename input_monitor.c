

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
    int keys_fid;
    int rc;
    struct input_event evt[8];
    int nevts;
    int e;
    int knob_fid[3];
    char tmp[64];
    int k;


    signal(SIGINT, quit);

    keys_fid = open(
        "/dev/input/by-path/platform-keys-event",
        O_RDONLY | O_NONBLOCK);

    for(k = 0; k < 3; k++) {
        snprintf(tmp, 64,
                 "/dev/input/by-path/platform-soc:knob%d-event",
                 k + 1);
        knob_fid[k] = open(tmp, O_RDONLY | O_NONBLOCK);
    }


    running = 1;

    while(running) {
        rc = read(keys_fid,
                  evt,
                  sizeof(struct input_event) * 8);
        if(rc != -1) {
            nevts = rc / sizeof(struct input_event);
            for(e = 0; e < nevts; e++) {
                if(evt[e].type) {
                    fprintf(stdout,
                            "button: %d %d\n",
                            evt[e].code,
                            evt[e].value);
                    fflush(stdout);
                }
            }
        }

        for(k = 0; k < 3; k++) {
            rc = read(knob_fid[k],
                    evt,
                    sizeof(struct input_event) * 8);
            if(rc != -1) {
                nevts = rc / sizeof(struct input_event);
                for(e = 0; e < nevts; e++) {
                    if(evt[e].type) {
                        fprintf(stdout,
                                "knob%d: %d\n",
                                k,
                                evt[e].value);
                        fflush(stdout);
                    }
                }
            }
        }

        usleep(10);
    }

    close(keys_fid);
    return 0;
}
