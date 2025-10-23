#version 330 core
layout (location = 0) in vec2 aPosScreen; // in screen coordinates (pixels)
layout (location = 1) in float aRadius;
layout (location = 2) in vec3 aColor;

out vec3 vColor;
out float vRadius;

uniform vec2 uResolution;

void main()
{
    // convert to NDC
    vec2 ndc = vec2((aPosScreen.x / uResolution.x) * 2.0 - 1.0,
                    (aPosScreen.y / uResolution.y) * 2.0 - 1.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    gl_PointSize = aRadius * 2.0; // diameter in pixels
    vColor = aColor;
    vRadius = aRadius;
}
