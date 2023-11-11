//------------------------------------------------------------------------
// Copyright(c) 2023 Sommet.
//------------------------------------------------------------------------

#include <chrono>
#include <winsock2.h>
#include <windows.h>

#include "mypluginprocessor.h"
#include "myplugincids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;
using namespace Vst;

namespace SommetApp {
//------------------------------------------------------------------------
// Control_MasterProcessor
//------------------------------------------------------------------------
Control_MasterProcessor::Control_MasterProcessor ()
{
	running = true;
	sendThread = std::thread(&Control_MasterProcessor::sendAudioData, this);

	//--- set the wanted controller for our processor
	setControllerClass (kControl_MasterControllerUID);
}

void Control_MasterProcessor::sendAudioData()
{
	WSADATA		wsaData;
	SOCKET		clientSocket;
	sockaddr_in serverAddr;

	// Initialiser Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "Failed to initialize Winsock." << std::endl;
		return;
	}

	// Creer la socket UDP.
	clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create socket." << std::endl;
		WSACleanup();
		return;
	}

	// Initialiser les détails du serveur
	serverAddr.sin_family		= AF_INET;
	serverAddr.sin_port			= htons(6980);
	serverAddr.sin_addr.s_addr	= inet_addr("95.217.192.154");

	while (running)
	{
		int currentReaderIndex	= this->readerIndex.load();
		int currentWriterIndex	= this->writerIndex.load();

		// Buffer has not been wrapped yet.
		if (currentWriterIndex >= currentReaderIndex) {
			int chunkSize = currentWriterIndex - currentReaderIndex;

			// We need at least 256 samples (buffer delay).
			if (chunkSize >= (256 * sizeof(Sample64))) {

				// Send the chunk from current reader index to the writer index.
				sendto(clientSocket, &this->circularBuffer[currentReaderIndex], chunkSize, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

				// Update the reader index to the current writer index.
				this->readerIndex.store(currentWriterIndex);
			}

		} else {
			// Buffer has wrapped around.
			int firstChunkSize	= sizeof(this->circularBuffer) - currentReaderIndex;
			int secondChunkSize = currentWriterIndex;
			int chunkSize		= firstChunkSize + secondChunkSize;

			// We need at least 256 samples (buffer delay).
			if (chunkSize >= (256 * sizeof(Sample64))) {

				// Send the first chunk from current reader index to the end of the buffer.
				sendto(clientSocket, &this->circularBuffer[currentReaderIndex], firstChunkSize, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

				// Send the second chunk from the beginning of the buffer to the current writer index.
				sendto(clientSocket, &this->circularBuffer[0], secondChunkSize, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

				// Update the reader index to the current writer index.
				this->readerIndex.store(currentWriterIndex);
			}
		}

		// @Warning: Bottleneck of 50ms.
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// Fermer la socket et lib�rer les ressources
	closesocket(clientSocket);
}

//------------------------------------------------------------------------
Control_MasterProcessor::~Control_MasterProcessor ()
{
	if (running) {
		running = false;
		sendThread.join();
		WSACleanup();
	}
}

//------------------------------------------------------------------------
tresult PLUGIN_API Control_MasterProcessor::initialize (FUnknown* context)
{
	tresult result = AudioEffect::initialize(context);

	if (result == kResultTrue)
	{
		addAudioInput(STR16("AudioInput"), SpeakerArr::kStereo);
		addAudioOutput(STR16("AudioOutput"), SpeakerArr::kStereo);
	}

	return result;
}

tresult PLUGIN_API Control_MasterProcessor::setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) 
{
	// we only support one in and output bus and these busses must have the same number of channels
	if (numIns == 1 && numOuts == 1 && inputs[0] == outputs[0])
		return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Control_MasterProcessor::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API Control_MasterProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

inline uint32 getSampleFramesSizeInBytes(const ProcessSetup& processSetup, int32 numSamples)
{
	return processSetup.symbolicSampleSize == kSample32
		? numSamples * sizeof(Sample32)
		: numSamples * sizeof(Sample64);
}

//------------------------------------------------------------------------

void interleaveChannels(float* destination, const Sample32* channelLeft, const Sample32* channelRight, int numSamples) {
	for (int i = 0; i < numSamples; i++) {
		int j = (i * 2);

		destination[j]		= channelLeft[i];
		destination[j + 1]	= channelRight[i];
	}
}

tresult PLUGIN_API Control_MasterProcessor::process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------

    /*if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 index = 0; index < numParamsChanged; index++)
        {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData (index))
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
                switch (paramQueue->getParameterId ())
                {
				}
			}
		}
	}*/

	//--- Here you have to implement your processing
	if (data.numSamples > 0)
	{
		SpeakerArrangement arr;
		getBusArrangement(kOutput, 0, arr);

		int32 numChannels		= SpeakerArr::getChannelCount(arr);
		uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);

		// Set output silence flags based on input flags.
		data.outputs->silenceFlags = data.inputs->silenceFlags ? 0x7FFFF : 0;

		// Process silence flags.
		if (data.inputs->silenceFlags)
		{
			for (int32 channel = 0; channel < numChannels; channel++)
			{
				float* inputChannel = data.inputs[0].channelBuffers32[channel];
				float* outputChannel = data.outputs[0].channelBuffers32[channel];

				memset(inputChannel, 0, sampleFramesSize);
				memset(outputChannel, 0, sampleFramesSize);
			}

			return kResultOk;
		}

		// Passthrough, map input channels to output channels.
		for (int32 channel = 0; channel < numChannels; channel++)
		{
			float* inputChannel = data.inputs[0].channelBuffers32[channel];
			float* outputChannel = data.outputs[0].channelBuffers32[channel];

			// Copy input to output.
			memcpy_s(outputChannel, sampleFramesSize, inputChannel, sampleFramesSize);
		}

		// Copy buffers for networking.
		if (data.processMode == kRealtime) {
			int currentWriterIndex		= this->writerIndex.load();
			int currentWriterIndexEnd	= currentWriterIndex + (sampleFramesSize * 2);
			int circularBufferSize		= sizeof(this->circularBuffer);

			// We should not wrap now.
			if (currentWriterIndexEnd < circularBufferSize) {

				// Copy input channels to the circular buffer in an interleaved format (LRLRLR...).
				interleaveChannels((float*)&this->circularBuffer[currentWriterIndex], data.inputs[0].channelBuffers32[0], data.inputs[0].channelBuffers32[1], data.numSamples);

			} else {
				/*
				// Buffer will wrap around.
				int firstChunkSize	= circularBufferSize - currentWriterIndex;
				int lastChunkSize	= (sampleFramesSize * 2) - firstChunkSize;

				// @TODO: How to write interleaved if the buffer has wrapped ?
				// Such as L|R (where R channel should wrap to the start of the buffer).

				// Copy input channels to the circular buffer in an interleaved format (LRLRLR...).
				interleaveChannels((float*)&this->circularBuffer[currentWriterIndex], data.outputs[0].channelBuffers32[0], data.outputs[0].channelBuffers32[1], firstChunkSize);

				// Copy input channels to the circular buffer in an interleaved format (LRLRLR...).
				interleaveChannels((float*)&this->circularBuffer[0], data.outputs[0].channelBuffers32[0], data.outputs[0].channelBuffers32[1], data.numSamples);
				*/

				// Ensure the writer index wraps.
				currentWriterIndexEnd %= circularBufferSize;
			}

			// Set current writer index.
			currentWriterIndex = currentWriterIndexEnd;

			// Store the new writer index position.
			this->writerIndex.store(currentWriterIndex);
		}
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Control_MasterProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API Control_MasterProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	// if (symbolicSampleSize == Vst::kSample64)
		// return kResultTrue;

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Control_MasterProcessor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Control_MasterProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace SommetApp
