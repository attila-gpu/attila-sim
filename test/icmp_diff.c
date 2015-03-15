/*
** ICMP - A utility to find statistics of the error between two images
** Copyright (C) 1998-2000 Fred Wheeler
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** The GNU General Public License can also be found at
**   http://www.gnu.org/copyleft/gpl.html
**
** Fred Wheeler
** wheeler@cipr.rpi.edu
*/

/*
** OVERVIEW
**
**  ICMP is a program for comparing images and printing statistics of
**  the difference between the images.  It is handy for comparing
**  original and decompressed images to evaluate lossy compression
**  performance.  PSNR, RMSE, max difference and mean absolute error
**  are all computed and printed.  ICMP can also be given the name of
**  a compressed data file and print its size in bytes and in bits per
**  pixel.
**
**  The format in which the statistics are printed is configurable on
**  the command line.  For example, the format string '%b %p %r\n'
**  causes ICMP to print the compressed file size in bits-per-pixel
**  followed by the distortion in PSNR and then the RMSE.  You can
**  also use complicated format strings with text and tabs such as
**  'psnr: %p\trmse: %r\n'.
**
**  ICMP understands image files in RAW (raster scan with no header or
**  encoding), PGM and PPM format.  RAW format images can be grayscale
**  or coler and can have one or two bytes per pixel.
**
**  ICMP should compile with just about any C compiler. I have
**  compiled it with a recent gcc, an old gcc and cc on HP-UX, Solaris
**  and SunOS. If your C compiler won't compile ICMP, please e-mail me
**  the error messages the compiler generates.
**
**  The latest version of ICMP can be found at
**    http://www.cipr.rpi.edu/wheeler/icmp
**
** COMPILING
**
**   ICMP can be compiled with either an ANSI or an older K&R C compiler.
**   It was written to be very portable.  Example compile commands are:
**     gcc -O icmp.c -o icmp -lm
**     cc -O icmp.c -o icmp -lm
**
** EXAMPLE
**
**   For verification, the following ICMP commands compare the
**   standard 512 by 512 lena and goldhill images with the lena raw
**   format image used as the optional encoded file in lieu an actual
**   encoded file.  Of course, it makes no sense to find the error
**   between these two unrelated images.  This is just a test.  The
**   first command below uses raw format images and the second uses
**   the PGM format.  The output from the commands follows.
**
**     icmp -i1 lena.raw -i2 goldhill.raw -e lena.raw -s 512 512
**     icmp -i1 lena.pgm -i2 goldhill.pgm -e lena.pgm
**
**     psnr: 11.1265 dB
**     rmse: 70.8295
**     mean absolute error: 58.6452
**     max absolute error: 201
**     max positive error: 201
**     max negative error: 177
**     compressed file size: 262144 bytes, 8 bits per pixel
**
** HELP
**
**   Use the -h option to ICMP for help.  All help is built into the
**   ICMP program.  There is no additional man or info page.
**
** SUGGESTIONS AND BUGS
**
**   Please contact the author with any suggestions or bug reports.
**
** CREDIT
**
**   Thanks to Alan Chien for finding a bug in how version 1.0
**   computed maximum error.
**
** CHANGE LOG
**
**   Version 1.0 to 1.1 -- Dec. 2, 1998
**     Fixed bug in maximum absolute error computation
**     Now compute and report max pos/neg/abs error
**     Stopped check for EOF when image is from stdin
**     Changed many variable names for clarity
**     Made default format more verbose
**     Added and fixed many comments
**
**   Version 1.1 to 1.2 -- May 31, 2000
**     Fixed some comments
**     Added PPM image format support
**     Find bits-per-PIXEL, not bits-per-COMPONENT-PIXEL
**     Allow PGM/PPM headers to end with CR/LF pair instead of a single char
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* SEEK_END is not defined on some systems. */
#ifndef SEEK_END
#define SEEK_END 2
#endif

/* name of the program used when printing error messages */
static char * progname;

/*
**  Each function declaration is given in both ANSI style and K&R style
**  If the __ANSI__ macro is defined the ANSI format definition is used,
**  otherwise the K&R format definition is used.
*/

/*
**  Print detailed usage information and exit.  All ICMP help is right here.
**  There is no additional man page or other information.
*/

