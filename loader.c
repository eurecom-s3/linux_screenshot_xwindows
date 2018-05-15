#include "loader.h"


char map_file[4096];

int load_mapping(char *dir, char *file){
    int pid;
    long long address;

    sscanf(file, "task.%d.%llx", &pid, &address);

    memset(map_file, 0, 4096);
    sprintf(map_file, "%s/%s", dir, file);
    
    int fd = open(map_file,O_RDWR);
    if(fd < 0){
        printf("[-] loader: opening '%s' failed, aborting!\n", map_file);
        exit(-1);
    }        
        
    struct stat stats;
    fstat(fd,&stats);
    int filesize = stats.st_size;
    unsigned long long res = (unsigned long long)mmap((void *) address, filesize,
                                                      PROT_READ|PROT_WRITE|PROT_EXEC,
                                                      MAP_PRIVATE|MAP_FIXED, fd, 0);
    if(res < 1){
        printf("[-] loader: mmap failed, aborting!\n");
        exit(-1);
    } 
    return 0;   
}

void dump_to_fd(FILE *fd, char *output_buf,int w){
    int i = 0;
    int k;
    k=w*3;
    char *zero = malloc(k);
    memset(zero, 0, k);
    int written = 0;
    while(((unsigned int *)(output_buf))[i/4] != 0x0 ){
        fwrite(output_buf+i, 1, 3, fd);
        written += 3;
        i+=4;
        if (written == k){ 
            fwrite(zero, 1,w, fd);
            written = 0;
        }
    }
    return;
}


compGetImage_t compGetImage = 0x00;

void compGetImage_wrapper(DrawablePtr pDrawable,
                           int sx, int sy, int w, int h,
                           unsigned int format, unsigned long planemask, char *pdstLine,int OUTPUT_SIZE){
    memset(pdstLine, 0, OUTPUT_SIZE);
    compGetImage(pDrawable, sx, sy, w, h, format, planemask, pdstLine); 
}


void
_swaplong (register char *bp, register unsigned n)
{
    register char c;
    register char *ep = bp + n;

    while (bp < ep) {
        c = bp[3];
        bp[3] = bp[0];
        bp[0] = c;
        c = bp[2];
        bp[2] = bp[1];
        bp[1] = c;
        bp += 4;
    }
}

void
_swapshort (register char *bp, register unsigned n)
{
    register char c;
    register char *ep = bp + n;

    while (bp < ep) {
        c = *bp;
        *bp = *(bp + 1);
        bp++;
        *bp++ = c;
    }
}
static int Get24bitDirectColors(XColor **colors)
{
    int i , ncolors = 256 ;
    XColor *tcol ;

    *colors = tcol = (XColor *)malloc(sizeof(XColor) * ncolors) ;

    for(i=0 ; i < ncolors ; i++)
        {
            tcol[i].pixel = i << 16 | i << 8 | i ;
            tcol[i].red = tcol[i].green = tcol[i].blue = i << 8   | i ;
        }
    
    return ncolors ;
}

