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

    // 초기화
    result = gFmodSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD::System::init 실패\n";
        return false;
    }

    // BGM 로드
    gFmodSystem->createSound("0paze.wav",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmPaze[0]);
    gFmodSystem->createSound("1paze.mp3",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmPaze[1]);
    gFmodSystem->createSound("2paze.mp3",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmPaze[2]);
    gFmodSystem->createSound("3paze.mp3",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmPaze[3]);

    gFmodSystem->createSound("Loading_2.mp3",
        FMOD_DEFAULT | FMOD_LOOP_NORMAL, 0, &gBgmLoading);

    return true;
}

// 하나의 BGM만 재생하도록
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

// 종료
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

// fade out 기능
static bool   gIsFadingOut = false;
static float  gFadeDuration = 0.0f;
static float  gFadeTimer = 0.0f;
static float  gFadeStartVolume = 1.0f;

void StartFadeOutBGM(float duration)
{
    if (!gBgmChannel) return;

    gIsFadingOut = true;
    gFadeDuration = duration;
    gFadeTimer = 0.0f;
    gFadeStartVolume = 1.0f;

    gBgmChannel->getVolume(&gFadeStartVolume);
}

void UpdateFadeOutBGM(float deltaTime)
{
    if (!gIsFadingOut || !gBgmChannel) return;

    gFadeTimer += deltaTime * 1.5f;

    float t = gFadeTimer / gFadeDuration;
    if (t >= 1.0f)
    {
        // 완전히 꺼졌으면 stop
        gBgmChannel->setVolume(0.0f);
        gBgmChannel->stop();
        gBgmChannel = nullptr;
        gIsFadingOut = false;
    }
    else
    {
        float newVol = gFadeStartVolume * (1.0f - t);
        gBgmChannel->setVolume(newVol);
    }
}