#pragma once
#include <vector>
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>
#include <Tool/Logger/Logger.h>
#include "ShaderShelf.h"

enum class SamplerID : uint32_t {
	Non = 0,
	Default = 1 << 0,
	ClampT = 1 << 1,
	MirrorT = 1 << 2,
	ClampS = 1 << 3,
	MirrorS = 1 << 4,

	MagNearest = 1 << 5,
	MagLinear = 1 << 6,

	MinNearest = 1 << 7,
	MinLinear = 1 << 8,
	MinNearestMipmapNearest = 1 << 9,
	MinLinearMipmapNearest = 1 << 10,
	MinNearestMipmapLinear = 1 << 11,
	MinLinearMipmapLinear = 1 << 12,

	ClampClamp_MinMagNearest = 1 << 13,
};

uint32_t operator|(SamplerID a, SamplerID b);
uint32_t operator|(uint32_t a, SamplerID b);
bool operator<(SamplerID a, SamplerID b);

struct RootSignatureConfig {
	std::pair<int, int> cbvNums{};							// <Vertex, Pixel>
	std::pair<int, int> srvNums{};							// <Vertex, Pixel> 上限8
	bool useTexture = false;								// テクスチャを使うか
	uint32_t samplers = uint32_t(SamplerID::Default);		//SamplerIDのビットマスク

	bool operator<(const RootSignatureConfig& other) const;
	bool operator==(const RootSignatureConfig& other) const;
};

namespace std {
	template<>
	struct hash<RootSignatureConfig> {
		size_t operator()(const RootSignatureConfig& cfg) const {
			size_t h = 0;
			hash_combine(h, hash<int>()(cfg.cbvNums.first));
			hash_combine(h, hash<int>()(cfg.cbvNums.second));
			hash_combine(h, hash<int>()(cfg.srvNums.first));
			hash_combine(h, hash<int>()(cfg.srvNums.second));
			hash_combine(h, hash<bool>()(cfg.useTexture));
			hash_combine(h, hash<uint32_t>()(cfg.samplers));
			return h;
		}
	private:
		static void hash_combine(size_t& seed, size_t value) {
			seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
}

class RootSignatureShelf {
public:

	RootSignatureShelf(ID3D12Device* device);
	~RootSignatureShelf();

	ID3D12RootSignature* GetRootSignature(const RootSignatureConfig& config);

private:

	ID3D12RootSignature* CreateRootSignature(const RootSignatureConfig& config);

	ID3D12Device* device_ = nullptr;

	std::map<RootSignatureConfig, ID3D12RootSignature*> rootSignatureMap_;
	std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers_;

	std::shared_ptr<spdlog::logger> logger_ = nullptr;

};
