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
cmake --build build/plugin --target ehl_stage_products MP3CrusherIntegrationTests --parallel 2
ctest --test-dir build/plugin --output-on-failure
```

On local macOS builds outside CI, VST3 and AU products are also copied to the current user's standard plug-in folders. Override this with `-DEHL_COPY_PLUGIN_AFTER_BUILD=ON|OFF`. Standalone products are not copied to `Audio/Plug-Ins`; they remain in the build and artifact trees.

Stable artifacts are staged under `artifacts/plugin-release/<platform>/`:

- `artifacts/plugin-release/macos-arm64/vst3/mp3crusher_vst3_plugin.vst3`
- `artifacts/plugin-release/macos-arm64/au/mp3crusher_au_plugin.component` on macOS
- `artifacts/plugin-release/macos-arm64/standalone/mp3crusher_standalone_plugin.app` on macOS
- `artifacts/plugin-release/windows-x64/vst3/mp3crusher_vst3_plugin.vst3`
- `artifacts/plugin-release/windows-x64/standalone/mp3crusher_standalone_plugin.exe`
- `artifacts/plugin-release/linux-x64/vst3/mp3crusher_vst3_plugin.vst3`
- `artifacts/plugin-release/linux-x64/standalone/mp3crusher_standalone_plugin`

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
