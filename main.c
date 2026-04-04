#include "subvert.h"

char line[1024];

File input = {
   .extension = NULL,
   .filename  = NULL,
};

File output = {
   .extension = NULL,
   .filename  = NULL,
};

// TODO: Support more formats
fn_ptr matrix[FN_COUNT][FN_COUNT] = {
   {NULL,       NULL,       NULL      },
   {srt_to_lrc, NULL,       srt_to_vtt},
   {vtt_to_lrc, vtt_to_srt, NULL      },
};


// TODO: Add -i flag to read multiple input files from a text file (and maybe from stdin)
// TODO: Improve the -o flag so no need for specifying extension

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
         output.filename = argv[++i];
         continue;
      }

      if (!strcmp(argv[i], "-if")) {
         input.extension = argv[++i];
         continue;
      }

      if (!strcmp(argv[i], "-of")) {
         output.extension = argv[++i];
         continue;
      }

      if (!strcmp(argv[i], "-h")) {
         fprintf(stdout, "Usage: %s [-of format] <file>\n", argv[0]);
         fprintf(stdout, "Options: \n");
         fprintf(stdout, "  -h              show this help message and exit\n");
         fprintf(stdout, "  -if             specify input file format\n");
         fprintf(stdout, "  -of             specify output file format\n");
         fprintf(stdout, "  -o              specify output file\n");
         exit(0);
      }

      if (!strcmp(argv[i], "--")) {
         input.filename = argv[++i];
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
         exit(1);
      }

      input.filename = argv[i];
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

   if (!input.extension) {
      char *s = strdup(input.filename);
      s = strrchr(s, '.') + 1;
      input.extension = s;
   }

   sub_fmt input_fmt = get_enum(input.extension);
   sub_fmt output_fmt = get_enum(output.extension);
   if (!matrix[input_fmt][output_fmt]) {
      printf("Conversion not implemented\n");
      return 1;
   }

   if (!input.filename) {
      fprintf(stderr, "Error: Missing input file\n");
      return 2;
   }
   FILE *f_input = fopen(input.filename, "r");

   if (!f_input) {
      fprintf(stderr, "Error: filename %s cannot be opened\n", input.filename);
      return 2;
   }

   if (!output.filename) {
      char *s = get_basename_with_dot(input.filename);
      s = strncat(s, output.extension, 5);
      output.filename = s;
   }
   FILE *f_output = fopen(output.filename, "w");

   if (!f_output) {
      fprintf(stderr, "Error: filename %s cannot be opened\n", output.filename);
      return 2;
   }

   // Main loop
   matrix[input_fmt][output_fmt](f_input, f_output);
   fclose(f_input);
   fclose(f_output);

   return 0;
}
