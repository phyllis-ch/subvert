#include "subvert.h"

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
