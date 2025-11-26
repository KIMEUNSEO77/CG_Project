// hp_vertex shader

#version 330 core

// 0~1 좌표로 들어오는 정점
layout (location = 0) in vec2 aPos;

uniform float uHP;  // 0.0 ~ 1.0

void main()
{
    // 0~1 기준에서 x만 HP 비율대로 줄이기
    vec2 uv = aPos;   // (0~1, 0~1)
    uv.x *= uHP;      // HP가 줄면 오른쪽이 안 채워짐

    // 화면 좌표(NDC)로 변환
    // x: -0.9 ~ -0.3 (width = 0.6)
    // y:  0.85 ~ 0.90 (height = 0.05)
    vec2 pos;
    pos.x = -0.9 + uv.x * 0.6;
    pos.y =  0.85 + uv.y * 0.05;

    gl_Position = vec4(pos, 0.0, 1.0);
}