#ifdef __STDC__
static void print_usage (void)
#else
static void print_usage ()
#endif
{
  char **l;                     /* pointer to one of the help line strings */

  /* this help text is an array of strings; one string per line */
  static char *help_text[] = {
  "This is ICMP version 1.2.",
  "",
  "ICMP reads two image files and prints statistics of the error between",
  "the images.  It can also be given the name of a compressed data file.",
  "It will print the size of this file and bits per pixel.",
  "",
  "Command line options:",
  "",
  "  All command line options have an equivalent short and long form.",
  "",
  "-h, -help",
  "    Print this usage information.",
  "",
  "-i1 IMAGE1, -image1 IMAGE1 [ default: standard input ]",
  "    Specify the file name of image 1.  Use '-' for standard input.",
  "",
  "-i2 IMAGE2, -image2 IMAGE2 [ default: standard input ]",
  "    Specify the file name of image 2.  Use '-' for standard input.",
  "",
  "-s ROWS COLUMNS, -size ROWS COLUMNS [ default: 512 512 ]",
  "    Specify the size of the images.",
  "",
  "-p NUMPLANES, -planes NUMPLANES [ default: 1 ]",
  "    Specify the number of color planes in the images.  This will",
  "    most likely be 1 or 3, but any positive value can be used.",
  "",
  "-d DEPTH, -depth DEPTH [ default: 8 ]",
  "    Specify the bit depth of the images.  If the depth is between 1",
  "    and 8 one byte is read per pixel.  If the depth is between 9 and",
  "    16, two bytes are read per pixel, most signifigant byte first.",
  "    ICMP does not check to see whether the pixel value read actually",
  "    fits in DEPTH bits.",
  "",
  "-s1 NUM, -skip1 NUM [ default: 0 ]",
  "    Specify the number of header bytes to skip in image file 1.",
  "",
  "-s2 NUM, -skip2 NUM [ default: 0 ]",
  "    Specify the number of header bytes to skip in image file 2.",
  "",
  "-cee, -check-end-eof [ default: yes ]",
  "    Tell ICMP to make sure that after all data is read in both images",
  "    the end of the file has ben reached.",
  "",
  "-ncee, -no-check-end-eof [ default: yes ]",
  "    Tell ICMP to skip the end of file check.",
  "",
  "-e FILE, -encode FILE [ default: none ]",
  "    Specify the encoded data file.  If given, ICMP will print",
  "    its size in bytes and bits-per-pixel.",
  "",
  "-f FORMAT, -format FORMAT [ default: see below ]",
  "    Specify the format for printing results of the comparison.",
  "",
  "-silent",
  "    Do not output anything.",
  "",
  "The -f FORMAT option allows you to specify how the image statistics are",
  "printed in a flexible way similar to the format string for the printf",
  "command.  Each character from the format string is printed unless it is",
  "part of an escape sequence.  Escape sequences ate two-character sequences",
  "beginning with '\\' to print special characters or '%' to print computed",
  "statistics.  The escape sequences recognized are:",
  "  '\\b': backspace",
  "  '\\f': formfeed",
  "  '\\n': newline",
  "  '\\r': carriage return",
  "  '\\t': tab",
  "  '\\v': vertcal tab",
  "  '\\\\': \\",
  "  '%p': peak signal to noise ratio",
  "  '%r': root mean square error",
  "  '%x': maximum absolute error",
  "  '%y': maximum positive error",
  "  '%z': maximum negative error (as a positive number)",
  "  '%m': mean absolute error",
  "  '%b': compressed file bits per pixel",
  "  '%s': compressed file size in bytes",
  "  '%k': peak signal (2^depth-1)",
  "  '%%': %",
  "The default format displays all statistics except %k.",
  "",
  "Note that you cannot specify printf style field widths or other",
  "modifiers for the % escape sequences, as in '%7.2r'.",
  "",
  "For the purposes of computing maximum positive and negative error, the",
  "second image is subtracted from the first.",
  "",
  "ICMP will print an error message and exit if either input image has",
  "extra bytes at its end, unless you use the '-ncee' option.",
  "",
  "By default both images are read from standard input.  This default",
  "must be overidden by giving a file for at least one of the images",
  "otherwise, an error message is printed and the program exits.",
  "",
  "Please contact me (e-mail below) with bug reports, bug fixes, suggestions",
  "for improving this program, or if you would like to be notified after",
  "signifigant enhancements are made.",
  "",
  "Copyright (C) 1998-2000  Fred Wheeler",
  "Licensed under the terms of the GNU General Public License",
  "http://www.gnu.org/copyleft/gpl.html",
  "",
  "The latest version of ICMP can be found at",
  "http://www.cipr.rpi.edu/wheeler/icmp",
  "",
  "Fred Wheeler",
  "wheeler@cipr.rpi.edu",
  NULL};

  /* loop through each line of the above help text */
  for (l = help_text; NULL != *l; l++)
    fprintf (stderr, "%s\n", *l);

  exit (0);
  return;
}

