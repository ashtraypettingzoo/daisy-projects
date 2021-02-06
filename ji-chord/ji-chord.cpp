#include "daisysp.h"
#include "daisy_patch.h"
#include <string>
#include <cmath>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;
Oscillator osc[6];
Parameter freqctrl, numeratorCtrl, ampCtrl, denominatorCtrl;
bool ampLock = false;
bool numLock = false;
bool denLock = false;
float numStartVal = 1.f;
float denStartVal = 1.f;
float ampStartVal = .6f;
const float NUM_DEADZONE = .7f;
const float DEN_DEADZONE = .7f;
const float AMP_DEADZONE = .1f;
int mode = 0;
int numerator[4] = { 1, 1, 1, 1 };
int denominator[4] = { 1, 1, 1, 1 };
float amp[5] = { .6f };
float basefreq = 10.f;

void ProcessControls();
void UpdateOLED();

static void AudioCallback(float ** in, float ** out, size_t size)
{
	ProcessControls();
	
	float sig;

    for (size_t i = 0; i < size; i += 2)
    {
        osc[0].SetFreq(basefreq);
        osc[0].SetAmp(amp[0]*.5f);
		
		osc[1].SetFreq(basefreq * numerator[0] / denominator[0]);
        osc[1].SetAmp(amp[1]*.5f);
		
		osc[2].SetFreq(basefreq * numerator[1] / denominator[1]);
        osc[2].SetAmp(amp[2]*.5f);
		
		osc[3].SetFreq(basefreq * numerator[2] / denominator[2]);
        osc[3].SetAmp(amp[3]*.5f);
		
		osc[4].SetFreq(basefreq * numerator[3] / denominator[3]);
        osc[4].SetAmp(amp[4]*.5f);
		
    	sig = osc[0].Process() + osc[1].Process() + osc[2].Process() + osc[3].Process() + osc[4].Process();
		
        for (size_t chn = 0; chn < 4; chn++)
        {
            out[chn][i] = sig;
        }
    }
}

void UpdateOled()
{
    patch.display.Fill(false);
	
	std::string str;
	
	patch.display.SetCursor(0, 0);
	int ampNum = amp[0] * 100.f;
	int freqNum = basefreq;
    str = " OSC1:" + std::to_string(freqNum) + "Hz," + std::to_string(ampNum) + "%";
    patch.display.WriteString(const_cast<char*>(str.c_str()), Font_7x10, true);
    
    patch.display.SetCursor(0, 12 * 1);
	ampNum = amp[1] * 100.f;
    str = " OSC2:" + std::to_string(numerator[0]) + "/" + std::to_string(denominator[0]) + "," + std::to_string(ampNum) + "%";
    patch.display.WriteString(const_cast<char*>(str.c_str()), Font_7x10, true);
	
	patch.display.SetCursor(0, 12 * 2);
	ampNum = amp[2] * 100.f;
    str = " OSC3:" + std::to_string(numerator[1]) + "/" + std::to_string(denominator[1]) + "," + std::to_string(ampNum) + "%";
    patch.display.WriteString(const_cast<char*>(str.c_str()), Font_7x10, true);
	
	patch.display.SetCursor(0, 12 * 3);
	ampNum = amp[3] * 100.f;
    str = " OSC4:" + std::to_string(numerator[2]) + "/" + std::to_string(denominator[2]) + "," + std::to_string(ampNum) + "%";
    patch.display.WriteString(const_cast<char*>(str.c_str()), Font_7x10, true);
	
	patch.display.SetCursor(0, 12 * 4);
	ampNum = amp[4] * 100.f;
    str = " OSC5:" + std::to_string(numerator[3]) + "/" + std::to_string(denominator[3]) + "," + std::to_string(ampNum) + "%";
    patch.display.WriteString(const_cast<char*>(str.c_str()), Font_7x10, true);
	
	patch.display.SetCursor(0, mode * 12);
	patch.display.WriteString(const_cast<char*>(">"), Font_7x10, true);
    
    patch.display.Update();
}

void ProcessControls()
{
	
    patch.UpdateAnalogControls();
    patch.DebounceControls();
	
	basefreq = mtof(freqctrl.Process());
	if (mode != 0)
	{
		if (!numLock)
			numerator[mode - 1] = numeratorCtrl.Process();
		if (!denLock)
			denominator[mode - 1] = denominatorCtrl.Process();
	}
	if (!ampLock)
		amp[mode] = ampCtrl.Process();

	if (std::abs(numeratorCtrl.Process() - numStartVal) > NUM_DEADZONE)
		numLock = false;
	if (std::abs(denominatorCtrl.Process() - denStartVal) > DEN_DEADZONE)
		denLock = false;
	if (std::abs(ampCtrl.Process() - ampStartVal) > AMP_DEADZONE)
		ampLock = false;
	
    mode = (5 + patch.encoder.Increment() + mode) % 5;
	mode = mode >= 0 ? mode : 0;
	
	if (patch.encoder.Increment() != 0)
	{
		numLock = denLock = ampLock = true;
		numStartVal = numeratorCtrl.Process();
		denStartVal = denominatorCtrl.Process();
		ampStartVal = ampCtrl.Process();
	}
}


int main(void)
{
	patch.Init();
	
    float samplerate = patch.AudioSampleRate();
   
    osc[0].Init(samplerate);
	osc[0].SetWaveform(Oscillator::WAVE_SIN);
	
	osc[1].Init(samplerate);
	osc[1].SetWaveform(Oscillator::WAVE_SIN);
	
	osc[2].Init(samplerate);
	osc[2].SetWaveform(Oscillator::WAVE_SIN);
	
	osc[3].Init(samplerate);
	osc[3].SetWaveform(Oscillator::WAVE_SIN);
	
	osc[4].Init(samplerate);
	osc[4].SetWaveform(Oscillator::WAVE_SIN);
   
    freqctrl.Init(patch.controls[patch.CTRL_1], 30.0, 110.0f, Parameter::LINEAR);
    numeratorCtrl.Init(patch.controls[patch.CTRL_2], 1.f, 17.f, Parameter::LINEAR);
    denominatorCtrl.Init(patch.controls[patch.CTRL_3], 1.f, 17.f, Parameter::LINEAR);
    ampCtrl.Init(patch.controls[patch.CTRL_4], 0.f, 1.f, Parameter::LINEAR);
	
    patch.display.WriteString(const_cast<char*>("JI-CHORD"), Font_7x10, true);
    patch.display.Update();
    patch.DelayMs(2000);

    patch.StartAdc();
    patch.StartAudio(AudioCallback);
	
    while(1) 
    {
        UpdateOled();
    }
}