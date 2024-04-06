/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//==============================================================================
class FirstOrderAllPass
{
public:
	FirstOrderAllPass();

	void init(int sampleRate);
	void setFrequency(float frequency);
	void setCoef(float coef);
	float process(float in);

protected:
	float m_SampleRate;
	float m_a1 = -1.0f; // all pass filter coeficient
	float m_d = 0.0f;   // history d = x[n-1] - a1y[n-1]
};

//==============================================================================
class SecondOrderAllPass
{
public:
	SecondOrderAllPass();

	void init(int sampleRate);
	void setFrequency(float frequency, float Q);
	float process(float in);

protected:
	float m_SampleRate;

	float m_x2 = 0.0f;
	float m_x1 = 0.0f;
	float m_y2 = 0.0f;
	float m_y1 = 0.0f;

	float m_a0 = 0.0f;
	float m_a1 = 0.0f;
};

//==============================================================================
class LinkwitzRileySecondOrder
{
public:
	LinkwitzRileySecondOrder();

	void init(int sampleRate);
	void setFrequency(float frequency);
	float processLP(float in);
	float processHP(float in);

protected:
	float m_SampleRate;
	
	float m_b1 = 0.0f;
	float m_b2 = 0.0f;

	float m_a0_lp = 0.0f;
	float m_a1_lp = 0.0f;
	float m_a2_lp = 0.0f;

	float m_a0_hp = 0.0f;
	float m_a1_hp = 0.0f;
	float m_a2_hp = 0.0f;

	float m_x1_lp = 0.0f;
	float m_x0_lp = 0.0f;

	float m_x1_hp = 0.0f;
	float m_x0_hp = 0.0f;
};

//==============================================================================
class LinkwitzRileyForthOrder
{
public:
	LinkwitzRileyForthOrder();

	void init(int sampleRate);
	void setFrequency(float frequency);
	float processLP(float in);
	float processHP(float in);

protected:
	float m_SampleRate;

	float m_b1 = 0.0f;
	float m_b2 = 0.0f;
	float m_b3 = 0.0f;
	float m_b4 = 0.0f;

	float m_a0_lp = 0.0f;
	float m_a1_lp = 0.0f;
	float m_a2_lp = 0.0f;
	float m_a3_lp = 0.0f;
	float m_a4_lp = 0.0f;

	float m_a0_hp = 0.0f;
	float m_a1_hp = 0.0f;
	float m_a2_hp = 0.0f;
	float m_a3_hp = 0.0f;
	float m_a4_hp = 0.0f;

	float m_xm4_lp = 0.0f;
	float m_xm3_lp = 0.0f;
	float m_xm2_lp = 0.0f;
	float m_xm1_lp = 0.0f;
	float m_ym4_lp = 0.0f;
	float m_ym3_lp = 0.0f;
	float m_ym2_lp = 0.0f;
	float m_ym1_lp = 0.0f;

	float m_xm4_hp = 0.0f;
	float m_xm3_hp = 0.0f;
	float m_xm2_hp = 0.0f;
	float m_xm1_hp = 0.0f;
	float m_ym4_hp = 0.0f;
	float m_ym3_hp = 0.0f;
	float m_ym2_hp = 0.0f;
	float m_ym1_hp = 0.0f;
};

//==============================================================================
class MultibandMSAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    MultibandMSAudioProcessor();
    ~MultibandMSAudioProcessor() override;

	static const int N_ALL_PASS_FO = 100;
	static const int N_ALL_PASS_SO = 50;
	static const int FREQUENCY_MIN = 20;
	static const int FREQUENCY_MAX = 20000;
	static const std::string paramsNames[];

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	inline float FrequencyToMel(float frequency)
	{
		return 2595.0f * log10f(1.0f + frequency / 700.0f);
	}
	inline float MelToFrequency(float mel)
	{
		return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f);
	}
	inline float NoteToFrequency(float note)
	{
		const float f0 = 440.0f;
		const float a = powf(2, 1.0f / 12.0f);

		const float fn = f0 * powf(a, note);
		
		return fn;
	}

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================

	std::atomic<float>* widthLowParameter = nullptr;
	std::atomic<float>* frequencyLowMidParameter = nullptr;
	std::atomic<float>* widthMidParameter = nullptr;
	std::atomic<float>* frequencyMidHighParameter = nullptr;
	std::atomic<float>* widthHighParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	LinkwitzRileySecondOrder m_lowMidFilter[2] = {};
	LinkwitzRileySecondOrder m_midHighFilter[2] = {};
	FirstOrderAllPass m_allPassFilter[2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultibandMSAudioProcessor)
};
