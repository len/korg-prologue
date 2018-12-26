/*
 * File: tremolo.cpp
 * Author: Luciano Esteban Notarfrancesco (github.com/len)
 *
 * Tremolo effect
 *
 */

#include "usermodfx.h"

#include "simplelfo.hpp"

static dsp::SimpleLFO s_lfo;

#define TREMOLO_FREQUENCY_MIN 1
#define TREMOLO_FREQUENCY_MAX 10

static float tremolo_frequency = 5.5f; // hz
static float tremolo_depth = 0.f;

static const float s_fs_recip = 1.f / 48000.f;

void MODFX_INIT(uint32_t platform, uint32_t api)
{
  tremolo_frequency = 5.5f;
  tremolo_depth = 0.f;
  s_lfo.reset();
//  s_lfo.setF0(tremolo_frequency,s_fs_recip);
}

void MODFX_PROCESS(const float *main_xn, float *main_yn,
                   const float *sub_xn,  float *sub_yn,
                   uint32_t frames)
{
  const float * mx = main_xn;
  float * __restrict my = main_yn;
  const float * my_e = my + 2*frames;
  const float * sx = sub_xn;
  float * __restrict sy = sub_yn;

  const float depth = tremolo_depth;
  s_lfo.setF0(tremolo_frequency, s_fs_recip);
  
  for (; my != my_e; ) {

    s_lfo.cycle();

    const float lfo_value = 1.f - s_lfo.sine_uni() * depth;
    
    *(my++) = *(mx++)*lfo_value;
    *(my++) = *(mx++)*lfo_value;
    *(sy++) = *(sx++)*lfo_value;
    *(sy++) = *(sx++)*lfo_value;
  }
}

void MODFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case 0:
    tremolo_frequency = linintf(valf, TREMOLO_FREQUENCY_MIN, TREMOLO_FREQUENCY_MAX);
    break;
  case 1:
    tremolo_depth = valf;
    break;
  default:
    break;
  }
}

