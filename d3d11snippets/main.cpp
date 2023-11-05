#pragma once

#include "main.h"

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

// depth-stencil buffer
ID3D11Texture2D *depthStencilTexture;
ID3D11DepthStencilView *depthStencilView;
ID3D11DepthStencilState *depthStencilState;

int width = 640;
int height = 480;

HWND hWnd;

int checkVal = 0;
#define CHECKOK(fn) if((checkVal = fn) != S_OK) { \
	MessageBox(NULL, DXGetErrorString(checkVal), "CHECKOK", MB_OK | MB_ICONERROR); \
}

#define CHECKOK2(fn, msg) if((checkVal = fn) != S_OK) { \
	MessageBox(NULL, DXGetErrorString(checkVal), msg, MB_OK | MB_ICONERROR); \
}

ID3D11RasterizerState *rs;

ID3D11PixelShader *ps;
ID3D11VertexShader *vs;

ID3D11InputLayout *layout;

ID3D11Buffer *vb;
ID3D11Buffer *ib;
ID3D11Buffer *cb;

int Init(HWND wnd) {
	DXGI_SWAP_CHAIN_DESC scdesc = {0};
	scdesc.BufferCount = 1;
	//scdesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	scdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//scdesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scdesc.OutputWindow = wnd;
	scdesc.Windowed = TRUE;
	scdesc.BufferDesc.Width = width;
	scdesc.BufferDesc.Height = height;
	scdesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scdesc.SampleDesc.Count = 1;
	scdesc.SampleDesc.Quality = 0;

	D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_9_1;

#ifdef _DEBUG
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#endif
	int retVal = 0;
	if((retVal = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, 0, 0, D3D11_SDK_VERSION, &scdesc, &sc, &dev, &level, &ctx)) != S_OK) {
		MessageBox(NULL, DXGetErrorString(retVal), "D3D11CreateDeviceAndSwapChain", MB_OK | MB_ICONERROR);
		PostQuitMessage(1);
		return -1;
	}

	// backbuffer
	sc->GetBuffer(0, __uuidof( ID3D11Texture2D ), (LPVOID*) &backBuffer);
	CHECKOK2(dev->CreateRenderTargetView(backBuffer, 0, &renderTargetView), "CreateRenderTargetView");
	backBuffer->Release();

	// depth stencil buffer
	D3D11_TEXTURE2D_DESC depthDesc = { 0 };
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;

	CHECKOK2(dev->CreateTexture2D(&depthDesc, NULL, &depthStencilTexture), "CreateTexture2D (depth-stencil)");

	//D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = { 0 };
	//dsvDesc.Format = depthDesc.Format;
	//dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	//CHECKOK2(dev->CreateDepthStencilView(depthStencilTexture, &dsvDesc, &depthStencilView), "CreateDepthStencilView");
	CHECKOK2(dev->CreateDepthStencilView(depthStencilTexture, 0, &depthStencilView), "CreateDepthStencilView");

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	CHECKOK2(dev->CreateDepthStencilState(&depthStencilDesc, &depthStencilState), "CreateDepthStencilState");
	
	depthStencilTexture->Release();

	// set targets // moved to rendern
	//ctx->OMSetRenderTargets(1, &renderTargetView, depthStencilView); // add stencil here?

	// viewport // moved to render
	//D3D11_VIEWPORT vp = {0, 0, width, height, 0, 1};
	//ctx->RSSetViewports(1, &vp);

	// raster state
	D3D11_RASTERIZER_DESC rsDesc;
	memset(&rsDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.DepthClipEnable = true;
	rsDesc.FillMode = D3D11_FILL_SOLID;
	//rsDesc.FrontCounterClockwise = false;
	CHECKOK2(dev->CreateRasterizerState(&rsDesc, &rs), "CreateRasterizerState");
	//ctx->RSSetState(rs); // moved to render

	ID3DBlob *blob;

	// pixel
	CHECKOK2(D3DReadFileToBlob(L"pixelShader.cso", &blob), "D3DReadFileToBlob pixelShader.cso");
	CHECKOK2(dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), 0, &ps), "CreatePixelShader");
	blob->Release();

	// vertex
	CHECKOK2(D3DReadFileToBlob(L"vertexShader.cso", &blob), "D3DReadFileToBlob vertexShader.cso");
	CHECKOK2(dev->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), 0, &vs), "CreateVertexShader");
	
	// input layout (needs vertexshader blob)
	CHECKOK2(dev->CreateInputLayout(vertexDesc, sizeof(vertexDesc)/sizeof(vertexDesc[0]), blob->GetBufferPointer(), blob->GetBufferSize(), &layout), "CreateInputLayout");

	blob->Release();

	// set shaders
	ctx->PSSetShader(ps, 0, 0);
	ctx->VSSetShader(vs, 0, 0);

	// create vertexbuffer
	D3D11_BUFFER_DESC vbDesc = {0};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = 56 * sizeof(float); //sizeof(cube);// *sizeof(Vertex);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {0};
	vbData.pSysMem = cube;

	CHECKOK2(dev->CreateBuffer(&vbDesc, &vbData, &vb), "CreateBuffer (vb)");

	// create indexbuffer
	D3D11_BUFFER_DESC ibDesc = { 0 };
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = 36*sizeof(short);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = { 0 };
	ibData.pSysMem = cube_indices;

	CHECKOK2(dev->CreateBuffer(&ibDesc, &ibData, &ib), "CreateBuffer (ib)");

	// create constant buffer
	D3D11_BUFFER_DESC cbDesc = {0};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(Matrices);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA cbData = {0};
	cbData.pSysMem = &matrices;
	CHECKOK2(dev->CreateBuffer(&cbDesc, &cbData, &cb), "CreateBuffer (cb)");

	return 0;
}

