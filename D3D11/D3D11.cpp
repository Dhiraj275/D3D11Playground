// D3D11.cpp : Defines the entry point for the application.
//


#include "D3D11.h"
#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <exception>
#include <chrono>
#include <DirectXMath.h>
#include <cmath>
#include <vector>
#include "d3dcompiler.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")



UINT wWidth = 1280u;
UINT wHeight = 720u;
namespace wrl = Microsoft::WRL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void initializeD3D11(HWND hwnd, 
	wrl::ComPtr<IDXGISwapChain>& pSwap, 
	wrl::ComPtr<ID3D11Device>& pDevice, 
	wrl::ComPtr<ID3D11DeviceContext>& pContext,
	wrl::ComPtr<ID3D11RenderTargetView>& pTarget)
{
	// Initialize Direct3D
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = wWidth;
	sd.BufferDesc.Height = wHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;


	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0u,
		nullptr,
		0u,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	);


	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create D3D11 device and swap chain.", "Error", MB_OK);
		return;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
	hr = pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Height = (FLOAT)wHeight;
	vp.Width = (FLOAT)wWidth;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pContext->RSSetViewports(1u, &vp);

}


// Global variables
std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
float angle = 0.0f;
void renderFrame(
	wrl::ComPtr<IDXGISwapChain>& pSwap,
	wrl::ComPtr<ID3D11Device>& pDevice,
	wrl::ComPtr<ID3D11DeviceContext>& pContext,
	wrl::ComPtr<ID3D11RenderTargetView>& pTarget
) {
	angle -= 0.01f;
	HRESULT hr;
	// Clear the backbuffer
	const auto old = last;
	last = std::chrono::steady_clock::now();
	const std::chrono::duration<float> frameTime = last - old;
	float dt = frameTime.count();
	float c = sin(dt) / 2.0f + 0.5f;
	float clearColor[4] = { 0.0f,0.f, 0.0f, 1.0f}; // (R,G,B,A) = bluish color
	pContext->ClearRenderTargetView(pTarget.Get(), clearColor);

	//lets draw a triangle
	//vertex buffer
	
	struct Vertex {
		struct {
			float x, y, z;
		} pos;
	};
	
	Vertex vertices[] = {
		
		{ -1.0f,-1.0f,-1.0f	 },
		{ 1.0f,-1.0f,-1.0f	 },
		{ -1.0f,1.0f,-1.0f	 },
		{ 1.0f,1.0f,-1.0f	  },
		{ -1.0f,-1.0f,1.0f	 },
		{ 1.0f,-1.0f,1.0f	  },
		{ -1.0f,1.0f,1.0f	 },
		{ 1.0f,1.0f,1.0f	 },

	};
	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC vb = {};
	vb.ByteWidth = sizeof(vertices);
	vb.Usage = D3D11_USAGE_DEFAULT;
	vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb.CPUAccessFlags = 0;
	vb.MiscFlags = 0;
	vb.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA vsd = {};
	vsd.pSysMem = vertices;
	
	hr = pDevice->CreateBuffer(&vb, &vsd, &pVertexBuffer);
	
	//bind
	UINT stride = sizeof(Vertex);
	UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(),&stride, &offset);


	//transform c buf
	struct ConstantBuffer {
		DirectX::XMMATRIX transform;
	};

	ConstantBuffer cb = {
		{
				DirectX::XMMatrixTranspose(
				DirectX::XMMatrixRotationZ(angle)*
				DirectX::XMMatrixRotationX(angle)*
				DirectX::XMMatrixTranslation(0.0f,0.0f,10.0f)*
				DirectX::XMMatrixPerspectiveLH(1.0f,9.0f / 16.0f,0.5f,100.0f)
			)
		}
	};
	wrl::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd = {};
	cbd.ByteWidth = sizeof(cb);
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;

	hr = pDevice->CreateBuffer(&cbd,&csd,&pConstantBuffer);

	//bind
	
	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());


	//color constant buffer
	struct ConstantBuffer2
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		} face_colors[6];
	};
	const ConstantBuffer2 cb2 =
	{
		{
			{1.0f,0.0f,1.0f},
			{1.0f,0.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f},
			{1.0f,1.0f,0.0f},
			{0.0f,1.0f,1.0f},
		}
	};
	wrl::ComPtr<ID3D11Buffer> pConstantBuffer2;
	D3D11_BUFFER_DESC cbd2;
	cbd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd2.Usage = D3D11_USAGE_DEFAULT;
	cbd2.CPUAccessFlags = 0u;
	cbd2.MiscFlags = 0u;
	cbd2.ByteWidth = sizeof(cb2);
	cbd2.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd2 = {};
	csd2.pSysMem = &cb2;
	pDevice->CreateBuffer(&cbd2, &csd2, &pConstantBuffer2);

	// bind constant buffer to pixel shader
	pContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf());

	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create vertex buffer.", "Error", MB_OK);
		return;
	}

	const unsigned int indices[] =
	{
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
	};
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ib = {};
	ib.ByteWidth = sizeof(indices);
	ib.Usage = D3D11_USAGE_DEFAULT;
	ib.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ib.CPUAccessFlags = 0;
	ib.MiscFlags = 0;
	ib.StructureByteStride = sizeof(unsigned int);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;

	hr = pDevice->CreateBuffer(&ib, &isd, &pIndexBuffer);

	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create index buffer.", "Error", MB_OK);
		return;
	}

	//bind
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);



	// create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	hr = D3DReadFileToBlob(L"PixelShader.cso", &pBlob);
	if (FAILED(hr)) {
		MessageBoxA(NULL,"Failed Reading Pixel Shader", "", MB_OK);
	}
	
	hr = pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed Creating Pixel Shader", "", MB_OK);
	}
	//bind pixel shader
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);
	
	
	// create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	hr = D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed Reading Vertex Shader", "", MB_OK);
	}

	hr = pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed Creating Vertex Shader", "", MB_OK);
	}
	//bind pixel shader
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);

	//Input Layout
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"Position", 0u, DXGI_FORMAT_R32G32B32_FLOAT,0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u}
	};


	hr = pDevice->CreateInputLayout(	
		ied, (UINT)std::size(ied),
		pBlob->GetBufferPointer(), 
		pBlob->GetBufferSize(),
		&pInputLayout
		);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed Creating Input Layout", "", MB_OK);
	}

	//bind input layout 
	pContext->IASetInputLayout(pInputLayout.Get());


	//render target
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);

	//primitive topology
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//pContext->Draw((UINT)std::size(vertices), 0u);
	pContext->DrawIndexed((UINT)std::size(indices),0u, 0u);

	pSwap->Present(1, 0);
}
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	wchar_t WindowClassName[] = L"Dx3DClass";
	WNDCLASS wc = { };
	wc.lpszClassName = WindowClassName;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WindowProc;
	RegisterClass(&wc);
	HWND hwnd = CreateWindowEx(0u, WindowClassName, L"Agni Reload", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wWidth, wHeight, NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, nCmdShow);
	MSG msg;
	
	wrl::ComPtr<IDXGISwapChain> pSwap;
	wrl::ComPtr<ID3D11Device> pDevice;
	wrl::ComPtr<ID3D11DeviceContext> pContext;
	wrl::ComPtr<ID3D11RenderTargetView> pTarget;
	
	initializeD3D11(hwnd, pSwap,pDevice,pContext,pTarget);
	

	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Only render when there are NO messages
			if (pContext != nullptr) {
				renderFrame(pSwap, pDevice, pContext, pTarget);
			}
		}
	}
	return (int)msg.wParam;
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {


	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}