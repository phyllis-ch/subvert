#include "subvert.h"

const char *filename_output = NULL;
char *filename_input = NULL;
char line[1024];


void get_flags(int argc, char *argv[]) {
   if (argc < 2) {
      fprintf(stderr, "Usage: %s [-o output] <file>\n", argv[0]);
      fprintf(stderr, "Missing options and arguments\n");
      exit(69);
   }

   for (int i = 1; i < argc; ++i) {
      if (strcmp(argv[i], "-o") == 0) {
         if (i + 1 >= argc) {
            fprintf(stderr, "Error: -o requires an argument\n");
            exit(1);
         }
         filename_output = argv[++i];
         continue;
      }

      if (strcmp(argv[i], "-of") == 0) {
         if (strcmp(argv[i + 1], "lrc") == 0) translate = &vtt_to_lrc;
         ++i;
         continue;
      }

      if (strcmp(argv[i], "-h") == 0) {
         fprintf(stdout, "Usage: %s [-o output] <file>\n", argv[0]);
         exit(0);
      }

      if (strcmp(argv[i], "--") == 0) {
         filename_input = argv[++i];
         break;
      }

      if (strcmp(argv[i], "-") == 0) {
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
         fprintf(stdout, "%s", line);
         char *line_ptr = NULL;
         for (int i = 0; i < 2; ++i) {
            line_ptr = strchr(line, '.');
            *line_ptr = ',';
            fprintf(stdout, "%s\n", line_ptr);
            fprintf(stdout, "%s\n", line);
         }
         // sscanf(line, "%d:%d:%f", &h, &m, &s);

         // int m_total = (h * 60) + m;
         // fprintf(file_out, "[%02d:%.2f]", m_total, s);
      }
      fprintf(out, "%s", line);
   }
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

   char buf[256];
   if (!filename_output) {
      strcpy(buf, filename_input);
      filename_output = strrchr(buf, '/') + 1;
      char *dot = strrchr(buf, '.');
      *dot = '\0';
      strncat(buf, ".lrc", 5);
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
