#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MP3CrusherAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MP3CrusherAudioProcessorEditor(MP3CrusherAudioProcessor&);
    ~MP3CrusherAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MP3CrusherAudioProcessor& audioProcessor;

    // スライダー
    juce::Slider bitcrushSlider;
    juce::Slider downsampleSlider;
    juce::Slider bandwidthSlider;
    juce::Slider glitchSlider;
    juce::Slider noiseSlider;
    juce::Slider mixSlider;

    // ラベル
    juce::Label bitcrushLabel;
    juce::Label downsampleLabel;
    juce::Label bandwidthLabel;
    juce::Label glitchLabel;
    juce::Label noiseLabel;
    juce::Label mixLabel;
    juce::Label titleLabel;

    // アタッチメント
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bitcrushAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> downsampleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandwidthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MP3CrusherAudioProcessorEditor)
};
