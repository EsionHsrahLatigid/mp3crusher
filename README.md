# MP3 Crusher

MP3 Crusher is a JUCE audio effect that combines bit-depth reduction, sample-rate reduction, bandwidth limiting, digital noise, and random stutter loops for MP3-like degradation.

## Identity

- Company: EsionHsrahLatigid
- Bundle ID: `jp.ehl.mp3crusher`
- Manufacturer code: `EHL_`
- Plug-in code: `Mp3C`
- Formats: VST3, Standalone, and AU on macOS

## Build

```bash
cmake -S . -B build/plugin -DCMAKE_BUILD_TYPE=Release -DMP3CRUSHER_BUILD_PLUGIN=ON -DMP3CRUSHER_BUILD_TESTS=ON
cmake --build build/plugin --target MP3Crusher_Artifacts MP3CrusherIntegrationTests --parallel 2
ctest --test-dir build/plugin --output-on-failure
```

Built products are staged under `artifacts/Release/`:

- `artifacts/Release/VST3/MP3 Crusher.vst3`
- `artifacts/Release/AU/MP3 Crusher.component` on macOS
- `artifacts/Release/Standalone/MP3 Crusher.app` on macOS

## Parameters

- Bit Crush: quantization bit depth
- Downsample: sample-rate reduction ratio
- Bandwidth: simulated MP3 high-frequency cutoff
- Glitch: random stutter probability
- Noise: digital noise amount
- Mix: dry/wet balance

## Codec and License Notes

This plug-in does not encode or decode MP3 data and does not link to an MP3 codec. The sound is produced by direct DSP simulation, so there is no bundled MP3 codec dependency.

The source is licensed under the MIT License. JUCE is fetched at configure time and remains subject to the JUCE license terms selected by the builder.
