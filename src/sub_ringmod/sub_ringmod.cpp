/*
 * File: sub_ringmod.cpp
 * Author: Luciano Esteban Notarfrancesco (github.com/len)
 *
 * Ring modulator effect using the sub timbre as modulator and the main timbre as carrier
 *
 */

#include "usermodfx.h"

static float s_depth = 0.f;
static float s_scale = 1.f;

void MODFX_INIT(uint32_t platform, uint32_t api)
{
  s_depth = 0.f;
  s_scale = 1.f;
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

  const float depth = s_depth;
  const float scale = s_scale;
  const float mix = 1.f - depth;
  
  for (; my != my_e; ) {
    const float sub_left = *(sx++);
    const float sub_right = *(sx++);
    const float main_left = *(mx++);
    const float main_right = *(mx++);
    const float output_left = main_left*clip1m1f(sub_left*scale)*depth;
    const float output_right = main_right*clip1m1f(sub_right*scale)*depth;
    *(my++) = main_left*mix + output_left;
    *(my++) = main_right*mix + output_right;
    *(sy++) = sub_left*mix;
    *(sy++) = sub_right*mix;
  }
}


void MODFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case 0: // speed knob
    s_scale = valf * 16.f;
    break;
  case 1: // depth knob
    s_depth = valf;
    break;
  default:
    break;
  }
}

