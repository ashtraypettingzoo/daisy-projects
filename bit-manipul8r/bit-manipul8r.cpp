#include "daisysp.h"
#include "daisy_patch.h"
#include <string>
#include <cmath>

using namespace daisy;
using namespace daisysp;

struct inputCtrl
{
	Parameter param;
	float val[3] = {0.f};
	float startVal = 0.f;
	bool lock = false;
	float deadzone = 0.07f;
} ctrl[4];

void ProcessControls();
void UpdateOLED();

DaisyPatch patch;
float sampleRate;
int mode = 0;
float clampval[3] = {0.f};
float maxsig[3] = {0.f};
int numbits[3] = {0};
unsigned int flipbits[3] = {0};
unsigned int bitlevels[3] = {0};

static void AudioCallback(float ** in, float ** out, size_t size)
{
	ProcessControls();
	
	for (int i = 0; i < 3; ++i)
	{
		clampval[i] = 1.f - ctrl[0].val[i];
		clampval[i] = clampval[i] * clampval[i];
		maxsig[i] = ctrl[3].val[i];
		numbits[i] = (16.f * (1.f - ctrl[1].val[i])) + 1.f;
		numbits[i] = numbits[i] > 16 ? 16 : numbits[i];
		flipbits[i] = (numbits[i] + 1) * ctrl[2].val[i];
		bitlevels[i] = 1 << numbits[i];
	}
	
    for (size_t i = 0; i < size; i += 2)
    {
		float sigTotal = 0.f;
		
        for (size_t chn = 0; chn < 4; chn++)
        {
			float sig = 0;
			if (chn == 3)
				sig = sigTotal;
			else
			{
				sig = in[chn][i];
				sig = std::fmaxf(std::fminf(sig, clampval[chn]), -clampval[chn]);
				sig = sig / clampval[chn];
				sig = (1.f + sig) / 2.f;
				unsigned int crushed = std::floor(sig * bitlevels[chn]);
				crushed ^= (1u << flipbits[chn]) - 1u;
				sig = (float)crushed / (bitlevels[chn] - 1);
				sig = (2.f * sig) - 1.f;
				sig *= maxsig[chn];
				sigTotal += sig;
			}
            out[chn][i] = sig;
        }
    }
}

void UpdateOled()
{
    patch.display.Fill(false);
	
	std::string str;
	
	patch.display.SetCursor(0,0);
    str = "Channel_0" + std::to_string(mode);
    patch.display.WriteString((char*)str.c_str(), Font_7x10, true);
	
	patch.display.SetCursor(0,12);
	int dval = clampval[mode] * 100.f;
    str = "Limit:" + std::to_string(dval) + "%";
    patch.display.WriteString((char*)str.c_str(), Font_7x10, true);
	
	patch.display.SetCursor(0,24);
	dval = maxsig[mode] * 100.f;
    str = "Volume:" + std::to_string(dval) + "%";
    patch.display.WriteString((char*)str.c_str(), Font_7x10, true);
	
	patch.display.SetCursor(0,36);
    str = "Crush:" + std::to_string(numbits[mode]) + " bits";
    patch.display.WriteString((char*)str.c_str(), Font_7x10, true);
	
	patch.display.SetCursor(0,48);
    str = "Flip:" + std::to_string(flipbits[mode]) + " bits";
    patch.display.WriteString((char*)str.c_str(), Font_7x10, true);
    
    patch.display.Update();
}

void ProcessControls()
{
	
    patch.UpdateAnalogControls();
    patch.DebounceControls();
	
	mode = (3 + patch.encoder.Increment() + mode) % 3;
	mode = mode >= 0 ? mode : 0;
	
	for (int i = 0; i < 4; ++i)
	{
		if (patch.encoder.Increment() != 0)
		{
			ctrl[i].lock = true;
			ctrl[i].startVal = ctrl[i].param.Process();
		}
		else if (!ctrl[i].lock)
			ctrl[i].val[mode] = ctrl[i].param.Process();
		else if (std::abs(ctrl[i].param.Process() - ctrl[i].startVal) > ctrl[i].deadzone)
			ctrl[i].lock = false;
	}
}


int main(void)
{
	patch.Init();
	sampleRate = patch.AudioSampleRate();

	ctrl[0].param.Init(patch.controls[patch.CTRL_1], 0.f, 1.f, Parameter::LINEAR);
	ctrl[1].param.Init(patch.controls[patch.CTRL_2], 0.f, 1.f, Parameter::LINEAR);
	ctrl[2].param.Init(patch.controls[patch.CTRL_3], 0.f, 1.f, Parameter::LINEAR);
	ctrl[3].param.Init(patch.controls[patch.CTRL_4], 0.f, 1.f, Parameter::LINEAR);

	patch.display.WriteString(const_cast<char*>("BIT-MANIPUL8R"), Font_7x10, true);
	patch.display.Update();
	patch.DelayMs(2000);

	patch.StartAdc();
	patch.StartAudio(AudioCallback);

	while(1) 
	{
		UpdateOled();
	}
}