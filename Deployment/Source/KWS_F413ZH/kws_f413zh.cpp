/*
 * Copyright (C) 2018 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "kws_f413zh.h"

#define SCRATCH_BUFF_SIZE  512/4
#if defined ( __CC_ARM )  /* !< ARM Compiler */
int32_t Scratch [SCRATCH_BUFF_SIZE] __attribute__((at(0x2000E000)));
#elif defined ( __ICCARM__ )  /* !< ICCARM Compiler */
#pragma location=0x2000E000
int32_t Scratch [SCRATCH_BUFF_SIZE];
#elif defined ( __GNUC__ )  /* !< GNU Compiler */
int32_t Scratch [SCRATCH_BUFF_SIZE] __attribute__((section(".scratch_section")));
#endif

KWS_F413ZH::KWS_F413ZH(int recording_win, int sliding_window_len)
:KWS_DNN(recording_win, sliding_window_len)
//:KWS_DS_CNN(recording_win, sliding_window_len)
// Change the parent class to KWS_DNN to switch to DNN model
{
  audio_buffer = new int16_t[audio_buffer_size];
  audio_buffer_in = new int16_t[audio_block_size*4]; //2 (L/R) channels x 2 for ping-pong buffers
  audio_buffer_out = new int16_t[audio_block_size*4]; //2 (L/R) channels x 2 for ping-pong buffers
}

KWS_F413ZH::~KWS_F413ZH()
{
  delete audio_buffer;
  delete audio_buffer_in;
  delete audio_buffer_out;
}

void KWS_F413ZH::set_volume(int vol)
{
  BSP_AUDIO_IN_SetVolume(vol);
}

void KWS_F413ZH::start_kws()
{
  // Initialize buffers
  memset(audio_buffer_in, 0, audio_block_size*8);
  memset(audio_buffer_out, 0, audio_block_size*8);

  BSP_AUDIO_IN_AllocScratch (Scratch, SCRATCH_BUFF_SIZE);
  if (BSP_AUDIO_IN_Init(I2S_AUDIOFREQ_16K, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR) != AUDIO_OK)
  {
    while(1);
  }

  // May need to adjust volume to get better accuracy/user-experience
  BSP_AUDIO_IN_SetVolume(75);
  // Start Recording
  BSP_AUDIO_IN_Record((uint16_t*)audio_buffer_in, audio_block_size * 4);

  // Start Playback for listening to what is being classified
  BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 50, I2S_AUDIOFREQ_16K);
  BSP_AUDIO_OUT_Play((uint16_t*)audio_buffer_out, audio_block_size * 8);

}
