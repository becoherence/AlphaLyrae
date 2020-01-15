#include "GameApp.h"
#include "DXUtil.h"
#include "DXTrace.h"
#include "Geometry.h"
#include "DDSTextureLoader.h"

#include "BasicShape.h"

const D3D11_INPUT_ELEMENT_DESC VertexPosNormalTex::inputLayout[3] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC VertexPosNormalColor::inputLayout[3] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

GameApp::GameApp(HINSTANCE hInstance)
	:D3DApp(hInstance),m_VSConstantBuffer(),m_PSConstantBuffer()
	,Cube(nullptr)
{
	m_MainWindowCaption = L"GameApp";
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	if (!InitShader())
	{
		return false;
	}
	if (!InitResource())
	{
		return false;
	}
	return true;
}

void GameApp::SetRenderPipeLine()
{
	D3DApp::SetRenderPipeLine();
}

void GameApp::UpdateScene(float dt)
{
	using namespace DirectX;
	Cube->Update(dt);

	m_VSConstantBuffer.world = Cube->m_WorldMatrix;

	static float phi = 0.0f, theta = 0.0f;
	phi += 0.0001f, theta += 0.00015f;
	XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
	m_VSConstantBuffer.worldInvTranspose = XMMatrixInverse(nullptr, W); // 两次转置可以抵消

	// ...

	// 更新常量缓冲区，让立方体转起来
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dDeviceContext->Map(m_pConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
	m_pd3dDeviceContext->Unmap(m_pConstantBuffers[0].Get(), 0);

	HR(m_pd3dDeviceContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dDeviceContext->Unmap(m_pConstantBuffers[1].Get(), 0);

}

void GameApp::DrawScene()
{
	assert(m_pd3dDeviceContext);
	assert(m_pSwapChain);
	
	static float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// RGBA = (0,0,255,255)
	m_pd3dDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), black);
	m_pd3dDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pd3dDeviceContext->DrawIndexed(Cube->m_IndexCount, 0,0);
	HR(m_pSwapChain->Present(0, 0));

}

bool GameApp::InitShader()
{
	ComPtr<ID3DBlob> blob;
	
	//create VS
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS.cso", L"HLSL\\Basic_VS.hlsl", "main", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

	//绑定InputLayout
	HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(),
		m_pVertexInputLayout.GetAddressOf()));


	HR(CreateShaderFromFile(L"HLSL\\Basic_PS.cso", L"HLSL\\Basic_PS.hlsl", "main", "ps_5_0", blob.ReleaseAndGetAddressOf()));

	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

	return true;
}

bool GameApp::InitResource()
{
	using namespace DirectX;

	//初始化网格模型

	//auto meshData = Geometry::CreateCube<VertexPosNormalTex>();

	//UpdateMesh(meshData);
	Cube = new CubeShape();
	Cube->InitResource(m_pd3dDevice.Get(), m_pd3dDeviceContext.Get());



	//设置常量缓冲区
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(VSConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建用于VS和PS的常量缓冲区
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(PSConstantBuffer);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));


	// 初始化纹理

	
	// 初始化木箱纹理
	//HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, m_pWoodCrate.GetAddressOf()));
	

	// 初始化采样器状态
	//D3D11_SAMPLER_DESC sampDesc;
	//ZeroMemory(&sampDesc, sizeof(sampDesc));
	//sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//sampDesc.MinLOD = 0;
	//sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));


	// ******************

	m_DirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLight.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_DirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLight.Direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);

	// 初始化用于VS的常量缓冲区的值
	m_VSConstantBuffer.world = Cube->m_WorldMatrix;
	m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	));
	m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();

	// 初始化用于PS的常量缓冲区的值
	m_PSConstantBuffer.material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PSConstantBuffer.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_PSConstantBuffer.material.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);

	// 使用默认平行光

	m_PSConstantBuffer.dirLight = m_DirLight;
	m_PSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);


	// 更新PS常量缓冲区资源
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dDeviceContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dDeviceContext->Unmap(m_pConstantBuffers[1].Get(), 0);



	// 初始化光栅化状态

	 // ******************
	// 初始化光栅化状态
	//
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));



	//m_pd3dDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);


	m_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pd3dDeviceContext->IASetInputLayout(m_pVertexInputLayout.Get());

	//VS阶段
	m_pd3dDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	m_pd3dDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());

	//b1
	m_pd3dDeviceContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());



	// 像素着色阶段设置好采样器
	//m_pd3dDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());
	//m_pd3dDeviceContext->PSSetShaderResources(0, 1, m_pWoodCrate.GetAddressOf());
	//PS阶段
	m_pd3dDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);


	return true;
}
//template<class VertexType>
//bool GameApp::UpdateMesh(const Geometry::MeshData<VertexType>& meshData)
//{
//	// 释放旧资源
//	m_pVertexBuffer.Reset();
//	m_pIndexBuffer.Reset();
//
//	// 设置顶点缓冲区描述
//	D3D11_BUFFER_DESC vbd;
//	ZeroMemory(&vbd, sizeof(vbd));
//	vbd.Usage = D3D11_USAGE_IMMUTABLE;
//	vbd.ByteWidth = (UINT)meshData.vertexVector.size() * sizeof(VertexType);
//	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vbd.CPUAccessFlags = 0;
//	// 新建顶点缓冲区
//	D3D11_SUBRESOURCE_DATA InitData;
//	ZeroMemory(&InitData, sizeof(InitData));
//	InitData.pSysMem = meshData.vertexVector.data();
//	HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));
//
//	// 输入装配阶段的顶点缓冲区设置
//	UINT stride = sizeof(VertexType); // 跨越字节数
//	UINT offset = 0;                            // 起始偏移量
//
//	m_pd3dDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
//	// 设置索引缓冲区描述
//	m_IndexCount = (UINT)meshData.indexVector.size();
//	D3D11_BUFFER_DESC ibd;
//	ZeroMemory(&ibd, sizeof(ibd));
//	ibd.Usage = D3D11_USAGE_IMMUTABLE;
//	ibd.ByteWidth = m_IndexCount * sizeof(WORD);
//	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	ibd.CPUAccessFlags = 0;
//	// 新建索引缓冲区
//	InitData.pSysMem = meshData.indexVector.data();
//	HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
//	// 输入装配阶段的索引缓冲区设置
//	m_pd3dDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
//
//	return true;
//
//}
//
