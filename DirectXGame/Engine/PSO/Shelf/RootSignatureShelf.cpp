#include "RootSignatureShelf.h"
#include <cassert>

RootSignatureShelf::RootSignatureShelf(ID3D12Device* device) {
	logger_ = getLogger("Engine");
	device_ = device;
	rootSignatureMap_.clear();

#pragma region CreateSamplers

	D3D12_STATIC_SAMPLER_DESC base{};
	base.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	base.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	base.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	base.MinLOD = 0.0f;
	base.MaxLOD = D3D12_FLOAT32_MAX;
	base.MipLODBias = 0.0f;
	base.MaxAnisotropy = 1;
	base.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	base.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	base.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	base.ShaderRegister = 0;
	base.RegisterSpace = 0;
	base.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC def = base;
	samplers_[SamplerID::Default] = def;

	D3D12_STATIC_SAMPLER_DESC clampT = base;
	clampT.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers_[SamplerID::ClampT] = clampT;

	D3D12_STATIC_SAMPLER_DESC mirrorT = base;
	mirrorT.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	samplers_[SamplerID::MirrorT] = mirrorT;

	D3D12_STATIC_SAMPLER_DESC clampS = base;
	clampS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers_[SamplerID::ClampS] = clampS;

	D3D12_STATIC_SAMPLER_DESC mirrorS = base;
	mirrorS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	samplers_[SamplerID::MirrorS] = mirrorS;

	D3D12_STATIC_SAMPLER_DESC magNearest = base;
	magNearest.Filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	samplers_[SamplerID::MagNearest] = magNearest;

	D3D12_STATIC_SAMPLER_DESC magLinear = base;
	magLinear.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers_[SamplerID::MagLinear] = magLinear;

	D3D12_STATIC_SAMPLER_DESC minNearest = base;
	minNearest.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	samplers_[SamplerID::MinNearest] = minNearest;

	D3D12_STATIC_SAMPLER_DESC minLinear = base;
	minLinear.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers_[SamplerID::MinLinear] = minLinear;

	D3D12_STATIC_SAMPLER_DESC minNearestMipNearest = base;
	minNearestMipNearest.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplers_[SamplerID::MinNearestMipmapNearest] = minNearestMipNearest;

	D3D12_STATIC_SAMPLER_DESC minLinearMipNearest = base;
	minLinearMipNearest.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	samplers_[SamplerID::MinLinearMipmapNearest] = minLinearMipNearest;

	D3D12_STATIC_SAMPLER_DESC minNearestMipLinear = base;
	minNearestMipLinear.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	samplers_[SamplerID::MinNearestMipmapLinear] = minNearestMipLinear;

	D3D12_STATIC_SAMPLER_DESC minLinearMipLinear = base;
	minLinearMipLinear.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers_[SamplerID::MinLinearMipmapLinear] = minLinearMipLinear;

	D3D12_STATIC_SAMPLER_DESC clampClampMinMagNearest = base;
	clampClampMinMagNearest.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampClampMinMagNearest.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampClampMinMagNearest.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplers_[SamplerID::ClampClamp_MinMagNearest] = clampClampMinMagNearest;

#pragma endregion
}

RootSignatureShelf::~RootSignatureShelf() {
	for (auto& [config, rootSignature] : rootSignatureMap_) {
		if (rootSignature) {
			rootSignature->Release();
		}
	}
}

ID3D12RootSignature* RootSignatureShelf::GetRootSignature(const RootSignatureConfig& config) {
	auto it = rootSignatureMap_.find(config);
	if (it == rootSignatureMap_.end()) {
		return CreateRootSignature(config);
	}

	return rootSignatureMap_[config];
}



