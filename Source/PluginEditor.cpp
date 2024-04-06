/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const juce::Colour ZazzLookAndFeel::lightColour = juce::Colour::fromHSV(0.48f, 0.5f, 0.7f, 1.0f);
const juce::Colour ZazzLookAndFeel::mediumColour = juce::Colour::fromHSV(0.48f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::darkColour = juce::Colour::fromHSV(0.48f, 0.5f, 0.4f, 1.0f);

//==============================================================================
MultibandMSAudioProcessorEditor::MultibandMSAudioProcessorEditor (MultibandMSAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	const int fonthHeight = (int)(SLIDER_WIDTH / FONT_DIVISOR);

	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];

		//Lable
		label.setText(MultibandMSAudioProcessor::paramsNames[i], juce::dontSendNotification);
		label.setFont(juce::Font(fonthHeight, juce::Font::bold));
		label.setJustificationType(juce::Justification::centred);
		addAndMakeVisible(label);

		//Slider
		slider.setLookAndFeel(&zazzLookAndFeel);
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, SLIDER_FONT_SIZE);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, MultibandMSAudioProcessor::paramsNames[i], slider));
	}

	//Plugin and developer name
	m_pluginName.setText("Multiband MS", juce::dontSendNotification);
	m_pluginName.setFont(juce::Font(fonthHeight, juce::Font::bold));
	m_pluginName.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(m_pluginName);

	m_developerName.setText("zazz", juce::dontSendNotification);
	m_developerName.setFont(juce::Font(fonthHeight, juce::Font::bold));
	m_developerName.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(m_developerName);

	// Canvas
	setResizable(true, true);
	const float width = 5.6f * SLIDER_WIDTH;
	const float height = SLIDER_WIDTH + 2 * LOGO_HEIGHT;
	setSize(width, height);

	if (auto* constrainer = getConstrainer())
	{
		const float ratio = width / height;
		constrainer->setFixedAspectRatio(ratio);
		constrainer->setSizeLimits((int)(width * 0.7f), (int)(height * 0.7f), (int)(width * 2.0f), (int)(height * 2.0f));
	}
}

MultibandMSAudioProcessorEditor::~MultibandMSAudioProcessorEditor()
{
}

//==============================================================================
void MultibandMSAudioProcessorEditor::paint (juce::Graphics& g)
{
	// Background
	g.fillAll(ZazzLookAndFeel::lightColour);

	// Lines
	g.setColour(ZazzLookAndFeel::mediumColour);
	const int width = getWidth();
	const int height = getHeight();
	const int widthSlider = getSliderWidth();
	const int heightSlider = (int)(height * ((float)SLIDER_WIDTH / (float)(SLIDER_WIDTH + 2 * LOGO_HEIGHT)));
	const int heightLogo = (int)(height * ((float)LOGO_HEIGHT / (float)(SLIDER_WIDTH + 2 * LOGO_HEIGHT)));
	const int removePixels = (int)(heightLogo * 0.48f);

	juce::Rectangle<int> rectangle;

	// Top banner
	rectangle.setSize(width, heightLogo);
	rectangle.removeFromLeft(removePixels);
	rectangle.removeFromRight(removePixels);
	rectangle.removeFromTop(removePixels);
	rectangle.removeFromBottom(removePixels);

	g.fillRect(rectangle);

	// Bottom banner
	rectangle.setSize(width, heightLogo);
	rectangle.setPosition(0, heightLogo + heightSlider);
	rectangle.removeFromLeft(removePixels);
	rectangle.removeFromRight(removePixels);
	rectangle.removeFromTop(removePixels);
	rectangle.removeFromBottom(removePixels);

	g.fillRect(rectangle);

	g.setColour(ZazzLookAndFeel::lightColour);

	// Plugin name background
	rectangle.setSize(widthSlider, heightLogo);
	rectangle.setPosition((int)(0.5f * widthSlider), 0);
	const float removeRatio1 = 0.15f;
	rectangle.removeFromLeft((int)(removeRatio1 * widthSlider));
	rectangle.removeFromRight((int)(removeRatio1 * widthSlider));

	g.fillRect(rectangle);

	// Developer name background
	rectangle.setSize(widthSlider, heightLogo);
	rectangle.setPosition((int)(width - 1.5f * widthSlider), heightLogo + heightSlider);
	const float removeRatio2 = 0.35f;
	rectangle.removeFromLeft((int)(removeRatio2 * widthSlider));
	rectangle.removeFromRight((int)(removeRatio2 * widthSlider));

	g.fillRect(rectangle);
}

void MultibandMSAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int widthSlider = getSliderWidth();
	const int height = getHeight();
	const int sliderPositionY = (int)(height * ((float)LOGO_HEIGHT / (float)(SLIDER_WIDTH + 2 * LOGO_HEIGHT)));
	const float fonthHeight = (float)height / (float)FONT_DIVISOR;
	const int labelOffset = (int)(SLIDER_WIDTH / FONT_DIVISOR) + 5;

	juce::Rectangle<int> rectangle;

	// Sliders + Labels
	int xPos = 0.0f;

	for (int i = 0; i < N_SLIDERS; ++i)
	{
		rectangle.setPosition(xPos, sliderPositionY);

		if (i == 1 || i == 3)
		{
			rectangle.setSize((int)(0.8f * widthSlider), (int)(0.8f * widthSlider));
			xPos += (int)(0.8f * widthSlider);
		}
		else
		{
			rectangle.setSize(widthSlider, widthSlider);
			xPos += widthSlider;
		}
			
		m_sliders[i].setBounds(rectangle);

		rectangle.removeFromBottom(labelOffset);
		m_labels[i].setBounds(rectangle);

		m_labels[i].setFont(juce::Font(fonthHeight, juce::Font::bold));
	}

	//Plugin and developer name
	const int labelHeight = (int)(0.5f * (height - widthSlider));
	const float logoFonthHeight = fonthHeight * 0.85f;

	rectangle.setPosition((int)(0.5f * widthSlider), 0);
	rectangle.setSize(widthSlider, labelHeight);
	m_pluginName.setBounds(rectangle);
	m_pluginName.setColour(juce::Label::textColourId, ZazzLookAndFeel::mediumColour);
	m_pluginName.setFont(juce::Font(logoFonthHeight, juce::Font::bold));

	rectangle.setPosition((int)(width - 1.5f * widthSlider), (int)(0.95f * labelHeight + widthSlider));
	rectangle.setSize(widthSlider, labelHeight);
	m_developerName.setBounds(rectangle);
	m_developerName.setColour(juce::Label::textColourId, ZazzLookAndFeel::mediumColour);
	m_developerName.setFont(juce::Font(logoFonthHeight, juce::Font::bold));
}