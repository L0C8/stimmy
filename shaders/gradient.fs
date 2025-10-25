#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform float uTime;
uniform vec2 uResolution;

// A colorful animated gradient using sine waves
void main()
{
    vec2 uv = fragTexCoord;
    // centre and aspect correction
    vec2 pos = (uv - 0.5) * vec2(uResolution.x / uResolution.y, 1.0);
    // Slower motion: smaller multiplier
    float t = uTime * 0.18;

    vec3 c;
    c.r = 0.5 + 0.5 * sin(3.0*pos.x + t);
    c.g = 0.5 + 0.5 * sin(2.0*pos.y - t*1.2 + pos.x*1.5);
    c.b = 0.5 + 0.5 * sin(4.0*(pos.x+pos.y) + t*0.8);

    // add circular vignetting for depth
    float d = length(pos);
    c *= smoothstep(1.2, 0.4, d);

    // subtle bands
    c *= 0.85 + 0.15*sin(10.0*uv.x + t*0.7);

    finalColor = vec4(c, 1.0);
}
