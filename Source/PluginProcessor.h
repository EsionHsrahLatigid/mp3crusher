#pragma once

#include <JuceHeader.h>

class MP3CrusherAudioProcessor : public juce::AudioProcessor
{
public:
    MP3CrusherAudioProcessor();
    ~MP3CrusherAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateFilterCoefficients(float bandwidth);

    // MP3風の劣化エフェクト用
    double currentSampleRate = 44100.0;
    float currentBandwidth = -1.0f;
    float downsamplePhase = 0.0f;
    float lastSampleL = 0.0f;
    float lastSampleR = 0.0f;
    
    // バンドパスフィルター (MP3の帯域制限をシミュレート)
    juce::dsp::IIR::Filter<float> lowpassL, lowpassR;
    juce::dsp::IIR::Filter<float> highpassL, highpassR;
    
    // 量子化ノイズ用
    juce::Random random;
    
    // スタッター/グリッチ用バッファ
    std::vector<float> glitchBufferL, glitchBufferR;
    int glitchWritePos = 0;
    int glitchReadPos = 0;
    bool isGlitching = false;
    int glitchCounter = 0;
    int glitchLength = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MP3CrusherAudioProcessor)
};
