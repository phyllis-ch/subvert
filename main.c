#include "subvert.h"
#include <stdio.h>

const char *filename_output = NULL;
const char *input_extension = NULL;
const char *output_extension = NULL;
char *filename_input = NULL;
char line[1024];
char buf[256];

// TODO: Support more formats
// TODO: Add -i flag to read multiple input files from a text file (and maybe from stdin)
// TODO: Improve the -o flag so no need for specifying extension

// [input_extension][output_format]
// matrix of function?
// function to return first index and second index

typedef enum {LRC, SRT, VTT} sub_fmt;

#define FN_COUNT 3
typedef void (*fn_ptr)(FILE *, FILE *);
fn_ptr matrix[FN_COUNT][FN_COUNT] = {
   {NULL, NULL ,NULL },
   {srt_to_lrc, NULL, srt_to_vtt},
   {vtt_to_lrc, vtt_to_srt, NULL},
};

void get_flags(int argc, char *argv[]) {
   if (argc < 2) {
      fprintf(stderr, "Usage: %s [-o output] <file>\n", argv[0]);
      fprintf(stderr, "Missing options and arguments\n");
      exit(69);
   }

   for (int i = 1; i < argc; ++i) {
      if (!strcmp(argv[i], "-o")) {
         if (i + 1 >= argc) {
            fprintf(stderr, "Error: -o requires an argument\n");
            exit(1);
         }
         filename_output = argv[++i];
         continue;
      }

      if (!strcmp(argv[i], "-if")) {
         input_extension = argv[++i];
         continue;
      }

      if (!strcmp(argv[i], "-of")) {
         output_extension = argv[i + 1];
         ++i;
         continue;
      }

      if (!strcmp(argv[i], "-h")) {
         fprintf(stdout, "Usage: %s [-of format] <file>\n", argv[0]);
         exit(0);
      }

      if (!strcmp(argv[i], "--")) {
         filename_input = argv[++i];
         break;
      }

      if (!strcmp(argv[i], "-")) {
         fprintf(stderr, "Usage: %s [-o output] <file>\n", argv[0]);
         fprintf(stderr, "Error: No option specified\n");
         exit(1);
      }

      if (argv[i][0] == '-' && argv[i][1] != '\0') {
         fprintf(stderr, "Usage: %s [-o output] file\n", argv[0]);
         fprintf(stderr, "Unknown option '%c'\n", argv[i][1]);
         exit(2);
      }

      filename_input = argv[i];
   }
}

char *get_basename_with_dot(const char *input) {
   char *s = strdup(input);
   s = strrchr(s, '/') + 1;
   char *dot = strrchr(s, '.');
   dot += 1;
   *dot = '\0';

   return s;
}

void vtt_to_lrc(FILE *in, FILE *out) {
   int h, m;
   float s;

   while (fgets(line, sizeof(line), in)) {
      if (strncmp(line, "WEBVTT", 6) == 0) {
         fgets(line, sizeof(line), in);
         continue;
      }
      if (isdigit(line[0]) && !strchr(line, ':')) continue;

      if (strstr(line, "-->")) {
         if (sscanf(line, "%d:%d:%f", &h, &m, &s) != 3) {
            h = 0;
            sscanf(line, "%d:%f", &m, &s);
         }
         // fprintf(stdout, "After: [%d:%d:%.2f]\n", h, m, s);

         int m_total = (h * 60) + m;
         fprintf(out, "[%02d:%.2f]", m_total, s);
      } else {
         fprintf(out, "%s", line);
      }
   }
}

void vtt_to_srt(FILE *in, FILE *out) {
   while (fgets(line, sizeof(line), in)) {
      if (strncmp(line, "WEBVTT", 6) == 0) {
         fgets(line, sizeof(line), in);
         continue;
      }

      if (strstr(line, "-->")) {
         char *line_ptr = NULL;
         for (int i = 0; i < 2; ++i) {
            line_ptr = strchr(line, '.');
            *line_ptr = ',';
         }
         // sscanf(line, "%d:%d:%f", &h, &m, &s);

         // int m_total = (h * 60) + m;
         // fprintf(file_out, "[%02d:%.2f]", m_total, s);
      }
      fprintf(out, "%s", line);
   }
}

// TODO: Fix bug where <feff> is found at start of output file
void srt_to_vtt(FILE *in, FILE *out) {
   fprintf(out, "WEBVTT\n\n");
   while (fgets(line, sizeof(line), in)) {
      line[strcspn(line, "\r\n")] = '\0';

      if (strstr(line, "-->")) {
         char *line_ptr = NULL;
         for (int i = 0; i < 2; ++i) {
            line_ptr = strchr(line, ',');
            *line_ptr = '.';
         }
      }
      fprintf(out, "%s\n", line);
   }
}

void srt_to_lrc(FILE *in, FILE *out) {
   int h, m;
   float s, ms;

   // For some reason, the first number (1) skips the following check
   // This is a workaround until I figure out why
   fgets(line, sizeof(line), in);
   while (fgets(line, sizeof(line), in)) {
      if (isdigit(line[0]) && !strchr(line, ':')) continue;

      if (strstr(line, "-->")) {
         // if (sscanf(line, "%d:%d:%f,%f", &h, &m, &s, &ms) != 3) {
         //    h = 0;
         //    sscanf(line, "%d:%f", &m, &s);
         // }
         sscanf(line, "%d:%d:%f,%f", &h, &m, &s, &ms);

         s += ms / 1000;
         m += h * 60;
         fprintf(out, "[%d:%.2f]", m, s);
      } else {
         fprintf(out, "%s", line);
      }
   }
}

sub_fmt get_enum(const char *ext) {
   // Should probably make an array and loop instead
   if (!strcmp("lrc", ext)) {
      return LRC;
   }
   if (!strcmp("srt", ext)) {
      return SRT;
   }
   if (!strcmp("vtt", ext)) {
      return VTT;  
   }

   printf("Format not implemented\n");
   exit(1);
}


int main(int argc, char *argv[])
{
   get_flags(argc, argv);

   if (!input_extension) {
      char *s = strdup(filename_input);
      s = strrchr(s, '.') + 1;
      input_extension = s;
   }

   sub_fmt input_temp = get_enum(input_extension);
   sub_fmt output_temp = get_enum(output_extension);
   if (!matrix[input_temp][output_temp]) {
      printf("Conversion not implemented\n"); 
      return 1;
   }

   if (!filename_input) {
      fprintf(stderr, "Error: Missing input file\n");
      return 2;
   }

   FILE *f_input = fopen(filename_input, "r");
   if (!f_input) {
      fprintf(stderr, "Error: filename %s cannot be opened\n", filename_input);
      return 2;
   }

   if (!filename_output) {
      char *s = get_basename_with_dot(filename_input);
      s = strncat(s, output_extension, 5);
      filename_output = s;
   }
   FILE *f_output = fopen(filename_output, "w");

   matrix[input_temp][output_temp](f_input, f_output);
   

   fclose(f_input);
   fclose(f_output);

   return 0;
}
