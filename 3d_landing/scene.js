import * as THREE from 'three';

// Configuration
const CONFIG = {
    fogColor: 0x00ff8e,
    bgColor: 0x0a0a0a,
    particleCount: 15000,
    fogDensity: 0.05
};

let scene, camera, renderer, particles, analyser, audioData;
let clock = new THREE.Clock();
let isRunning = false;

// UI Elements
const startBtn = document.getElementById('start-btn');
const statusText = document.getElementById('audio-status');
const fpsText = document.getElementById('fps-counter');

// 1. Initialize Scene
function init() {
    scene = new THREE.Scene();
    scene.background = new THREE.Color(CONFIG.bgColor);

    camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.z = 5;

    renderer = new THREE.WebGLRenderer({
        canvas: document.getElementById('canvas3d'),
        antialias: true,
        alpha: true
    });
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));

    createFog();

    window.addEventListener('resize', onWindowResize);
    animate();
}

// 2. Create "Digital Fog" using Particles
function createFog() {
    const geometry = new THREE.BufferGeometry();
    const positions = new Float32Array(CONFIG.particleCount * 3);
    const sizes = new Float32Array(CONFIG.particleCount);

    for (let i = 0; i < CONFIG.particleCount * 3; i++) {
        positions[i] = (Math.random() - 0.5) * 20;
    }

    for (let i = 0; i < CONFIG.particleCount; i++) {
        sizes[i] = Math.random() * 2;
    }

    geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    geometry.setAttribute('size', new THREE.BufferAttribute(sizes, 1));

    // Simple Shader Material for the points
    const material = new THREE.ShaderMaterial({
        uniforms: {
            uTime: { value: 0 },
            uAudio: { value: 0 },
            uColor: { value: new THREE.Color(CONFIG.fogColor) }
        },
        vertexShader: `
            uniform float uTime;
            uniform float uAudio;
            attribute float size;
            varying float vOpacity;
            
            void main() {
                vec3 pos = position;
                
                // Audio-driven displacement
                pos.y += sin(pos.x * 2.0 + uTime) * (0.1 + uAudio * 1.5);
                pos.x += cos(pos.z * 1.5 + uTime * 0.5) * (0.1 + uAudio * 0.5);
                
                vec4 mvPosition = modelViewMatrix * vec4(pos, 1.0);
                gl_PointSize = size * (300.0 / -mvPosition.z) * (1.0 + uAudio * 3.0);
                gl_Position = projectionMatrix * mvPosition;
                
                vOpacity = (1.0 - length(pos) / 15.0) * (0.3 + uAudio);
            }
        `,
        fragmentShader: `
            uniform vec3 uColor;
            varying float vOpacity;
            
            void main() {
                float dist = length(gl_PointCoord - vec2(0.5));
                if (dist > 0.5) discard;
                
                float grad = 1.0 - dist * 2.0;
                gl_FragColor = vec4(uColor, grad * vOpacity);
            }
        `,
        transparent: true,
        blending: THREE.AdditiveBlending,
        depthWrite: false
    });

    particles = new THREE.Points(geometry, material);
    scene.add(particles);
}

// 3. Audio Setup
async function setupAudio() {
    try {
        const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
        const source = audioContext.createMediaStreamSource(stream);
        analyser = audioContext.createAnalyser();
        analyser.fftSize = 256;
        source.connect(analyser);

        audioData = new Uint8Array(analyser.frequencyBinCount);
        statusText.innerText = "AUDIO CONECTADO: BRÉTEMA REACTIVA ACTIVA";
        isRunning = true;
    } catch (err) {
        statusText.innerText = "ERROR ACCEDENDO AO MICRO: " + err.message;
        console.error(err);
    }
}

// 4. Animation Loop
function animate() {
    requestAnimationFrame(animate);

    const elapsedTime = clock.getElapsedTime();
    const delta = clock.getDelta();
    let audioValue = 0;

    if (isRunning && analyser) {
        analyser.getByteFrequencyData(audioData);
        // Average volume calculation
        let sum = 0;
        for (let i = 0; i < audioData.length; i++) {
            sum += audioData[i];
        }
        audioValue = (sum / audioData.length) / 255;
    }

    // Update particles
    if (particles) {
        particles.material.uniforms.uTime.value = elapsedTime;
        particles.material.uniforms.uAudio.value = audioValue;
        particles.rotation.y = elapsedTime * 0.05;
        particles.rotation.z = elapsedTime * 0.02;
    }

    renderer.render(scene, camera);

    // Safety check for FPS
    if (delta > 0) {
        fpsText.innerText = Math.round(1 / delta);
    }
}

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

// Interaction listener
console.log("Script cargado. Esperando click...");
startBtn.addEventListener('click', () => {
    console.log("Botón pulsado. Iniciando audio...");
    setupAudio();
    startBtn.classList.add('hidden');
    // Reveal technical status
    document.querySelector('.status-box').style.opacity = 1;
});

try {
    init();
    console.log("Three.js inicializado correctamente.");
} catch (e) {
    console.error("Erro na inicialización:", e);
    statusText.innerText = "ERRO TÉCNICO: " + e.message;
}
