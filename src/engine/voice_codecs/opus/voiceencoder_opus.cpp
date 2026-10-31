//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "ivoicecodec.h"
#include "iframeencoder.h"

#include <stdio.h>
#include <opus/opus.h>

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"
#include "tier0/dbg.h"

#define CHANNELS 1
#define PACKET_SIZE 28
#define RAW_FRAME_SIZE 480

class VoiceEncoder_Opus : public IFrameEncoder
{
public:
	VoiceEncoder_Opus();
	virtual ~VoiceEncoder_Opus();

	// Interfaces IFrameDecoder

	bool Init(int quality, int &rawFrameSize, int &encodedFrameSize);
	void Release();
	void DecodeFrame(const char *pCompressed, char *pDecompressedBytes);
	void EncodeFrame(const char *pUncompressedBytes, char *pCompressed);
	bool ResetState();

private:
	OpusEncoder* m_Encoder;
	OpusDecoder* m_Decoder;
};

extern IVoiceCodec* CreateVoiceCodec_Frame(IFrameEncoder *pEncoder);

void* CreateCeltVoiceCodec()
{
	IFrameEncoder *pEncoder = new VoiceEncoder_Opus;
	return CreateVoiceCodec_Frame( pEncoder );
}

EXPOSE_INTERFACE_FN(CreateCeltVoiceCodec, IVoiceCodec, "vaudio_opus")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VoiceEncoder_Opus::VoiceEncoder_Opus()
{
	m_Encoder = nullptr;
	m_Decoder = nullptr;
}

VoiceEncoder_Opus::~VoiceEncoder_Opus()
{
	if (m_Encoder) opus_encoder_destroy(m_Encoder);
	if (m_Decoder) opus_decoder_destroy(m_Decoder);
}

bool VoiceEncoder_Opus::Init( int quality, int &rawFrameSize, int &encodedFrameSize)
{
	rawFrameSize = RAW_FRAME_SIZE * BYTES_PER_SAMPLE;

	encodedFrameSize = PACKET_SIZE;

    int err;
    m_Encoder = opus_encoder_create(48000, 1, OPUS_APPLICATION_VOIP, &err);
    if (err != OPUS_OK) return false;

    m_Decoder = opus_decoder_create(48000, 1, &err);
    if (err != OPUS_OK) return false;

	opus_encoder_ctl(m_Encoder, 0x00000FA8, 0x00000000, 0x00000450);
	opus_encoder_ctl(m_Encoder, 0x00000FC8, 0x00000000, 0x0000138B);
	opus_encoder_ctl(m_Encoder, 0x00000FA6, 0x00000000, 0x00000001);
	opus_encoder_ctl(m_Encoder, 0x00000FB4, 0x00000001, 0x00000000);
    return true;
}

void VoiceEncoder_Opus::Release()
{
	delete this;
}

void VoiceEncoder_Opus::EncodeFrame(const char *pUncompressedBytes, char *pCompressed)
{
	short* in = (short*)pUncompressedBytes;
	int nbBytes = opus_encode(m_Encoder, in, 480, (unsigned char*)pCompressed, 28);
	if (nbBytes < 0) nbBytes = 0;
}

void VoiceEncoder_Opus::DecodeFrame(const char *pCompressed, char *pDecompressedBytes)
{
	short* out = (short*)pDecompressedBytes;
	int frame_size = opus_decode(m_Decoder, (const unsigned char*)pCompressed, 28, out, 480, 0);
	if (frame_size < 0) frame_size = 0;
}

bool VoiceEncoder_Opus::ResetState()
{
	opus_encoder_ctl(m_Encoder, OPUS_RESET_STATE);
	opus_decoder_ctl(m_Decoder, OPUS_RESET_STATE);

	return true;
}