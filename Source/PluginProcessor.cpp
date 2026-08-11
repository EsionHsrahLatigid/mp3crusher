#include "PluginProcessor.h"
#include "PluginEditor.h"

MP3CrusherAudioProcessor::MP3CrusherAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout()) {
  glitchBufferL.resize(44100, 0.0f);
  glitchBufferR.resize(44100, 0.0f);
}

MP3CrusherAudioProcessor::~MP3CrusherAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
MP3CrusherAudioProcessor::createParameterLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  // ビットクラッシュ (ビット深度を下げる)
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"bitcrush", 1}, "Bit Crush",
      juce::NormalisableRange<float>(1.0f, 16.0f, 0.1f), 8.0f));

  // サンプルレートリダクション
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"downsample", 1}, "Downsample",
      juce::NormalisableRange<float>(1.0f, 50.0f, 0.1f), 4.0f));

  // MP3風アーティファクト (帯域制限)
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"bandwidth", 1}, "Bandwidth",
      juce::NormalisableRange<float>(1000.0f, 20000.0f, 10.0f, 0.5f),
      12000.0f));

  // グリッチ/スタッター確率
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"glitch", 1}, "Glitch",
      juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 10.0f));

  // ノイズ量
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"noise", 1}, "Noise",
      juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 5.0f));

  // ドライ/ウェットミックス
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"mix", 1}, "Mix",
      juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));

  return {params.begin(), params.end()};
}

const juce::String MP3CrusherAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool MP3CrusherAudioProcessor::acceptsMidi() const { return false; }
bool MP3CrusherAudioProcessor::producesMidi() const { return false; }
bool MP3CrusherAudioProcessor::isMidiEffect() const { return false; }
double MP3CrusherAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int MP3CrusherAudioProcessor::getNumPrograms() { return 1; }
int MP3CrusherAudioProcessor::getCurrentProgram() { return 0; }
void MP3CrusherAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}
const juce::String MP3CrusherAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}
void MP3CrusherAudioProcessor::changeProgramName(int index,
                                                 const juce::String &newName) {
  juce::ignoreUnused(index, newName);
}

void MP3CrusherAudioProcessor::prepareToPlay(double sampleRate,
                                             int samplesPerBlock) {
  juce::ignoreUnused(samplesPerBlock);
  currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

  // フィルター初期化
  auto lowCoeffs =
      juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, 12000.0f);
  auto highCoeffs =
      juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, 80.0f);

  lowpassL.coefficients = lowCoeffs;
  lowpassR.coefficients = lowCoeffs;
  highpassL.coefficients = highCoeffs;
  highpassR.coefficients = highCoeffs;

  lowpassL.reset();
  lowpassR.reset();
  highpassL.reset();
  highpassR.reset();

  // グリッチバッファリサイズ
  glitchBufferL.resize(static_cast<size_t>(sampleRate), 0.0f);
  glitchBufferR.resize(static_cast<size_t>(sampleRate), 0.0f);
}

void MP3CrusherAudioProcessor::releaseResources() {}

bool MP3CrusherAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;

  return true;
}

void MP3CrusherAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                            juce::MidiBuffer &midiMessages) {
  juce::ignoreUnused(midiMessages);
  juce::ScopedNoDenormals noDenormals;

  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // パラメータ取得
  float bitDepth = apvts.getRawParameterValue("bitcrush")->load();
  float downsampleRate = apvts.getRawParameterValue("downsample")->load();
  float bandwidth = apvts.getRawParameterValue("bandwidth")->load();
  float glitchChance = apvts.getRawParameterValue("glitch")->load() / 100.0f;
  float noiseAmount = apvts.getRawParameterValue("noise")->load() / 100.0f;
  float mix = apvts.getRawParameterValue("mix")->load() / 100.0f;

  // フィルター係数更新
  bandwidth = juce::jlimit(20.0f, static_cast<float>(currentSampleRate * 0.45), bandwidth);
  auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(
      currentSampleRate, bandwidth);
  lowpassL.coefficients = lowCoeffs;
  lowpassR.coefficients = lowCoeffs;

  auto *leftChannel = buffer.getWritePointer(0);
  auto *rightChannel = buffer.getWritePointer(1);

  for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
    float dryL = leftChannel[sample];
    float dryR = rightChannel[sample];
    float wetL = dryL;
    float wetR = dryR;

    // 1. ダウンサンプリング (サンプルレートリダクション)
    downsamplePhase += 1.0f;
    if (downsamplePhase >= downsampleRate) {
      downsamplePhase = 0.0f;
      lastSampleL = wetL;
      lastSampleR = wetR;
    }
    wetL = lastSampleL;
    wetR = lastSampleR;

    // 2. ビットクラッシュ (量子化)
    float quantizeSteps = std::pow(2.0f, bitDepth);
    wetL = std::round(wetL * quantizeSteps) / quantizeSteps;
    wetR = std::round(wetR * quantizeSteps) / quantizeSteps;

    // 3. MP3風ノイズ追加
    if (noiseAmount > 0.0f) {
      float noise = (random.nextFloat() * 2.0f - 1.0f) * noiseAmount * 0.05f;
      wetL += noise;
      wetR += noise;
    }

    // 4. バンドパスフィルター (MP3の帯域制限シミュレート)
    wetL = lowpassL.processSample(wetL);
    wetR = lowpassR.processSample(wetR);
    wetL = highpassL.processSample(wetL);
    wetR = highpassR.processSample(wetR);

    // 5. グリッチ/スタッターエフェクト
    // バッファに書き込み
    glitchBufferL[glitchWritePos] = wetL;
    glitchBufferR[glitchWritePos] = wetR;
    glitchWritePos =
        (glitchWritePos + 1) % static_cast<int>(glitchBufferL.size());

    if (!isGlitching) {
      // グリッチ開始判定
      if (random.nextFloat() < glitchChance * 0.001f) {
        isGlitching = true;
        glitchLength = random.nextInt(juce::Range<int>(100, 2000));
        glitchCounter = 0;
        glitchReadPos =
            (glitchWritePos - random.nextInt(juce::Range<int>(100, 500)) +
             static_cast<int>(glitchBufferL.size())) %
            static_cast<int>(glitchBufferL.size());
      }
    } else {
      // グリッチ中：過去のサンプルをループ再生
      wetL = glitchBufferL[glitchReadPos];
      wetR = glitchBufferR[glitchReadPos];
      glitchReadPos =
          (glitchReadPos + 1) % static_cast<int>(glitchBufferL.size());
      glitchCounter++;

      if (glitchCounter >= glitchLength) {
        isGlitching = false;
      }
    }

    // 6. ドライ/ウェットミックス
    leftChannel[sample] = dryL * (1.0f - mix) + wetL * mix;
    rightChannel[sample] = dryR * (1.0f - mix) + wetR * mix;

    // クリッピング防止
    leftChannel[sample] = juce::jlimit(-1.0f, 1.0f, leftChannel[sample]);
    rightChannel[sample] = juce::jlimit(-1.0f, 1.0f, rightChannel[sample]);
  }
}

bool MP3CrusherAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor *MP3CrusherAudioProcessor::createEditor() {
  return new MP3CrusherAudioProcessorEditor(*this);
}

void MP3CrusherAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void MP3CrusherAudioProcessor::setStateInformation(const void *data,
                                                   int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new MP3CrusherAudioProcessor();
}
