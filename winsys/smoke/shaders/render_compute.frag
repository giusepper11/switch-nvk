#version 450

layout(push_constant) uniform Seed { uint value; } seed;
layout(location = 0) out vec4 color;

void main()
{
   uvec2 p = uvec2(gl_FragCoord.xy);
   uvec4 c = uvec4((p.x * 17u + seed.value * 3u + 11u) & 255u,
                   (p.y * 29u + seed.value * 5u + 19u) & 255u,
                   ((p.x ^ p.y) * 13u + seed.value * 7u + 23u) & 255u,
                   255u);
   color = vec4(c) / 255.0;
}
