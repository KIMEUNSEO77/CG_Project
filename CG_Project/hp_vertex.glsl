// hp_vertex.glsl
#version 330 core

layout (location = 0) in vec2 aPos;  // (0~1, 0~1)

uniform float uHP;        // 0.0 ~ 1.0
uniform vec2  uPos;       // NDC에서 바의 왼쪽 아래 위치 (-1~1)
uniform vec2  uSize;      // NDC에서 바의 (width, height)

void main()
{
    // 0~1 UV
    vec2 uv = aPos;
    uv.x *= uHP;    // HP 비율만큼만 채움

    // uPos에서 시작해서 uSize만큼 늘리기
    vec2 pos;
    pos.x = uPos.x + uv.x * uSize.x;
    pos.y = uPos.y + uv.y * uSize.y;

    gl_Position = vec4(pos, 0.0, 1.0);
}
