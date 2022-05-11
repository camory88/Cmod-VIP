#pragma once
#include "sdk.hpp"
namespace lol
{
    bool IsAlive(uintptr_t player);
    bool IsValidEnemy(uintptr_t player);
    bool IsVisible(uintptr_t ent, int index);
    bool IsValidPlayer(uintptr_t player);
    bool WorldToScreen(Vector from, float* m_vMatrix, int targetWidth, int targetHeight, Vector& to);
    DWORD64 GetEntityById(int Ent, DWORD64 base_address);
    float ToMeters(float x);
    std::vector<uintptr_t> GetPlayers();
    uintptr_t Glow(int r, int g, int b);
    uintptr_t GetBestTarget();
    uintptr_t glowplayers();
    uintptr_t GetMs();
    Vector CalcAngle(Vector src, Vector dst);
    Vector GetBonePos(uintptr_t ent, int id);
    void PredictPos(uintptr_t target, Vector* bonePos);
}

class Entity
{
private:

public:
    Entity(uint64_t ptr);


    //uintptr_t = getEntity();
    uint64_t GetPtr();
    uint64_t ptr = 0;
    
    Vector getBonePosition(int bone);
    Vector getPosition();
    Vector getCamPosition();
    QAngle getSwayAngles();
    QAngle getAimPunch();
    QAngle getViewAngles();
    float  getLastVisTime();
    float  ToMeters(float x);
    //bool   isVisible(SDK::CGlobalVars cGlobalVars);
    bool   IsVisible(int index);
};