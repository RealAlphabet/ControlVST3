//------------------------------------------------------------------------
// Copyright(c) 2023 Sommet.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include <iostream>
#include <thread>

using namespace Steinberg;
using namespace Vst;

namespace SommetApp {

//------------------------------------------------------------------------
//  Control_MasterProcessor
//------------------------------------------------------------------------
class Control_MasterProcessor : public AudioEffect
{
public:
	Control_MasterProcessor ();
	~Control_MasterProcessor () SMTG_OVERRIDE;

    // Create function
	static FUnknown* createInstance (void* /*context*/) 
	{ 
		return (IAudioProcessor*)new Control_MasterProcessor; 
	}

	void sendAudioData();

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;

	tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) SMTG_OVERRIDE;

	/** Called at the end before destructor */
	tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	/** Switch the Plug-in on/off */
	tresult PLUGIN_API setActive (TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	tresult PLUGIN_API setupProcessing (ProcessSetup& newSetup) SMTG_OVERRIDE;
	
	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	tresult PLUGIN_API canProcessSampleSize (int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Here we go...the process call */
	tresult PLUGIN_API process (ProcessData& data) SMTG_OVERRIDE;
		
	/** For persistence */
	tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:

private:
	std::atomic<int> writerIndex;
	std::atomic<int> readerIndex;

	// Allow up to 100 seconds buffer.
	char circularBuffer[100 * 48000 * sizeof(Sample64)];

	std::thread sendThread;
	bool running;
};

//------------------------------------------------------------------------
} // namespace SommetApp
