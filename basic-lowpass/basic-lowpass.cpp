#include "daisysp.h"
#include "daisy_patch.h"
#include <string>
#include <cmath>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;
Parameter parameter[4];
float sigVal[4] = { 0 };

static void AudioCallback(float **in, float **out, size_t size)
{
	// get control values
	float ctrlVal[4];
	for(int i = 0; i < 4; i++)
	{
		ctrlVal[i] = patch.controls[i].Process();
	}
	
	// process audio
    for (size_t i = 0; i < size; i += 2)
    {
        for (size_t chn = 0; chn < 4; chn++)
        {
			float delta = in[chn][i] - sigVal[chn];
			sigVal[chn] += ctrlVal[chn] * delta;
            out[chn][i] = sigVal[chn];
        }
    }
}

int main(void)
{
    patch.Init();
	
    patch.display.WriteString(const_cast<char*>("BASIC-LOWPASS"), Font_7x10, true);
    patch.display.Update();
    patch.DelayMs(2000);

    patch.StartAdc();
    patch.StartAudio(AudioCallback);
	
    while(1) 
    {
        patch.DisplayControls(false);
    }
}