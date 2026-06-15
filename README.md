# Groove Station

A professional-grade sampler groove station built with the [JUCE](https://juce.com/) audio framework. Features a 16-pad MPC-style sampler, step sequencer, per-pad effects, and a polished dark-theme UI — all in a standalone app or VST3/AU plugin.

![Groove Station](docs/screenshot-placeholder.png)

## Features

### Audio Engine
- **16-pad sampler** with 4-voice polyphony per pad
- **Per-pad controls**: Volume, Pan, Pitch (±24 semitones), Reverse, Start/End markers
- **ADSR envelope** per pad with customizable Attack, Decay, Sustain, Release
- **Sample format support**: WAV, AIFF, MP3, FLAC, OGG
- **Drag-and-drop** sample loading onto pads

### Effects (Per-Pad & Master Bus)
- **Filter**: Low-pass, High-pass, Band-pass with cutoff & resonance
- **Reverb**: Room size, damping, wet/dry mix
- **Delay**: Time, feedback, wet/dry mix
- **Distortion**: Drive amount with waveshaper, wet/dry mix
- **3-Band EQ**: Low shelf (200 Hz), Mid peak (1 kHz), High shelf (5 kHz)

### Step Sequencer
- **Up to 64 steps** per pattern
- **16 patterns** with copy/paste support
- **Per-step velocity** and **probability** controls
- **Swing** control for groove feel
- **Quantize options**: 1/4, 1/8, 1/16, 1/32 notes
- **Visual step indicator** with beat markers

### Transport & Timing
- **BPM**: 30–300 with 0.1 resolution
- **Time signatures**: 4/4, 3/4, 6/8, 7/8
- **Play / Pause / Stop** with position display (Bar.Beat.Tick)

### UI
- **Dark theme** with custom look-and-feel (neon accents on dark surfaces)
- **Waveform display** with draggable start/end markers
- **16-pad grid** (MPC layout — bottom-to-top) with mute/solo indicators
- **Mixer panel** with rotary knobs for all per-pad parameters
- **Effects panel** with PAD FX / MASTER FX tabs
- **MIDI keyboard** widget for direct note input
- **Resizable window** (1280×800 default, 900×600 minimum)

### State Management
- Full **state save/recall** (samples, patterns, parameters) via XML serialization
- Plugin state compatible with DAW session save/load

### MIDI
- **MIDI input** support — pads mapped from C2 (MIDI note 36) upward
- On-screen MIDI keyboard for testing

## Building

### Prerequisites
- **CMake** 3.22+
- **C++17** compiler (GCC 9+, Clang 10+, MSVC 2019+)
- **JUCE 8** (included as sibling directory or submodule)

#### Linux Dependencies
```bash
sudo apt-get install -y build-essential cmake \
  libasound2-dev libfreetype6-dev libx11-dev libxrandr-dev \
  libxinerama-dev libxcursor-dev mesa-common-dev \
  libwebkit2gtk-4.0-dev libcurl4-openssl-dev
```

#### macOS
Xcode command line tools are sufficient. JUCE will use CoreAudio and CoreMIDI.

### Build Steps
```bash
# Clone JUCE alongside this project (or adjust CMakeLists.txt path)
git clone --branch 8.0.6 https://github.com/juce-framework/JUCE.git

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release -j$(nproc)
```

### Run
```bash
# Standalone
./build/GrooveStation_artefacts/Release/Standalone/GrooveStation

# VST3 plugin will be in:
# ./build/GrooveStation_artefacts/Release/VST3/
```

## Architecture

```
Source/
├── PluginProcessor.cpp/h     # Main audio processor (entry point)
├── PluginEditor.cpp/h        # Main UI editor (layout + wiring)
├── Audio/
│   ├── SamplerEngine.cpp/h   # 16-pad sampler manager
│   ├── SampleVoice.cpp/h     # Individual voice (playback + interpolation)
│   ├── ADSREnvelope.cpp/h    # Custom ADSR with per-sample resolution
│   └── EffectsChain.cpp/h    # Filter, reverb, delay, distortion, EQ
├── Sequencer/
│   ├── StepSequencer.cpp/h   # Hi-res timer sequencer with swing
│   └── Pattern.cpp/h         # Step data (active, velocity, probability)
└── UI/
    ├── CustomLookAndFeel.cpp/h  # Dark theme + custom knob/slider/button rendering
    ├── PadGrid.cpp/h            # 4×4 MPC pad layout with drag-and-drop
    ├── WaveformDisplay.cpp/h    # Waveform view with start/end markers
    ├── TransportBar.cpp/h       # Play/Stop/Rec + BPM/Swing/Quantize
    ├── MixerPanel.cpp/h         # Per-pad Vol/Pan/Pitch/ADSR + Mute/Solo
    ├── EffectsPanel.cpp/h       # Per-pad & master effects controls
    └── SequencerGrid.cpp/h      # Step grid with pattern selector
```

## License

This project uses the JUCE framework under the [JUCE License](https://juce.com/legal/juce-8-licence/).