void Release() {
	if(cb) {
		cb->Release();
	}

	if (ib) {
		ib->Release();
	}

	if(vb) {
		vb->Release();
	}

	if(layout) {
		layout->Release();
	}

	if (depthStencilState) {
		depthStencilState->Release();
	}

	if (depthStencilTexture) {
		depthStencilTexture->Release();
	}

	if (depthStencilView) {
		depthStencilView->Release();
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

float angle = 0;

void Render() {
	// rasterizer state
	ctx->RSSetState(rs);

	// viewport
	D3D11_VIEWPORT vp = { 0, 0, width, height, 0, 1 };
	ctx->RSSetViewports(1, &vp);

	// set targets
	ctx->OMSetRenderTargets(1, &renderTargetView, depthStencilView); // add stencil here?
	ctx->OMSetDepthStencilState(depthStencilState, FALSE);

	// do render

	float bg[] = {0, 0.2, 0.5, 1.0};
	ctx->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ctx->ClearRenderTargetView(renderTargetView, bg);

	ctx->IASetInputLayout(layout);

	angle++;
	if (angle >= 360.0f)
		angle = angle - 360.0f;

	// update matrices
	D3D11_MAPPED_SUBRESOURCE map = {0};
	ctx->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);

	XMMATRIX m = ::XMMatrixRotationNormal(XMVectorSet(0.25, 0.5, 0.7, 1), XMConvertToRadians(angle)); // ::XMMatrixIdentity();
	XMMATRIX v = ::XMMatrixLookAtLH(XMVectorSet(0,0,5,0), XMVectorSet(0,0,0,0), XMVectorSet(0,1,0,0));
	XMMATRIX p = ::XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), (float)width/(float)height, 0.1f, 100.0f);
	XMMATRIX mvp = m*v*p; //XMMATRIX mvp = ::XMMatrixMultiply(v, p);
	memcpy(map.pData, &mvp, sizeof(float)*16);
	ctx->Unmap(cb, 0);
	ctx->VSSetConstantBuffers(0, 1, &cb);

	// draw object
	UINT32 strides = sizeof(Vertex);
	UINT32 offsets = 0;
	/*ctx->IASetVertexBuffers(0, 1, &vb, &strides, &offsets);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->Draw(3, 0);*/

	ctx->IASetVertexBuffers(0, 1, &vb, &strides, &offsets);
	ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0); // tegra 3 wants shorts and r16?
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->DrawIndexed(36, 0, 0);

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