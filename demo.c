#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/input.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <math.h>
#include <jack/jack.h>

int running = 1;

typedef struct {
    int fbfd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize;
    char *fbp;
    unsigned char *buf;
} framebuffer;

#define BUFSIZE 62

typedef struct {
    framebuffer fb;
    float vals[3];
    int please_draw;
    jack_port_t *out[2];
    jack_client_t *client;
    float lphs;
    int sr;
    int counter;
    int subcount;
    float buf[4096];
} main_data;

void quit(int sig)
{
    printf("Closing.\n");
    running = 0;
}

int open_knob(int knob)
{
    char path[256];
    sprintf(path,
            "/dev/input/by-path/platform-soc:knob%d-event",
            knob);
    return open(path, O_RDONLY | O_NONBLOCK);
}

void pixel(framebuffer *fb,
           int x,
           int y,
           unsigned char val)
{
    long int loc;
    if(x >= fb->vinfo.xres || y >= fb->vinfo.yres) return;

    loc = x + y * fb->vinfo.xres;
    fb->buf[loc] = val;

}

void copy_to_fb(framebuffer *fb)
{
    long int location;
    int x, y;
    unsigned char val;
    int w, h;

    h = fb->vinfo.yres;
    w = fb->vinfo.xres;
    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            location = (x + fb->vinfo.xoffset) *
                (fb->vinfo.bits_per_pixel/8) +
                (y + fb->vinfo.yoffset) * fb->finfo.line_length;
            val = fb->buf[x + y * w];
            fb->fbp[location] = 0x0;
            fb->fbp[location + 1] = val;
        }
    }
}

void clearscreen(framebuffer *fb)
{
    int x, y;

    for(y = 0; y < fb->vinfo.yres; y++) {
        for(x = 0; x < fb->vinfo.xres; x++) {
            pixel(fb, x, y, 0);
        }
    }
}

void vslider(framebuffer *fb, int x, int y, float val)
{
    float w;
    float h;
    int p1, p2;
    int n, k;
    int nlines;

    w = 8;
    h = 32;


    for (n = 0; n < w; n++) {
        pixel(fb, x + n, y, 255);
    }

    p1 = y + h - 1;
    for (n = 0; n < w; n++) {
        pixel(fb, x + n, p1, 255);
    }

    for (n = 0; n < h; n++) {
        pixel(fb, x, y + n, 255);
    }

    p1 = x + w - 1;
    for (n = 0; n < h; n++) {
        pixel(fb, p1, y + n, 255);
    }

    /* draw value */

    nlines = round(val * 28);
    for(k = 0; k < nlines; k++) {
        for (n = 0; n < (w - 4); n++) {
            pixel(fb, x + 2 + n, (y + (h - k) - 3), 255);
        }
    }

}

void wavedisplay(framebuffer *fb, float *buf, int size)
{
    float x, y;
    float w, h;
    int n;
    float val;
    int pos;
    int jmp;

    x = 48;
    y = 24;

    w = 64;
    h = 32;

    for(n = 0; n < w; n++) {
        pixel(fb, x+n, y, 255);
    }

    for(n = 0; n < w; n++) {
        pixel(fb, x+n, y + h - 1, 255);
    }

    for(n = 0; n < h; n++) {
        pixel(fb, x, y + n, 255);
    }

    for(n = 0; n < h; n++) {
        pixel(fb, x + w - 1, y + n, 255);
    }

    for(n = 0; n < 62; n++) {
        val = buf[n];
        val = 1 - ((val + 1) * 0.5);
        pos = floor(val * 30);
        pixel(fb, x + 1 + n, y + pos, 255);
    }
}

void* draw(void *data)
{
    framebuffer *fb;
    main_data *m;
    int draw;

    m = data;
    fb = &m->fb;

    clearscreen(fb);
    while(running) {
        draw = m->please_draw;

        if(draw) {
            clearscreen(fb);
            vslider(fb, 16, 24, m->vals[0]);
            vslider(fb, 24, 24, m->vals[1]);
            vslider(fb, 32, 24, m->vals[2]);
            wavedisplay(fb, m->buf, 4096);
            m->please_draw = 0;
            copy_to_fb(fb);
        }
        usleep(800);
    }

    fprintf(stderr, "quitting\n");
    pthread_exit(NULL);
}

int fb_setup(framebuffer *fb)
{
    int rc;

    fb->fbfd = open("/dev/fb0", O_RDWR);
    if (fb->fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        return 1;
    }

    rc = ioctl(fb->fbfd, FBIOGET_FSCREENINFO, &fb->finfo);

    if (rc == -1) {
        perror("Error reading fixed information");
        return 2;
    }

    rc = ioctl(fb->fbfd, FBIOGET_VSCREENINFO, &fb->vinfo);

    if (rc == -1) {
        perror("Error reading variable information");
        return 3;
    }

    fb->screensize = fb->vinfo.xres *
        fb->vinfo.yres *
        fb->vinfo.bits_per_pixel / 8;

    fb->fbp = (char *)mmap(0,
                           fb->screensize,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED,
                           fb->fbfd,
                           0);

    fb->buf = calloc(1, fb->vinfo.xres * fb->vinfo.xres);
    if ((int)fb->fbp == -1) {
        return 4;
    }

    return 0;
}

