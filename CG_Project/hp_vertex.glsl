// hp_vertex shader

#version 330 core

// 화면 좌표(-1 ~ 1)로 미리 넣어줄 거라 2D만 받기
layout (location = 0) in vec2 aPos;

uniform float uHP; // 0.0 ~ 1.0

void main()
{
    // 기본 위치
    vec2 pos = aPos;

    // 오른쪽 끝 x만 HP에 따라 줄이기
    // aPos.x 가 -0.9 ~ -0.3 라고 하면:
    float left  = -0.9;
    float right = -0.3;
    float width = right - left;

    // 원래 x가 right일 때만 줄어들게 간단히 처리
    if (abs(aPos.x - right) < 0.0001)
        pos.x = left + width * uHP;

    gl_Position = vec4(pos, 0.0, 1.0);
}