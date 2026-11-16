# In the Weeds

Hendrix-inspired octave fuzz for Windows x64 VST3 hosts. Smoke drives the fuzz.
Octave Up blends in a full-wave-rectified octave effect. Octave Down mixes in
a pitch-shifted lower octave. Level sets the output.

Build with CMake and Visual Studio C++ desktop tools:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target WeedsFuzz_VST3
```

Copy the entire In the Weeds.vst3 folder into your VST3 directory and rescan
in REAPER. Publishing a GitHub release builds and attaches the Windows ZIP.
