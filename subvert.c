#include "subvert.h"

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
