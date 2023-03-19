#include "voice_data.h"
#include "m6x118pt7b.h"

ParamLockPool VoiceData::lockPool;

// incorporates the lock if any
uint8_t VoiceData::GetParamValue(ParamType param, uint8_t lastNotePlayed, uint8_t step, uint8_t pattern)
{
    uint8_t value;
    // instrument special case
    if(GetInstrumentType() == INSTRUMENT_MACRO)
    {
        switch(param)
        {
            case Timbre: return HasLockForStep(step, pattern, Timbre, value)?value:timbre;
            case Color: return HasLockForStep(step, pattern, Color, value)?value:color;
        }
    }
    if(GetInstrumentType() == INSTRUMENT_MIDI)
    {
        switch(param)
        {
            case Timbre: return HasLockForStep(step, pattern, Timbre, value)?value:timbre;
            case PlayingMidiNotes: return HasLockForStep(step, pattern, PlayingMidiNotes, value)?value:color;
        }
    }
    if(GetInstrumentType() == INSTRUMENT_SAMPLE)
    {
        switch(param)
        {
            case SampleIn: return HasLockForStep(step, pattern, SampleIn, value)?value:sampleStart[lastNotePlayed];
            case SampleOut: return HasLockForStep(step, pattern, SampleOut, value)?value:sampleLength[lastNotePlayed];
            case AttackTime: return HasLockForStep(step, pattern, AttackTime, value)?value:sampleAttackTime;
            case DecayTime: return HasLockForStep(step, pattern, DecayTime, value)?value:sampleDecayTime;
        }
    }
    switch(param)
    {
        case Cutoff: return HasLockForStep(step, pattern, Cutoff, value)?value:cutoff;
        case Resonance: return HasLockForStep(step, pattern, Resonance, value)?value:resonance;
        case Volume: return HasLockForStep(step, pattern, Volume, value)?value:volume;
        case Pan: return HasLockForStep(step, pattern, Pan, value)?value:pan;
        case Octave: return HasLockForStep(step, pattern, Octave, value)?value:octave;
        case AttackTime: return HasLockForStep(step, pattern, AttackTime, value)?value:attackTime;
        case DecayTime: return HasLockForStep(step, pattern, DecayTime, value)?value:decayTime;
        case AttackTime2: return HasLockForStep(step, pattern, AttackTime2, value)?value:attackTime2;
        case DecayTime2: return HasLockForStep(step, pattern, DecayTime2, value)?value:decayTime2;
        case Env1Target: return HasLockForStep(step, pattern, Env1Target, value)?value:env1Target;
        case Env1Depth: return HasLockForStep(step, pattern, Env1Depth, value)?value:env1Depth;
        case Env2Target: return HasLockForStep(step, pattern, Env2Target, value)?value:env2Target;
        case Env2Depth: return HasLockForStep(step, pattern, Env2Depth, value)?value:env2Depth;
        case LFORate: return HasLockForStep(step, pattern, LFORate, value)?value:lfoRate;
        case LFODepth: return HasLockForStep(step, pattern, LFODepth, value)?value:lfoDepth;
        case Lfo1Target: return HasLockForStep(step, pattern, Lfo1Target, value)?value:lfo1Target;
        case Length: return length[pattern];
        case DelaySend: return HasLockForStep(step, pattern, DelaySend, value)?value:delaySend;
        case ReverbSend: return HasLockForStep(step, pattern, ReverbSend, value)?value:reverbSend;
        case ConditionMode: return HasLockForStep(step, pattern, ConditionMode, value)?value:conditionMode;
        case ConditionData: return HasLockForStep(step, pattern, ConditionData, value)?value:conditionData;
    }
    return 0;
}