/*
**  If COND is true, print an error message that includes string MSG
**  and then exit with an error status.  This is used like assert().
*/

#ifdef __STDC__
static void checke (int cond, char * msg)
#else
static void checke (cond, msg) int cond; char * msg;
#endif
{
  if (cond) {
    fprintf (stderr, "%s: %s\n", progname, msg);
    fprintf (stderr, "%s: use '-h' option for help\n", progname);
    exit (1);
  }
}

/*
**  If COND is true, print an error message that includes strings MSG
**  and MSG2 and then exit with an error status.  This is used like
**  assert().
*/

#ifdef __STDC__
static void checke2 (int cond, char * msg, char * msg2)
#else
static void checke2 (cond, msg, msg2) int cond; char * msg; char * msg2;
#endif
{
  if (cond) {
    fprintf (stderr, "%s: %s '%s'\n", progname, msg, msg2);
    fprintf (stderr, "%s: use -h option for help\n", progname);
    exit (1);
  }
}

/*
**  Read and ignore white space characters and commented characters
**  from the file stream.  Comments are delimited by a '#' character
**  and a newline character or the end of the file.  This is the
**  syntax of comments for PNM headers and many script languages.
*/

#ifdef __STDC__
static void eat_white (FILE *fp)
#else
static void eat_white (fp) FILE *fp;
#endif
{
  int ch;

  while ((ch=getc(fp)) != EOF) {
    if (ch == '#') {
      /* read and discard characters up to the end of a line or file */
      do {
        ch = getc (fp);
      } while (ch != '\n' && ch != EOF);
    } else if (ch == ' ' || ch == '\t' || ch == '\n') {
      /* keep eating since this is a white space character */
      continue;
    } else {
      /* this character is OK, so put it back and return */
      ungetc (ch, fp);
      return;
    }
  }

  return;
}

/*
**  Read a PGM header.  See read_header() for explanation of arguments.
*/

#ifdef __STDC__
static void read_pgm_header
(char *fn, FILE *fp, int *nr, int *nc, int *np, int *dp, int *pk)
#else
static void read_pgm_header (fn, fp, nr, nc, np, dp, pk)
  char *fn; FILE *fp; int *nr; int *nc; int *np; int *dp; int *pk;
#endif
{
  int rv;

  eat_white (fp);
  rv = fscanf(fp, "%d", nc);
  checke2 (1!=rv, "invalid PGM header in image file", fn);
  rv = fscanf(fp, "%d", nr);
  checke2 (1!=rv, "invalid PGM header in image file", fn);
  rv = fscanf(fp, "%d", pk);
  checke2 (1!=rv, "invalid PGM header in image file", fn);
  *dp = 8;
  *np = 1;
  rv = fgetc (fp);
  checke2 (EOF==rv, "reached EOF during header in image file", fn);
  /* the header should end with a single character, but files from DOS */
  /* may mistakenly end the header with a CR/LF pair */
  if (rv == '\r') {
    rv = fgetc (fp);
    checke2 (EOF==rv, "reached EOF during header in image file", fn);
    checke2 ('\n'!=rv, "LF without CR in header of image file", fn);
  }

  return;
}

/*
**  Read a PPM header.  See read_header() for explanation of arguments.
*/

