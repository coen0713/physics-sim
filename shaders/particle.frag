#version 330 core
in vec3 vColor;
in float vRadius;

out vec4 FragColor;

void main()
{
    // point sprite circle mask
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    float dist = length(p);
    float alpha = smoothstep(1.0, 0.98, 1.0 - dist);
    if (dist > 1.0) discard;
    FragColor = vec4(vColor, alpha);
}