// used for setting the value in place
// currentPattern is used for alterning things that have per pattern values (pattern length)
// last n
uint8_t& VoiceData::GetParam(uint8_t param, uint8_t lastNotePlayed, uint8_t currentPattern)
{
    if(param == 28)
    {
        return delaySend;
    }
    if(param == 29)
    {
        return reverbSend;
    }
    if(GetInstrumentType() != INSTRUMENT_GLOBAL && param == 30)
    {
        return instrumentTypeBare;
    }
    
    if (GetInstrumentType() == INSTRUMENT_GLOBAL)
    {
        switch (param)
        {
            // 0
            case 0: return bpm;
            case 2: return amp_enabled;
            case 4: return chromatic;
            case 8: return syncIn;
            case 9: return syncOut;
            default:
                break;
        }
    }

    // shared instrument params
    switch(param)
    {
        case 2: return cutoff;
        case 3: return resonance;
        case 4: return volume;
        case 5: return pan;
        case 6: return octave;
        case 10: return attackTime2;
        case 11: return decayTime2;
        case 12: return lfoRate;
        case 13: return lfoDepth;
        case 16: return env1Target;
        case 17: return env1Depth;
        case 18: return env2Target;
        case 19: return env2Depth;
        case 20: return lfo1Target;
        case 21: return lfo1Delay;
        case 24: return length[currentPattern];
        case 25: return rate[currentPattern];
        case 26: return conditionMode;
        case 27: return conditionData;
    }
    if(GetInstrumentType() == INSTRUMENT_MACRO)
    {
        switch (param)
        {
            case 0: return timbre;
            case 1: return color;
            case 8: return attackTime;
            case 9: return decayTime;
            case 31: return shape;
            default:
                break;
        }
    }    
    if(GetInstrumentType() == INSTRUMENT_MIDI)
    {
        switch (param)
        {
            case 0: return timbre;
            case 1: return color;
            case 31: return midiChannel;
            default:
                break;
        }
    }    
    if(GetInstrumentType() == INSTRUMENT_SAMPLE)
    {
        switch (param)
        {
            case 0: return sampleStart[GetSampler()!=SAMPLE_PLAYER_SLICE?0:lastNotePlayed];
            case 1: return sampleLength[GetSampler()!=SAMPLE_PLAYER_SLICE?0:lastNotePlayed];
            case 8: return sampleAttackTime;
            case 9: return sampleDecayTime;
            case 31: return samplerTypeBare;
            default:
                break;
        }
    }    
    return nothing;
}

const char *rates[7] = { 
    "2x",
    "3/2x",
    "1x",
    "3/4x",
    "1/2x",
    "1/4x",
    "1/8x"
};

const char *conditionStrings[4] = { 
    "none",
    "Rnd",
    "Len",
};
const char *syncInStrings[4] = { 
    "none",
    "midi",
    "PO",
    "VL",
};

const char *envTargets[7] = { 
    "Vol",
    "Timb",
    "Col",
    "Cut",
    "Res",
    "Pit",
    "Pan"
};

// this can probably be done with some math. I'm not going to do that tonight, my brain

const uint8_t ConditionalEvery[70] = {
    1, 2, 2, 2, 1, 3, 2, 3, 3, 3, 1, 4, 2, 4, 3, 4, 4, 4,
    1, 5, 2, 5, 3, 5, 4, 5, 5, 5, 1, 6, 2, 6, 3, 6, 4, 6, 5, 6, 6, 6,
    1, 7, 2, 7, 3, 7, 4, 7, 5, 7, 6, 7, 7, 7,
    1, 8, 2, 8, 3, 8, 4, 8, 5, 8, 6, 8, 7, 8, 8, 8
};


bool VoiceData::CheckLockAndSetDisplay(uint8_t step, uint8_t pattern, uint8_t param, uint8_t value, char *paramString)
{
    uint8_t valA = 0;
    // we use the high bit here to signal if we are checking for a step or not
    // so it needs to be stripped befor asking about the specific step
    if((step & 0x80) && HasLockForStep(step&0x7f, pattern, param, valA))
    {
        sprintf(paramString, "%i", valA);
        return true;
    }
    sprintf(paramString, "%i", value);
    return false;
}

