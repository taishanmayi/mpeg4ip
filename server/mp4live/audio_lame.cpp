/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MPEG4IP.
 * 
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2000, 2001.  All Rights Reserved.
 * 
 * Contributor(s): 
 *		Dave Mackie		dmackie@cisco.com
 */

#include "mp4live.h"
#include "video_xvid.h"

CLameAudioEncoder::CLameAudioEncoder()
{
	m_leftBuffer = NULL;
	m_rightBuffer = NULL;
	m_mp3FrameBuffer = NULL;
}

bool CLameAudioEncoder::Init(CLiveConfig* pConfig, bool realTime)
{
	m_pConfig = pConfig;

	lame_init(&m_lameParams);

	m_lameParams.num_channels = 
		m_pConfig->GetIntegerValue(CONFIG_AUDIO_CHANNELS);
	m_lameParams.in_samplerate = 
		m_pConfig->GetIntegerValue(CONFIG_AUDIO_SAMPLE_RATE);
	m_lameParams.brate = 
		m_pConfig->GetIntegerValue(CONFIG_AUDIO_BIT_RATE);
	m_lameParams.mode = 0;
	m_lameParams.quality = 2;
	m_lameParams.silent = 1;
	m_lameParams.gtkflag = 0;

	lame_init_params(&m_lameParams);

	m_pConfig->m_audioMp3SampleRate = 
		m_lameParams.out_samplerate;

	if (m_pConfig->m_audioMp3SampleRate > 24000) {
		m_pConfig->m_audioMp3SamplesPerFrame = 
			MP3_MPEG1_SAMPLES_PER_FRAME;
	} else {
		m_pConfig->m_audioMp3SamplesPerFrame = 
			MP3_MPEG2_SAMPLES_PER_FRAME;
	}

	return true;
}

bool CLameAudioEncoder::EncodeSamples(
	u_int8_t* pBuffer, u_int32_t bufferLength)
{
	u_int32_t mp3DataLength = 0;

	if (pBuffer) {
		u_int16_t* leftBuffer;

		// de-interleave input if doing stereo
		if (m_pConfig->GetIntegerValue(CONFIG_AUDIO_CHANNELS) == 2) {
			// de-interleave raw frame buffer
			u_int16_t* s = pBuffer;
			for (int i = 0; i < m_pConfig->m_audioMp3SamplesPerFrame; i++) {
				m_leftBuffer[i] = *s++;
				m_rightBuffer[i] = *s++;
			}
			leftBuffer = m_leftBuffer;
		} else {
			leftBuffer = pBuffer;
		}

		// call lame encoder
		mp3DataLength = lame_encode_buffer(
			&m_lameParams,
			(short*)leftBuffer, (short*)m_rightBuffer, 
			m_pConfig->m_audioMp3SamplesPerFrame,
			(char*)&m_mp3FrameBuffer[m_mp3FrameBufferLength], 
			m_mp3FrameBufferSize - m_mp3FrameBufferLength);

	} else { // pBuffer == NULL, signal to stop encoding
		mp3DataLength = lame_encode_finish(
			&m_lameParams,
			(char*)&m_mp3FrameBuffer[m_mp3FrameBufferLength], 
			m_mp3FrameBufferSize - m_mp3FrameBufferLength);
	}

	if (mp3DataLength == 0) {
		return false;
	}

	m_mp3FrameBufferLength += mp3DataLength;
	return true;
}

bool CLameAudioEncoder::GetEncodedFrame(
	u_int8_t** ppBuffer, u_int32_t* pBufferLength)
{
	// *ppBuffer = 
	// *pBufferLength = 

	return true;
}

void CLameAudioEncoder::Stop()
{
	free(m_leftBuffer);
	m_leftBuffer = NULL;

	free(m_rightBuffer);
	m_rightBuffer = NULL;

	free(m_mp3FrameBuffer);
	m_mp3FrameBuffer = NULL;
}

