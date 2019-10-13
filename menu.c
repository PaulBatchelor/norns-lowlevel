#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/input.h>

#include "font.xbm"

typedef struct {
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    char *fbp;
    int fbfd;
    long int screensize;
} the_screen;

void pixel(struct fb_var_screeninfo *vinfo,
           struct fb_fix_screeninfo *finfo,
           char *fbp,
           int x,
           int y,
           unsigned char val)
{
    long int location;

    if(x >= vinfo->xres || y >= vinfo->yres) return;

    location = (x + vinfo->xoffset) * (vinfo->bits_per_pixel/8) +
               (y + vinfo->yoffset) * finfo->line_length;

    fbp[location] = 0x0;
    fbp[location + 1] = val;
}

void clearscreen(struct fb_var_screeninfo *vinfo,
                 struct fb_fix_screeninfo *finfo,
                 char *fbp)
{
    int x, y;

    for(y = 0; y < vinfo->yres; y++) {
        for(x = 0; x < vinfo->xres; x++) {
            pixel(vinfo, finfo, fbp, x, y, 0);
        }
    }
}

void draw_glyph(struct fb_var_screeninfo *vinfo,
                struct fb_fix_screeninfo *finfo,
                char *fbp,
                int offx, int offy,
                int gx, int gy, int invert)
{
    int x, y;
    unsigned char v;
    char b;
    int stride;

    stride = font_width / 8; /* divide by 8 */

    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            b = font_bits[(8 * gy + y) * stride + gx];
            v = ((b & (1 << x)) != 0) * 0xff;
            if (invert) {
                v = ((b & (1 << x)) == 0) * 0xff;
            } else {
                v = ((b & (1 << x)) != 0) * 0xff;
            }
            pixel(vinfo, finfo, fbp, x + offx, y + offy, v);
        }
    }
}

void draw_char(struct fb_var_screeninfo *vinfo,
               struct fb_fix_screeninfo *finfo,
               char *fbp,
               int offx, int offy,
               char c, int invert)
{
    int gx, gy;
    char o;

    o = c - ' '; /* start at 0 */

    gx = o % (font_width >> 3);
    gy = o / (font_width >> 3);

    draw_glyph(vinfo, finfo, fbp, offx, offy, gx, gy, invert);
}

void draw_string(struct fb_var_screeninfo *vinfo,
                 struct fb_fix_screeninfo *finfo,
                 char *fbp,
                 int offx, int offy,
                 const char *str,
                 int invert)
{
    int len;
    int n;
    len = strlen(str);

    for (n = 0; n < len; n++) {
        draw_char(vinfo, finfo, fbp, offx + 8*n, offy, str[n], invert);
    }
}

void draw_bar(struct fb_var_screeninfo *vinfo,
              struct fb_fix_screeninfo *finfo,
              char *fbp,
              int row)
{
    int start;
    int x, y;

    start = row * 8;

    for (y = 0; y < 8; y++) {
        for (x = 0; x < vinfo->xres; x++) {
            pixel(vinfo, finfo, fbp, x, start + y, 0xff);
        }
    }
}

const char *menu_items[10] =
{
"Lions",
"Tigers",
"Bears",
"Penguins",
"Cats",
"Dogs",
"Hamsters",
"Elephants",
"Mice",
"Aardvarks",
};

void open_screen(the_screen *scrn)
{
    scrn->fbp = NULL;
    scrn->fbfd = 0;
    scrn->fbfd = open("/dev/fb0", O_RDWR);

    if (scrn->fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }

    if (ioctl(scrn->fbfd, FBIOGET_FSCREENINFO, &scrn->finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    if (ioctl(scrn->fbfd, FBIOGET_VSCREENINFO, &scrn->vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    scrn->screensize = scrn->vinfo.xres *
        scrn->vinfo.yres *
        scrn->vinfo.bits_per_pixel / 8;

    scrn->fbp = (char *)mmap(0,
                             scrn->screensize,
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED,
                             scrn->fbfd,
                             0);

    if ((int)scrn->fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }

}

void close_screen(the_screen *scrn)
{
    munmap(scrn->fbp, scrn->screensize);
    close(scrn->fbfd);
}

typedef struct {
    int start;
    int selected;
    int row;
} menu_stuff;

static void draw_menu(the_screen *scrn, menu_stuff *ms)
{
    int m;
    int is_selected;
    clearscreen(&scrn->vinfo, &scrn->finfo, scrn->fbp);
    for (m = 0; m < 8; m++) {
        is_selected = m == ms->row;
        if (is_selected) {
            draw_bar(&scrn->vinfo,
                     &scrn->finfo,
                     scrn->fbp, m);
        }
        draw_string(&scrn->vinfo, &scrn->finfo, scrn->fbp,
                    0, 8*m,
                    menu_items[m + ms->start],
                    is_selected);
    }

}

int open_buttons(void)
{
    return open(
        "/dev/input/by-path/platform-keys-event",
        O_RDONLY | O_NONBLOCK);
}

int open_knob(int knob)
{
    char path[256];
    sprintf(path,
            "/dev/input/by-path/platform-soc:knob%d-event",
            knob);
    return open(path, O_RDONLY | O_NONBLOCK);
}

void open_all_knobs(int *fid)
{
    int k;
    for (k = 0; k < 3; k++) {
        fid[k] = open_knob(k + 1);
    }
}

void close_all_knobs(int *fid)
{
    int k;
    for (k = 0; k < 3; k++) {
        close(fid[k]);
    }
}

int main()
{
    the_screen scrn;
    menu_stuff ms;

    /* peripherals */
    int fid[3];
    int bfid;

    int running;

    /* input polling */
    struct input_event evt[8];
    int nevts;
    int e;
    int k;
    int changed;
    int code;
    int rc;

    running = 1;
    bfid = open_buttons();
    open_all_knobs(fid);
    changed = 0;

    open_screen(&scrn);

    ms.start = 0;
    ms.selected = 0;
    ms.row = 0;

    draw_menu(&scrn, &ms);

    while (running) {
        changed = 0;
        for (k = 0; k < 3; k++) {
            rc = read(fid[k],
                      evt,
                      sizeof(struct input_event) * 8);

            if (rc != -1) {
                nevts = rc / sizeof(struct input_event);
                for (e = 0; e < nevts; e++) {
                    if (evt[e].type) {
                        ms.row += evt[e].value;
                        if (ms.row < 0) {
                            ms.start--;
                            if(ms.start < 0) ms.start = 0;
                            ms.row = 0;
                        } else if (ms.row >= 8) {
                            ms.start++;
                            if (ms.start + 8 > 10) {
                                ms.start = 10 - 8;
                            }
                            ms.row = 7;
                        }
                        changed = 1;
                    }
                }
            }
        }
        rc = read(bfid, evt, sizeof(struct input_event) * 8);
        if(rc != -1) {
            nevts = rc / sizeof(struct input_event);
            for(e = 0; e < nevts; e++) {
                if(evt[e].type) {
                    fprintf(stdout,
                            "%d %d\n",
                            evt[e].code,
                            evt[e].value);
                    fflush(stdout);

                    code = evt[e].code;
                    switch(code) {
                        case 1:
                            running = 0;
                            break;
                        case 2:
                            break;
                        case 3:
                            break;
                    }
                }
            }
        }
        if (changed) {
            draw_menu(&scrn, &ms);
        }
        usleep(800);
    }

    close_screen(&scrn);
    close(bfid);
    close_all_knobs(fid);
    return 0;
}
