/**
 * VocoderEngine - Implements a real-time vocoder using the Web Audio API
 * The vocoder works by analyzing the input signal (modulator) and using it to control 
 * the amplitude of different frequency bands in a carrier signal (oscillator).
 */
class VocoderEngine {
    /**
     * Creates a new VocoderEngine instance
     */
    constructor() {
        this.ctx = null;
        this.initialized = false;
        this.bands = CONSTANTS.BAND_COUNT;
        this.bandFrequencies = CONSTANTS.BAND_FREQUENCIES;
        this.filterNodes = [];
        this.params = {
            pitch: CONSTANTS.PITCH_DEFAULT,
            intensity: CONSTANTS.INTENSITY_DEFAULT,
            vibrato: 0,
            tremolo: 0,
            echo: 0,
            wave: 'sawtooth'
        };
        this.modLevel = 0;
        this.noiseThreshold = CONSTANTS.NOISE_THRESHOLD;

        this.micStream = null;
        this.micSource = null;
        this.fileBuffer = null;
        this.fileSource = null;
        this.preAmp = null;
    }

    /**
     * Initializes the audio context and sets up the audio processing graph
     * @returns {Promise<AudioContext>} The initialized audio context
     */
    async create() {
        if (this.ctx) { try { await this.ctx.close(); } catch (e) { } }
        this.ctx = new (window.AudioContext || window.webkitAudioContext)();

        const now = this.ctx.currentTime;

        // Compresor de entrada modo "Limitador" para la moduladora
        this.inputCompressor = this.ctx.createDynamicsCompressor();
        this.inputCompressor.threshold.setTargetAtTime(CONSTANTS.COMPRESSOR_THRESHOLD, now, 0);
        this.inputCompressor.knee.setTargetAtTime(CONSTANTS.COMPRESSOR_KNEE, now, 0);
        this.inputCompressor.ratio.setTargetAtTime(CONSTANTS.COMPRESSOR_RATIO, now, 0);
        this.inputCompressor.attack.setTargetAtTime(CONSTANTS.COMPRESSOR_ATTACK, now, 0);
        this.inputCompressor.release.setTargetAtTime(CONSTANTS.COMPRESSOR_RELEASE, now, 0);

        this.preAmp = this.ctx.createGain();
        this.preAmp.gain.setTargetAtTime(CONSTANTS.PRE_AMP_GAIN, now, 0); // Bajado de 2.5 para evitar saturación
        this.preAmp.connect(this.inputCompressor);

        // Bus maestro muy conservador
        this.masterOut = this.ctx.createGain();
        this.masterOut.gain.setTargetAtTime(CONSTANTS.MASTER_GAIN, now, 0); // Bajado de 0.4 para evitar suma de bandas excesiva

        // Limitador final agresivo
        this.limiter = this.ctx.createDynamicsCompressor();
        this.limiter.threshold.setTargetAtTime(CONSTANTS.LIMITER_THRESHOLD, now, 0);
        this.limiter.ratio.setTargetAtTime(CONSTANTS.LIMITER_RATIO, now, 0);
        this.limiter.attack.setTargetAtTime(CONSTANTS.COMPRESSOR_ATTACK, now, 0);
        this.limiter.release.setTargetAtTime(CONSTANTS.COMPRESSOR_RELEASE, now, 0);

        this.analyser = this.ctx.createAnalyser();
        this.analyser.fftSize = CONSTANTS.FFT_SIZE;

        this.setupGraph();
        this.masterOut.connect(this.limiter);
        this.limiter.connect(this.analyser);
        this.analyser.connect(this.ctx.destination);

        this.vuAnalyser = this.ctx.createAnalyser();
        this.vuAnalyser.fftSize = CONSTANTS.VU_FFT_SIZE;
        this.inputCompressor.connect(this.vuAnalyser);

        this.initialized = true;
        this.runEngine();
        return this.ctx;
    }

