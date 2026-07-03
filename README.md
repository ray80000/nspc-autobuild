# NS Parental Control - AMS 1.10.2 Compatibility Build

[![Build Status](https://github.com/YOUR_USERNAME/nspc-autobuild/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/nspc-autobuild/actions/workflows/build.yml)

This repository provides automated builds of NS Parental Control with fixes for **AMS 1.10.2 + Firmware 21.1.0** compatibility.

## 🎯 What This Fixes

- **AMS 1.10.2 compatibility**: Updated service initialization for newer AMS versions
- **Retry logic**: Added service connection retry (3 attempts)
- **Error handling**: Tesla framework now gracefully handles initialization failures
- **Offline mode**: Overlay won't crash if service connection fails

## 📥 Download

Go to [Actions](https://github.com/YOUR_USERNAME/nspc-autobuild/actions) tab and download the latest build artifacts:

| File | Description |
|------|-------------|
| `nspc-sysmodule` | `exefs.nsp` - The sysmodule |
| `nspc-overlay` | `parental_control.ovl` - The overlay GUI |

## 🚀 Manual Trigger Build

You can trigger a manual build at any time:

1. Go to [Actions](https://github.com/YOUR_USERNAME/nspc-autobuild/actions/workflows/build.yml)
2. Click "Run workflow"
3. The build will start automatically

## 📦 Installation

1. Download the latest artifacts from Actions
2. Extract `NSPC_AMS1.10.2_Fix.zip`
3. Copy to your Switch SD card

```
SD Card/
├── atmosphere/
│   └── contents/
│       └── 4200000000003103/
│           ├── exefs.nsp          ← From sysmodule artifact
│           ├── toolbox.json
│           └── flags/
│               └── boot2.flag     ← Create empty file
└── switch/
    └── .overlays/
        └── parental_control.ovl   ← From overlay artifact
```

## 🔧 Building Locally

### Prerequisites

- [devkitPro](https://devkitpro.org/wiki/Getting_Started)
- devkitA64
- libnx
- libultrahand

### Steps

```bash
# Clone with submodules
git clone --recursive https://github.com/TristanIsrael/NSParentalControl.git
cd NSParentalControl

# Apply patch
cp patches/main.cpp.patch overlay/src/
cd overlay/src
patch -p4 < main.cpp.patch

# Build sysmodule
cd ../../sysmodule
make

# Build overlay
cd ../overlay
make
```

## 📝 Patch Details

### main.cpp.patch

**Changes:**
1. Added `smInitialize()` retry logic for AMS 1.10.2 compatibility
2. Wrapped `tsl::loop` in try-catch block
3. Added service connection retry (3 attempts with 100ms delay)
4. Added offline mode when service is unavailable
5. Improved error logging for debugging

## 🐛 Known Issues

- If Tesla framework completely fails, overlay won't open but sysmodule continues to work
- Notifications require Ultrahand Overlay (Tesla doesn't support notifications)

## 📄 License

This is a compatibility patch for [NSParentalControl](https://github.com/TristanIsrael/NSParentalControl). Original project is under GPLv3.

## 🙏 Credits

- Original NSPC by [TristanIsrael](https://github.com/TristanIsrael)
- Tesla Menu by [WerWolv](https://github.com/WerWolv/Tesla-Menu)
- Ultrahand by [ppkantorski](https://github.com/ppkantorski/Ultrahand-Overlay)
- devkitPro team for the toolchain
