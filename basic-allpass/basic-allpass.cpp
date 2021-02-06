#include "daisysp.h"
#include "daisy_patch.h"
#include <string>
#include <cmath>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;
float xkm1[4] = { 0.f }; // x[k - 1]
float xkm2[4] = { 0.f }; // x[k - 2]
float ykm1[4] = { 0.f }; // y[k - 1]
float ykm2[4] = { 0.f }; // y[k - 2]

static void AudioCallback(float ** in, float ** out, size_t size)
{
	// get control values
	float ctrlVal[4]; // 0.f - 1.f
	for (int i = 0; i < 4; i++)
		ctrlVal[i] = patch.controls[i].Process();
	
	// compute coefficients & process audio
	// reference: https://en.wikipedia.org/wiki/All-pass_filter#Digital_implementation
	float ztheta  = ctrlVal[0] * 2.f * M_PI,
	      zradius = ctrlVal[1],
		  zreal   = zradius * std::cos(ztheta);
	
    for (size_t i = 0; i < size; i += 2)
    {
        for (size_t chn = 0; chn < 4; chn++)
        {
			float xk = in[chn][i];
			float yk = xk; // send input directly into output
			// TODO: implement allpass algorithm
            out[chn][i] = yk;
        }
    }
}

int main(void)
{
    patch.Init();
	
    patch.display.WriteString(const_cast<char*>("BASIC-ALLPASS"), Font_7x10, true);
    patch.display.Update();
    patch.DelayMs(2000);

    patch.StartAdc();
    patch.StartAudio(AudioCallback);
	
    while(1) 
    {
        patch.DisplayControls(false);
    }
}