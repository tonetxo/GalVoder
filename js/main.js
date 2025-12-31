// Main application logic

// Global variables
let engine;
let uiController;

// Initialize the application when the DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    engine = new VocoderEngine();
    uiController = new UIController(engine);
});

/**
 * Updates the VU meter needle position based on the current input level
 */
function updateVU() {
    const targetAngle = CONSTANTS.VU_MIN_ANGLE + (Number(engine.modLevel || 0) * (CONSTANTS.VU_MAX_ANGLE - CONSTANTS.VU_MIN_ANGLE));
    const needle = document.getElementById('vu-needle');
    needle.style.transform = `rotate(${targetAngle}deg)`;
    requestAnimationFrame(updateVU);
}

/**
 * Handles movement of the XY pad, updating parameters based on position
 * @param {Event|Object} e - The mouse/touch event or an object with x,y coordinates for programmatic updates
 * @param {boolean} isAuto - Whether this is a programmatic update rather than user interaction
 */
function movePad(e, isAuto = false) {
    const pad = document.getElementById('pad');
    const handle = document.getElementById('pad-handle');
    const rect = pad.getBoundingClientRect();
    let x, y;
    if (isAuto) {
        x = Number.isFinite(e.x) ? e.x : 0.5;
        y = Number.isFinite(e.y) ? e.y : 0.5;
    } else {
        const ev = e.touches ? e.touches[0] : e;
        x = Math.max(0, Math.min(1, (ev.clientX - rect.left) / rect.width));
        y = Math.max(0, Math.min(1, (ev.clientY - rect.top) / rect.height));
    }
    handle.style.left = `${x * 100}%`;
    handle.style.top = `${y * 100}%`;
    document.getElementById('val-x').innerText = `${Math.round(x*100)}%`;
    document.getElementById('val-y').innerText = `${Math.round((1-y)*100)}%`;
    const updateParam = (type, val) => {
        const p = {};
        if (type === 'pitch') p.pitch = CONSTANTS.PITCH_MIN + (val * (CONSTANTS.PITCH_MAX - CONSTANTS.PITCH_MIN)); // 50 + (val * 600) -> 50 to 400
        if (type === 'intensity') p.intensity = CONSTANTS.INTENSITY_MIN + (val * (CONSTANTS.INTENSITY_MAX - CONSTANTS.INTENSITY_MIN)); // 0.2 + (val * 1.0) -> 0.2 to 1.2
        if (type === 'vibrato') p.vibrato = val;
        if (type === 'tremolo') p.tremolo = val;
        if (type === 'echo') p.echo = val * CONSTANTS.ECHO_MAX; // val * 0.7
        engine.updateParams(p);
    };
    updateParam(document.getElementById('select-x').value, x);
    updateParam(document.getElementById('select-y').value, 1 - y);
}

/**
 * Draws the oscilloscope visualization of the audio waveform
 */
function drawOsc() {
    const canvas = document.getElementById('oscilloscope');
    const ctx = canvas.getContext('2d');
    const bufferLength = engine.analyser.frequencyBinCount;
    const dataArray = new Uint8Array(bufferLength);
    const render = () => {
        if (!engine.initialized || engine.ctx.state !== 'running') return;
        requestAnimationFrame(render);
        engine.analyser.getByteTimeDomainData(dataArray);
        ctx.fillStyle = 'rgba(10, 8, 5, 0.5)';
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        ctx.lineWidth = CONSTANTS.CANVAS_LINE_WIDTH; 
        ctx.strokeStyle = CONSTANTS.CANVAS_STROKE_COLOR; 
        ctx.beginPath();
        let x = 0; 
        const sliceWidth = canvas.width / bufferLength;
        for(let i=0; i<bufferLength; i++) {
            const v = dataArray[i] / 128.0; 
            const y = (v * canvas.height / 2);
            if(i===0) ctx.moveTo(x,y); else ctx.lineTo(x,y);
            x += sliceWidth;
        }
        ctx.stroke();
    };
    render();
}