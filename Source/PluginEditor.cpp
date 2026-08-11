#include "PluginEditor.h"
#include "PluginProcessor.h"

MP3CrusherAudioProcessorEditor::MP3CrusherAudioProcessorEditor(
    MP3CrusherAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(400, 500);

  // タイトル
  titleLabel.setText("MP3 CRUSHER", juce::dontSendNotification);
  titleLabel.setFont(juce::Font(juce::FontOptions(28.0f).withStyle("Bold")));
  titleLabel.setJustificationType(juce::Justification::centred);
  titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ff88));
  addAndMakeVisible(titleLabel);

  // スライダーセットアップ
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
}

MP3CrusherAudioProcessorEditor::~MP3CrusherAudioProcessorEditor() {}

void MP3CrusherAudioProcessorEditor::setupSlider(
    juce::Slider &slider, juce::Label &label, const juce::String &labelText) {
  slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
  slider.setColour(juce::Slider::rotarySliderFillColourId,
                   juce::Colour(0xff00ff88));
  slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffff0088));
  addAndMakeVisible(slider);

  label.setText(labelText, juce::dontSendNotification);
  label.setFont(juce::Font(juce::FontOptions(12.0f)));
  label.setJustificationType(juce::Justification::centred);
  label.setColour(juce::Label::textColourId, juce::Colours::white);
  addAndMakeVisible(label);
}

void MP3CrusherAudioProcessorEditor::paint(juce::Graphics &g) {
  // グラデーション背景
  juce::ColourGradient gradient(juce::Colour(0xff1a1a2e), 0, 0,
                                juce::Colour(0xff16213e), 0, (float)getHeight(),
                                false);
  g.setGradientFill(gradient);
  g.fillAll();

  // デコレーションライン
  g.setColour(juce::Colour(0xff00ff88).withAlpha(0.3f));
  g.drawLine(20, 60, getWidth() - 20, 60, 2.0f);
  g.drawLine(20, getHeight() - 20, getWidth() - 20, getHeight() - 20, 2.0f);
}

void MP3CrusherAudioProcessorEditor::resized() {
  auto bounds = getLocalBounds();

  // タイトル
  titleLabel.setBounds(bounds.removeFromTop(60));

  // スライダー配置 (2列3行)
  auto sliderArea = bounds.reduced(20);
  int sliderWidth = sliderArea.getWidth() / 2;
  int sliderHeight = sliderArea.getHeight() / 3;
  int knobSize = 80;
  int labelHeight = 20;

  auto placeSlider = [&](juce::Slider &slider, juce::Label &label, int col,
                         int row) {
    int x =
        sliderArea.getX() + col * sliderWidth + (sliderWidth - knobSize) / 2;
    int y = sliderArea.getY() + row * sliderHeight;
    label.setBounds(x, y, knobSize, labelHeight);
    slider.setBounds(x, y + labelHeight, knobSize, knobSize + 20);
  };

  placeSlider(bitcrushSlider, bitcrushLabel, 0, 0);
  placeSlider(downsampleSlider, downsampleLabel, 1, 0);
  placeSlider(bandwidthSlider, bandwidthLabel, 0, 1);
  placeSlider(glitchSlider, glitchLabel, 1, 1);
  placeSlider(noiseSlider, noiseLabel, 0, 2);
  placeSlider(mixSlider, mixLabel, 1, 2);
}
