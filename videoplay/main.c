/************************************************************************************
***
***	Copyright 2020 Dell(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, 2020-11-22 13:18:11
***
************************************************************************************/

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <nimage/image.h>
#include <nimage/video.h>

int play_video_with_image(char *input_filename, int start, int n,
                          char *output_dir) {
  IMAGE *image;
  VIDEO *video;
  char output_file_name[256];

  video = video_open(input_filename, start);
  check_video(video);

  video_info(video);

  image = image_create(video->height, video->width);
  check_image(image);

  // Real start
  time_reset();
  while (n > 0 && !video_eof(video)) {
    syslog_info("Playing frame %d ...", start);

    frame_toimage(video_read(video), image);
    snprintf(output_file_name, sizeof(output_file_name), "%s/%06d.png",
             output_dir, start++);
    image_save(image, output_file_name);
    n--;
  }
  time_spend("Playing");

  image_destroy(image);

  video_close(video);

  return RET_OK;
}

int play_video_with_tensor(char *input_filename, int start, int n,
                           char *output_dir) {
  IMAGE *image;
  VIDEO *video;
  TENSOR *tensor;
  char output_file_name[256];

  video = video_open(input_filename, start);
  check_video(video);

  video_info(video);

  // Real start
  time_reset();
  while (n > 0 && !video_eof(video)) {
    syslog_info("Playing frame %d ...", start);

    // Test Tensor
    video_read(video);
    video_read(video);
    video_read(video);
    video_read(video);
    video_read(video);

    for (int i = -4; i <= 0; i++) {
      tensor = video_tensor(video, i);

      image = image_from_tensor(tensor, 0 /* batch*/);
      snprintf(output_file_name, sizeof(output_file_name), "%s/%06d.png",
               output_dir, start++);
      image_save(image, output_file_name);
      image_destroy(image);
    }
    n--;
  }
  time_spend("Playing");

  video_close(video);

  return RET_OK;
}

void help(char *cmd) {
  printf("This is an example for video play with nimage\n");

  printf("Usage: %s [option] video_file\n", cmd);
  printf("    -h, --help                   Display this help.\n");
  printf("    -s, --start                  Start frame (default: 0).\n");
  printf("    -n, --number                 Output frames (default: 1).\n");
  printf(
      "    -o, --output <dir>           Output diretory (default: output).\n");
  printf("    -t, --tensor                 Test with tensor.\n");

  exit(1);
}

int main(int argc, char **argv) {
  int optc;
  int option_index = 0;
  int start = 0;
  int number = 1;
  int tensor = 0;
  char *output_dir = (char *)"output";

  struct option long_opts[] = {{"help", 0, 0, 'h'},   {"start", 0, 0, 's'},
                               {"end", 1, 0, 'e'},    {"output", 1, 0, 'o'},
                               {"tensor", 0, 0, 't'}, {0, 0, 0, 0}};

  if (argc <= 1)
    help(argv[0]);

  while ((optc = getopt_long(argc, argv, "h s: n: o: t", long_opts,
                             &option_index)) != EOF) {
    switch (optc) {
    case 's':
      start = atoi(optarg);
      break;
    case 'n':
      number = atoi(optarg);
      break;
    case 'o': // Output
      output_dir = optarg;
      break;
    case 't': // tensor
      tensor = 1;
      break;
    case 'h': // help
    default:
      help(argv[0]);
      break;
    }
  }

  if (argc > optind) {
    return (tensor == 0)
               ? play_video_with_image(argv[optind], start, number, output_dir)
               : play_video_with_tensor(argv[optind], start, number,
                                        output_dir);
  } else {
    printf("----- Warning: miss video input file -----\n");
    help(argv[0]);
  }

  return RET_ERROR;
}