    /**
     * Sets up the audio processing graph for the vocoder
     * Creates the carrier oscillator, filters for both modulator and carrier signals,
     * and connects all nodes in the processing chain
     */
    setupGraph() {
        const now = this.ctx.currentTime;
        this.carrier = this.ctx.createOscillator();
        this.carrier.type = String(this.params.wave || 'sawtooth');
        this.carrier.frequency.setTargetAtTime(Number(this.params.pitch || CONSTANTS.PITCH_DEFAULT), now, 0);
        // Ganancia del carrier (1.0 = activo)
        this.carrierGain = this.ctx.createGain();
        this.carrierGain.gain.setTargetAtTime(1.0, now, 0); // ACTIVADO
        this.carrier.connect(this.carrierGain);
        console.log('[VOCODER] Carrier ACTIVO (gain=1)');

        this.vibratoOsc = this.ctx.createOscillator();
        this.vibratoGain = this.ctx.createGain();
        this.vibratoOsc.frequency.setTargetAtTime(CONSTANTS.VIBRATO_FREQ, now, 0);
        this.vibratoGain.gain.setTargetAtTime(0, now, 0);
        this.vibratoOsc.connect(this.vibratoGain);
        this.vibratoGain.connect(this.carrier.frequency);
        this.vibratoOsc.start();

        this.echo = this.ctx.createDelay(1.0);
        this.echo.delayTime.setTargetAtTime(CONSTANTS.ECHO_DELAY, now, 0);
        this.echoFeedback = this.ctx.createGain();
        this.echoFeedback.gain.setTargetAtTime(0, now, 0);

        this.masterOut.connect(this.echo);
        this.echo.connect(this.echoFeedback);
        this.echoFeedback.connect(this.echo);
        this.echo.connect(this.limiter);

        this.filterNodes = [];
        for (let i = 0; i < this.bands; i++) {
            const freq = Number(this.bandFrequencies[i]);

            // Modulator path: analyze the input signal
            const modFilter = this.ctx.createBiquadFilter();
            modFilter.type = "bandpass";
            modFilter.frequency.setTargetAtTime(freq, now, 0);
            modFilter.Q.setTargetAtTime(12, now, 0); // Q=12 para mejor selectividad

            const modAnalyser = this.ctx.createAnalyser();
            modAnalyser.fftSize = CONSTANTS.BANDPASS_FFT_SIZE;
            modFilter.connect(modAnalyser);
            this.inputCompressor.connect(modFilter);

            // Carrier path: generate and filter the carrier signal
            const carFilter = this.ctx.createBiquadFilter();
            carFilter.type = "bandpass";
            carFilter.frequency.setTargetAtTime(freq, now, 0);
            carFilter.Q.setTargetAtTime(12, now, 0); // Q=12 para mejor selectividad

            // Gain node to control the amplitude of this frequency band
            const bandGain = this.ctx.createGain();
            bandGain.gain.setTargetAtTime(0, now, 0);

            // Connect carrierGain -> carrier filter -> gain -> master out
            this.carrierGain.connect(carFilter);
            carFilter.connect(bandGain);
            bandGain.connect(this.masterOut);

            this.filterNodes.push({ modFilter, modAnalyser, bandGain });
        }

        this.carrier.start();
    }

    /**
     * Toggles the microphone input on or off
     * @param {boolean} active - Whether to activate or deactivate the microphone
     * @returns {Promise<string>} Connection status ("CONNECTED", "PERMISSION_DENIED", "NO_DEVICE", "ERROR", or "DISCONNECTED")
     */
    async toggleMic(active) {
        if (active) {
            try {
                // Intento de llamar al micrófono con constraints básicos
                this.micStream = await navigator.mediaDevices.getUserMedia({
                    audio: {
                        echoCancellation: false,
                        noiseSuppression: false,
                        autoGainControl: true
                    }
                });
                this.micSource = this.ctx.createMediaStreamSource(this.micStream);
                this.micSource.connect(this.preAmp);
                return "CONNECTED";
            } catch (e) {
                console.error("MIC_ERROR:", e);
                if (e.name === 'NotAllowedError') return "PERMISSION_DENIED";
                if (e.name === 'NotFoundError') return "NO_DEVICE";
                return "ERROR";
            }
        } else {
            if (this.micSource) {
                try { this.micSource.disconnect(); } catch (e) { }
                this.micSource = null;
            }
            if (this.micStream) {
                this.micStream.getTracks().forEach(t => t.stop());
                this.micStream = null;
            }
            return "DISCONNECTED";
        }
    }

    /**
     * Toggles the file input on or off
     * @param {boolean} active - Whether to activate or deactivate the file playback
     * @returns {boolean} Whether the file is now active
     */
    toggleFile(active) {
        if (active && this.fileBuffer) {
            this.fileSource = this.ctx.createBufferSource();
            this.fileSource.buffer = this.fileBuffer;
            this.fileSource.loop = true;
            this.fileSource.connect(this.preAmp);
            this.fileSource.start();
            return true;
        } else {
            if (this.fileSource) {
                try { this.fileSource.stop(); } catch (e) { }
                this.fileSource.disconnect();
                this.fileSource = null;
            }
            return false;
        }
    }

