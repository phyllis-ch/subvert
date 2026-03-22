#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


void get_flags(int argc, char *argv[]);
void vtt_to_lrc(FILE *in, FILE *out);
void vtt_to_srt(FILE *in, FILE *out);
void srt_to_vtt(FILE *in, FILE *out);

// Function Pointers
void (*translate)(FILE *, FILE *) = NULL;
