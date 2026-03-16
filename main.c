#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char *argv[])
{
   if (argc < 2) {
      fprintf(stderr, "Usage: %s <file>\n", argv[0]);
      return 1;
   }

   const char *file_vtt = argv[1];
   FILE *file_in = fopen(file_vtt, "r");
   if (!file_in) {
      fprintf(stderr, "Filenme %s is not valid\n", file_vtt);
      return 69;
   }
   FILE *file_out = fopen("./out.lrc", "w");

   char line[1024];
   int h, m;
   float s;

   while (fgets(line, sizeof(line), file_in)) {
      if (strncmp(line, "WEBVTT", 6) == 0) {
         fgets(line, sizeof(line), file_in);
         continue;
      }
      if (isdigit(line[0]) && !strchr(line, ':')) continue;

      if (strstr(line, "-->")) {
         // fprintf(stdout, "Before: %s", line);
         sscanf(line, "%d:%d:%f", &h, &m, &s);
         // fprintf(stdout, "After: [%d:%d:%.2f]\n", h, m, s);

         int m_total = (h * 60) + m;
         fprintf(file_out, "[%02d:%.2f]", m_total, s);
      } else {
         fprintf(file_out, "%s", line);
      }
   }

   fclose(file_in);
   fclose(file_out);

   return 0;
}
