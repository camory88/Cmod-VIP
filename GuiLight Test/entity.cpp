#include "entity.hpp"
#include "vector.h"

globalVars globals;

Entity::Entity(uint64_t entityPtr)
{
    ptr = entityPtr;
}

Vector Entity::getBonePosition(int id)
{
    Vector position = getPosition();
    uintptr_t boneArray = Read<uintptr_t>(ptr + Offsets::OFFSET_BONES);
    Vector bone = Vector();
    uint32_t boneloc = (id * 0x30);
    bone_t bo = Read<bone_t>(boneArray + boneloc);
    bone.x = bo.x + position.x;
    bone.y = bo.y + position.y;
    bone.z = bo.z + position.z;
    return bone;
}

Vector Entity::getPosition()
{
    return Read<Vector>(ptr + Offsets::OFFSET_ORIGIN);
}

Vector Entity::getCamPosition()
{
    return Read<Vector>(ptr + Offsets::OFFSET_CAMERAPOS);
}

QAngle Entity::getAimPunch()
{
    return Read<QAngle>(ptr + Offsets::OFFSET_AIMPUNCH);
}

QAngle Entity::getViewAngles()
{
    return Read<QAngle>(ptr + Offsets::OFFSET_VIEWANGLES);
}

float Entity::getLastVisTime()
{
    return Read<float>(ptr + Offsets::OFFSET_VISIBLE_TIME);
}
struct visibleTime
{
    float lastTime[100]; // last visible time in float
    bool lastState[100]; // last visibility state
    uintptr_t lastCheck[100]; // last visibility check time
} lastVis;

bool Entity::IsVisible(int index)
{
    if (ptr >= (lastVis.lastCheck[index] + 10)) // only check every 10ms else its gonna flicker
    {
        float visTime = Read<float>(ptr + Offsets::OFFSET_VISIBLE_TIME);
        lastVis.lastState[index] = visTime > lastVis.lastTime[index] || visTime < 0.f && lastVis.lastTime[index] > 0.f;
        lastVis.lastTime[index] = visTime;
        lastVis.lastCheck[index] = ptr;
    }
    return lastVis.lastState[index];
}

//bool Entity::isVisible(CGlobalVars cGlobalVars)
//{
//    return cGlobalVars.curtime - getLastVisTime() < cGlobalVars.interval_per_tick;
//}

float Entity::ToMeters(float x)
{
    return x / 39.62f;
}


bool lol::WorldToScreen(Vector from, float* m_vMatrix, int targetWidth, int targetHeight, Vector& to)
{
    float w = m_vMatrix[12] * from.x + m_vMatrix[13] * from.y + m_vMatrix[14] * from.z + m_vMatrix[15];

    if (w < 0.01f) return false;

    to.x = m_vMatrix[0] * from.x + m_vMatrix[1] * from.y + m_vMatrix[2] * from.z + m_vMatrix[3];
    to.y = m_vMatrix[4] * from.x + m_vMatrix[5] * from.y + m_vMatrix[6] * from.z + m_vMatrix[7];

    float invw = 1.0f / w;
    to.x *= invw;
    to.y *= invw;

    float x = targetWidth / 2;
    float y = targetHeight / 2;

    x += 0.5 * to.x * targetWidth + 0.5;
    y -= 0.5 * to.y * targetHeight + 0.5;

    to.x = x;
    to.y = y;
    to.z = 0;
    return true;
}


DWORD64 lol::GetEntityById(int Ent, DWORD64 base_address)
{
    DWORD64 EntityList = base_address + Offsets::cl_entitylist; //updated
    DWORD64 BaseEntity = Read<DWORD64>(EntityList);
    if (!BaseEntity)
        return NULL;
    return  Read<DWORD64>(EntityList + (Ent << 5));
}

std::vector<uintptr_t> lol::GetPlayers()
{
    std::vector<uintptr_t> vec;
    for (int i = 0; i < 64; i++)
    {
        DWORD64 entity = GetEntityById(i, base_address);
        if (entity == 0)
            continue;
        DWORD64 EntityHandle = Read<DWORD64>(entity + Offsets::OFFSET_NAME);
        std::string Identifier = Read<std::string>(EntityHandle);
        LPCSTR IdentifierC = Identifier.c_str();
        int health = Read<int>(entity + Offsets::OFFSET_HEALTH);
        uint64_t LocalPlayerPtr = Read<uint64_t>(base_address + Offsets::local_player);
        int localTeam = Read<int>(LocalPlayerPtr + Offsets::OFFSET_TEAM);
        if (health == 0) continue;
        if (strcmp(IdentifierC, "player"))
            vec.push_back(entity);
    }
    return vec;
}