void VoiceData::GetParamsAndLocks(uint8_t param, uint8_t step, uint8_t pattern, char *strA, char *strB, uint8_t lastNotePlayed, char *pA, char *pB, bool &lockA, bool &lockB, bool showForStep)
{

    // use the high bit here to signal that we want to actually check the lock for a particular step
    if(showForStep)
        step = step | 0x80;
    
    uint8_t valA = 0, valB = 0;
    InstrumentType instrumentType = GetInstrumentType();
    ConditionModeEnum conditionModeTmp = CONDITION_MODE_NONE;
    switch(param)
    {
        case 14:
            sprintf(strA, "Dely");
            sprintf(strB, "Verb");
            lockA = CheckLockAndSetDisplay(step, pattern, DelaySend, delaySend, pA);
            lockB = CheckLockAndSetDisplay(step, pattern, ReverbSend, reverbSend, pB);
            return;
    }
    
    // all non global instruments
    switch (param)
    {
        case 12:
            sprintf(strA, "Len");
            sprintf(strB, "Rate");
            sprintf(pA, "%i", length[pattern]/4+1);
            sprintf(pB,rates[(rate[pattern]*7)>>8]);
            return;
        case 13:
            sprintf(strA, "Cnd");
            sprintf(strB, "Rate");
            if(HasLockForStep(step, pattern, 26, valA))
            {
                conditionModeTmp = GetConditionMode(valA);
                lockA = true;
            }
            else
                conditionModeTmp = GetConditionMode();
            sprintf(pA, "%s", conditionStrings[conditionModeTmp]);
            uint8_t tmp = 0;
            uint8_t conditionDataTmp = conditionData;
            if(HasLockForStep(step, pattern, 27, valB))
            {
                conditionDataTmp = valB;
                lockB = true;
            }            
            switch(conditionModeTmp)
            {
                case CONDITION_MODE_RAND:
                    sprintf(pB, "%i%", ((uint16_t)conditionDataTmp*100)>>8);
                    break;
                case CONDITION_MODE_LENGTH:
                    tmp = ((uint16_t)conditionDataTmp*35)>>8;
                    sprintf(pB, "%i:%i", ConditionalEvery[tmp*2], ConditionalEvery[tmp*2+1]);
                    break;
                default:
                    sprintf(pB, "%i", conditionDataTmp);
                    break;
            }
            return;
    }
    int p = pan;
    if(instrumentType == INSTRUMENT_MACRO || instrumentType == INSTRUMENT_SAMPLE)
    {
        switch (param)
        {
            case 1:
                sprintf(strA, "Cut");
                sprintf(strB, "Res");
                lockA = CheckLockAndSetDisplay(step, pattern, 2, cutoff, pA);
                lockB = CheckLockAndSetDisplay(step, pattern, 3, resonance, pB);
                return;
            // volume / pan
            case 2:
                sprintf(strA, "Volm");
                sprintf(strB, "Pan");
                lockA = CheckLockAndSetDisplay(step, pattern, 4, volume, pA);
                if(HasLockForStep(step, pattern, 5, valB))
                {
                    p = valB;
                    lockB = true;
                }
                if(p==0x7f)
                {
                    sprintf(pB, "Cent");
                }
                else if(p < 0x80){
                    sprintf(pB, "L:%i", (0x7f-p));
                }
                else
                {
                    sprintf(pB, "R:%i", (p-0x7f));
                }
                return;
            case 3:
                // sprintf(strA, "Octv");
                // sprintf(strB, "");
                // sprintf(pA, "%i", GetOctave());
                // sprintf(pB, "");
                return;
            case 5:
                sprintf(strA, "Atk");
                sprintf(strB, "Dcy");
                lockA = CheckLockAndSetDisplay(step, pattern, 10, attackTime2, pA);
                lockB = CheckLockAndSetDisplay(step, pattern, 11, decayTime2, pB);
                return;
            case 6:
                sprintf(strA, "Rate");
                sprintf(strB, "Dpth");
                lockA = CheckLockAndSetDisplay(step, pattern, 12, lfoRate, pA);
                lockB = CheckLockAndSetDisplay(step, pattern, 13, lfoDepth, pB);
                return;
            case 8:
                sprintf(strA, "Trgt");
                sprintf(strB, "Dpth");
                if(HasLockForStep(step, pattern, 16, valA))
                {
                    sprintf(pA, "%s", envTargets[(((uint16_t)valA)*Target_Count) >> 8]);
                    lockA = true;
                }
                else
                    sprintf(pA, "%s", envTargets[(((uint16_t)env1Target)*Target_Count)>>8]);
                
                if(HasLockForStep(step, pattern, 17, valB))
                {
                    sprintf(pB, "%i", (valB-0x80));
                    lockB = true;
                }
                else
                    sprintf(pB, "%i", (env1Depth-0x80));
                return;
            case 9:
                sprintf(strA, "Trgt");
                sprintf(strB, "Dpth");
                if(HasLockForStep(step, pattern, 18, valA))
                {
                    sprintf(pA, "%s", envTargets[(((uint16_t)valA)*Target_Count) >> 8]);
                    lockA = true;
                }
                else
                    sprintf(pA, "%s", envTargets[(((uint16_t)env2Target)*Target_Count)>>8]);
                if(HasLockForStep(step, pattern, 19, valB))
                {
                    sprintf(pB, "%i", (valB-0x80));
                    lockB = true;
                }
                else
                    sprintf(pB, "%i", (env2Depth-0x80));
                return;
            case 10:
                sprintf(strA, "Trgt");
                sprintf(strB, "");
                if(HasLockForStep(step, pattern, 20, valB))
                {
                    sprintf(pA, "%s", envTargets[(((uint16_t)valA)*Target_Count) >> 8]);
                    lockA = true;
                }
                else
                    sprintf(pA, "%s", envTargets[(((uint16_t)lfo1Target)*Target_Count)>>8]);
                sprintf(pB, "");
                return;
            case 12:
                sprintf(strA, "Len");
                sprintf(strB, "Rate");
                sprintf(pA, "%i", length[pattern]/4+1);
                sprintf(pB,rates[(rate[pattern]*7)>>8]);
                return;
        }
    }
    if(GetInstrumentType() == INSTRUMENT_SAMPLE)
    {
        switch (param)
        {
            // 0
            case 0:
                sprintf(strA, "In");
                sprintf(strB, "Len");
                if(GetSampler() == SAMPLE_PLAYER_SLICE)
                {
                    sprintf(pA, "%i", sampleStart[lastNotePlayed]);
                    sprintf(pB, "%i", sampleLength[lastNotePlayed]);
                }
                else
                {
                    sprintf(pA, "%i", sampleStart[0]);
                    sprintf(pB, "%i", sampleLength[0]);
                }
                return;
            case 4:
                sprintf(strA, "Atk");
                sprintf(strB, "Dcy");
                lockA = CheckLockAndSetDisplay(step, pattern, 8, sampleAttackTime, pA);
                lockB = CheckLockAndSetDisplay(step, pattern, 9, sampleDecayTime, pB);
                return;
            case 15:
                sprintf(strA, "Type");
                sprintf(strB, "");
                sprintf(pA, "Samp");
                switch(GetSampler())
                {
                    case 0:
                        sprintf(pB, "Slice");
                        break;
                    case 1:
                        sprintf(pB, "Pitch");
                        break;
                    default:
                        sprintf(pB, "S-Eql");
                }
                return;
            default:
                return;
        }
    }

    if(GetInstrumentType() == INSTRUMENT_MACRO)
    {
        switch (param)
        {
            // 0
            case 0:
                sprintf(strA, "Timb");
                sprintf(strB, "Colr");
                lockA = CheckLockAndSetDisplay(step, pattern, 0, timbre, pA);
                lockB = CheckLockAndSetDisplay(step, pattern, 1, color, pB);
                return;
            case 4:
                sprintf(strA, "Atk");
                sprintf(strB, "Dcy");
                lockA = CheckLockAndSetDisplay(step, pattern, 8, attackTime, pA);
                lockB = CheckLockAndSetDisplay(step, pattern, 9, decayTime, pB);
                return;
            case 15:
                sprintf(strA, "Type");
                sprintf(strB, "");
                sprintf(pA, "Synt");
                if(HasLockForStep(step, pattern, 31, valB))
                {
                    sprintf(pB, "%s", algo_values[(MacroOscillatorShape)((((uint16_t)valB)*41) >> 8)]);
                    lockB = true;
                }
                else
                    sprintf(pB, "%s", algo_values[GetShape()]);
                return;
            default:
                return;
        }
    }
    if(GetInstrumentType() == INSTRUMENT_MIDI)
    {
        switch (param)
        {
            case 0:
                sprintf(strA, "Vel");
                sprintf(strB, "NNte");
                lockA = CheckLockAndSetDisplay(step, pattern, 0, timbre, pA);
                lockB = CheckLockAndSetDisplay(step, pattern, 1, color, pB);
                return;
            // 0
            case 15:
                sprintf(strA, "Type");
                sprintf(strB, "");
                sprintf(pA, "Midi");
                sprintf(pB, "%i", (midiChannel>>4)+1);
                return;
            default:
                return;
        }
    }
}
uint8_t head_map[] = {
  0x00, 0x00, 0x00, 0x00, 
  0x00, 0x3f, 0xfc, 0x00, 
  0x01, 0xff, 0xff, 0x80, 
  0x0f, 0x00, 0x03, 0xf0, 
  0x10, 0x00, 0x00, 0x18, 
  0x20, 0xe0, 0x07, 0x0c, 
  0x61, 0xf0, 0x0f, 0x8e, 
  0x60, 0xe0, 0x07, 0x0e, 
  0x60, 0x00, 0x00, 0x0e, 
  0x60, 0x20, 0x10, 0x0e, 
  0x20, 0x30, 0x30, 0x1c, 
  0x1c, 0x1f, 0xe0, 0x38, 
  0x0f, 0x00, 0x00, 0x70, 
  0x01, 0xff, 0xff, 0xc0, 
  0x00, 0x3f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x00, 
};

