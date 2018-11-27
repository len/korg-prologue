
/*
 * File: organ.cpp
 * Author: Luciano Esteban Notarfrancesco (github.com/len)
 *
 * organ-like additive oscillator
 *
 * TODO:
 * - think a better function for the shape knob, or implement a nicer detuning
 * - add a parameter to change the wave
 *
 */

#include "userosc.h"

#define SEMITONE 1.05946309436f
#define MINOR_THIRD (SEMITONE * SEMITONE * SEMITONE)
#define MAJOR_THIRD (MINOR_THIRD * SEMITONE)
#define PERFECT_FIFTH (MAJOR_THIRD * MINOR_THIRD)

#define H_SUB .5f
#define H_SUB_3RD PERFECT_FIFTH
#define H_2ND 2.f
#define H_3RD (H_2ND * PERFECT_FIFTH)
#define H_4TH 4.f
#define H_6TH (H_4TH * MAJOR_THIRD)
#define H_8TH 8.f

enum {
  k_flags_none    = 0,
  k_flag_drawbars = 1<<0,
  k_flag_reset    = 1<<1
};

typedef struct Params {
  float shape, shiftshape, drawbar_16, drawbar_8, drawbar_4, drawbar_II, drawbar_III;
} Params;

typedef struct State {
  float    phi1, phi2, phi3, phi4, phi5, phi6, phi7, phi8;
  float    w01, w02, w03, w04, w05, w06, w07, w08;
  float    a1, a2, a3, a4, a5, a6, a7, a8;
  float    lfo, lfoz;
  uint32_t flags:8;
} State;

static Params p;
static State s;

inline void updatePitch(float w0) {
  s.w01 = w0 * H_SUB;
  s.w02 = w0;
  s.w03 = w0 * H_SUB_3RD;
  s.w04 = w0 * H_2ND;
  s.w05 = w0 * H_3RD;
  s.w06 = w0 * H_4TH;
  s.w07 = w0 * H_6TH;
  s.w08 = w0 * H_8TH;
}

#define MAX(a,b) (a > b : a : b)

inline void updateAmplitudes(void) {
//    const float total_amplitude = MAX(MAX(MAX(MAX(params.drawbar_16, para
//ms.drawbar_8), params.drawbar_4), params.drawbar_II), params.drawbar_III);
//    const float scale = 5.f / total_amplitude * (params.drawbar_16 + para
//ms.drawbar_8 + params.drawbar_4 + params.drawbar_II*2 + params.drawbar_III*
//3);
  const float scale = 5.f;
  s.a1 = p.drawbar_16 / scale;
  s.a2 = p.drawbar_8 / scale;
  s.a3 = s.a7 = p.drawbar_II / scale;
  s.a4 = p.drawbar_4 / scale;
  s.a5 = s.a6 = s.a8 = p.drawbar_III / scale;
}

inline void resetPhase(void) {
//  s.phi1 = s.phi2 = s.phi3 = s.phi4 = s.phi5 = s.phi6 = s.phi7 = s.phi8 = 0.f;
  s.lfo = s.lfoz;
}

void OSC_INIT(uint32_t platform, uint32_t api)
{
  p.drawbar_16 = 1.f;
  p.drawbar_8 = 1.f;
  p.drawbar_4 = 1.f;
  p.drawbar_II = 1.f;
  p.drawbar_III = 1.f;
  updatePitch(440.f);
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
  // Handle events.
  {
    const uint32_t flags = s.flags;
    s.flags = k_flags_none;
    
    updatePitch(osc_w0f_for_note((params->pitch)>>8, params->pitch & 0xFF));
    
    if (flags & k_flag_drawbars)
      updateAmplitudes();
    if (flags & k_flag_reset)
      resetPhase();
    
    s.lfo = q31_to_f32(params->shape_lfo);
  }
  
  // Temporaries.
  float phi1 = s.phi1;
  float phi2 = s.phi2;
  float phi3 = s.phi3;
  float phi4 = s.phi4;
  float phi5 = s.phi5;
  float phi6 = s.phi6;
  float phi7 = s.phi7;
  float phi8 = s.phi8;

  const float shape = p.shape;
  const float drive = p.shiftshape;
  const float w01 = s.w01, w02 = s.w02, w03 = s.w03, w04 = s.w04, w05 = s.w05, w06 = s.w06, w07 = s.w07, w08 = s.w08;

  float lfoz = s.lfoz;
  const float lfo_inc = (s.lfo - lfoz) / frames;
  
  q31_t * __restrict y = (q31_t *)yn;
  const q31_t * y_e = y + frames;
  
  for (; y != y_e; ) {
    const float d1 = clipminmaxf(0.f, shape + shape * lfoz, 1.f);
    const float d0 = 1.f - d1;

    float sig;

    sig = s.a1 * osc_sinf(phi1) +
        + s.a2 * osc_sinf(phi2) +
        + s.a3 * osc_sinf(phi3) +
        + s.a4 * osc_sinf(phi4) +
        + s.a5 * osc_sinf(phi5) +
        + s.a6 * osc_sinf(phi6) +
        + s.a7 * osc_sinf(phi7) +
        + s.a8 * osc_sinf(phi8);

    //sig = clip1m1f(sig);
    const float main_sig = osc_softclipf(0.05f, drive * sig);    
    
    *(y++) = f32_to_q31(main_sig);
    
    phi1 += w01*d0 + w02*d1;
    phi1 -= (uint32_t)phi1;
    phi2 += w02*d0 + w03*d1;
    phi2 -= (uint32_t)phi2;
    phi3 += w03*d0 + w04*d1;
    phi3 -= (uint32_t)phi3;
    phi4 += w04*d0 + w05*d1;
    phi4 -= (uint32_t)phi4;
    phi5 += w05*d0 + w06*d1;
    phi5 -= (uint32_t)phi5;
    phi6 += w06*d0 + w07*d1;
    phi6 -= (uint32_t)phi6;
    phi7 += w07*d0 + w08*d1;
    phi7 -= (uint32_t)phi7;
    phi8 += w08*d0 + w01*d1;
    phi8 -= (uint32_t)phi8;
    lfoz += lfo_inc;
  }
  
  s.phi1 = phi1;
  s.phi2 = phi2;
  s.phi3 = phi3;
  s.phi4 = phi4;
  s.phi5 = phi5;
  s.phi6 = phi6;
  s.phi7 = phi7;
  s.phi8 = phi8;
  s.lfoz = lfoz;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  s.flags |= k_flag_reset;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  (void)params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{ 
  switch (index) {
  case k_osc_param_id1:
    p.drawbar_16 = clip01f(value / 8.f);
    s.flags |= k_flag_drawbars;
    break;
    
  case k_osc_param_id2:
    p.drawbar_8 = clip01f(value / 8.f);
    s.flags |= k_flag_drawbars;
    break;
    
  case k_osc_param_id3:
    p.drawbar_4 = clip01f(value / 8.f);
    s.flags |= k_flag_drawbars;
    break;
    
  case k_osc_param_id4:
    p.drawbar_II = clip01f(value / 8.f);
    s.flags |= k_flag_drawbars;
    break;
    
  case k_osc_param_id5:
    p.drawbar_III = clip01f(value / 8.f);
    s.flags |= k_flag_drawbars;
    break;
    
  case k_osc_param_id6:
    break;
    
  case k_osc_param_shape:
    p.shape = param_val_to_f32(value);
    break;
    
  case k_osc_param_shiftshape:
    p.shiftshape = 1.f + param_val_to_f32(value); 
    break;
    
  default:
    break;
  }
}