void fb_cleanup(framebuffer *fb)
{
    munmap(fb->fbp, fb->screensize);
    close(fb->fbfd);
    free(fb->buf);
}

static int jack_process(jack_nframes_t nframes, void *arg);



static int jack_process(jack_nframes_t nframes, void *arg)
{
    jack_default_audio_sample_t *o[2];
    int n;
    float smp;
    float frq;
    main_data *m;

    jack_port_t **out;
    jack_client_t *client;

    m = arg;
    out = m->out;
    client = m->client;

    o[0] = (jack_default_audio_sample_t*)jack_port_get_buffer(out[0], nframes);
    o[1] = (jack_default_audio_sample_t*)jack_port_get_buffer(out[1], nframes);

    for(n = 0; n < nframes; n++) {
        smp = sin(m->lphs);
        smp *= m->vals[0];
        o[0][n] = smp;
        o[1][n] = smp;
        frq = 200 + 2000 * m->vals[1];
        m->lphs += (float)frq * ((2.0 * M_PI) / m->sr);
        m->lphs = fmod(m->lphs, 2.0 * M_PI);
        if(m->subcount == 0) {
            m->buf[m->counter] = smp;
        }
        m->counter++;
        if(m->counter >= BUFSIZE) {
            m->counter = 0;
            if(m->subcount == 0) m->please_draw = 1;
            m->subcount = (m->subcount + 1) % 8;
        }
    }

    return 0;
}

void audio_start(main_data *m)
{
    const char **ports;
    const char *client_name;
    const char *server_name;
    jack_options_t options;
    jack_status_t status;

    jack_port_t **out;
    jack_client_t *client;

    out = m->out;
    client = m->client;

    if(client != NULL) {
        fprintf(stderr, "JACK audio server seems to be started already\n");
        return;
    }

    options = JackNullOption;

    server_name=NULL;
    client_name="Monolith"; /* set the client name */

    client = jack_client_open(client_name, options, &status, server_name);

    if(client == NULL) {
        fprintf(stderr, "JACK has failed you.\n");
        if (status & JackServerFailed) {
            fprintf (stderr, "It was unable to connect to the JACK server\n");
        }
        return;
    }

    if (status & JackServerStarted) {
        fprintf(stderr, "JACK server started\n");
    }

    if (status & JackNameNotUnique) {
        client_name = jack_get_client_name(client);
        fprintf(stderr, "unique name `%s' assigned\n", client_name);
    }

    jack_set_process_callback (client, jack_process, m);

    out[0] = jack_port_register(client, "output1",
                                JACK_DEFAULT_AUDIO_TYPE,
                                JackPortIsOutput, 0);
    out[1] = jack_port_register (client, "output2",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);

    if((out[0] == NULL) || (out[1] == NULL)) {
        fprintf(stderr, "no more JACK ports available\n");
        return;
    }

    if(jack_activate(client)) {
        fprintf(stderr, "cannot activate client\n");
        return;
    }

    ports = jack_get_ports (client, NULL, NULL,
                            JackPortIsPhysical|JackPortIsInput);

    if (ports == NULL) {
        fprintf(stderr, "no physical playback ports\n");
        return;
    }

    if (jack_connect(client, jack_port_name(out[0]), ports[0])) {
        fprintf (stderr, "cannot connect output ports\n");
    }

    if (jack_connect(client, jack_port_name(out[1]), ports[1])) {
        fprintf (stderr, "cannot connect output ports\n");
    }

    jack_free (ports);
}

void audio_stop(main_data *m)
{
    if(m->client != NULL) {
        jack_client_close (m->client);
        m->client = NULL;
    }
}

int main(int argc, char *argv[])
{
    int fid[3];
    int rc;
    struct input_event evt[8];
    int nevts;
    int e;
    int k;
    pthread_t draw_thread;
    main_data m;
    int pd;
    float tmp;

    signal(SIGINT, quit);

    fb_setup(&m.fb);

    for (k = 0; k < 3; k++) {
        fid[k] = open_knob(k + 1);
        m.vals[k] = 0;
    }

    running = 1;

    pthread_create(&draw_thread, NULL, draw, &m);
    m.client = NULL;
    m.lphs = 0;
    m.counter = 0;
    m.subcount = 0;
    m.sr = 44100; /* TODO: make this dynamic */
    audio_start(&m);

    while (running) {
        pd = 0;
        for (k = 0; k < 3; k++) {
            rc = read(fid[k],
                      evt,
                      sizeof(struct input_event) * 8);

            if (rc != -1) {
                nevts = rc / sizeof(struct input_event);
                pd = 1;
                for (e = 0; e < nevts; e++) {
                    if (evt[e].type) {
                        /* fprintf(stderr, "%d: %d\n", k, evt[e].value); */
                        tmp = m.vals[k];
                        tmp += evt[e].value * 0.01;
                        if (tmp < 0) tmp = 0;
                        if (tmp > 1) tmp = 1;
                        m.vals[k] = tmp;
                    }
                }
            }
        }
        m.please_draw = pd;
        usleep(800);
    }

    audio_stop(&m);
    pthread_join(draw_thread, NULL);
    for (k = 0; k < 3; k++) close(fid[k]);
    fb_cleanup(&m.fb);
    return 0;
}