float lol::ToMeters(float x)
{
    return x / 39.62f;
}

bool lol::IsAlive(uintptr_t player)
{
    return (Read<int>(player + Offsets::OFFSET_BLEED_OUT_STATE) == 0 && Read<int>(player + Offsets::OFFSET_LIFE_STATE) == 0);
}

uintptr_t lol::GetMs()
{
    static LARGE_INTEGER s_frequency;
    static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    }
    else {
        return GetTickCount64();
    }
}
bool lol::IsValidPlayer(uintptr_t player)
{
    int health = Read<int>(player + Offsets::OFFSET_HEALTH);
    int teamID = Read<int>(player + Offsets::OFFSET_TEAM);

    if (!lol::IsAlive(player)) return false;
    if (health < 0 || health > 100 || teamID < 0 || teamID > 32) return false;

    return true;
}

bool lol::IsVisible(uintptr_t ent, int index)
{
    if (lol::GetMs() >= (lastVis.lastCheck[index] + 10)) // only check every 10ms else its gonna flicker
    {
        float visTime = Read<float>(ent + Offsets::OFFSET_VISIBLE_TIME);
        lastVis.lastState[index] = visTime > lastVis.lastTime[index] || visTime < 0.f && lastVis.lastTime[index] > 0.f;
        lastVis.lastTime[index] = visTime;
        lastVis.lastCheck[index] = lol::GetMs();
    }
    return lastVis.lastState[index];
}

bool lol::IsValidEnemy(uintptr_t player)
{
    int health = Read<int>(player + Offsets::OFFSET_HEALTH);
    int teamID = Read<int>(player + Offsets::OFFSET_TEAM);

    if (health < 0 || health > 100 || teamID < 0 || teamID > 32) return false;
    if (!teamID == Read<int>(globals.localPlayer + Offsets::OFFSET_TEAM)) return false;
    if (!lol::IsAlive(player)) return false;

    return true;
}

Vector lol::GetBonePos(uintptr_t ent, int id)
{
    Vector pos = Read<Vector>(ent + Offsets::OFFSET_ORIGIN);
    uintptr_t bones = Read<uintptr_t>(ent + Offsets::OFFSET_BONES);
    Vector bone = {};
    UINT32 boneloc = (id * 0x30);
    bone_t bo = {};
    bo = Read<bone_t>(bones + boneloc);

    bone.x = bo.x + pos.x;
    bone.y = bo.y + pos.y;
    bone.z = bo.z + pos.z;
    return bone;
}

void lol::PredictPos(uintptr_t target, Vector* bonePos)
{
    uintptr_t actWeaponID = Read<uintptr_t>(globals.localPlayer + Offsets::OFFSET_WEAPON) & 0xFFFF;
    uintptr_t currentWeapon = Read<uintptr_t>(globals.entityList + (actWeaponID << 5));

    if (currentWeapon != 0)
    {
        float bulletSpeed = Read<float>(currentWeapon + Offsets::OFFSET_BULLET_SPEED);
        float bulletGravity = Read<float>(currentWeapon + Offsets::OFFSET_BULLET_SCALE);

        if (bulletSpeed > 1.f)
        {
            Vector muzzle = Read<Vector>(globals.localPlayer + Offsets::OFFSET_CAMERAPOS);
            float time = bonePos->DistTo(muzzle) / bulletSpeed;
            bonePos->z += (700.f * bulletGravity * 0.5f) * (time * time);
            Vector velDelta = (Read<Vector>(target + Offsets::OFFSET_ORIGIN - 0xC) * time);
            bonePos->x += velDelta.x;
            bonePos->y += velDelta.y;
            bonePos->z += velDelta.z;
        }
    }
}
Vector old_aimpunch;
#include "overlay.h"
void cheats::NoRecoil()
{
    uint64_t LocalPlayerPtr = Read<uint64_t>(base_address + Offsets::local_player);

    Vector viewAngles = Read<Vector>(LocalPlayerPtr + Offsets::OFFSET_VIEWANGLES);
    Vector punchAngle = Read<Vector>(LocalPlayerPtr + Offsets::OFFSET_AIMPUNCH);

    // calculate the new angles by adding the punchangle to the viewangles, it is important to subtract the old punch angle
    Vector newAngle = viewAngles + (old_aimpunch - punchAngle) * (globals.rcs / 100.f);;

    newAngle.Normalize(); // clamp angles
    Write<Vector2D>(LocalPlayerPtr + Offsets::OFFSET_VIEWANGLES, { newAngle.x, newAngle.y }); // overwrite old angles

    old_aimpunch = punchAngle;
}

