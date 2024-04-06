/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FirstOrderAllPass::FirstOrderAllPass()
{
}

void FirstOrderAllPass::init(int sampleRate)
{
	m_SampleRate = sampleRate;
}

void FirstOrderAllPass::setFrequency(float frequency)
{
	if (m_SampleRate == 0)
	{
		return;
	}

	const float pi = 3.141592653589793f;

	const float tmp = tanf(pi * frequency / m_SampleRate);
	m_a1 = (tmp - 1.0f) / (tmp + 1.0f);
}

void FirstOrderAllPass::setCoef(float coef)
{
	m_a1 = coef;
}

float FirstOrderAllPass::process(float in)
{
	const float tmp = m_a1 * in + m_d;
	m_d = in - m_a1 * tmp;
	return tmp;
}

//==============================================================================
SecondOrderAllPass::SecondOrderAllPass()
{
}

void SecondOrderAllPass::init(int sampleRate)
{
	m_SampleRate = sampleRate;
}

void SecondOrderAllPass::setFrequency(float frequency, float Q)
{
	if (m_SampleRate == 0)
	{
		return;
	}

	const float pi = 3.141592653589793f;

	const float w = 2.0f * pi * frequency / m_SampleRate;
	const float cosw = cos(w);
	const float alpha = sin(w) * (2.0f * Q);

	const float a2 = 1 + alpha;

	m_a0 = (1.0f - alpha) / a2;
	m_a1 = (-2.0f * cosw) / a2;
}

float SecondOrderAllPass::process(float in)
{
	const float y0 = m_a0 * (in - m_y2) + m_a1 * (m_x1 - m_y1) + m_x2;

	m_x2 = m_x1;
	m_x1 = in;
	m_y2 = m_y1;
	m_y1 = y0;

	return y0;
}

//==============================================================================
LinkwitzRileySecondOrder::LinkwitzRileySecondOrder()
{
}

void LinkwitzRileySecondOrder::init(int sampleRate)
{
	m_SampleRate = sampleRate;
}

void LinkwitzRileySecondOrder::setFrequency(float frequency)
{
	if (m_SampleRate == 0)
	{
		return;
	}

	const float pi = 3.141592653589793f;

	const float fpi = pi * frequency;
	const float wc = 2.0f * fpi;
	const float wc2 = wc * wc;
	const float wc22 = 2.0f * wc2;
	const float k = wc / tanf(fpi / m_SampleRate);
	const float k2 = k * k;
	const float k22 = 2 * k2;
	const float wck2 = 2 * wc * k;
	const float tmpk = k2 + wc2 + wck2;
	
	m_b1 = (-k22 + wc22) / tmpk;
	m_b2 = (-wck2 + k2 + wc2) / tmpk;
	
	//---------------
	// low-pass
	//---------------
	m_a0_lp = wc2 / tmpk;
	m_a1_lp = wc22 / tmpk;
	m_a2_lp = wc2 / tmpk;
	
	//----------------
	// high-pass
	//----------------
	m_a0_hp = k2 / tmpk;
	m_a1_hp = -k22 / tmpk;
	m_a2_hp = k2 / tmpk;	
}

float LinkwitzRileySecondOrder::processLP(float in)
{
	const float y0 = m_a0_lp * in + m_x0_lp;
	m_x0_lp = m_a1_lp * in - m_b1 * y0 + m_x1_lp;
	m_x1_lp = m_a2_lp * in - m_b2 * y0;

	return y0;
}

float LinkwitzRileySecondOrder::processHP(float in)
{
	const float y0 = m_a0_hp * in + m_x0_hp;
	m_x0_hp = m_a1_hp * in - m_b1 * y0 + m_x1_hp;
	m_x1_hp = m_a2_hp * in - m_b2 * y0;

	return -y0;
}

//==============================================================================
LinkwitzRileyForthOrder::LinkwitzRileyForthOrder()
{
}

void LinkwitzRileyForthOrder::init(int sampleRate)
{
	m_SampleRate = sampleRate;
}

