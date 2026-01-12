#pragma once
#include <Core/DXDevice.h>
#include <Utility/DirectUtilFuncs.h>

struct DrawData {
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vbv;
	D3D12_INDEX_BUFFER_VIEW ibv;
	uint32_t vertexNum;
	uint32_t indexNum;
};

class DrawDataManager {
public:

	void Initialize(DXDevice* device);
	
	template <typename T>
	void AddVertexBuffer(const std::vector<T>& data);
	void AddIndexBuffer(std::vector<uint32_t> indices);
	int CreateDrawData();

	DrawData GetDrawData(int index) const {
		assert(index >= 0 && index < static_cast<int>(drawDatas_.size()) && "DrawDataManager::GetDrawData: Invalid DrawData index");
		return drawDatas_[index].first;
	}
	std::vector<DrawData> GetDrawData(const std::vector<int>& indices) const;

	//即席
	void CoppyBufferData(int drawDataIndex, const void* data, size_t size) {
		assert(drawDataIndex >= 0 && drawDataIndex < static_cast<int>(drawDatas_.size()) && "DrawDataManager::CoppyBufferData: Invalid DrawData index");
		MapData& mapData = drawDatas_[drawDataIndex].second;
		assert(size <= mapData.size && "DrawDataManager::CoppyBufferData: Data size exceeds mapped buffer size");
		std::memcpy(mapData.mapped, data, size);
	}

private:

	DXDevice* device_ = nullptr;
	Logger logger_ = nullptr;

	struct MapData {
		void* mapped = nullptr;
		size_t size = 0;
	};
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	MapData mapData_;

	uint32_t vertexNum_ = 0;
	uint32_t indexNum_ = 0;
	std::vector<std::pair<DrawData, MapData>> drawDatas_;

	struct Resource {
		Microsoft::WRL::ComPtr<ID3D12Resource> res;
	};
	std::vector<Resource> resources_;

};

template<typename T>
inline void DrawDataManager::AddVertexBuffer(const std::vector<T>& data) {
	Resource res{};
	size_t bufferSize = sizeof(T) * data.size();
	res.res.Attach(CreateBufferResource(device_->GetDevice(), bufferSize));	//256の倍数に揃えたうえでnum倍
	resources_.push_back(res);

	//データ転送
	mapData_.size = bufferSize;
	
	HRESULT hr = res.res->Map(0, nullptr, &mapData_.mapped);
	assert(SUCCEEDED(hr));
	std::memcpy(mapData_.mapped, data.data(), mapData_.size);
	res.res->Unmap(0, nullptr);
	
	//VBV作成
	D3D12_VERTEX_BUFFER_VIEW vbv{};
	vbv.BufferLocation = res.res->GetGPUVirtualAddress();
	vbv.SizeInBytes = static_cast<UINT>(bufferSize);
	vbv.StrideInBytes = sizeof(T);

	vertexBufferViews_.push_back(vbv);
	vertexNum_ = static_cast<int>(data.size());

	logger_->debug("Vertex Buffer Added: {} vertices", data.size());
}