    /**
     * Starts the main audio processing loop
     * This function continuously analyzes the modulator signal and applies its
     * spectral characteristics to the carrier signal in real-time
     */
    runEngine() {
        const vuData = new Uint8Array(256);
        let debugCounter = 0;
        const process = () => {
            if (!this.initialized || this.ctx.state !== 'running') {
                requestAnimationFrame(process);
                return;
            }

            // Calculate overall modulator level for VU meter
            if (this.vuAnalyser) {
                this.vuAnalyser.getByteFrequencyData(vuData);
                let sum = 0;
                for (let i = 0; i < 32; i++) {
                    sum += vuData[i];
                }
                this.modLevel = (sum / 32) / 255;
            }

            // Debug cada 60 frames (~1 segundo)
            debugCounter++;
            if (debugCounter % 60 === 0) {
                console.log('[VOCODER DEBUG] modLevel:', this.modLevel.toFixed(3),
                    'threshold:', this.noiseThreshold,
                    'intensity:', this.params.intensity);
            }

            const now = this.ctx.currentTime;

            // Process each frequency band
            const bandLevels = [];
            this.filterNodes.forEach((band, index) => {
                const bandData = new Uint8Array(band.modAnalyser.frequencyBinCount);
                band.modAnalyser.getByteFrequencyData(bandData);

                // Sum all bins to get total energy in this band
                let sum = 0;
                for (let i = 0; i < bandData.length; i++) {
                    sum += bandData[i];
                }
                let level = bandData.length > 0 ? (sum / bandData.length) / 255 : 0;

                // Puerta de ruido: silenciar si no hay señal moduladora
                if (this.modLevel < this.noiseThreshold) {
                    level = 0;
                }

                // Apply intensity scaling and gain multiplier
                const targetGain = level * Number(this.params.intensity || CONSTANTS.INTENSITY_DEFAULT) * CONSTANTS.MAX_GAIN_MULTIPLIER;

                // Guardar para debug
                if (index === 0 || index === 5 || index === 10 || index === 15 || index === 19) {
                    bandLevels.push({ band: index, level: level.toFixed(3), gain: targetGain.toFixed(2) });
                }

                band.bandGain.gain.setTargetAtTime(targetGain, now, targetGain > 0 ? 0.005 : 0.05);
            });

            // Debug: mostrar niveles de varias bandas cada segundo
            if (debugCounter % 60 === 0 && this.modLevel > this.noiseThreshold) {
                console.log('[BANDS] modLevel:', this.modLevel.toFixed(3), 'Bands:', bandLevels);
            }

            requestAnimationFrame(process);
        };
        process();
    }

    /**
     * Updates the vocoder parameters in real-time
     * @param {Object} p - Object containing parameter updates (pitch, intensity, vibrato, tremolo, echo, wave)
     */
    updateParams(p) {
        console.log('[updateParams] Recibido:', p);
        Object.assign(this.params, p);
        if (!this.initialized || !this.ctx) return;
        const now = this.ctx.currentTime;
        if (this.carrier && Number.isFinite(this.params.pitch)) {
            this.carrier.frequency.setTargetAtTime(Number(this.params.pitch), now, 0.05);
            console.log('[updateParams] Pitch:', this.params.pitch);
        }
        if (this.vibratoGain && Number.isFinite(this.params.vibrato)) {
            this.vibratoGain.gain.setTargetAtTime(Number(this.params.vibrato) * CONSTANTS.VIBRATO_MAX, now, 0.05);
            console.log('[updateParams] Vibrato:', this.params.vibrato * CONSTANTS.VIBRATO_MAX);
        }
        if (this.echoFeedback && Number.isFinite(this.params.echo)) {
            this.echoFeedback.gain.setTargetAtTime(Number(this.params.echo) * CONSTANTS.ECHO_MAX, now, 0.05);
            console.log('[updateParams] Echo:', this.params.echo * CONSTANTS.ECHO_MAX);
        }
        if (this.carrier && this.params.wave) {
            this.carrier.type = String(this.params.wave);
            console.log('[updateParams] Wave:', this.carrier.type);
        }
    }
}