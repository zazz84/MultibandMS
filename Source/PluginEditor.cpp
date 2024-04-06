/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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

	// Canvas
	setResizable(true, true);
	const float width = 5.6f * SLIDER_WIDTH;
	setSize(width, SLIDER_WIDTH);

	if (auto* constrainer = getConstrainer())
	{
		constrainer->setFixedAspectRatio(width / (double)(SLIDER_WIDTH));
		constrainer->setSizeLimits(width * 0.7f, SLIDER_WIDTH * 0.7, width * 2.0f, SLIDER_WIDTH * 2.0f);
	}
}

MultibandMSAudioProcessorEditor::~MultibandMSAudioProcessorEditor()
{
}

//==============================================================================
void MultibandMSAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(juce::Colour::fromHSV(0.42f, 0.5f, 0.7f, 1.0f));
}

void MultibandMSAudioProcessorEditor::resized()
{
	const int width = (int)(getWidth() / 5.6f);
	const int height = getHeight();
	const int fonthHeight = (int)(height / FONT_DIVISOR);
	const int labelOffset = (int)(SLIDER_WIDTH / FONT_DIVISOR) + 5;

	// Sliders + Labels
	int xPos = 0.0f;

	for (int i = 0; i < N_SLIDERS; ++i)
	{
		juce::Rectangle<int> rectangle;

		rectangle.setPosition(xPos, 0);

		if (i == 1 || i == 3)
		{
			rectangle.setSize((int)(0.8f * width), (int)(0.8f * height));
			xPos += (int)(0.8f * width);
		}
		else
		{
			rectangle.setSize(width, height);
			xPos += width;
		}
			
		m_sliders[i].setBounds(rectangle);

		rectangle.removeFromBottom(labelOffset);
		m_labels[i].setBounds(rectangle);

		m_labels[i].setFont(juce::Font(fonthHeight, juce::Font::bold));
	}
}