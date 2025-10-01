# ⚡ Quick Start - Steal The Mojo

**Get up and running in 60 seconds**

---

## 🚀 Build & Run

```bash
# Navigate to build directory
cd /Volumes/MP\ 1/MoreMojo/ChatGPT/ProGUI/Tools/JuceRunner/build

# Build (30 seconds)
cmake --build . --config Debug

# Run
open "ProGUI JUCE Runner.app"
```

---

## 🎨 Test Steal The Mojo

1. **Open window**: Click "Steal The Mojo" (top-right)
2. **Upload file**: Click "UPLOAD AUDIO FILE" → select WAV/AIFF
3. **Choose instrument**: Select from dropdown (Vocal, Drums, etc.)
4. **Analyze**: Click "🔮 ANALYZE MOJO"
5. **Wait**: ~1 second (DSP) or ~60s (AI if installed)
6. **Verify**: Radar chart fills, meters animate ✅

---

## ✅ Success Indicators

You'll see:
- 🎨 Rainbow gradient border around window
- 📁 Filename appears after upload: "🎵 yourfile.wav"
- 🔮 Status updates: "Extracting... 50%"
- 📊 Radar chart fills with 8 dimensions
- 📈 5 character meters animate with colors
- ✨ Final status: "Stem extracted in X.Xs using DSP"

---

## 🐛 Quick Fixes

**Build fails?**
```bash
rm -rf /Volumes/MP\ 1/MoreMojo/ChatGPT/ProGUI/Tools/JuceRunner/build/*
cd /Volumes/MP\ 1/MoreMojo/ChatGPT/ProGUI/Tools/JuceRunner/build
cmake ..
cmake --build .
```

**App crashes?**
- Check audio file is valid WAV/AIFF
- Try different audio file
- Check debug console for errors

**No progress updates?**
- Normal! DSP is very fast (~1s)
- For AI: Install Python deps first

---

## 📚 Full Documentation

- `BUILD_AND_TEST.md` - Detailed testing
- `INTEGRATION_COMPLETE.md` - Technical details
- `MORNING_INTEGRATION_SUMMARY.md` - Complete overview

---

## 🎯 Expected Results

### DSP Method (Default)
- **Time**: ~1 second
- **Quality**: 60-70% separation
- **Status**: "using DSP (Enhanced)"
- **Requires**: Nothing (always works)

### AI Method (Optional)
- **Time**: ~60 seconds for 3-min song
- **Quality**: 90%+ separation  
- **Status**: "using AI (Demucs)"
- **Requires**: `pip3 install -r scripts/requirements.txt`

---

**That's it! You're ready to steal some mojo! 🎵✨**

*For issues, check BUILD_AND_TEST.md or INTEGRATION_COMPLETE.md*
