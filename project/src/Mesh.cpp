#include "pch.h"
#include "Mesh.h"

Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertices, const std::vector<uint32_t>& indices)
{
	m_pEffect = new Effect{ pDevice, L"PosCol3D.fx" };
	m_pTechnique = m_pEffect->GetTechnique();

	CreateLayoutAndBuffers(pDevice, vertices, indices);
}

Mesh::~Mesh()
{
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}
	if (m_pInputLayout)
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}
	if (m_pEffect)
	{
		delete m_pEffect;
		m_pEffect = nullptr;
	}
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
{
	// 1. Set Primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pEffect->GetInputLayout());

	// 3. Set VertexBuffer
	constexpr UINT stride = sizeof(Vertex_PosCol);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// 4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void Mesh::CreateLayoutAndBuffers(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertices, const std::vector<uint32_t>& indices)
{
	// Create Vertex Layout
	static constexpr uint32_t numElements{ 2 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;

	// Create Vertex Buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_PosCol) * uint32_t(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	// Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);

	if (FAILED(result))
		return;

	// Create Index Buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	initData.pSysMem = vertices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;
}