



/*
 * This source code is public domain.
 *
 * Authors: Kenton Varda <temporal@gauge3d.org> (C interface wrapper)
 */

#include "modplug.h"
#include "stdafx.h"
#include "sndfile.h"

struct _ModPlugFile
{
	CSoundFile mSoundFile;
};

namespace ModPlug
{
	ModPlug_Settings gSettings =
	{
		MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION,

		2,
		16,
		44100,
		MODPLUG_RESAMPLE_LINEAR,

		0,
		0,
		0,
		0,
		0,
		0,
		0
	};

	int gSampleSize;

	void UpdateSettings(bool updateBasicConfig)
	{
		if(gSettings.mFlags & MODPLUG_ENABLE_REVERB)
		{
			CSoundFile::SetReverbParameters(gSettings.mReverbDepth,
			                                gSettings.mReverbDelay);
		}

		if(gSettings.mFlags & MODPLUG_ENABLE_MEGABASS)
		{
			CSoundFile::SetXBassParameters(gSettings.mBassAmount,
			                               gSettings.mBassRange);
		}
		else // modplug seems to ignore the SetWaveConfigEx() setting for bass boost
			CSoundFile::SetXBassParameters(0, 0);

		if(gSettings.mFlags & MODPLUG_ENABLE_SURROUND)
		{
			CSoundFile::SetSurroundParameters(gSettings.mSurroundDepth,
			                                  gSettings.mSurroundDelay);
		}

		if(updateBasicConfig)
		{
			CSoundFile::SetWaveConfig(gSettings.mFrequency,
			                          gSettings.mBits,
			                          gSettings.mChannels);

			gSampleSize = gSettings.mBits / 8 * gSettings.mChannels;
		}

		CSoundFile::SetWaveConfigEx(gSettings.mFlags & MODPLUG_ENABLE_SURROUND,
		                            !(gSettings.mFlags & MODPLUG_ENABLE_OVERSAMPLING),
		                            gSettings.mFlags & MODPLUG_ENABLE_REVERB,
		                            true,
		                            gSettings.mFlags & MODPLUG_ENABLE_MEGABASS,
		                            gSettings.mFlags & MODPLUG_ENABLE_NOISE_REDUCTION,
		                            false);
		CSoundFile::SetResamplingMode(gSettings.mResamplingMode);
	}
}

ModPlugFile* ModPlug_Load(const void* data, int size)
{
	ModPlugFile* result = new ModPlugFile;
	ModPlug::UpdateSettings(true);
	if(result->mSoundFile.Create((const BYTE*)data, size))
	{
		result->mSoundFile.SetRepeatCount(ModPlug::gSettings.mLoopCount);
		return result;
	}
	else
	{
		delete result;
		return NULL;
	}
}

void ModPlug_Unload(ModPlugFile* file)
{
	file->mSoundFile.Destroy();
	delete file;
}

int ModPlug_Read(ModPlugFile* file, void* buffer, int size)
{
	return file->mSoundFile.Read(buffer, size) * ModPlug::gSampleSize;
}

const char* ModPlug_GetName(ModPlugFile* file)
{
	return file->mSoundFile.GetTitle();
}

int ModPlug_GetLength(ModPlugFile* file)
{
	return file->mSoundFile.GetSongTime() * 1000;
}

void ModPlug_Seek(ModPlugFile* file, int millisecond)
{
	int maxpos;
	int maxtime = file->mSoundFile.GetSongTime() * 1000;
	float postime;

	if(millisecond > maxtime)
		millisecond = maxtime;
	maxpos = file->mSoundFile.GetMaxPosition();
	postime = (float)maxpos / (float)maxtime;

	file->mSoundFile.SetCurrentPos((int)(millisecond * postime));
}

void ModPlug_SetRowCallback(ModPlugFile* file, row_callback_f cb, void* closure)
{
    file->mSoundFile.SetRowCallback(cb, closure);
}

void ModPlug_SetEndOfModuleCallback(ModPlugFile* file, eom_callback_f cb, void* closure)
{
    file->mSoundFile.SetEndOfModuleCallback(cb, closure);
}

void ModPlug_SetRepeatCount(ModPlugFile* file, int count)
{
    file->mSoundFile.SetRepeatCount(count);
}

/*
 * get details about the current speed parameters
 */
void ModPlug_GetMusicSpeed(ModPlugFile* file, 
			   double* tick_duration, 
			   unsigned int* ticks_per_row)
{
    if(tick_duration) {
	*tick_duration = 
	    5000.0 * file->mSoundFile.m_nTempoFactor / 
	    (file->mSoundFile.m_nMusicTempo << 8);
    }
    if(ticks_per_row) {
	*ticks_per_row = file->mSoundFile.GetMusicSpeed();
    }
}

void ModPlug_GetSettings(ModPlug_Settings* settings)
{
	memcpy(settings, &ModPlug::gSettings, sizeof(ModPlug_Settings));
}

void ModPlug_SetSettings(const ModPlug_Settings* settings)
{
	memcpy(&ModPlug::gSettings, settings, sizeof(ModPlug_Settings));
	ModPlug::UpdateSettings(false);
}
