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

int main(int argc, char *argv[])
{
   char opt;
   char *file_lrc = NULL;
   FILE *file_out;

   if (argc == 1) {
      fprintf(stderr, "Missing options and arguments\n");
      fprintf(stderr, "Usage: %s [-o] <file>\n", argv[0]);
      return 69;
   }

   opterr = 0;
   while ((opt = getopt(argc, argv, "o:h")) != -1) {
      switch (opt) {
         case 'o':
            file_lrc = optarg;
            break;
         case 'h':
            fprintf(stdout, "Usage: %s [-o] <file>\n", argv[0]);
            return -1;
         default:
            if (optopt == 'o') {
               fprintf(stderr, "Error: -o requires an argument\n");
            } else {
               fprintf(stderr, "Unknown option '%c'\n", optopt);
               fprintf(stderr, "Usage: %s [-o output] file\n", argv[0]);
            }
            return 2;
      }
   }

   if (optind >= argc) {
      fprintf(stderr, "Error: Missing input file\n");
      return 1;
   }
   const char *file_vtt = argv[optind];
   FILE *file_in = fopen(file_vtt, "r");
   if (!file_in) {
      fprintf(stderr, "Filenme %s is invalid\n", file_vtt);
      return 2;
   }
   if (!file_lrc) file_out = fopen("./out.lrc", "w");
   else file_out = fopen(file_lrc, "w");

   vtt_to_lrc(file_in, file_out);

   fclose(file_in);
   fclose(file_out);

   return 0;
}
