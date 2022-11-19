#pragma once

#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "dxerr.h"

int Init();
void Render();
void Release();

struct Vertex {
	/*union {
		struct {
			float x,y,z;
		};
		float vertex[3];
	};
	union {
		struct {
			float r,g,b,a;
		};
		float color[4];
	};*/
	float v[7];
};

D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

/*Vertex triangle[] = {
	{-1, -1, 0,		1, 0, 0, 1},
	{1, -1, 0,		0, 1, 0, 1},
	{0, 1, 0,		0, 0, 1, 1}
};*/

Vertex cube[] = {
	{-1, -1, -1,	1, 0, 0, 1},
	{1, -1, -1,		0, 1, 0, 1},
	{1, 1, -1,		1, 0, 1, 1},
	{-1, 1, -1,		1, 1, 0, 1},

	{-1, -1, 1,		0, 0, 1, 1},
	{1, -1, 1,		0, 1, 0, 1},
	{1, 1, 1,		1, 0, 0, 1},
	{-1, 1, 1,		0, 1, 1, 1},
};

unsigned short cube_indices[] = {
	0,1,3,	1,2,3,
	4,5,7,	5,6,7,
	4,7,0,	7,3,0,
	5,6,1,	6,2,1,
	0,4,1,	5,1,4,
	3,7,6,	6,2,3
};

struct Matrices {
	float modelViewProjection[16];
};

Matrices matrices = {
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};