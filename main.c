#include "subvert.h"

const char *filename_output = NULL;
const char *input_extension = NULL;
const char *output_extension = NULL;
char *filename_input = NULL;
char line[1024];
char buf[256];

typedef enum file_format {
   LRC, SRT, VTT
} file_format;

// TODO: Support more formats
// TODO: Add -i flag to read multiple input files from a text file (and maybe from stdin)
// TODO: Improve the -o flag so no need for specifying extension

// [input_extension][output_format]
// matrix of function?
// function to return first index and second index

void have_extension() {

}

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
         if (strcmp(argv[i + 1], "lrc") == 0) translate = &vtt_to_lrc;
         if (strcmp(argv[i + 1], "srt") == 0) translate = &vtt_to_srt;
         if (strcmp(argv[i + 1], "vtt") == 0) translate = &srt_to_vtt;
         output_extension = argv[i + 1];
         ++i;
         continue;
      }

      if (!strcmp(argv[i], "-h")) {
         fprintf(stdout, "Usage: %s [-o output] <file>\n", argv[0]);
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

void conversion_match() {
   // A hashmap would probably be better
   file_format in, out;

   if (!strcmp(output_extension, "lrc")) {
      out = LRC;
   }

   if (!strcmp(output_extension, "srt")) {
      out = SRT;
   }

   if (!strcmp(output_extension, "vtt")) {
      out = VTT;
   }

   if (!strcmp(input_extension, "lrc")) {
      in = LRC;
   }

   if (!strcmp(input_extension, "srt")) {
      in = SRT;
   }

   if (!strcmp(input_extension, "vtt")) {
      in = VTT;
   }


   if (!in || !out) {
      exit(1);
   }

   // translate = &function_array[in][out];
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

char *get_basename_with_dot(const char *input) {
   char *s = strdup(input);
   s = strrchr(s, '/') + 1;
   char *dot = strrchr(s, '.');
   dot += 1;
   *dot = '\0';
   // This gives memory leak, but probably doesnt matter

   return s;
}


int main(int argc, char *argv[])
{
   get_flags(argc, argv);

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

   if (!translate) {
      translate = &vtt_to_lrc;
   }
   translate(f_input, f_output);

   fclose(f_input);
   fclose(f_output);

   return 0;
}
