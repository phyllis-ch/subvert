#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

char line[1024];

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
         // fprintf(stdout, "Before: %s", line);
         sscanf(line, "%d:%d:%f", &h, &m, &s);
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
   char opt;
   char *file_lrc = NULL;
   const char *file_vtt = NULL;

   if (argc < 2) {
      fprintf(stderr, "Usage: %s [-o output] <file>\n", argv[0]);
      fprintf(stderr, "Missing options and arguments\n");
      return 69;
   }

   for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-o") == 0) {
         // ./subvert -o ./test.lrc
         //  1         2   3
         if (i + 1 >= argc) {
            fprintf(stderr, "Error: -o requires an argument\n");
            return 1;
         }
         printf("%d\n", i);
         file_lrc = argv[++i];
         printf("%s\n", file_lrc);
         printf("%d\n", i);
         continue;
      }

      if (strcmp(argv[i], "-h") == 0) {
         fprintf(stdout, "Usage: %s [-o output] <file>\n", argv[0]);
         return 0;
      }

      if (strcmp(argv[i], "--") == 0) {
         file_vtt = argv[++i];
         break;
      }

      if (strcmp(argv[i], "-") == 0) {
         fprintf(stderr, "Usage: %s [-o output] <file>\n", argv[0]);
         fprintf(stderr, "Error: No option specified\n");
         return 1;
      }

      if (argv[i][0] == '-' && argv[i][1] != '\0') {
         fprintf(stderr, "Usage: %s [-o output] file\n", argv[0]);
         fprintf(stderr, "Unknown option '%c'\n", argv[i][1]);
         return 2;
      }

      file_vtt = argv[i];
   }

   if (optind >= argc) {
      fprintf(stderr, "Error: Missing input file\n");
      return 1;
   }
   // const char *file_vtt = argv[optind];
   FILE *file_in = fopen(file_vtt, "r");
   if (!file_in) {
      fprintf(stderr, "Error: filenme %s cannot be opened\n", file_vtt);
      return 2;
   }

   char buf[256];
   if (!file_lrc) {
      strcpy(buf, file_vtt);
      file_lrc = strrchr(buf, '/') + 1;
      char *dot = strrchr(file_lrc, '.');
      *dot = '\0';
      strncat(file_lrc, ".lrc", 5);
   }
   FILE *file_out = fopen(file_lrc, "w");

   vtt_to_lrc(file_in, file_out);
   // vtt_to_srt(file_in, file_out);

   fclose(file_in);
   fclose(file_out);

   return 0;
}