ID3D12RootSignature* RootSignatureShelf::CreateRootSignature(const RootSignatureConfig& config) {
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//RootParameter作成
	std::vector<D3D12_ROOT_PARAMETER> rootParameters = {};
	uint32_t registerIndex;

	// === CBV ===
	//VertexShader
	registerIndex = 0;
	for (int i = 0; i < config.cbvNums.first; ++i) {
		rootParameters.emplace_back();
		rootParameters.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;        //CBVを使う
		rootParameters.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;    //VertexShaderで使う
		rootParameters.back().Descriptor.ShaderRegister = registerIndex++;          //レジスタ番号0とバインド
	}
	registerIndex = 0;
	for (int i = 0; i < config.cbvNums.second; ++i) {
		rootParameters.emplace_back();
		rootParameters.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;        //CBVを使う
		rootParameters.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;     //PixelShaderで使う
		rootParameters.back().Descriptor.ShaderRegister = registerIndex++;          //レジスタ番号0とバインド
	}

	// === SRV ===
	//VertexShader
	//DescriptorTableでまとめる
	D3D12_DESCRIPTOR_RANGE vertexSrvDescriptor[1] = {};
	vertexSrvDescriptor[0].BaseShaderRegister = 0;
	vertexSrvDescriptor[0].NumDescriptors = config.srvNums.first;
	vertexSrvDescriptor[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	vertexSrvDescriptor[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	if (config.srvNums.first > 0) {
		rootParameters.emplace_back();
		rootParameters.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		rootParameters.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;                //VertexShaderで使う
		rootParameters.back().DescriptorTable.pDescriptorRanges = vertexSrvDescriptor;         //テーブルの中身
		rootParameters.back().DescriptorTable.NumDescriptorRanges = _countof(vertexSrvDescriptor); //テーブルの数
	}

	//PixelShader
	//DescriptorTableでまとめる
	D3D12_DESCRIPTOR_RANGE pixelSrvDescriptor[1] = {};
	pixelSrvDescriptor[0].BaseShaderRegister = 0;
	pixelSrvDescriptor[0].NumDescriptors = config.srvNums.second;
	pixelSrvDescriptor[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pixelSrvDescriptor[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	if (config.srvNums.second > 0) {
		rootParameters.emplace_back();
		rootParameters.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		rootParameters.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                //PixelShaderで使う
		rootParameters.back().DescriptorTable.pDescriptorRanges = pixelSrvDescriptor;         //テーブルの中身
		rootParameters.back().DescriptorTable.NumDescriptorRanges = _countof(pixelSrvDescriptor); //テーブルの数
	}

	// === Texture ===
	D3D12_DESCRIPTOR_RANGE textureDescriptor[1] = {};
	textureDescriptor[0].BaseShaderRegister = 8;
	textureDescriptor[0].NumDescriptors = 1024;
	textureDescriptor[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textureDescriptor[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	if (config.useTexture) {

		rootParameters.emplace_back();
		rootParameters.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;           //SRVを使う
		rootParameters.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                       //全Shaderから見れる
		rootParameters.back().DescriptorTable.pDescriptorRanges = textureDescriptor;                //テーブルの中身
		rootParameters.back().DescriptorTable.NumDescriptorRanges = _countof(textureDescriptor);    //テーブルの数
	}

	descriptionRootSignature.NumParameters = UINT(rootParameters.size());
	descriptionRootSignature.pParameters = rootParameters.data();

	// === Sampler ===
	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
	if (config.samplers > 0) {

		for (const auto& [id, desc] : samplers_) {
			if (config.samplers & uint32_t(id)) {
				staticSamplers.push_back(desc);
			}
		}

		descriptionRootSignature.NumStaticSamplers = UINT(staticSamplers.size());
		descriptionRootSignature.pStaticSamplers = staticSamplers.data();
	}

	//作成
	Microsoft::WRL::ComPtr<ID3D10Blob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	//シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		logger_->error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	//バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device_->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	rootSignatureMap_[config] = rootSignature;

	return rootSignature;
}

uint32_t operator|(SamplerID a, SamplerID b) {
	return uint32_t(a) | uint32_t(b);
}

uint32_t operator|(uint32_t a, SamplerID b) {
	return a | uint32_t(b);
}

bool operator<(SamplerID a, SamplerID b) {
	return uint32_t(a) < uint32_t(b);
}

bool RootSignatureConfig::operator<(const RootSignatureConfig& other) const {
	if (cbvNums != other.cbvNums) return cbvNums < other.cbvNums;
	if (srvNums != other.srvNums) return srvNums < other.srvNums;
	if (useTexture != other.useTexture) return useTexture < other.useTexture;
	return samplers < other.samplers;
}

bool RootSignatureConfig::operator==(const RootSignatureConfig& other) const {
	return cbvNums == other.cbvNums &&
		srvNums == other.srvNums &&
		useTexture == other.useTexture &&
		samplers == other.samplers;
}
