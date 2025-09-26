#version 330 core
out vec4 FragColor;

uniform float uTime;
uniform vec2 uResolution;

float quantize(float v, float steps) {
    return floor(v*steps)/steps;
}

// Simple rainbow gradient
vec3 rainbow(float t)
{
    float r = 0.5 + 0.5*sin(6.2831*t + 0.0);
    float g = 0.5 + 0.5*sin(6.2831*t + 2.094); // 2pi/3 phase shift
    float b = 0.5 + 0.5*sin(6.2831*t + 4.188); // 4pi/3
    return vec3(r,g,b);
}

void main()
{
    float pixelSize = 8.0;
    vec2 uv = floor(gl_FragCoord.xy / pixelSize);

    float rows = uResolution.y / pixelSize;

    // Independent wave per column
    float phase = uv.x * 0.15;
    float base = 0.5;          // center of wave
    float amp1 = 0.25;         // amplitude of main sine
    float amp2 = 0.2;          // amplitude of vertical oscillation
    float wave = base + amp1 * sin(phase - uTime*2.0);
    wave += amp2 * sin(uTime*1.5 + uv.x*0.1);
    wave = clamp(wave, 0.0, 1.0);

    wave = clamp(wave,0.0,1.0);
    float waveBlock = quantize(wave*rows,rows);

    float bar = step(float(uv.y), waveBlock);

    // Color varies per column using rainbow
    vec3 color = mix(vec3(0.0), rainbow(uv.x / uResolution.x), bar);

    FragColor = vec4(color,1.0);
}
