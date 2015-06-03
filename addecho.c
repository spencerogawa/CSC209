/* CSC209H1 Winter 2015
 * Assignment 1
 * addecho.c
 * Spencer Ogawa
 * 994417751
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_VOLUME 4
#define DEFAULT_DELAY 8000
#define WAV_HEADER_SIZE 44

int main(int argc, char **argv)
{

  /* Set WAV file format */
  typedef struct
  {
    char riff[4];     // "RIFF"
    int total_size;   // Total Size
    char fmt[8];      //  "WAVEfmt"
    int fmt_size;     // fmt chunk size
    short format;        // type of format
    short channel;    // channel
    int rate;         // sampling rate
    int avgbyte;      // rate * block
    short block;      // channels * bit / 8
    short bit;        // bits
    char data[ 4 ];   // = "data"
    int data_size;    // data size 
  } WAVFILEFORMAT;

  WAVFILEFORMAT wavHeader;
  short int *buff_orig;
  short buff;
  int option;
  int vvalue = DEFAULT_VOLUME, dvalue = DEFAULT_DELAY;
  char *ptr;
  FILE *fp_in, *fp_out;

  /* Process options for Volume, Delay, and IN/OUT Files */
  while((option = getopt(argc, argv, "v:d:")) != -1)
    switch(option) {
      case 'v':
        vvalue = strtol(optarg, &ptr, 10);
        if(vvalue <= 0) {
          fprintf(stderr,
          "Usage: %s [-d delay] [-v volume] <input wav> <output wav>\n"
          "-v option requires positive integer.\n", argv[0]);
          return 1;
        }
        break;
      case 'd':
        dvalue = strtol(optarg, &ptr, 10);
        if(dvalue <= 0) {
          fprintf(stderr,
          "Usage: %s [-d delay] [-v volume] <input wav> <output wav>\n"
          "-d option requires positive integer.\n", argv[0]);
          return 1;
        }
        buff_orig = malloc(dvalue * sizeof(short));
        break;
      default:
          fprintf(stderr,
          "Usage: %s [-d delay] [-v volume] <input wav> <output wav>\n",
            argv[0]);
          return 1;
    }
    if(argv[optind] == NULL || argv[optind + 1] == NULL) {
      fprintf(stderr,
      "Usage: %s [-d delay] [-v volume] <input wav> <output wav>\n"
      "Input/Output arguments are missing.\n", argv[0]);
      return 1;
    }

  /* Open Input/Outpu files */
  if((fp_in = fopen(argv[optind], "r")) == NULL) {
    perror(argv[1]);
    exit(1);
  }
  if((fp_out = fopen(argv[optind + 1], "w")) == NULL) {
    perror(argv[2]);
    exit(1);
  }

  /* Read WAV header file, adjust size data, and write it out. */
  fread(&wavHeader, sizeof(wavHeader), 1, fp_in);
  wavHeader.total_size += dvalue * 2;
  wavHeader.data_size += dvalue * 2;
  fwrite(&wavHeader, sizeof(wavHeader), 1, fp_out);

  /* Read first delay blocks into buff_orig, and write it out. */
  int i = 0;
  for(; i < dvalue; i++) {
    size_t n = fread(&buff, sizeof(short), 1, fp_in);
    if(n < 1) { break; }
    buff_orig[i] = buff;
    fwrite(&buff, sizeof(short), 1, fp_out);
  }
  /* Read the rest of blocks and mix each block, and write it out. */
  int j = 0;
  short buff_temp;
  for(;;) {
    size_t n = fread(&buff, sizeof(short), 1, fp_in);
    if(n < 1) { break; }
    if(j == dvalue) { j = 0; }
    buff_temp = ((buff_orig[j] / vvalue) + buff);
    buff_orig[j] = buff;
    j++;
    fwrite(&buff_temp, sizeof(short), 1, fp_out);
  }
  /* If dvalue exceeds original file, add zero to the end. */
  int k, zero = 0;
  for(k = i; k < dvalue; k++) {
    fwrite(&zero, sizeof(short), 1, fp_out);
  }
  /* Write out the rest of remaining delays. */
  int p, q;
  if(i < dvalue) { dvalue = i; }
  for(p = j; p < dvalue; p++) {
    buff_temp = buff_orig[p] / vvalue;
    fwrite(&buff_temp, sizeof(short), 1, fp_out);
  }
  for(q = 0; q < j; q++) {
    buff_temp = buff_orig[q] / vvalue;
    fwrite(&buff_temp, sizeof(short), 1, fp_out);
  }

  fclose(fp_in);
  fclose(fp_out);

  return 0;
}
