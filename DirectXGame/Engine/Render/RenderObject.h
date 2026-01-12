#pragma once
#include "DrawDataManager.h"
#include <Screen/Window.h>
#include <Utility/Color.h>
#include <Camera/Camera.h>

class RenderObject {
public:

	RenderObject(std::string debugName = "");
	~RenderObject();

	static void StaticInitialize(DXDevice* device);

	void Initialize();

	void SetDrawData(const DrawData& data);
	void SetDrawData(const std::vector<DrawData>& datas);
	int CreateCBV(size_t size, ShaderType type, std::string debugName = "");
	int CreateSRV(size_t size, uint32_t num, ShaderType type, std::string debugName = "");

	void CopyBufferData(int index, const void* data, size_t size);

	void Draw(Window* window);

	void SetUseTexture(bool useTexture);

	PSOConfig psoConfig_{};
	uint32_t instanceNum_ = 1;

private://static

	static DXDevice* device_;
	static Logger logger_;

private://データ管理

	std::vector<DrawData> drawData_{};

	struct BufferData {
		void* mapped = nullptr;
		size_t size = 0;
	};
	std::vector<std::vector<BufferData>> bufferDatas_{};

	std::vector<std::vector<D3D12_GPU_VIRTUAL_ADDRESS>> cbvAddresses_{};
	std::vector<std::vector<std::unique_ptr<SRVHandle>>> srvHandles_{};

	//スワップチェーン対策
	int index_ = 0;

private://寿命管理

	struct Resource {
		Microsoft::WRL::ComPtr<ID3D12Resource> res = nullptr;
	};
	std::vector<Resource> resources_{};

private://Debug

	std::string debugName_;

};
