#include "uisystem_internal.h"

int MouseIndexFromCode(ButtonCode_t code)
{
    const int index = code - MOUSE_LEFT;
    return (index < 0 || index > 2) ? 0 : index;
}

bool FingerToPixels(const InputEvent_t& event, int& outX, int& outY)
{
    if (!g_pMaterialSystem)
        return false;

    int w = 0, h = 0;
    g_pMaterialSystem->GetBackBufferDimensions(w, h);

    float nx = 0.f, ny = 0.f;
    std::memcpy(&nx, &event.m_nData2, sizeof(nx));
    std::memcpy(&ny, &event.m_nData3, sizeof(ny));

    outX = static_cast<int>(w * nx);
    outY = static_cast<int>(h * ny);
    return true;
}
