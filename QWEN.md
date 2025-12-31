# Vocoder Steampunk Aethereum MK-XII (AI Enhanced)

## Project Overview

This is a sophisticated web-based vocoder application with a distinctive steampunk aesthetic. The application implements a real-time audio processing system that combines traditional vocoder techniques with modern Web Audio API and AI integration. It features a Victorian-era industrial design with brass, copper, and bronze color schemes, creating an immersive "Neural Steam DSP MK-XII" experience.

## Key Features

- **Real-time Vocoder Processing**: Implements a 20-band vocoder with carrier and modulator signals
- **Steampunk UI Design**: Rich visual interface with brass/copper styling, rivets, and vintage aesthetic
- **XY Pad Control**: Interactive 2D controller for mapping parameters to X and Y axes
- **Multiple Input Sources**: Supports both microphone input and audio file playback
- **AI Integration**: Connects to Google's Gemini API for generating steampunk-themed messages and parameter calibration
- **Visual Feedback**: Includes VU meter, oscilloscope display, and status indicators
- **Audio Effects**: Vibrato, tremolo, echo, and pitch control with adjustable parameters
- **Waveform Selection**: Choose between sawtooth, square, and triangle waveforms

## Technical Architecture

### Audio Processing
- Uses Web Audio API for real-time audio processing
- Implements 20-band filter bank with bandpass filters
- Features input compressor, pre-amp, and master limiter
- Supports both microphone and file input sources
- Real-time parameter updates with smooth transitions

### UI Components
- Built with Tailwind CSS and custom CSS for steampunk styling
- Canvas-based oscilloscope visualization
- Interactive XY pad with draggable handle
- VU meter with needle animation
- Status lights and steampunk-themed buttons

### AI Integration
- Connects to Google's Gemini API (requires API key)
- Generates thematic messages and parameter suggestions
- Supports "vibe-based" parameter calibration using AI

## Building and Running

This is a single-file HTML application that can be run directly in any modern browser:

1. Open `aethereum.html` in a modern web browser (Chrome, Firefox, Safari, Edge)
2. The application will load with the steampunk interface
3. Click "Energizar Sistema" to initialize the audio engine
4. Grant microphone permissions when prompted
5. Use the various controls to process audio in real-time

### Dependencies
- Internet connection (for CDN-hosted Tailwind CSS and Google Fonts)
- Google Gemini API key (for AI features) - currently empty in the code
- Modern browser with Web Audio API support

## Development Conventions

- Single HTML file architecture with embedded CSS and JavaScript
- ES6 JavaScript with async/await for API calls
- Object-oriented approach with the `VocoderEngine` class
- Event-driven UI interactions
- Responsive design using Tailwind CSS grid and flexbox
- Steampunk-themed color palette and styling

## Key Files

- `aethereum.html`: Main application file containing all HTML, CSS, and JavaScript

## Usage Notes

- The application requires microphone permissions for real-time audio processing
- AI features require a valid Google Gemini API key to be configured
- The vocoder works best with speech as the modulator signal and harmonic content as the carrier
- The XY pad allows for real-time parameter mapping and modulation
- The oscilloscope provides visual feedback of the processed audio waveform