/*
 * File: split.cpp
 * Author: Luciano Esteban Notarfrancesco (github.com/len)
 *
 */

#include "usermodfx.h"

void MODFX_INIT(uint32_t platform, uint32_t api)
{
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

  for (; my != my_e; ) {
    const float mv = (*(mx++) + *(mx++)) * .5f;
    const float sv = (*(sx++) + *(sx++)) * .5f;

    *(my++) = mv;
    *(my++) = sv;
    *(sy++) = mv;
    *(sy++) = sv;
  }
}


void MODFX_PARAM(uint8_t index, int32_t value)
{
}

