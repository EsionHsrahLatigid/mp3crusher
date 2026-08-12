#include "PluginEditor.h"
#include "PluginProcessor.h"

MP3CrusherAudioProcessorEditor::MP3CrusherAudioProcessorEditor(
    MP3CrusherAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setLookAndFeel(&ehlLookAndFeel);
  setResizable(true, true);
  setResizeLimits(ehl::juce_design::Metrics::minimumWidth,
                  ehl::juce_design::Metrics::minimumHeight,
                  ehl::juce_design::Metrics::maximumWidth,
                  ehl::juce_design::Metrics::maximumHeight);

  addAndMakeVisible(parameterDisplay);

  setupSlider(bitcrushSlider, bitcrushLabel, "BIT CRUSH");
  setupSlider(downsampleSlider, downsampleLabel, "DOWNSAMPLE");
  setupSlider(bandwidthSlider, bandwidthLabel, "BANDWIDTH");
  setupSlider(glitchSlider, glitchLabel, "GLITCH");
  setupSlider(noiseSlider, noiseLabel, "NOISE");
  setupSlider(mixSlider, mixLabel, "MIX");

  // アタッチメント
  bitcrushAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "bitcrush", bitcrushSlider);
  downsampleAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "downsample", downsampleSlider);
  bandwidthAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "bandwidth", bandwidthSlider);
  glitchAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "glitch", glitchSlider);
  noiseAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "noise", noiseSlider);
  mixAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "mix", mixSlider);

  updateDisplay();
  setSize(ehl::juce_design::Metrics::defaultWidth,
          ehl::juce_design::Metrics::defaultHeight);
  startTimerHz(15);
}

MP3CrusherAudioProcessorEditor::~MP3CrusherAudioProcessorEditor() {
  stopTimer();
  setLookAndFeel(nullptr);
}

void MP3CrusherAudioProcessorEditor::setupSlider(
    juce::Slider &slider, juce::Label &label, const juce::String &labelText) {
  ehl::juce_design::styleSlider(slider);
  addAndMakeVisible(slider);

  label.setText(labelText, juce::dontSendNotification);
  ehl::juce_design::styleLabel(label);
  label.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(label);
}

void MP3CrusherAudioProcessorEditor::paint(juce::Graphics &g) {
  ehl::juce_design::paintEditorChrome(g, getLocalBounds(), "MP3 CRUSHER",
                                      "MP3-STYLE BIT CRUSH");
}

void MP3CrusherAudioProcessorEditor::resized() {
  const auto bounds = getLocalBounds();
  parameterDisplay.setBounds(ehl::juce_design::parameterDisplayArea(bounds));

  auto placeSlider = [&](juce::Slider &slider, juce::Label &label,
                         std::size_t index) {
    ehl::juce_design::layoutLabelledControl(
        label, slider, ehl::juce_design::controlCell(bounds, index));
  };

  placeSlider(bitcrushSlider, bitcrushLabel, 0);
  placeSlider(downsampleSlider, downsampleLabel, 1);
  placeSlider(bandwidthSlider, bandwidthLabel, 2);
  placeSlider(glitchSlider, glitchLabel, 3);
  placeSlider(noiseSlider, noiseLabel, 4);
  placeSlider(mixSlider, mixLabel, 5);
}

void MP3CrusherAudioProcessorEditor::timerCallback() { updateDisplay(); }

void MP3CrusherAudioProcessorEditor::updateDisplay() {
  auto readNormalized = [this](const char *id) {
    if (auto *parameter = audioProcessor.apvts.getParameter(id))
      return parameter->getValue();
    return 0.0f;
  };

  parameterDisplay.setValues({readNormalized("bitcrush"),
                              readNormalized("downsample"),
                              readNormalized("bandwidth"),
                              readNormalized("glitch")});
}