void LinkwitzRileyForthOrder::setFrequency(float frequency)
{
	if (m_SampleRate == 0)
	{
		return;
	}

	const double pi = 3.14285714285714f;

	const double wc = 2 * pi * frequency;
	const double wc2 = wc * wc;
	const double wc3 = wc2 * wc;
	const double wc4 = wc2 * wc2;
	const double k = wc / tan(pi * frequency / m_SampleRate);
	const double k2 = k * k;
	const double k3 = k2 * k;
	const double k4 = k2 * k2;
	const double sqrt2 = sqrt(2.0f);
	const double sq_tmp1 = sqrt2 * wc3 * k;
	const double sq_tmp2 = sqrt2 * wc * k3;
	const double a_tmp = 4.0f * wc2*k2 + 2 * sq_tmp1 + k4 + 2.0f * sq_tmp2 + wc4;

	m_b1 = (4.0f * (wc4 + sq_tmp1 - k4 - sq_tmp2)) / a_tmp;
	m_b2 = (6.0f * wc4 - 8.0f * wc2 * k2 + 6.0f * k4) / a_tmp;
	m_b3 = (4.0f * (wc4 - sq_tmp1 + sq_tmp2 - k4)) / a_tmp;
	m_b4 = (k4 - 2.0f * sq_tmp1 + wc4 - 2.0f * sq_tmp2 + 4.0f * wc2 * k2) / a_tmp;

	//================================================
	// low-pass
	//================================================
	m_a0_lp = wc4 / a_tmp;
	m_a1_lp = 4.0f * wc4 / a_tmp;
	m_a2_lp = 6.0f * wc4 / a_tmp;
	m_a3_lp = m_a1_lp;
	m_a4_lp = m_a0_lp;

	//=====================================================
	// high-pass
	//=====================================================
	m_a0_hp = k4 / a_tmp;
	m_a1_hp = -4.0f * k4 / a_tmp;
	m_a2_hp = 6.0f * k4 / a_tmp;
	m_a3_hp = m_a1_hp;
	m_a4_hp = m_a0_hp;
}

float LinkwitzRileyForthOrder::processLP(float in)
{
	const float tempx = in;

	const float tempy = m_a0_lp * tempx + m_a1_lp * m_xm1_lp + m_a2_lp * m_xm2_lp + m_a3_lp * m_xm3_lp + m_a4_lp * m_xm4_lp - m_b1 * m_ym1_lp - m_b2 * m_ym2_lp - m_b3 * m_ym3_lp - m_b4 * m_ym4_lp;
	m_xm4_lp = m_xm3_lp;
	m_xm3_lp = m_xm2_lp;
	m_xm2_lp = m_xm1_lp;
	m_xm1_lp = tempx;
	m_ym4_lp = m_ym3_lp;
	m_ym3_lp = m_ym2_lp;
	m_ym2_lp = m_ym1_lp;
	m_ym1_lp = tempy;

	return tempy;
}

float LinkwitzRileyForthOrder::processHP(float in)
{
	const float tempx = in;

	const float tempy = m_a0_hp * tempx + m_a1_hp * m_xm1_hp + m_a2_hp * m_xm2_hp + m_a3_hp * m_xm3_hp + m_a4_hp * m_xm4_hp - m_b1 * m_ym1_hp - m_b2 * m_ym2_hp - m_b3 * m_ym3_hp - m_b4 * m_ym4_hp;
	m_xm4_hp = m_xm3_hp;
	m_xm3_hp = m_xm2_hp;
	m_xm2_hp = m_xm1_hp;
	m_xm1_hp = tempx;
	m_ym4_hp = m_ym3_hp;
	m_ym3_hp = m_ym2_hp;
	m_ym2_hp = m_ym1_hp;
	m_ym1_hp = tempy;

	return tempy;
}

//==============================================================================

const std::string MultibandMSAudioProcessor::paramsNames[] = { "Low", "FreqLM", "Mid", "FreqMH", "High", "Volume" };

//==============================================================================
MultibandMSAudioProcessor::MultibandMSAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	widthLowParameter         = apvts.getRawParameterValue(paramsNames[0]);
	frequencyLowMidParameter  = apvts.getRawParameterValue(paramsNames[1]);
	widthMidParameter         = apvts.getRawParameterValue(paramsNames[2]);
	frequencyMidHighParameter = apvts.getRawParameterValue(paramsNames[3]);
	widthHighParameter        = apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter           = apvts.getRawParameterValue(paramsNames[5]);
}

MultibandMSAudioProcessor::~MultibandMSAudioProcessor()
{
}

//==============================================================================
const juce::String MultibandMSAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultibandMSAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultibandMSAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultibandMSAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultibandMSAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MultibandMSAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultibandMSAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultibandMSAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MultibandMSAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultibandMSAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MultibandMSAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	int sr = (int)(sampleRate);
	m_lowMidFilter[0].init(sr);
	m_lowMidFilter[1].init(sr);
	m_midHighFilter[0].init(sr);
	m_midHighFilter[1].init(sr);
	m_allPassFilter[0].init(sr);
	m_allPassFilter[1].init(sr);
}

void MultibandMSAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultibandMSAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MultibandMSAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	const int channels = getTotalNumOutputChannels();
	if (channels != 2)
		return;

	// Get params
	const auto widthLow = widthLowParameter->load();
	const auto frequencyLowMid = frequencyLowMidParameter->load();
	const auto widthMid = widthMidParameter->load();
	const auto frequencyMidHigh = frequencyMidHighParameter->load();
	const auto widthHigh = widthHighParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const int samples = buffer.getNumSamples();

	// Left channel
	auto* channelBufferLeft = buffer.getWritePointer(0);
	auto& lowMidFilterLeft = m_lowMidFilter[0];
	auto& midHighFilterLeft = m_midHighFilter[0];
	auto& allPassFilterLeft = m_allPassFilter[0];

	lowMidFilterLeft.setFrequency(frequencyLowMid);
	midHighFilterLeft.setFrequency(frequencyMidHigh);
	allPassFilterLeft.setFrequency(frequencyMidHigh);

	// Right channel
	auto* channelBufferRight = buffer.getWritePointer(1);
	auto& lowMidFilterRight = m_lowMidFilter[1];
	auto& midHighFilterRight = m_midHighFilter[1];
	auto& allPassFilterRight = m_allPassFilter[1];

	lowMidFilterRight.setFrequency(frequencyLowMid);
	midHighFilterRight.setFrequency(frequencyMidHigh);
	allPassFilterRight.setFrequency(frequencyMidHigh);

	for (int sample = 0; sample < samples; ++sample)
	{
		// Process left channel
		const float inLeft = channelBufferLeft[sample];

		const float lowLeft = lowMidFilterLeft.processLP(inLeft);
		const float midHighLeft = lowMidFilterLeft.processHP(inLeft);
		float midLeft = midHighFilterLeft.processLP(midHighLeft);
		float highLeft = midHighFilterLeft.processHP(midHighLeft);
		float lowLeftAllPass = allPassFilterLeft.process(lowLeft);

		// Process right channel
		const float inRight = channelBufferRight[sample];

		const float lowRight = lowMidFilterRight.processLP(inRight);
		const float midHighRight = lowMidFilterRight.processHP(inRight);
		float midRight = midHighFilterRight.processLP(midHighRight);
		float highRight = midHighFilterRight.processHP(midHighRight);
		float lowRightAllPass = allPassFilterRight.process(lowRight);

		// MS encoding
		const float midAttenuation = 1.0f - juce::Decibels::decibelsToGain(-8.0f);
		const float lowMidFactor = (widthLow < 1.0f) ? 1.0f + (1.0f - widthLow) * 0.5f : 1.0f - (midAttenuation * (widthLow - 1.0f));
		const float lowSideFactor = widthLow;
		const float midMidFactor = (widthMid < 1.0f) ? 1.0f + (1.0f - widthMid) * 0.5f : 1.0f - (midAttenuation * (widthMid - 1.0f));
		const float midSideFactor = widthMid;
		const float highMidFactor = (widthHigh < 1.0f) ? 1.0f + (1.0f - widthHigh) * 0.5f : 1.0f - (midAttenuation * (widthHigh - 1.0f));
		const float highSideFactor = widthHigh;

		const float lowMid = lowMidFactor * (lowLeftAllPass + lowRightAllPass);
		const float lowSide = lowSideFactor * (lowLeftAllPass - lowRightAllPass);
		const float midMid = midMidFactor * (midLeft + midRight);
		const float midSide = midSideFactor * (midLeft - midRight);
		const float highMid = highMidFactor * (highLeft + highRight);
		const float highSide = highSideFactor * (highLeft - highRight);

		// MS decoder
		lowLeftAllPass = lowMid + lowSide;
		lowRightAllPass = lowMid - lowSide;
		midLeft = midMid + midSide;
		midRight = midMid - midSide;
		highLeft = highMid + highSide;
		highRight = highMid - highSide;

		// Apply volume, mix and send to output
		channelBufferLeft[sample] = 0.5f * volume * (lowLeftAllPass + midLeft + highLeft);
		channelBufferRight[sample] = 0.5f * volume * (lowRightAllPass + midRight + highRight);		
	}
}

//==============================================================================
bool MultibandMSAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MultibandMSAudioProcessor::createEditor()
{
    return new MultibandMSAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void MultibandMSAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MultibandMSAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MultibandMSAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(    0.0f,    2.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   80.0f,     880,  1.0f, 0.4f),  440.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(    0.0f,    2.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( 1760.0f, 7040.0f,  1.0f, 0.4f), 3520.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(    0.0f,    2.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(  -18.0f,   18.0f,  0.1f, 1.0f),    0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultibandMSAudioProcessor();
}