#ifdef __STDC__
static void read_ppm_header
(char *fn, FILE *fp, int *nr, int *nc, int *np, int *dp, int *pk)
#else
static void read_ppm_header (fn, fp, nr, nc, np, dp, pk)
  char *fn; FILE *fp; int *nr; int *nc; int *np; int *dp; int *pk;
#endif
{
  int rv;

  eat_white (fp);
  rv = fscanf(fp, "%d", nc);
  checke2 (1!=rv, "invalid PPM header in image file", fn);
  rv = fscanf(fp, "%d", nr);
  checke2 (1!=rv, "invalid PPM header in image file", fn);
  rv = fscanf(fp, "%d", pk);
  checke2 (1!=rv, "invalid PPM header in image file", fn);
  *dp = 8;
  *np = 3;
  rv = fgetc (fp);
  checke2 (EOF==rv, "reached EOF during header in image file", fn);
  /* the header should end with a single character, but files from DOS */
  /* may mistakenly end the header with a CR/LF pair */
  if (rv == '\r') {
    rv = fgetc (fp);
    checke2 (EOF==rv, "reached EOF during header in image file", fn);
    checke2 ('\n'!=rv, "LF without CR in header of image file", fn);
  }

  return;
}

/*
**  Read an image file header.  Right now PGM and PPM are supported.
**  This is set up to allow future expansion to formats that can be
**  determined by the first two characters of the header.
**
**  Arguments:
**    fn: [input] image file name string for use in error messages
**    fp: [input] file pointer to image file at beginning of file
**    nr: [output] number of image rows
**    nc: [output] number of image columns
**    np: [output] number of image color planes
**    dp: [output] depth, number of bit planes
**    pk: [output] peak pixel value
*/

#ifdef __STDC__
static void read_header
(char *fn, FILE *fp, int *nr, int *nc, int *np, int *dp, int *pk)
#else
static void read_header (fn, fp, nr, nc, np, dp, pk)
  char *fn; FILE *fp; int *nr; int *nc; int *np; int *dp; int *pk;
#endif
{
  int ch1, ch2;                 /* first and second characters of header */

  ch1 = fgetc(fp);
  checke (EOF == ch1, "end of file reached in header");

  switch (ch1) {
  case 'P':
    ch2 = fgetc(fp);
    checke (EOF == ch2, "end of file reached in header");
    switch (ch2) {
    case '5':
      read_pgm_header (fn, fp, nr, nc, np, dp, pk);
      break;
    case '6':
      read_ppm_header (fn, fp, nr, nc, np, dp, pk);
      break;
    default:
      checke2 (1, "unknown magic number in image file", fn);
      break;
    }
    break;
  default:
    checke2 (1, "unknown magic number in image file", fn);
    break;
  }

  return;
}

void writePPMfile(char *fname, int* buffer, unsigned int height, unsigned int width, unsigned int maxrgb)
{
    FILE *fout = NULL; 
    int i, j;
    int* index;

    fout = fopen(fname, "wb");
    checke2 ( ! fout, "could not open output image file", fname);

    // Print header        
    fprintf(fout, "P6 %d %d %d ", width, height, maxrgb);

    // Image data

    index = buffer;
 
    for (  i = 0 ; i < height; i++ )
    {
        for ( j = 0; j < width; j++ )
        {
            fputc( (*index), fout ); index++;
            fputc( (*index), fout ); index++;
            fputc( (*index), fout ); index++;
        }
    }
    
    fclose(fout);
};

/*
**  Main.
*/

