
/*
 * File: pluck.cpp
 * Author: Luciano Esteban Notarfrancesco (github.com/len)
 *
 * Karplus-Strong plucked string algorithm
 *
 */

#include "userosc.h"
#include "delayline.hpp"

enum {
  k_flags_none    = 0,
  k_flag_reset    = 1<<0
};

#define DELAY_BUFFER_SIZE 4096 // 2048 is too small to fit the lowest octave, and it must be a power of 2 due to DelayLine implementation
static float delay_buffer[DELAY_BUFFER_SIZE];

typedef struct State {
  dsp::DelayLine delay;
  float damping, attack, attenuation, inharmonicity;
  uint32_t burst;
  float lfo, lfoz;
  uint32_t flags:8;
} State;

static State s;

void OSC_INIT(uint32_t platform, uint32_t api)
{
  s.delay.setMemory(delay_buffer, DELAY_BUFFER_SIZE);

  s.damping = .5f;
  s.attack = 10; // 10 milliseconds
  s.attenuation = 0;
  s.inharmonicity = 0;
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
  // Handle events.
  {
    const uint32_t flags = s.flags;
    s.flags = k_flags_none;
    
    if (flags & k_flag_reset) {
      s.delay.clear();
      s.burst = 48.f*s.attack; // milliseconds at 48khz
    }
    
    s.lfo = q31_to_f32(params->shape_lfo);
  }
  
  const float attenuation = s.attenuation;
  const float length = clipminmaxf(2.f, 1.f / osc_w0f_for_note((params->pitch)>>8, params->pitch & 0xFF), DELAY_BUFFER_SIZE);

  uint32_t burst = s.burst;

  float lfoz = s.lfoz;
  const float lfo_inc = (s.lfo - lfoz) / frames;
  
  q31_t * __restrict y = (q31_t *)yn;
  const q31_t * y_e = y + frames;
  float lastSig = 0;
  for (; y != y_e; ) {
    float sig = s.delay.readFrac(length);

    // low-pass filter for damping
    const float damping = clipminmaxf(.000001f, s.damping + lfoz, .999999f);
    sig = (1.f - attenuation) * (sig*damping + lastSig*(1.f - damping));

    if (burst>0) {
      burst--;
      //sig = clip1m1f(sig + osc_white());
      sig = osc_softclipf(0.05f, sig + osc_white());
    }

    // all-pass filter for inharmonicity
    // ... TODO

    s.delay.write(sig);

    *(y++) = f32_to_q31(sig);

    lastSig = sig; 

    lfoz += lfo_inc;
  }
  
  s.burst = burst;
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
  case k_osc_param_id2:
  case k_osc_param_id3:
  case k_osc_param_id4:
  case k_osc_param_id5:
  case k_osc_param_id6:
    break;
    
  case k_osc_param_shape:
    s.damping = 1.f - clipminmaxf(.0000001f, param_val_to_f32(value), .999999f);
    break;
    
  case k_osc_param_shiftshape:
    {
      const float x = param_val_to_f32(value)*.6f + .1f;
      s.attenuation = x*x*x; // values between 0.01 and 0.343 with more resolution in the lower values
    }
    break;
    
  default:
    break;
  }
}

