#pragma once

#include "main.h"

//#define noexcept
//#define constexpr
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
using namespace DirectX;

#include <comdef.h>

const CHAR*  DXGetErrorStringA(HRESULT hr) {
	static char tmp[256] = {0};
	sprintf(tmp, "%ws", DXGetErrorStringW(hr));
	return tmp;
}

#undef DXGetErrorString
#define DXGetErrorString DXGetErrorStringA

ID3D11Device *dev;
ID3D11DeviceContext *ctx;
IDXGISwapChain *sc;

// render buffer
ID3D11Texture2D *backBuffer;
ID3D11RenderTargetView *renderTargetView;

int width = 640;
int height = 480;

HWND hWnd;

int checkVal = 0;
#define CHECKOK(fn) if((checkVal = fn) != S_OK) { \
	MessageBox(NULL, DXGetErrorString(checkVal), "CHECKOK", MB_OK | MB_ICONERROR); \
}

ID3D11RasterizerState *rs;

ID3D11PixelShader *ps;
ID3D11VertexShader *vs;

ID3D11InputLayout *layout;

ID3D11Buffer *vb;
ID3D11Buffer *cb;

int Init(HWND wnd) {
	DXGI_SWAP_CHAIN_DESC scdesc = {0};
	scdesc.BufferCount = 1;
	//scdesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	scdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scdesc.OutputWindow = wnd;
	scdesc.Windowed = TRUE;
	scdesc.BufferDesc.Width = width;
	scdesc.BufferDesc.Height = height;
	scdesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scdesc.SampleDesc.Count = 1;
	scdesc.SampleDesc.Quality = 0;

	D3D_FEATURE_LEVEL level;

	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG;
	int retVal = 0;
	if((retVal = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, 0, 0, D3D11_SDK_VERSION, &scdesc, &sc, &dev, &level, &ctx)) != S_OK) {
		//char str[256] = {0};
		MessageBox(NULL, DXGetErrorString(retVal), "Error", MB_OK | MB_ICONERROR);
		PostQuitMessage(1);
		return -1;
	}

	sc->GetBuffer(0, __uuidof( ID3D11Texture2D ), (LPVOID*) &backBuffer);
	dev->CreateRenderTargetView(backBuffer, 0, &renderTargetView);
	backBuffer->Release();

	ctx->OMSetRenderTargets(1, &renderTargetView, 0); // add stencil here?

	// viewport
	D3D11_VIEWPORT vp = {0, 0, width, height, 0, 1};
	ctx->RSSetViewports(1, &vp);

	// raster state
	D3D11_RASTERIZER_DESC rsDesc;
	memset(&rsDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.DepthClipEnable = true;
	rsDesc.FillMode = D3D11_FILL_SOLID;
	//rsDesc.FrontCounterClockwise = false;
	CHECKOK(dev->CreateRasterizerState(&rsDesc, &rs));
	ctx->RSSetState(rs);

	ID3DBlob *blob;

	// pixel
	CHECKOK(D3DReadFileToBlob(L"pixelShader.cso", &blob));
	CHECKOK(dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), 0, &ps));
	blob->Release();

	// vertex
	CHECKOK(D3DReadFileToBlob(L"vertexShader.cso", &blob));
	CHECKOK(dev->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), 0, &vs));
	
	// input layout (needs vertexshader blob)
	CHECKOK(dev->CreateInputLayout(vertexDesc, sizeof(vertexDesc)/sizeof(vertexDesc[0]), blob->GetBufferPointer(), blob->GetBufferSize(), &layout));

	blob->Release();

	// set shaders
	ctx->PSSetShader(ps, 0, 0);
	ctx->VSSetShader(vs, 0, 0);

	// create vertexbuffer
	D3D11_BUFFER_DESC vbDesc = {0};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(Vertex)*3;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {0};
	vbData.pSysMem = triangle;

	CHECKOK(dev->CreateBuffer(&vbDesc, &vbData, &vb));

	// create constant buffer
	D3D11_BUFFER_DESC cbDesc = {0};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(Matrices);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA cbData = {0};
	cbData.pSysMem = &matrices;
	CHECKOK(dev->CreateBuffer(&cbDesc, &cbData, &cb));

	return 0;
}

void Release() {
	if(cb) {
		cb->Release();
	}

	if(vb) {
		vb->Release();
	}

	if(layout) {
		layout->Release();
	}

	if(ps) {
		ps->Release();
		ps = 0;
	}

	if(vs) {
		vs->Release();
		vs = 0;
	}
	
	if(rs) {
		rs->Release();
	}

	if(sc) {
		sc->Release();
		sc = 0;
	}

	if(ctx) {
		ctx->Release();
		ctx = 0;
	}

	if(dev) {
		dev->Release();
		dev = 0;
	}
}

void Render() {
	float bg[] = {0, 0.2, 0.5, 1.0};
	ctx->ClearRenderTargetView(renderTargetView, bg);

	ctx->IASetInputLayout(layout);

	D3D11_MAPPED_SUBRESOURCE map = {0};
	ctx->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);

	XMMATRIX m = ::XMMatrixIdentity();
	XMMATRIX v = ::XMMatrixLookAtLH(XMVectorSet(0,0,5,0), XMVectorSet(0,0,0,0), XMVectorSet(0,1,0,0));
	//XMMATRIX p = ::XMMatrixPerspectiveLH(width, height, 0.1f, 100.0f);
	XMMATRIX p = ::XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), (float)width/(float)height, 0.1f, 100.0f);
	//XMMATRIX mvp = m*v*p;
	XMMATRIX mvp = ::XMMatrixMultiply(v, p);
	//XMMATRIX mvp = ::XMMatrixMultiply(p, v);
	memcpy(map.pData, &mvp, sizeof(float)*16);
	ctx->Unmap(cb, 0);
	ctx->VSSetConstantBuffers(0, 1, &cb);

	UINT32 strides = sizeof(Vertex);
	UINT32 offsets = 0;
	ctx->IASetVertexBuffers(0, 1, &vb, &strides, &offsets);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->Draw(3, 0);

	sc->Present(0, 0); // if vsync present(1, 0)
}

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_CREATE:
			return Init(hWnd);
		case WM_DESTROY:
			Release();
			return 0;
		case WM_PAINT:
			Render();
			return FALSE;
		case WM_ERASEBKGND:
			return FALSE;
		default:
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	WNDCLASS		wc;
//	RECT			r;
    wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) COLOR_GRAYTEXT;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = "myclass";

    if (!RegisterClass (&wc)) {
		MessageBox(0, "Unable to register window class", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	hWnd = CreateWindowEx(
		 0, //WS_EX_TOPMOST, 
		 wc.lpszClassName,
		 "DX11",
		 WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		 0, 0, 640, 480,
		 NULL,
		 NULL,
		 hInstance,
		 NULL);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg = {0};
	BOOL ret;
	while((ret = GetMessage(&msg, hWnd, 0, 0)) != 0) {
		if(ret == -1)
			break;
		TranslateMessage(&msg); 
		DispatchMessage(&msg);
	}

	return 0;
}