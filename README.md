# 🎵 Desktop Audio to Virtual Microphone Router

A C++ console application that captures desktop audio (e.g., music from YouTube or Spotify) and routes it to a virtual microphone, enabling applications like Roblox or Discord to transmit your desktop audio as microphone input. Built using the **Windows Core Audio API (WASAPI)** and **VB-Audio Virtual Cable**, this tool is perfect for sharing audio in voice chats or streaming scenarios.

![GitHub](https://img.shields.io/github/license/FlamesIsCool/Desktop-Audio-to-Virtual-Microphone-Router?style=flat-square)
![GitHub last commit](https://img.shields.io/github/last-commit/FlamesIsCool/Desktop-Audio-to-Virtual-Microphone-Router?style=flat-square)
![Windows](https://img.shields.io/badge/platform-Windows-blue?style=flat-square)

## 🚀 Features

- **Real-Time Audio Routing**: Captures desktop audio and routes it to a virtual microphone with minimal latency.
- **WASAPI Integration**: Uses Windows Core Audio API for robust, low-level audio handling.
- **VB-Audio Virtual Cable Support**: Leverages VB-Audio Virtual Cable as a virtual microphone for seamless integration with voice chat apps.
- **Format Fallback**: Automatically adjusts to compatible audio formats (e.g., 44100 Hz, 16-bit, stereo) if the default format is unsupported.
- **Error Handling**: Comprehensive error checking and debugging output for reliable operation.
- **Open Source**: Licensed under MIT for community contributions and customization.

## 📋 Prerequisites

To build and run this application, you need:

- **Operating System**: Windows 10 or 11
- **Development Environment**: Visual Studio (2019 or later) with C++ support and Windows SDK
- **VB-Audio Virtual Cable**: Download and install from [VB-Audio](https://vb-audio.com/Cable/)
- **Libraries**:
  - Windows SDK (for `mmdeviceapi.h`, `audioclient.h`, `functiondiscoverykeys_devpkey.h`)
  - Linked libraries: `ole32.lib`, `winmm.lib`
- **C++ Standard**: C++17 or later
- **Permissions**: Run the application as an administrator for audio device access

## 🛠️ Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/FlamesIsCool/Desktop-Audio-to-Virtual-Microphone-Router.git
   cd Desktop-Audio-to-Virtual-Microphone-Router
   ```

2. **Install VB-Audio Virtual Cable**:
   - Download and install [VB-Audio Virtual Cable](https://vb-audio.com/Cable/).
   - In Windows Sound Settings or your Game Settings:
     - Go to **Playback** tab and ensure "CABLE Input (VB-Audio Virtual Cable)" is enabled.
     - Go to **Recording** tab, enable "CABLE Output (VB-Audio Virtual Cable)," and set it as the default recording device.

3. **Set Up Development Environment**:
   - Install [Visual Studio](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload.
   - Ensure the Windows SDK is installed (included with Visual Studio).
   - Open the project in Visual Studio or configure your build system (e.g., MinGW with Windows SDK headers).

4. **Build the Project**:
   - Open the `.sln` file in Visual Studio or compile manually:
     ```bash
     g++ main.cpp -o AudioRouter -lole32 -lwinmm
     ```
   - Ensure the project links against `ole32.lib` and `winmm.lib`.

## 🎮 Usage

1. **Configure Audio Settings**:
   - In Windows Sound Settings:
     - Set your speakers (e.g., "Speakers (K66)") as the default playback device.
     - Set "CABLE Output (VB-Audio Virtual Cable)" as the default recording device.
   - In your voice chat application (e.g., Roblox, Discord), select "CABLE Output (VB-Audio Virtual Cable)" as the microphone input.

2. **Run the Application**:
   - Open a terminal or command prompt as an administrator.
   - Navigate to the project directory and run the executable:
     ```bash
     .\AudioRouter.exe
     ```
   - The console will display the selected devices and audio format:
     ```
     Found device: Speakers (K66)
     Found device: CABLE Input (VB-Audio Virtual Cable)
     Using render device: Speakers (K66)
     Using virtual cable device: CABLE Input (VB-Audio Virtual Cable)
     Mix format: 48000 Hz, 2 channels, 32 bits
     Capturing and routing audio to virtual cable. Press Ctrl+C to stop...
     ```

3. **Test the Audio**:
   - Play audio on your desktop (e.g., YouTube, Spotify).
   - Join a voice chat in Roblox or another application and confirm that others can hear the audio.
   - Use a recording tool like Audacity to record from "CABLE Output" to verify audio routing.

4. **Stop the Application**:
   - Press `Ctrl+C` in the console to stop routing and cleanly exit.

## 🔧 Troubleshooting

- **"VB-Audio Virtual Cable not found"**:
  - Ensure VB-Audio Virtual Cable is installed and enabled in Windows Sound Settings.
  - Check the console output for listed devices and verify "CABLE Input" appears in the Playback tab.
- **No Audio in Voice Chat**:
  - Confirm that your voice chat app is set to use "CABLE Output (VB-Audio Virtual Cable)" as the microphone.
  - Test with Audacity: Record from "CABLE Output" while playing desktop audio.
- **Format Errors**:
  - If the mix format (e.g., 48000 Hz, 32-bit) is unsupported, the program falls back to 44100 Hz, 16-bit, stereo. Check supported formats in Sound Settings > Playback > CABLE Input > Properties > Advanced.
- **Latency or Choppy Audio**:
  - Adjust `REFTIMES_PER_SEC` in the code (e.g., to `5000000` for 0.5 seconds) to reduce latency.
  - Ensure no other applications are using the virtual cable in exclusive mode (Sound Settings > CABLE Input > Properties > Advanced > uncheck "Allow applications to take exclusive control").
- **Error Codes**:
  - If errors like `0x88890008` (unsupported format) or `0x88890003` (wrong endpoint type) appear, check the console output and share it for further debugging.

## 🛠️ Code Overview

The application uses the **Windows Core Audio API (WASAPI)** to:
1. **Capture Desktop Audio**: Uses loopback mode on the default render device (e.g., "Speakers (K66)") to capture audio output.
2. **Route to Virtual Microphone**: Sends captured audio to the VB-Audio Virtual Cable’s render endpoint ("CABLE Input"), which appears as a microphone input ("CABLE Output") for other applications.
3. **Handle Audio Formats**: Dynamically adjusts audio formats to ensure compatibility.
4. **Error Handling**: Provides detailed error messages and device enumeration for debugging.

Key components:
- **Device Enumeration**: Finds the VB-Audio Virtual Cable render endpoint.
- **Audio Processing**: Real-time copying of audio buffers from the loopback capture to the virtual cable.
- **Format Fallback**: Automatically switches to a compatible format if needed.

## 📡 Future Improvements

- **Microphone Mixing**: Add support for mixing real microphone input with desktop audio.
- **Cross-Platform Support**: Integrate PortAudio for macOS and Linux compatibility.
- **GUI Interface**: Develop a graphical interface for easier configuration.
- **Latency Optimization**: Fine-tune buffer sizes for lower latency.
- **Audio Effects**: Add options for volume control or audio filters.

## 🤝 Contributing

Contributions are welcome! To contribute:
1. Fork the repository.
2. Create a new branch (`git checkout -b feature/your-feature`).
3. Make your changes and commit (`git commit -m "Add your feature"`).
4. Push to your branch (`git push origin feature/your-feature`).
5. Open a Pull Request.

Please follow the [Code of Conduct](CODE_OF_CONDUCT.md) and ensure your changes include appropriate documentation.

## 📜 License

This project is licensed under the [MIT License](LICENSE). Feel free to use, modify, and distribute the code as per the license terms.

## 🙌 Acknowledgments

- **VB-Audio**: For providing the Virtual Cable software.
- **Microsoft**: For the Windows Core Audio API documentation.
- **FlamesIsCool**: For creating this project and sharing it with the community.

## 📬 Contact

For questions, issues, or suggestions, reach out via:
- GitHub Issues: [Create an Issue](https://github.com/FlamesIsCool/Desktop-Audio-to-Virtual-Microphone-Router/issues)
- Email: [your-email@example.com] (optional)
- Discord: FlamesIsCool (optional)

---

⭐ **Star this repository** if you find it useful!  
Happy audio routing! 🎧