#ifdef __STDC__
int main (int argc, char **argv)
#else
int main (argc, argv) int argc; char **argv;
#endif
{
  char *fn1 = "-";              /* file name for image 1 */
  char *fn2 = "-";              /* file name for image 2 */
  char *fne = NULL;             /* file name for encoded data */
  char *output_filename = NULL; /* file name for diff image */
  FILE *fp1 = NULL;             /* file stream for image 1 */
  FILE *fp2 = NULL;             /* file stream for image 1 */
  FILE *fpe = NULL;             /* file stream for encoded data */

  /* image size parameters, set to the defaults */
  int nrows = 512;              /* number of rows in the images */
  int ncols = 512;              /* number of columns in the images */
  int nplanes = 1;              /* number of color planes in the images */
  int depth = 8;                /* bit depth of color planes in both images */
  int peak = 0;                 /* peak component value in both images */

  /* image size parameters for each image, used until we know both */
  /* images have the same parameter values */
  int nrows1, nrows2;           /* number of rows in each image */
  int ncols1, ncols2;           /* number ofcolumns in each image */
  int nplanes1, nplanes2;       /* number of color planes in each image */
  int depth1, depth2;           /* bit depth of color planes in each image */
  int peak1, peak2;             /* peak component value in each image */

  int skip1 = 0;                /* header bytes to skip in image 1 */
  int skip2 = 0;                /* header bytes to skip in image 2 */
  int iskip = 0;                /* counter for skipped header bytes */
  int ch = 0;                   /* byte read from header */

  int ncpix = 0;                /* total number of component-pixels */
  int icpix = 0;                /* counter for component-pixels */
  int cpix1 = 0, cpix2 = 0;     /* current cpix for each image */
  int cpix_msb=0, cpix_lsb=0;   /* MSB and LSB of a two byte cpix */
  int sgndiff = 0;              /* difference between two cpix */
  int rv = 0;                   /* return value from various system calls */
  int *diff = NULL;             /* output difference image */

  /* These long integers accumulate the sum-square and sum-abs-diff until */
  /* they get too large at which point they are added into the main */
  /* floating point accumulators.  Longs are used as much as possible */
  /* for accuracy, but for some image comparisons they can overflow. */
  /* So, when part_max is reached they are dumped into the floats. */
  /* Type long is used because long long is not portable enough. */

  long psum_sqr = 0;            /* partial sum-square component-pixels */
  long psum_absdiff = 0;        /* partial sum-abs-diff component-pixels */
  /* this limit is half of the maximum positive value a long can hold */
  long part_max = 1L << (sizeof(long)*8 - 1);

  double sum_sqr = 0.0;         /* total sum-square component-pixels */
  double sum_absdiff = 0.0;     /* total sum-abs-diff component-pixels */

  int check_end_eof = 1;        /* should we check the EOF is reached? */
  int raw_fmt1 = 0;             /* is image 1 in raw format? */
  int raw_fmt2 = 0;             /* is image 2 in raw format? */

  int silent = 0;               /* If true skips any output. */

  int argi = 0;                 /* command line argument counter */

  char * format = "\
psnr: %p dB\n\
rmse: %r\n\
mean absolute error: %m\n\
max absolute error: %x\n\
max positive error: %y\n\
max negative error: %z\n\
compressed file size: %s bytes, %b bits per pixel\n\
";

  char *f;                      /* format character by character pointer */

  /* the final statistics computed */
  double rmse = 0.0;            /* root-mean-squared error */
  int maxposdiff = 0;           /* maximum positive difference */
  int maxnegdiff = 0;           /* maximum negative difference */
  int maxabsdiff = 0;           /* maximum absolute difference */
  double mad = 0.0;             /* mean-absolute-difference */
  double psnr = 0.0;            /* peak-signal-to-noise-ratio in dB */
  long size = 0L;               /* size of the compressed file in bytes */
  double bpp = 0.0;             /* bits-per-pixel of the compressed file */

  /* name of the program in a file-scope location for use in error messages */
  progname = argv[0];

  /* process the command line arguments */
  for (argi=1; argi<argc; argi++)  {

    if ( ! strcmp(argv[argi], "-h" ) || ! strcmp(argv[argi], "-help" ) ) {
      print_usage ();
    }

    if ( ! strcmp(argv[argi], "-i1" ) || ! strcmp(argv[argi], "-image1" ) ) {
      checke (argi+1>=argc, "option -i1 not followed by image name");
      argi += 1; fn1 = argv[argi];
      continue;
    }

    if ( ! strcmp(argv[argi], "-i2" ) || ! strcmp(argv[argi], "-image2" ) ) {
      checke (argi+1>=argc, "option -i2 not followed by image name");
      argi += 1; fn2 = argv[argi];
      continue;
    }

    if ( ! strcmp(argv[argi], "-s" ) || ! strcmp(argv[argi], "-size" ) ) {
      checke (argi+2>=argc, "option -i2 not followed by rows and cols");
      argi += 1; nrows = atoi (argv[argi]);
      checke (1>nrows, "number of rows cannot be negative or zero");
      argi += 1; ncols = atoi (argv[argi]);
      checke (1>ncols, "number of rows cannot be negative or zero");
      continue;
    }

    if ( ! strcmp(argv[argi], "-p" ) || ! strcmp(argv[argi], "-planes" ) ) {
      checke (argi+1>=argc,"option -p not followed by the number of planes");
      argi += 1; nplanes = atoi (argv[argi]);
      checke (1>nplanes, "number of planes cannot be negative or zero");
      continue;
    }

    if ( ! strcmp(argv[argi], "-d" ) || ! strcmp(argv[argi], "-depth" ) ) {
      checke (argi+1>=argc, "option -d not followed the depth");
      argi += 1; depth = atoi (argv[argi]);
      peak =  (0x1 << depth) - 1;
      checke (1>depth, "depth be negative or zero");
      checke (16<depth, "depth greater than 16 not supported");
      continue;
    }

    if ( ! strcmp(argv[argi], "-s1" ) || ! strcmp(argv[argi], "-skip1" ) ) {
      checke (argi+1>=argc, "option -s1 not followed by image name");
      argi += 1; skip1 = atoi (argv[argi]);
      checke (0>skip1, "number of skipped bytes cannot be negative");
      continue;
    }

    if ( ! strcmp(argv[argi], "-s2" ) || ! strcmp(argv[argi], "-skip2" ) ) {
      checke (argi+1>=argc, "option -s2 not followed by image name");
      argi += 1; skip2 = atoi (argv[argi]);
      checke (0>skip2, "number of skipped bytes cannot be negative");
      continue;
    }

    if ( ! strcmp(argv[argi], "-cee" ) ||
         ! strcmp(argv[argi], "-check-end-eof" ) ) {
      check_end_eof = 1;
      continue;
    }

    if ( ! strcmp(argv[argi],"-ncee") ||
         ! strcmp(argv[argi],"-no-check-end-eof") ) {
      check_end_eof = 0;
      continue;
    }

    if ( ! strcmp(argv[argi], "-e" ) || ! strcmp(argv[argi], "-encode" ) ) {
      checke (argi+1>=argc, "option -e not followed by a file name");
      argi += 1; fne = argv[argi];
      continue;
    }

    if ( ! strcmp(argv[argi], "-f" ) || ! strcmp(argv[argi], "-format" ) ) {
      checke (argi+1>=argc, "option -f not followed the format");
      argi += 1; format = argv[argi];
      continue;
    }

    if ( ! strcmp(argv[argi], "-od" ) || ! strcmp(argv[argi], "-outputdiff" ) ) {
      checke (argi+1>=argc, "option -od not followed by a file name");
      argi += 1; output_filename = argv[argi];
      continue;
    }

    if ( ! strcmp(argv[argi], "-silent" ) ) {
      checke (argi+1>=argc, "option -od not followed by a file name");
      silent = 1;
      continue;
    }

    checke2 (1, "unrecognized option", argv[argi]);
  }

  /* open the file for image 1 */
  if ( ! strcmp (fn1, "-") ) {
    fp1 = stdin;
  } else {
    fp1 = fopen (fn1, "rb");
    checke2 ( ! fp1, "could not open image file", fn1);
  }

  /* open the file for image 2 */
  if ( ! strcmp (fn2, "-") ) {
    checke (stdin == fp1, "both images cannot be stdin");
    fp2 = stdin;
  } else {
    fp2 = fopen (fn2, "rb");
    checke2 ( ! fp2, "could not open image file", fn2);
  }

  /*
    Assume image files are in RAW format if name does not contain a
    ".pgm" or ".ppm" extension.  This will cause trouble if a file
    name has .pgm or .ppm somewhere other than the extension.
  */

  if (strstr (fn1, ".pgm") || strstr (fn1, ".ppm"))
    raw_fmt1 = 0;
  else
    raw_fmt1 = 1;

  if (strstr (fn2, ".pgm") || strstr (fn2, ".ppm"))
    raw_fmt2 = 0;
  else
    raw_fmt2 = 1;

  if (raw_fmt1) {
    /* skip header characters if requested on command line */
    for (iskip=0; iskip<skip1; iskip++) {
      ch = getc (fp1);
      checke2 (EOF == ch, "EOF reached in skipped header of image file", fn1);
    }
    /* rows, cols, planes and depth are default or as given on command line */
    nrows1 = nrows;
    ncols1 = ncols;
    nplanes1 = nplanes;
    depth1 = depth;
    peak1 = (0x1 << depth) - 1;
  } else {
    read_header (fn1, fp1, &nrows1, &ncols1, &nplanes1, &depth1, &peak1);
  }

  if (raw_fmt2) {
    /* skip header characters if requested on command line */
    for (iskip=0; iskip<skip2; iskip++) {
      ch = getc (fp2);
      checke2 (EOF == ch, "EOF reached in skipped header of image file", fn2);
    }
    /* rows, cols, planes and depth are default or as given on command line */
    nrows2 = nrows;
    ncols2 = ncols;
    nplanes2 = nplanes;
    depth2 = depth;
    peak2 = (0x1 << depth) - 1;
  } else {
    read_header (fn2, fp2, &nrows2, &ncols2, &nplanes2, &depth2, &peak2);
  }

  /* make sure the images are the same size */
  checke (nrows1!=nrows2, "images have different number of rows");
  checke (ncols1!=ncols2, "images have different number of columns");
  checke (nplanes1!=nplanes2, "images have different number of planes");
  checke (depth1!=depth2, "images have different depths");
  checke (peak1!=peak2, "images have different peak values");

  /* now that we know the images are the same size copy the sizes to */
  /* the non-image specific variables */
  nrows = nrows1;
  ncols = ncols1;
  nplanes = nplanes1;
  depth = depth1;
  peak = peak1;

  /* total number of values (not really pixels since planes are treated */
  /* independently) */
  ncpix = nrows * ncols * nplanes;

  /* allocate diff image space */
  diff = (int *) malloc(ncpix * sizeof(int));

  checke (!diff, "error allocating diff image space");

  /* main loop over each value */
  for (icpix=0; icpix<ncpix; icpix++) {
    if (depth <= 8) {

      /* read one byte per pixel */
      cpix1 = getc (fp1);
      if ( EOF == cpix1 )
        checke2 (1, "unexpected EOF while reading image file", fn1);
      cpix2 = getc (fp2);
      if ( EOF == cpix2 )
        checke2 (1, "unexpected EOF while reading image file", fn2);

    } else if (depth <= 16) {

      /* read two bytes, MSB first, per pixel */

      cpix_msb = getc (fp1);
      cpix_lsb = getc (fp1);
      if ( EOF == cpix_msb || EOF == cpix_lsb )
        checke2 (1, "unexpected EOF while reading image file", fn1);
      cpix1 = (cpix_msb << 8) + cpix_lsb;

      cpix_msb = getc (fp2);
      cpix_lsb = getc (fp2);
      if ( EOF == cpix_msb || EOF == cpix_lsb )
        checke2 (1, "unexpected EOF while reading image file", fn2);
      cpix2 = (cpix_msb << 8) + cpix_lsb;

    } else {
      /* the argument 1 forces an error */
      checke (1, "depth too high");
    }

    /* signed difference */
    sgndiff = cpix1 - cpix2;

    /* write output diff image pixel */
    diff[icpix] = abs(sgndiff); 

    /* add to the partial accumulators and make sure they do not overflow */
    psum_absdiff += abs (sgndiff);
    psum_sqr += sgndiff*sgndiff;
    checke (psum_absdiff < 0, "sum difference overflow");
    checke (psum_sqr < 0, "sum square overflow");

    /* check against the max pos and neg differences */
    if (maxposdiff < sgndiff)
      maxposdiff = sgndiff;
    if (maxnegdiff > sgndiff)
      maxnegdiff = sgndiff;

    /* if the partials are large enough, dump to the main accumulators */
    if (psum_absdiff > part_max) {
      sum_absdiff += psum_absdiff;
      psum_absdiff = 0;
    }
    if (psum_sqr > part_max) {
      sum_sqr += psum_sqr;
      psum_sqr = 0;
    }

  }

  /* add the last of the partial sums to the main sums */
  /* setting the partial to zero here is not needed but done as a precaution */
  sum_absdiff += psum_absdiff;
  psum_absdiff = 0;
  sum_sqr += psum_sqr;
  psum_sqr = 0;

  /* if desired, make sure we have reached the end of the image files */
  /* don't check for EOF if the image is from stdin */
  if (check_end_eof) {
    if ( stdin != fp1 ) {
      ch = getc (fp1);
      checke2 (EOF != ch, "did not reach EOF in image file", fn1);
    }
    if ( stdin != fp2 ) {
      ch = getc (fp2);
      checke2 (EOF != ch, "did not reach EOF in image file", fn2);
    }
  }

  /* close the image files if they are not stdin */
  if ( stdin != fp1 ) {
    rv = fclose (fp1);
    checke2 (0 != rv, "error closing image file", fn1);
  }
  if ( stdin != fp2 ) {
    rv = fclose (fp2);
    checke2 (0 != rv, "error closing image file", fn2);
  }

  /* find the size of the compressed file if it is given */
  if ( NULL != fne ) {
    fpe = fopen (fne, "rb");
    checke2 ( ! fpe, "could not open file", fne);
    /* move to the end of the file */
    rv = fseek (fpe, 0L, SEEK_END);
    checke2 (0 != rv, "error seeking end of file", fne);
    /* see how many bytes we are from the beginning of the file */
    size = ftell (fpe);
    checke2 (-1L == size, "error finding end position of", fne);
    /* close the file */
    rv = fclose (fpe);
    checke2 (0 != rv, "error closing", fne);
    /* compute bits-per-pixel */
    bpp = 8.0 * (double)size / (double)(nrows * ncols);
  }

  /* compute the root-mean-square-error */
  rmse = sqrt((double)sum_sqr / (double)ncpix);
  /* compute the mean absolute difference */
  mad = (double)sum_absdiff / (double)ncpix;
  /* if the error is non-zero compute PSNR */
  if (rmse > 0.0)
    psnr = 20*log10 ((double)peak / rmse);

  /* turn the maximum negative difference into a positive number */
  maxnegdiff = - maxnegdiff;
  /* max abs difference is the greater of max pos and max neg */
  maxabsdiff = (maxposdiff > maxnegdiff) ? maxposdiff : maxnegdiff;

  /* loop through each character in the format, handling escape sequences */
  if (!silent)
  {
    for (f=format; *f!='\0'; f++) {
      switch (*f) {
      case '\\':
        f++;
        switch (*f) {
          /* if end of sting, back up so the for loop sees the end too */
        case '\0':  putchar('\\');  f--;  continue;
        case 'b':   putchar('\b');  continue;
        case 'f':   putchar('\f');  continue;
        case 'n':   putchar('\n');  continue;
        case 'r':   putchar('\r');  continue;
        case 't':   putchar('\t');  continue;
        case 'v':   putchar('\v');  continue;
        case '\\':  putchar('\\');  continue;
        default:    putchar('\\');  putchar(*f);  continue;
        }
      case '%':
        f++;
        switch (*f) {
          /* if end of sting, back up so the for loop sees the end too */
        case '\0':  putchar('%');  f--;          continue;
        case '%':   putchar('%');                continue;
        case 'p':   printf ("%g",  psnr);        continue;
        case 'r':   printf ("%g",  rmse);        continue;
        case 'x':   printf ("%d",  maxabsdiff);  continue;
        case 'y':   printf ("%d",  maxposdiff);  continue;
        case 'z':   printf ("%d",  maxnegdiff);  continue;
        case 'm':   printf ("%g",  mad);         continue;
        case 'b':   printf ("%g",  bpp);         continue;
        case 's':   printf ("%ld", size);        continue;
        case 'k':   printf ("%d",  peak);        continue;
        default:    putchar('%');  putchar(*f);  continue;
        }
      default:  putchar(*f);  continue;
      }
    }
  }
  /* dump difference image if requested */
  if (output_filename)
  {
       writePPMfile(output_filename, diff, nrows, ncols, peak); 
  }

  /*  Deallocate diff image space  */
  free(diff);

  exit (0);
  return 0;
}
