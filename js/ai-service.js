class AIService {
    static async callGemini(prompt, isJson = false) {
        // Check if API key is provided
        if (!API_CONFIG.apiKey || API_CONFIG.apiKey.trim() === "") {
            throw new Error("API key not configured. AI features are disabled.");
        }

        const url = `https://generativelanguage.googleapis.com/v1beta/models/${API_CONFIG.model}:generateContent?key=${API_CONFIG.apiKey}`;
        const payload = {
            contents: [{ parts: [{ text: prompt }] }],
            generationConfig: isJson ? {
                responseMimeType: "application/json",
                responseSchema: {
                    type: "OBJECT",
                    properties: {
                        pitch_val: { type: "NUMBER" },
                        x_pos: { type: "NUMBER" },
                        y_pos: { type: "NUMBER" },
                        wave: { type: "STRING" },
                        explanation: { type: "STRING" }
                    },
                    required: ["pitch_val", "x_pos", "y_pos", "wave", "explanation"]
                }
            } : undefined
        };

        for (let i = 0; i < CONSTANTS.MAX_RETRY_ATTEMPTS; i++) {
            try {
                const response = await fetch(url, {
                    method: 'POST',
                    headers: { 
                        'Content-Type': 'application/json' 
                    },
                    body: JSON.stringify(payload)
                });
                
                if (!response.ok) {
                    const errorData = await response.json().catch(() => ({}));
                    throw new Error(`API Error: ${response.status} - ${errorData.error?.message || response.statusText}`);
                }
                
                const data = await response.json();
                const text = data.candidates?.[0]?.content?.parts?.[0]?.text;
                
                if (!text) {
                    throw new Error("No content returned from API");
                }
                
                return text;
            } catch (err) {
                console.warn(`API attempt ${i + 1} failed:`, err.message);
                
                if (i === CONSTANTS.MAX_RETRY_ATTEMPTS - 1) {
                    // Last attempt, throw the error
                    throw err;
                }
                
                // Wait before retrying (exponential backoff)
                await new Promise(r => setTimeout(r, Math.pow(2, i) * CONSTANTS.RETRY_DELAY_BASE));
            }
        }
    }
}

// Wrapper function to maintain compatibility with existing code
async function callGemini(prompt, isJson = false) {
    try {
        return await AIService.callGemini(prompt, isJson);
    } catch (error) {
        console.error("AI Service Error:", error.message);
        throw error;
    }
}