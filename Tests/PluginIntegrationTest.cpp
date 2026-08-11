#include "PluginProcessor.h"

#include <juce_events/juce_events.h>
#include <cmath>
#include <iostream>

namespace
{
bool check(bool condition, const char* message)
{
    if (!condition)
        std::cerr << "[FAIL] " << message << '\n';
    return condition;
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI initialiseJuce;
    MP3CrusherAudioProcessor processor;
    bool passed = true;

    passed &= check(processor.getName() == "MP3 Crusher", "product name should be MP3 Crusher");
    passed &= check(!processor.acceptsMidi(), "processor should not accept MIDI");
    passed &= check(!processor.producesMidi(), "processor should not produce MIDI");
    passed &= check(!processor.isMidiEffect(), "processor should be an audio effect");

    juce::AudioProcessor::BusesLayout stereo;
    stereo.inputBuses.add(juce::AudioChannelSet::stereo());
    stereo.outputBuses.add(juce::AudioChannelSet::stereo());
    passed &= check(processor.isBusesLayoutSupported(stereo), "stereo input/output should be supported");

    auto* mix = processor.apvts.getParameter("mix");
    passed &= check(mix != nullptr, "Mix parameter should exist");
    if (mix != nullptr)
    {
        mix->setValueNotifyingHost(mix->convertTo0to1(25.0f));
        juce::MemoryBlock state;
        processor.getStateInformation(state);
        mix->setValueNotifyingHost(mix->convertTo0to1(75.0f));
        processor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        passed &= check(std::abs(processor.apvts.getRawParameterValue("mix")->load() - 25.0f) < 0.001f,
                        "APVTS state should round-trip");
    }

    constexpr double sampleRate = 44100.0;
    processor.prepareToPlay(sampleRate, 1024);
    int generatedSamples = 0;
    const int blockSizes[] { 32, 64, 128, 256, 512, 1024 };

    for (const auto blockSize : blockSizes)
    {
        juce::AudioBuffer<float> audio(2, blockSize);
        for (int sample = 0; sample < blockSize; ++sample)
        {
            const auto value = static_cast<float>(0.2 * std::sin(2.0 * juce::MathConstants<double>::pi
                                                                 * 330.0 * generatedSamples / sampleRate));
            audio.setSample(0, sample, value);
            audio.setSample(1, sample, value);
            ++generatedSamples;
        }

        juce::MidiBuffer midi;
        processor.processBlock(audio, midi);

        for (int channel = 0; channel < audio.getNumChannels(); ++channel)
            for (int sample = 0; sample < audio.getNumSamples(); ++sample)
            {
                const auto value = audio.getSample(channel, sample);
                passed &= check(std::isfinite(value), "processed audio should remain finite");
                passed &= check(value >= -1.0f && value <= 1.0f, "processed audio should remain clipped");
            }
    }

    if (passed)
        std::cout << "MP3 Crusher plug-in integration checks passed\n";
    return passed ? 0 : 1;
}
