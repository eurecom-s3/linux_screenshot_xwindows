#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

# if defined (_LP64) || defined(__LP64__) ||    \
    defined(__alpha) || defined(__alpha__) ||   \
    defined(__ia64__) || defined(ia64) ||       \
    defined(__sparc64__) ||                     \
    defined(__s390x__) ||                       \
    defined(__amd64__) || defined(amd64) ||     \
    defined(__powerpc64__)
#  if !defined(__ILP32__) /* amd64-x32 is 32bit */
#   define LONG64				/* 32/64-bit architecture */
#  endif /* !__ILP32__ */
# endif

# define _SIZEOF(x) sz_##x
# define SIZEOF(x) _SIZEOF(x)
#define XWD_FILE_VERSION 7
#define sz_XWDheader 100
#define sz_XWDColor 12
#define lowbit(x) ((x) & (~(x) + 1))
#define TrueColor	4
#define DirectColor	5
# define B32
# define B16
# ifdef LONG64
typedef long INT64;
typedef int INT32;
# else
typedef long INT32;
# endif
typedef short INT16;
typedef signed char    INT8;
# ifdef LONG64
typedef unsigned long CARD64;
typedef unsigned int CARD32;
# else
typedef unsigned long long CARD64;
typedef unsigned long CARD32;
# endif
typedef unsigned short CARD16;
typedef unsigned char  CARD8;
typedef CARD32		BITS32;
typedef CARD16		BITS16;


typedef struct _xwd_file_header {
	
	CARD32 header_size B32;		
	CARD32 file_version B32;	/* = XWD_FILE_VERSION above */
	CARD32 pixmap_format B32;	/* ZPixmap or XYPixmap */
	CARD32 pixmap_depth B32;	/* Pixmap depth */
	CARD32 pixmap_width B32;	/* Pixmap width */
	CARD32 pixmap_height B32;	/* Pixmap height */
	CARD32 xoffset B32;		/* Bitmap x offset, normally 0 */
	CARD32 byte_order B32;		/* of image data: MSBFirst, LSBFirst */

	/* bitmap_unit applies to bitmaps (depth 1 format XY) only.
	 * It is the number of bits that each scanline is padded to. */
	CARD32 bitmap_unit B32;		
	CARD32 bitmap_bit_order B32;	/* bitmaps only: MSBFirst, LSBFirst */

	/* bitmap_pad applies to pixmaps (non-bitmaps) only.
	 * It is the number of bits that each scanline is padded to. */
	CARD32 bitmap_pad B32;		
	CARD32 bits_per_pixel B32;	/* Bits per pixel */

	/* bytes_per_line is pixmap_width padded to bitmap_unit (bitmaps)
	 * or bitmap_pad (pixmaps).  It is the delta (in bytes) to get
	 * to the same x position on an adjacent row. */
	CARD32 bytes_per_line B32;
	CARD32 visual_class B32;	/* Class of colormap */
	CARD32 red_mask B32;		/* Z red mask */
	CARD32 green_mask B32;		/* Z green mask */
	CARD32 blue_mask B32;		/* Z blue mask */
	CARD32 bits_per_rgb B32;	/* Log2 of distinct color values */
	CARD32 colormap_entries B32;	/* Number of entries in colormap; not used? */
	CARD32 ncolors B32;		/* Number of XWDColor structures */
	CARD32 window_width B32;	/* Window width */
	CARD32 window_height B32;	/* Window height */
	CARD32 window_x B32;		/* Window upper left X coordinate */
	CARD32 window_y B32;		/* Window upper left Y coordinate */
	CARD32 window_bdrwidth B32;	/* Window border width */
} XWDFileHeader;

typedef struct {
    CARD32	pixel B32;
    CARD16	red B16;
	CARD16	green B16;
	CARD16	blue B16;
    CARD8	flags;
    CARD8	pad;
} XWDColor;


/* taken from xorg-server/include/pixmapstr.h */
typedef unsigned int XID;
#pragma pack(1)
struct _Visual {
    int vid;
    short class;
    short bitsPerRGBValue;
    short ColormapEntries;
    short nplanes;
    int nplanes11;
    unsigned long redMask;
    unsigned long greenMask;
    unsigned long blueMask;
    int offsetRed;
    int offsetGreen;
    int offsetBlue;
} ;
//visual structure was modified in order to get the right values !! 
typedef struct _Visual *VisualPtr;
struct  _Drawable {
    unsigned char type;         /* DRAWABLE_<type> */
    unsigned char class;        /* specific to type */
    unsigned char depth;
    unsigned char bitsPerPixel;
    XID id;                     /* resource id */
    short x;                    /* window: screen absolute, pixmap: 0 */
    short y;                    /* window: screen absolute, pixmap: 0 */
    unsigned short width;
    unsigned short height;
    unsigned long long *pScreen;
    unsigned long serialNumber;
};


typedef struct {
	unsigned long pixel;			/* pixel value */
	unsigned short red, green, blue;	/* rgb values */
	char flags;				/* DoRed, DoGreen, DoBlue */	
	char pad;
} XColor;

typedef struct _Drawable *DrawablePtr;
typedef int (*compGetImage_t)(DrawablePtr, int, int, int, int, unsigned int, unsigned long, char *);
