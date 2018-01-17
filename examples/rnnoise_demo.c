/* Copyright (c) 2017 Mozilla */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <time.h>
#include "rnnoise.h"

#define FRAME_SIZE 480
#define SAMPLE_RATE 48000 // Hard code here for the input audio format requirement

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s <noisy speech> <output denoised>\n", argv[0]);
    return 1;
  }

  int i;
  int first = 1;
  float x[FRAME_SIZE];

  DenoiseState *st = rnnoise_create();

  //const double frame_length = (double)FRAME_SIZE / (double)SAMPLE_RATE;
  //fprintf(stderr, "frame_length : %.2f\n", frame_length);

  FILE *fin = fopen(argv[1], "r");
  FILE *fout = fopen(argv[2], "w");
  size_t frame_index = 0;
  short tmp[FRAME_SIZE];

  clock_t begin = clock();
  while (1) {
    
    // short : 2bytes - 16 Bits Per Sample
    fread(tmp, sizeof(short), FRAME_SIZE, fin);

    if (feof(fin))
      break;

    for (i=0;i<FRAME_SIZE;i++)
      x[i] = tmp[i];

    // float rnnoise_process_frame(DenoiseState *st, float *out, const float *in)
    rnnoise_process_frame(st, x, x);
    //float vad_prob = rnnoise_process_frame(st, x, x);
    //fprintf(stderr, "frame_index:%d, vad_prob:%.3f\n", frame_index, vad_prob);

    for (i=0;i<FRAME_SIZE;i++)
      tmp[i] = x[i];

    if (!first)
      fwrite(tmp, sizeof(short), FRAME_SIZE, fout);

    first = 0;
    frame_index ++;
  }
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  
  rnnoise_destroy(st);
  fclose(fin);
  fclose(fout);

  size_t process_frame_count = frame_index * FRAME_SIZE;
  fprintf(stderr, "[config] SAMPLE_RATE : %d\n", SAMPLE_RATE);
  fprintf(stderr, "Processed Frame count : %zu, Audio length : %.2f sec\n", process_frame_count, (double)process_frame_count / (double)SAMPLE_RATE);
  fprintf(stderr, "Throughput : %.0f frame/sec, %.2f sec audio data/sec\n", process_frame_count / time_spent, process_frame_count/ (double)SAMPLE_RATE / time_spent);
  fprintf(stderr, "Time Spent: %.2f ms\n", (1000.0 * time_spent));

  return 0;
}