void VoiceData::DrawParamString(uint8_t param, char *str, uint4 lastNotePlayed, uint8_t currentPattern, uint8_t paramLock, bool showForStep)
{
    ssd1306_t* disp = GetDisplay();
    uint8_t width = 36;
    uint8_t column4 = 128-width;
    if(param == 3 || param == 7 || param == 11)
    {
        // lol
        uint8_t x = 0;
        uint8_t y = 0;
        for(int i=0;i<64;i++)
        {
            for(int b=0;b<8;b++)
            {
                if((head_map[i]&(0x1<<(7-b))) > 0)
                {
                    ssd1306_draw_pixel(disp, x+column4, y);
                }
                else
                {
                    ssd1306_clear_pixel(disp, x+column4, y);
                }
                x++;
                if(x>=32)
                {
                    x = 0;
                    y++;
                }
            }
        }

    }
    else
    {
        bool lockA = false, lockB = false;
        GetParamsAndLocks(param, paramLock, currentPattern, str, str+16, (uint8_t)lastNotePlayed.value, str+32, str+48, lockA, lockB, showForStep);
        if(lockA)
            ssd1306_draw_square_rounded(disp, column4, 0, width, 15);
        if(lockB)
            ssd1306_draw_square_rounded(disp, column4, 17, width, 15);
        ssd1306_draw_string_gfxfont(disp, column4+3, 12, str+32, !lockA, 1, 1, &m6x118pt7b);
        ssd1306_draw_string_gfxfont(disp, column4+3, 17+12, str+48, !lockB, 1, 1, &m6x118pt7b);
        
        ssd1306_draw_string_gfxfont(disp, column4-33, 12, str, true, 1, 1, &m6x118pt7b);    
        ssd1306_draw_string_gfxfont(disp, column4-33, 17+12, str+16, true, 1, 1, &m6x118pt7b);
    }
}

