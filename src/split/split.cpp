/*
 * File: split.cpp
 * Author: Luciano Esteban Notarfrancesco (github.com/len)
 *
 */

#include "usermodfx.h"

static float main_balance = 0, sub_balance = 0;

void MODFX_INIT(uint32_t platform, uint32_t api)
{
  main_balance = 0;
  sub_balance = 0;
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

  float m_left_mix_to_left, m_right_mix_to_left, m_left_mix_to_right, m_right_mix_to_right;
  if (main_balance <= 0) {
    m_left_mix_to_left = 1.f + main_balance / 2.f; // 0 to 0.5
    m_right_mix_to_left = 0.5f - m_left_mix_to_left; // 0.5 to 0
    m_left_mix_to_right = 0;
    m_right_mix_to_right = 1.f + main_balance; // 0 to 1
  } else {
    m_left_mix_to_left = 1.f - main_balance; // 1 to 0
    m_right_mix_to_left = 0;
    m_left_mix_to_right = main_balance / 2.f; // 0 to 0.5
    m_right_mix_to_right = 1.f - m_left_mix_to_right; // 1 to 0.5
  }

  float s_left_mix_to_left, s_right_mix_to_left, s_left_mix_to_right, s_right_mix_to_right;
  if (sub_balance <= 0) {
    s_left_mix_to_left = 1.f + sub_balance / 2.f; // 0 to 0.5
    s_right_mix_to_left = 0.5f - s_left_mix_to_left; // 0.5 to 0
    s_left_mix_to_right = 0;
    s_right_mix_to_right = 1.f + sub_balance; // 0 to 1
  } else {
    s_left_mix_to_left = 1.f - sub_balance; // 1 to 0
    s_right_mix_to_left = 0;
    s_left_mix_to_right = sub_balance / 2.f; // 0 to 0.5
    s_right_mix_to_right = 1.f - s_left_mix_to_right; // 1 to 0.5
  }

  for (; my != my_e; ) {
    const float m_left = *(mx++);
    const float m_right = *(mx++);
    const float s_left = *(sx++);
    const float s_right = *(sx++);

    *(my++) = m_left * m_left_mix_to_left + m_right * m_right_mix_to_left;
    *(my++) = m_left * m_left_mix_to_right + m_right * m_right_mix_to_right;
    *(sy++) = s_left * s_left_mix_to_left + s_right * s_right_mix_to_left;
    *(sy++) = s_left * s_left_mix_to_right + s_right * s_right_mix_to_right;
  }
}


void MODFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case 0:
    sub_balance = valf * 2.f - 1.f; // -1 to 1
    break;
  case 1:
    main_balance = valf * 2.f - 1.f; // -1 to 1
    break;
  default:
    break;
  }
}

