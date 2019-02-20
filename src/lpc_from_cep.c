/* Copyright (c) 2017-2018 Mozilla */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kiss_fft.h"
#include "common.h"
#include <math.h>
#include "freq.h"
#include "pitch.h"
#include "arch.h"
#include "celt_lpc.h"
#include <assert.h>


#define PITCH_MIN_PERIOD 32
#define PITCH_MAX_PERIOD 256
#define PITCH_FRAME_SIZE 320
#define PITCH_BUF_SIZE (PITCH_MAX_PERIOD+PITCH_FRAME_SIZE)

#define CEPS_MEM 8
#define NB_DELTA_CEPS 6

#define NB_FEATURES (2*NB_BANDS+3+LPC_ORDER)


typedef struct {
    float analysis_mem[OVERLAP_SIZE];
    float cepstral_mem[CEPS_MEM][NB_BANDS];
    float pitch_buf[PITCH_BUF_SIZE];
    float last_gain;
    int last_period;
    float lpc[LPC_ORDER];
    float sig_mem[LPC_ORDER];
    int exc_mem;
} DenoiseState;

static int rnnoise_get_size() {
  return sizeof(DenoiseState);
}

static int rnnoise_init(DenoiseState *st) {
  memset(st, 0, sizeof(*st));
  return 0;
}

static DenoiseState *rnnoise_create() {
  DenoiseState *st;
  st = malloc(rnnoise_get_size());
  rnnoise_init(st);
  return st;
}

static void rnnoise_destroy(DenoiseState *st) {
  free(st);
}

int main(int argc, char **argv) {
  const int BUF_SIZE = 1024;
  const int CEP_PITCH_DIM = 39;
  char * cep_file = argv[1];
  char * lpc_file = argv[2];

  DenoiseState *st = rnnoise_create();

  FILE * fin = fopen(cep_file, "r");
  FILE * fout = fopen(lpc_file, "w");
  char line[BUF_SIZE];
  memset(line,0,BUF_SIZE);

  float cep_features[CEP_PITCH_DIM];
  int line_count = 0;
  while(!feof(fin)) {
    fgets(line,BUF_SIZE,fin);
    char * b;
    b = line;
    char * tp = strsep(&b, " ");
    int p = 0;
    while(tp != NULL) {
      if(p >= CEP_PITCH_DIM) break;
      double d = atof(tp);
      //printf("%f\t", d);
      cep_features[p++] = d;
      tp = strsep(&b, " ");
    }
    //printf("\n");

    float g = lpc_from_cepstrum(st->lpc, cep_features);

    for (int j = 0; j < (LPC_ORDER-1); j++) {
      fprintf(fout, "%f ", st->lpc[j]);
    }
    fprintf(fout, "%f\n", st->lpc[LPC_ORDER-1]);
  }
  fclose(fin);
  fclose(fout);
  rnnoise_destroy(st);
}