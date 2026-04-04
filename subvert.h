#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef void (*fn_ptr)(FILE *, FILE *);

typedef enum {LRC, SRT, VTT , FN_COUNT} sub_fmt;

// File struct
typedef struct {
   const char *filename;
   const char *extension;
} File;


// Functions
void get_flags(int argc, char *argv[]);
char *get_basename_with_dot(const char *input);

void vtt_to_lrc(FILE *in, FILE *out);
void vtt_to_srt(FILE *in, FILE *out);
void srt_to_vtt(FILE *in, FILE *out);
void srt_to_lrc(FILE *in, FILE *out);
