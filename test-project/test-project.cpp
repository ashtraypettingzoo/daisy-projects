#include "daisysp.h"
#include "daisy_patch.h"
#include <string>
#include <cmath>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;
Parameter parameter[4];

void ProcessControls();

static void AudioCallback(float **in, float **out, size_t size)
{
	ProcessControls();
	
    for (size_t i = 0; i < size; i += 2)
    {
        for (size_t chn = 0; chn < 4; chn++)
        {
            out[chn][i] = in[chn][i];
        }
    }
}

void ProcessControls()
{
    patch.UpdateAnalogControls();
    patch.DebounceControls();
}


int main(void)
{
    patch.Init();
	
	parameter[0].Init(patch.controls[patch.CTRL_1], 0.f, 1.f, Parameter::LINEAR);
	parameter[1].Init(patch.controls[patch.CTRL_2], 0.f, 1.f, Parameter::LINEAR);
	parameter[2].Init(patch.controls[patch.CTRL_3], 0.f, 1.f, Parameter::LINEAR);
	parameter[3].Init(patch.controls[patch.CTRL_4], 0.f, 1.f, Parameter::LINEAR);
	
    std::string str = "TEST-PROGRAM";
    patch.display.WriteString(const_cast<char*>(str.c_str()), Font_7x10, true);
    patch.display.Update();
    patch.DelayMs(2000);

    patch.StartAdc();
    patch.StartAudio(AudioCallback);
	
    while(1) 
    {
        patch.DisplayControls(false);
    }
}