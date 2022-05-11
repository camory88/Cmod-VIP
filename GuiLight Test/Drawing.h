#pragma once
#include "overlay.h"
#include "Drawing.h"

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

void DrawFilledRectangle(int x, int y, int w, int h, ImColor color);
void DrawBorderBox(int x, int y, int x2, int y2, int thickness, ImColor color);
void DrawLine(int x1, int y1, int x2, int y2, int thickness, ImColor color);
void DrawCircle(int x, int y, float radius, float width, float rez, ImColor color);