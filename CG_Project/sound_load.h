#pragma once

#include "fmod.hpp"
#include <iostream>

FMOD::System* gFmodSystem = nullptr;

FMOD::Sound* gBgmPaze[4] = { nullptr, nullptr, nullptr, nullptr };
FMOD::Sound* gBgmLoading = nullptr;

FMOD::Channel* gBgmChannel = nullptr;
int gLastBGMStage = 9999;  // 마지막으로 재생한 스테이지 기억

bool InitSound()
{
    FMOD_RESULT result;

    // FMOD 시스템 생성
    result = FMOD::System_Create(&gFmodSystem);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD::System_Create 실패\n";
        return false;
    }

    // 초기화 (채널 32개 정도면 충분)
    result = gFmodSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD::System::init 실패\n";
        return false;
    }

    // ==== BGM 로드 ====
    // 실행 파일(.exe)과 같은 폴더에 paze0~3, Loading.mp3 있다고 가정
    gFmodSystem->createSound("paze0.wav",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmPaze[0]);
    gFmodSystem->createSound("paze1.mp3",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmPaze[1]);
    gFmodSystem->createSound("paze2.mp3",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmPaze[2]);
    gFmodSystem->createSound("paze3.mp3",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmPaze[3]);

    gFmodSystem->createSound("Loading.mp3",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmLoading);

    return true;
}

// 하나의 BGM만 재생하도록 도와주는 함수
void PlayBGM(FMOD::Sound* bgm)
{
    if (!gFmodSystem) return;

    // 이전에 재생 중이던 BGM 정지
    if (gBgmChannel)
    {
        gBgmChannel->stop();
        gBgmChannel = nullptr;
    }

    if (bgm)
    {
        gFmodSystem->playSound(bgm, nullptr, false, &gBgmChannel);
    }
}

// (선택) 종료 시 정리용
void ReleaseSound()
{
    for (int i = 0; i < 4; ++i)
    {
        if (gBgmPaze[i])
        {
            gBgmPaze[i]->release();
            gBgmPaze[i] = nullptr;
        }
    }
    if (gBgmLoading)
    {
        gBgmLoading->release();
        gBgmLoading = nullptr;
    }

    if (gFmodSystem)
    {
        gFmodSystem->close();
        gFmodSystem->release();
        gFmodSystem = nullptr;
    }
}