int write_header(DrawablePtr draw,VisualPtr vis,unsigned short border_width,int imageByteOrder,int bitmapScanlineUnit,int bitmapScanlinePad,int bitmapBitOrder,FILE *fd){
	CARD32 header_size;
	XWDFileHeader header;
	XWDColor xwdcolor;
	size_t win_name_size;
	int ncolors;
	XColor *colors;
	char win_name[] = "xwdump";
    int i;

    win_name_size = strlen(win_name) + sizeof(char);
    header_size = SIZEOF(XWDheader) + (CARD32) win_name_size;

    /*
     * Write out header information.
     */
    header.header_size = (CARD32) header_size;
    header.file_version = (CARD32) XWD_FILE_VERSION;
    header.pixmap_format = (CARD32) 2; //format zpixmap
    header.pixmap_depth = (CARD32) (*draw).depth;
    header.pixmap_width = (CARD32) (*draw).width;
    header.pixmap_height = (CARD32) (*draw).height;
    header.xoffset = (CARD32) 0;   /* Bitmap x offset, normally 0 */
    header.byte_order = (CARD32) imageByteOrder;   /* of image data: MSBFirst, LSBFirst */ 
    header.bitmap_unit = (CARD32) bitmapScanlineUnit;/* bitmap_unit applies to bitmaps (depth 1 format XY) only It is the number of bits that each scanline is padded to. */
    header.bitmap_bit_order = (CARD32) bitmapBitOrder; /* bitmaps only: MSBFirst, LSBFirst */
    header.bitmap_pad = (CARD32) bitmapScanlinePad;/*  8    bitmap_pad applies to pixmaps (non-bitmaps) only.*/
    header.bits_per_pixel = 24; //(CARD32) (*draw).bitsPerPixel;
    header.bytes_per_line = (CARD32)bitmapScanlinePad* (*draw).width / 8; //bytes_per_line is pixmap_width padded to bitmap_unit (bitmaps)
    header.visual_class = (CARD32) (*vis).class; // 5 or 4	  
    header.red_mask = (CARD32) (*vis).redMask;
    header.green_mask = (CARD32) (*vis).greenMask;
    header.blue_mask = (CARD32) (*vis).blueMask;
    header.bits_per_rgb = (CARD32) (*vis).bitsPerRGBValue;
    header.colormap_entries = (CARD32) (*vis).ColormapEntries; 

    ncolors = Get24bitDirectColors(&colors) ;
    header.ncolors = (CARD32) (*vis).ColormapEntries;//256;//ncolors;
    header.window_width = (CARD32) (*draw).width;
    header.window_height = (CARD32) (*draw).height;
    header.window_x = (*draw).x;
    header.window_y = (*draw).y;
    header.window_bdrwidth = (CARD32) border_width;

    _swaplong((char *) &header, sizeof(header)); 
    for (i = 0; i < ncolors; i++) {
        _swaplong((char *) &colors[i].pixel, sizeof(CARD32)); 
        _swapshort((char *) &colors[i].red, 3 * sizeof(short));
    }

    if (fwrite((char *)&header, SIZEOF(XWDheader), 1, fd) != 1 ||
        fwrite(win_name, win_name_size, 1, fd) != 1) {
        perror("xwd");
        exit(1);
    }
    
    for (i = 0; i < ncolors; i++) {
        xwdcolor.pixel = colors[i].pixel;
        xwdcolor.red = colors[i].red;
        xwdcolor.green = colors[i].green;
        xwdcolor.blue = colors[i].blue;
        xwdcolor.flags = colors[i].flags;
        if (fwrite((char *) &xwdcolor, SIZEOF(XWDColor), 1, fd) != 1) {
            perror("xwd");
            exit(1);
        }
    }

    return 0;
}


int dump_screenshot(unsigned long long pdrawableAddr, unsigned long long pvisualAddr, unsigned short border_width,
                    int imageByteOrder, int bitmapScanlineUnit, int bitmapScanlinePad, int bitmapBitOrder, char *out_dir){

    DrawablePtr pDrawable = (DrawablePtr) pdrawableAddr;
    VisualPtr pVisual = (VisualPtr)pvisualAddr;    
    char filename[256];
    /* __asm__("int $3;"); */
    if ((pVisual->class!=4) && (pVisual->class!=5)){
        return 0;
    }

    int OUTPUT_SIZE=((bitmapScanlinePad* (*pDrawable).width / 8) * (*pDrawable).height);;
    char *output_buf = (char *)malloc(OUTPUT_SIZE);
    compGetImage_wrapper(pDrawable, 0, 0, (*pDrawable).width, (*pDrawable).height, /*ZPixmap*/ 2, 4294967295, output_buf, OUTPUT_SIZE);

    if (strlen(output_buf) == 0)
        return 0;
    
    snprintf(filename, 256, "%s/screenshot_%x.xwd", out_dir, pDrawable->id);
    
    FILE *fd = fopen(filename, "w+");
    write_header(pDrawable, pVisual, border_width, imageByteOrder, bitmapScanlineUnit, bitmapScanlinePad, bitmapBitOrder, fd);
    dump_to_fd(fd, output_buf, (*pDrawable).width);
    printf("[+] loader: saving xwd image %s for ID : %x\n", filename, pDrawable->id);
    
    fclose(fd);
    free(output_buf);
    return 0;
}

int main(int argc, char **argv){
    DIR *p;
    struct dirent *pp;
    char *dir = argv[9];
    printf("[+] loader: Loading mappings from %s\n", argv[9]);
    p = opendir(dir);
    if (p != NULL){
        while ((pp = readdir (p)) != NULL) {
            int length = strlen(pp->d_name);
            if (strncmp(pp->d_name + length - 4, ".vma", 4) == 0) {
                load_mapping(dir, pp->d_name);
            }
        }        
        closedir (p);
    }
    compGetImage = (compGetImage_t)atoll(argv[8]);
    dump_screenshot(atoll(argv[1]), atoll(argv[2]), atoi(argv[3]), atoi(argv[4]),
                    atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), argv[10]);
    
    return 0;
}
