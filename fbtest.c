#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define SMILES_WIDTH 8
#define SMILES_HEIGHT 8

const char *smiles[]= {
"--####--",
"-#----#-",
"#-#--#-#",
"#------#",
"#-#--#-#",
"#--##--#",
"-#----#-",
"--####--",
};

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

void draw_smiles(struct fb_var_screeninfo *vinfo,
                 struct fb_fix_screeninfo *finfo,
                 char *fbp)
{
    int x, y;
    unsigned char v;
    int offx, offy;

    offx = (vinfo->xres/2) - SMILES_WIDTH;
    offy = (vinfo->yres/2) - SMILES_HEIGHT;

    for(y = 0; y < SMILES_HEIGHT; y++) {
        for(x = 0; x < SMILES_WIDTH; x++) {
            if(smiles[y][x] == '#') v = 0xff;
            else v = 0;
            pixel(vinfo, finfo, fbp, x + offx, y + offy, v);
        }
    }
}

int main()
{
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;

    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }

    clearscreen(&vinfo, &finfo, fbp);

    draw_smiles(&vinfo, &finfo, fbp);

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