void VoiceData::SerializeStatic(Serializer &s)
{
    uint8_t *lockPoolReader = (uint8_t*)&lockPool;
    for (size_t i = 0; i < sizeof(ParamLockPool); i++)
    {
        s.AddData(*lockPoolReader);
        lockPoolReader++;
    }
}

void VoiceData::DeserializeStatic(Serializer &s)
{
    uint8_t *lockPoolReader = (uint8_t*)&lockPool;
    for (size_t i = 0; i < sizeof(ParamLockPool); i++)
    {
        *lockPoolReader = s.GetNextValue();
        lockPoolReader++;
    }
}


/* PARAMETER LOCK BEHAVIOR */
void VoiceData::StoreParamLock(uint8_t param, uint8_t step, uint8_t pattern, uint8_t value)
{
    ParamLock *lock;
    if(GetLockForStep(&lock, step, pattern, param))
    {
        lock->value = value;
        // printf("updated param lock step: %i param: %i value: %i\n", step, param, value);
        return;
    }
    if(lockPool.GetFreeParamLock(&lock))
    {
        if(!lockPool.IsValidLock(lock))
        {
            printf("out of lock space\n failed to add new lock");
            return;
        }
        lock->param = param;
        lock->step = step;
        lock->value = value;
        lock->next = locksForPattern[pattern];
        locksForPattern[pattern] = lockPool.GetLockPosition(lock);
        // printf("added param lock step: %i param: %i value: %i at lock position: %i\n", step, param, value, lockPool.GetLockPosition(lock));
        return;
    }
    printf("failed to add param lock\n");
}
void VoiceData::ClearParameterLocks(uint8_t pattern)
{
    ParamLock* lock = lockPool.GetLock(locksForPattern[pattern]);
    while(lockPool.IsValidLock(lock))
    {
        ParamLock* nextLock = lockPool.GetLock(lock->next);
        lockPool.ReturnLockToPool(lock);
        lock = nextLock;
    }
    locksForPattern[pattern] = ParamLockPool::InvalidLockPosition();
}
void VoiceData::RemoveLocksForStep(uint8_t pattern, uint8_t step)
{
    ParamLock* lock = lockPool.GetLock(locksForPattern[pattern]);
    ParamLock* lastLock = lock;
    while(lockPool.IsValidLock(lock))
    {
        ParamLock* nextLock = lockPool.GetLock(lock->next);
        if(lock->step == step)
        {
            lastLock->next = lock->next;
            lockPool.ReturnLockToPool(lock);
            if(lockPool.GetLockPosition(lock) == locksForPattern[pattern])
            {
                locksForPattern[pattern] = lockPool.GetLockPosition(nextLock);
            }
        }
        else
        {
            lastLock = lock;
        }
        lock = nextLock;
    }
}
void VoiceData::CopyParameterLocks(uint8_t fromPattern, uint8_t toPattern)
{
    ParamLock* lock = lockPool.GetLock(locksForPattern[fromPattern]);
    while(lockPool.IsValidLock(lock))
    {
        StoreParamLock(lock->param, lock->step, toPattern, lock->value);
        lock = lockPool.GetLock(lock->next);
    }
}
bool VoiceData::HasLockForStep(uint8_t step, uint8_t pattern, uint8_t param, uint8_t &value)
{
    // because we use the highbit to signal if we are checking a specific step in the callsite, this must be stripped here
    step = step&0x7f;
    ParamLock *lock;
    if(GetLockForStep(&lock, step, pattern, param))
    {
        value = lock->value;
        return true;
    }
    return false;
}
bool VoiceData::HasAnyLockForStep(uint8_t step, uint8_t pattern)
{
    ParamLock* lock = lockPool.GetLock(locksForPattern[pattern]);
    while(lockPool.IsValidLock(lock))
    {
        if(lock->step == step)
        {
            return true;
        }
        lock = lockPool.GetLock(lock->next);
    }
    return false;
}
bool VoiceData::GetLockForStep(ParamLock **lockOut, uint8_t step, uint8_t pattern, uint8_t param)
{
    ParamLock* lock = lockPool.GetLock(locksForPattern[pattern]);
    while(lockPool.IsValidLock(lock))
    {
        if(lock->param == param && lock->step == step)
        {
            *lockOut = lock;
            return true;
        }
        lock = lockPool.GetLock(lock->next);
    }
    return false;
}



