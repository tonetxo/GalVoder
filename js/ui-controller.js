/**
 * UIController - Manages all UI interactions and separates them from the audio engine
 */
class UIController {
    /**
     * Creates a new UIController instance
     * @param {VocoderEngine} engine - The audio engine instance to control
     */
    constructor(engine) {
        this.engine = engine;
        this.micActive = false;
        this.fileActive = false;
        
        // Cache DOM elements
        this.btnPower = document.getElementById('btn-power');
        this.needle = document.getElementById('vu-needle');
        this.msgBox = document.getElementById('msg-box');
        this.ledPower = document.getElementById('led-master');
        this.pad = document.getElementById('pad');
        this.handle = document.getElementById('pad-handle');
        
        this.initializeEventListeners();
    }

    /**
     * Sets up all event listeners for UI elements
     */
    initializeEventListeners() {
        this.btnPower.addEventListener('click', async () => {
            if (!this.engine.initialized) await this.engine.create();
            if (this.engine.ctx.state === 'running') {
                await this.engine.ctx.suspend();
                this.btnPower.innerText = "Energizar Sistema";
                this.ledPower.classList.remove('on');
                this.msgBox.innerText = "SISTEMA LATENTE";
            } else {
                await this.engine.ctx.resume();
                this.btnPower.innerText = "Cesar Inducción";
                this.ledPower.classList.add('on');
                this.msgBox.innerText = "INDUCCIÓN EN CURSO";
                requestAnimationFrame(updateVU);
                drawOsc();
            }
        });

        document.getElementById('btn-mic').addEventListener('click', async (e) => {
            this.msgBox.innerHTML = "SOLICITANDO PERMISOS AL ÉTER...";
            await this.ensureEngine();

            if (!this.micActive) {
                const result = await this.engine.toggleMic(true);
                if (result === "CONNECTED") {
                    this.micActive = true;
                    e.target.classList.add('active');
                    this.msgBox.innerText = "CONEXIÓN CON EL MICRO ESTABLECIDA";
                } else if (result === "PERMISSION_DENIED") {
                    this.msgBox.innerHTML = "<span class='text-red-400'>ERROR: PERMISO DENEGADO POR EL NAVEGADOR</span>";
                } else if (result === "NO_DEVICE") {
                    this.msgBox.innerText = "ERROR: NO SE ENCUENTRA DISPOSITIVO DE AUDIO";
                } else {
                    this.msgBox.innerText = "ERROR DESCONOCIDO AL CONECTAR MICRO";
                }
            } else {
                await this.engine.toggleMic(false);
                this.micActive = false;
                e.target.classList.remove('active');
                this.msgBox.innerText = "CAPTACIÓN DE MICRO CERRADA";
            }
        });

        document.getElementById('btn-file-toggle').addEventListener('click', async (e) => {
            if (!this.engine.fileBuffer) {
                this.msgBox.innerText = "CARGUE UN CILINDRO PRIMERO";
                return;
            }
            await this.ensureEngine();
            this.fileActive = !this.fileActive;
            this.engine.toggleFile(this.fileActive);
            e.target.classList.toggle('active', this.fileActive);
            this.msgBox.innerText = this.fileActive ? "REPRODUCCIÓN DE CILINDRO ACTIVA" : "CILINDRO DETENIDO";
        });

        document.getElementById('btn-file-load').addEventListener('click', () => document.getElementById('file-input').click());

        document.getElementById('file-input').addEventListener('change', (e) => {
            const file = e.target.files[0];
            if (file) {
                const reader = new FileReader();
                reader.onload = async (ev) => {
                    this.msgBox.innerText = "DECODIFICANDO CILINDRO...";
                    await this.ensureEngine();
                    try {
                        this.engine.fileBuffer = await this.engine.ctx.decodeAudioData(ev.target.result);
                        this.msgBox.innerText = "CILINDRO PREPARADO PARA INDUCCIÓN";
                    } catch(err) {
                        this.msgBox.innerText = "ERROR AL DECODIFICAR ARCHIVO";
                    }
                };
                reader.readAsArrayBuffer(file);
            }
        });
        
        // AI button event listeners
        document.getElementById('btn-ai-message').addEventListener('click', async () => {
            // Check if API key is configured before attempting to call AI
            if (!API_CONFIG.apiKey || API_CONFIG.apiKey.trim() === "") {
                document.getElementById('ai-output').innerText = "API key no configurada. Por favor, introduce una clave de API para habilitar las funciones de IA.";
                return;
            }
            
            document.getElementById('ai-status').classList.remove('hidden');
            try {
                const text = await callGemini("Genera una frase críptica y steampunk para un vocoder. Máximo 10 palabras.");
                document.getElementById('ai-output').innerText = `"${text}"`;
            } catch (e) { 
                document.getElementById('ai-output').innerText = e.message.includes("API key not configured") ? 
                    "Funciones de IA deshabilitadas. Introduce una clave de API para activarlas." : "Enlace fallido.";
            }
            document.getElementById('ai-status').classList.add('hidden');
        });

        document.getElementById('btn-ai-vibe').addEventListener('click', async () => {
            // Check if API key is configured before attempting to call AI
            if (!API_CONFIG.apiKey || API_CONFIG.apiKey.trim() === "") {
                document.getElementById('ai-output').innerText = "API key no configurada. Por favor, introduce una clave de API para habilitar las funciones de IA.";
                return;
            }
            
            const vibe = document.getElementById('ai-vibe-input').value || "Robot victoriano";
            document.getElementById('ai-status').classList.remove('hidden');
            try {
                const prompt = `Devuelve parámetros JSON para la vibe "${vibe}": pitch_val (50-400), x_pos (0-1), y_pos (0-1), wave (sawtooth/square/triangle), explanation (frase corta).`;
                const jsonStr = await callGemini(prompt, true);
                const data = JSON.parse(jsonStr);
                document.getElementById('wave-type').value = data.wave;
                this.engine.updateParams({ wave: data.wave, pitch: Number(data.pitch_val) });
                movePad({ x: Number(data.x_pos), y: Number(data.y_pos) }, true);
                document.getElementById('ai-output').innerText = data.explanation;
            } catch (e) { 
                document.getElementById('ai-output').innerText = e.message.includes("API key not configured") ? 
                    "Funciones de IA deshabilitadas. Introduce una clave de API para activarlas." : "Calibración fallida.";
            }
            document.getElementById('ai-status').classList.add('hidden');
        });
        
        // Pad event listeners
        this.pad.addEventListener('mousedown', () => this.pad.addEventListener('mousemove', movePad));
        window.addEventListener('mouseup', () => this.pad.removeEventListener('mousemove', movePad));
        this.pad.addEventListener('touchstart', (e) => { e.preventDefault(); movePad(e); });
        this.pad.addEventListener('touchmove', (e) => { e.preventDefault(); movePad(e); });
    }

    /**
     * Ensures the audio engine is initialized and running
     * @returns {Promise<void>}
     */
    async ensureEngine() {
        if (!this.engine.initialized) await this.engine.create();
        if (this.engine.ctx.state === 'suspended') {
            await this.engine.ctx.resume();
        }
        this.btnPower.innerText = "Cesar Inducción";
        this.ledPower.classList.add('on');
    }
}