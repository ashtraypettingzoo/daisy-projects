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
	// ctrl 0 = dry/wet
	// ctrl 1 = z radius
	// ctrl 2 = z theta
	float ctrlVal[4]; // 0.f - 1.f
	for (int i = 0; i < 4; i++)
		ctrlVal[i] = patch.controls[i].Process();
	
	// compute coefficients & process audio
	float ztheta = ctrlVal[2] * 2.f * M_PI,
	      zrad   = ctrlVal[1],
		  zrad2  = zrad * zrad,
		  zreal  = zrad * std::cos(ztheta);
	
    for (size_t i = 0; i < size; i += 2)
    {
        for (size_t chn = 0; chn < 4; chn++)
        {
			float yk, xk = in[chn][i];
			
			// reference: https://en.wikipedia.org/wiki/All-pass_filter#Digital_implementation
			// y[k] = 2*R(z)*(y[k-1]-x[k-1]) + (|z|^2)*(x[k]-y[k-2]) + x[k-2]
			yk = 2.f * zreal * (ykm1[chn] - xkm1[chn]) + zrad2 * (xk - ykm2[chn]) + xkm2[chn];
			
			// mix dry & wet signal
            out[chn][i] = ctrlVal[0] * yk + (1.f - ctrlVal[0]) * xkm2[chn];
			
			// set feedforward & feedback values
			ykm2[chn] = ykm1[chn];
			ykm1[chn] = yk;
			xkm2[chn] = xkm1[chn];
			xkm1[chn] = xk;
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

	while (true)
		patch.DisplayControls(false);
}