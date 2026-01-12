#include "DrawDataManager.h"
#include <Utility/DirectUtilFuncs.h>

void DrawDataManager::Initialize(DXDevice* device) {
	device_ = device;
	drawDatas_.clear();
	resources_.clear();
	logger_ = getLogger("Engine");
	logger_->info("DrawDataManager Initialized");
}

void DrawDataManager::AddIndexBuffer(std::vector<uint32_t> indices) {
	Resource res{};
	size_t bufferSize = ((sizeof(uint32_t) * indices.size() + 255) & ~255);
	res.res.Attach(CreateBufferResource(device_->GetDevice(), bufferSize));	//256の倍数に揃えたうえでnum倍
	resources_.push_back(res);

	//データ転送
	void* mapped = nullptr;
	HRESULT hr = res.res->Map(0, nullptr, &mapped);
	assert(SUCCEEDED(hr));
	std::memcpy(mapped, indices.data(), sizeof(uint32_t) * indices.size());
	res.res->Unmap(0, nullptr);
	
	//IBV作成
	D3D12_INDEX_BUFFER_VIEW ibv{};
	ibv.BufferLocation = res.res->GetGPUVirtualAddress();
	ibv.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices.size());
	ibv.Format = DXGI_FORMAT_R32_UINT;
	
	indexBufferView_ = ibv;
	indexNum_ = static_cast<uint32_t>(indices.size());

	logger_->debug("Index Buffer Added: Size {} bytes", ibv.SizeInBytes);
}

int DrawDataManager::CreateDrawData() {
	if (vertexBufferViews_.empty()) {
		logger_->error("CreateDrawData || No vertex buffer added");
		assert(false && "DrawDataManager::CreateDrawData: No vertex buffer added");
	}
	if(indexBufferView_.SizeInBytes == 0) {
		logger_->warn("No Index Buffer View. Creating default index buffer.");
		std::vector<uint32_t> defaultIndices(vertexNum_);
		for (uint32_t i = 0; i < vertexNum_; ++i) {
			defaultIndices[i] = i;
		}
		AddIndexBuffer(defaultIndices);
	}

	DrawData drawData{};
	drawData.vbv = vertexBufferViews_;
	drawData.ibv = indexBufferView_;
	drawData.vertexNum = vertexNum_;
	drawData.indexNum = indexNum_;
	drawDatas_.push_back({ drawData, mapData_ });
	logger_->debug("DrawData Created: VertexCount {}, IndexCount {}", vertexNum_, drawData.indexNum);

	//使用したVBVとIBVをクリア
	vertexBufferViews_.clear();
	indexBufferView_ = {};
	vertexNum_ = 0;
	indexNum_ = 0;

	return static_cast<int>(drawDatas_.size() - 1);
}

std::vector<DrawData> DrawDataManager::GetDrawData(const std::vector<int>& indices) const {
	std::vector<DrawData> result;

	for(const auto& index : indices) {
		assert(index >= 0 && index < static_cast<int>(drawDatas_.size()) && "DrawDataManager::GetDrawData: Invalid DrawData index");
		result.push_back(drawDatas_[index].first);
	}

	return result;
}
