/* CSC209H1 Winter 2015
 * Assignment 1
 * remvocals.c
 * This program removes vocal from a wav file.
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  char wavHeader[44];
  short int buff_left, buff_right, new_buff;
  FILE *fp, *fp_out;

  /* Read argc for required arguments. */
  if(argc != 3) {
    fprintf(stderr, "Usage: %s <input wave> <out put wave>\n", argv[0]);
    exit(1);
  }
  if((fp = fopen(argv[1], "r")) == NULL) {
    perror(argv[1]);
    exit(1);
  }
  if((fp_out = fopen(argv[2], "w")) == NULL) {
    perror(argv[2]);
    exit(1);
  }
  /*  Read the first 44 bytes of WAV file */
   fread(wavHeader, sizeof(wavHeader), 1, fp);

  /* Write out the first 44 bytes of WAV file */
   fwrite(wavHeader, sizeof(wavHeader), 1, fp_out);

  /* Read WAV data, and write out new data at (left - right) / 2 */
  for(;;) {
    size_t n_left = fread(&buff_left, sizeof(short), 1, fp);
    if(n_left < 1) { break; }
    fread(&buff_right, sizeof(short), 1, fp);
    new_buff = (short int) ((buff_left - buff_right) / 2);
    fwrite(&new_buff, sizeof(buff_left), 1, fp_out);
    fwrite(&new_buff, sizeof(buff_right), 1, fp_out);
  }
  fclose(fp);
  fclose(fp_out);
  return 0;
}