void TestVoiceData()
{
    VoiceData voiceData;
    voiceData.StoreParamLock(1, 1, 1, 5);
    uint8_t lockValue;
    bool hasLock = voiceData.HasLockForStep(0x80|1, 1, 1, lockValue);
    printf("%i, %i\n", hasLock, lockValue);
    voiceData.StoreParamLock(1, 1, 1, 127);
    hasLock = voiceData.HasLockForStep(0x80|1, 1, 1, lockValue);
    printf("%i, %i\n", hasLock, lockValue);

    int lostLockCount = 0;
    for(int l=0;l<16*256;l++)
    {
        ParamLock *searchingForLock = voiceData.lockPool.GetLock(l);
        bool foundLock = false;
        {
            for(int p=0;p<16;p++)
            {
                int lockCount = 0;
                ParamLock *lock = voiceData.lockPool.GetLock(voiceData.locksForPattern[p]);
                while(voiceData.lockPool.IsValidLock(lock))
                {
                    if(lock == searchingForLock){
                        foundLock = true;
                        break;
                    }
                    if(lock == voiceData.lockPool.GetLock(lock->next))
                    {
                        break;
                    }
                    lock = voiceData.lockPool.GetLock(lock->next);
                }
            }
        }
    }
    printf("lost lock count: %i\n", lostLockCount);

}