void cheats::glow()
{
    for (uintptr_t& player : lol::GetPlayers())
    {
        Write<int>(player + Offsets::glow_color, RGB(0, 0, 0)); // red
        Write<int>(player + Offsets::glow_enable, 1); // Enable Glow
        Write<int>(player + Offsets::OFFSET_GLOW_THROUGH_WALLS, 2); // Enable Glow Through Walls
        Write<GlowMode>(player + Offsets::glow_type, { 0, 118, 100, 100 }); //Glow Mode
    }
}

uintptr_t lol::GetBestTarget()
{
    if (true)
    {
        float oldDist = FLT_MAX;
        float newDist = 0;
        uintptr_t aimTarget = NULL;
        int r = 50;
        int index = 0;
        for (uintptr_t& player : lol::GetPlayers())
        {
            //int lol = sizeof(player);
            //printf("%i %\n", lol);
            Write<int>(player + Offsets::OFFSET_GLOW_T1, 16656);
            Write<int>(player + Offsets::OFFSET_GLOW_T2, 1193322764);
            Write<int>(player + Offsets::glow_enable, 7);
            Write<int>(player + Offsets::OFFSET_GLOW_THROUGH_WALLS, 2);

            bool visible = lol::IsVisible(player, index);
            index++;

            if (!lol::IsValidEnemy(player)) continue;

            Vector localHead = Read<Vector>(globals.localPlayer + Offsets::OFFSET_CAMERAPOS);
            Vector targetHead = lol::GetBonePos(player, 2);
            Vector entityPos = Read<Vector>(player + Offsets::OFFSET_ORIGIN);
            lol::PredictPos(aimTarget, &targetHead);
            //Vector2D screenMiddle = { globals.windowWH.x / 2.f, globals.windowWH.y / 2.f };

            Vector ScreenPosition;
            //lol::WorldToScreen(entityPos, vMatrix, globals.windowWH.x, globals.windowWH.y, ScreenPosition);

            if (ScreenPosition.z > 0) continue;
            if (!visible) continue;
            //cout << targetHeadScreen.x << "<->" << targetHeadScreen.y << "<->" << targetHeadScreen.z << endl;


            aimTarget = player;
        }
        return aimTarget;
    }
    else
        return NULL;
}

Vector oldPunch;

Vector lol::CalcAngle(Vector src, Vector dst)
{
    Vector origin = dst - src;
    float dist = sqrt(origin.x * origin.x + origin.y * origin.y + origin.z * origin.z);
    Vector angles = { 0.f, 0.f, 0.f };
    angles.x = -asinf(origin.z / dist) * (180 / M_PI);
    angles.y = atan2f(origin.y, origin.x) * (180 / M_PI);
    return angles;
}

void cheats::Aimbot()
{
    // check if localplayer is valid and aimbot is on
    //if (!lol::IsPlayer(globals.localPlayer)) return;

    globals.currentAimTarget = lol::GetBestTarget(); // find closest enemy to crosshair

    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // aimbot key
    {
        if (globals.currentAimTarget != NULL)
        {
            // get positions and predict the enemy position
            Vector localHead = Read<Vector>(globals.localPlayer + Offsets::OFFSET_CAMERAPOS);
            Vector targetHead = lol::GetBonePos(globals.currentAimTarget, globals.AimTarget);
            lol::PredictPos(globals.currentAimTarget, &targetHead);

            // get all the angles
            Vector oldAngle = Read<Vector>(globals.localPlayer + Offsets::OFFSET_VIEWANGLES);
            Vector punchAngle = Read<Vector>(globals.localPlayer + Offsets::OFFSET_AIMPUNCH);
            Vector breathAngle = Read<Vector>(globals.localPlayer + Offsets::OFFSET_BREATH_ANGLES);

            // calculate the new angles
            Vector newAngles = lol::CalcAngle(localHead, targetHead);

            // subtracting punchangles and breath angles
            newAngles -= breathAngle * (100 / 100.f);
            newAngles -= (punchAngle * 0.05f) * (100 / 100.f);
            newAngles += oldAngle * (100 / 100.f);
            oldPunch = punchAngle; // do this so the rcs doesnt jump down after unlocking from the enemy

            newAngles.Normalize(); // clamp angles
            Vector delta = newAngles - oldAngle;
            Write<Vector2D>(globals.localPlayer + Offsets::OFFSET_VIEWANGLES, { newAngles.x, newAngles.y }); // overwrite old angles
        }
    }
}