#include "RenderObject.h"
#include <Utility/DirectUtilFuncs.h>
#include <Utility/ConvertString.h>

DXDevice* RenderObject::device_ = nullptr;
Logger RenderObject::logger_ = getLogger("Engine");

RenderObject::RenderObject(std::string debugName) {
	debugName_ = debugName == "" ? "NoName" : debugName;
	drawData_.reserve(4);
	logger_->debug("RenderObject Created: {}", debugName_);
}

RenderObject::~RenderObject() {
	logger_->debug("RenderObject Destroyed: {}", debugName_);
}

void RenderObject::StaticInitialize(DXDevice* device) {
	device_ = device;
}

void RenderObject::Initialize() {
	logger_->debug("RenderObject Initialized: {}", debugName_);
	index_ = 0;
	psoConfig_ = PSOConfig{};
}

void RenderObject::SetDrawData(const DrawData& data) {
	drawData_.clear();
	drawData_.push_back(data);
	logger_->debug("Draw Data Set: {}", debugName_);
}

void RenderObject::SetDrawData(const std::vector<DrawData>& datas) {
	drawData_ = datas;
	logger_->debug("Draw Data Set: {} ({} sets)", debugName_, datas.size());
}

int RenderObject::CreateCBV(size_t size, ShaderType type, std::string debugName) {
	std::vector<BufferData> cbvBufferDatas;
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> cbvAddresses;

	for (int i = 0; i < 2; ++i) {
		Resource res{};
		res.res.Attach(CreateBufferResource(device_->GetDevice(), (size + 255) & ~255));	//256の倍数に揃える
		res.res->SetName(ConvertString(debugName).c_str());
		resources_.push_back(res);

		//マッピング
		BufferData bufferData{};
		bufferData.size = size;
		HRESULT hr = res.res->Map(0, nullptr, &bufferData.mapped);
		assert(SUCCEEDED(hr));
		cbvBufferDatas.push_back(bufferData);

		//CBVアドレスの取得
		cbvAddresses.push_back(res.res->GetGPUVirtualAddress());
	}

	bufferDatas_.push_back(cbvBufferDatas);
	cbvAddresses_.push_back(cbvAddresses);

	logger_->debug("CBV Created: {}", debugName_);
	logger_->debug("  Size: {}", size);

	if (type == ShaderType::VERTEX_SHADER) {
		psoConfig_.rootConfig.cbvNums.first++;
	} else {
		psoConfig_.rootConfig.cbvNums.second++;
	}

	//indexの交付
	return int(bufferDatas_.size() - 1);
}

int RenderObject::CreateSRV(size_t size, uint32_t num, ShaderType type, std::string debugName) {
	std::vector<BufferData> srvBufferDatas;
	std::vector<std::unique_ptr<SRVHandle>> srvHandles;

	auto srv = device_->GetSRVManager();
	for (int i = 0; i < 2; ++i) {
		Resource res{};
		size_t bufferSize = size;
		res.res.Attach(CreateBufferResource(device_->GetDevice(), bufferSize * num));	//256の倍数に揃えたうえでnum倍
		res.res->SetName(ConvertString(debugName).c_str());
		resources_.push_back(res);

		//SRVハンドルの取得
		std::unique_ptr<SRVHandle> srvHandle = std::make_unique<SRVHandle>();
		srvHandle->UpdateHandle(srv);

		//ParticleDataのSRV作成
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.NumElements = num;
		srvDesc.Buffer.StructureByteStride = UINT(bufferSize);

		device_->GetDevice()->CreateShaderResourceView(res.res.Get(), &srvDesc, srvHandle->GetCPU());

		srvHandles.push_back(std::move(srvHandle));

		//マッピング
		BufferData bufferData{};
		bufferData.size = size * num;
		res.res->Map(0, nullptr, &bufferData.mapped);
		srvBufferDatas.push_back(bufferData);
	}

	bufferDatas_.push_back(srvBufferDatas);
	srvHandles_.push_back(std::move(srvHandles));

	logger_->debug("SRV Created: {}", debugName_);
	logger_->debug("  Size: {}", srvBufferDatas.front().size);
	logger_->debug("  Num: {}", num);
	logger_->debug("  Index: {}", srvHandles_.back().back()->GetOffset());

	if (type == ShaderType::VERTEX_SHADER) {
		psoConfig_.rootConfig.srvNums.first++;
	} else {
		psoConfig_.rootConfig.srvNums.second++;
	}

	return int(bufferDatas_.size() - 1);
}

void RenderObject::CopyBufferData(int index, const void* data, size_t size) {
	auto& gpuData = bufferDatas_[index][index_];

	if (size > gpuData.size) {
		logger_->error("=========== CopyBufferData || Size exceeds buffer size ===========");
		logger_->error("  RenderObject: {}\n  GPUSize: {}\n  DataSize: {}", debugName_, gpuData.size, size);
		assert(false && "RenderObject::CopyBufferData: Size exceeds buffer size");
		return;
	}

	std::memcpy(gpuData.mapped, data, size);
}

void RenderObject::Draw(Window* window) {

	if (drawData_.empty()) {
		logger_->error("=========== Draw || No draw data set ===========");
		logger_->error("  RenderObject: {}", debugName_);
		assert(false && "RenderObject::Draw: No draw data set");
		return;
	}

	for (const auto& data : drawData_) {

		//描画できる状態か確認
		if (data.vbv.size() == 0) {
			logger_->error("=========== Draw || No vertex buffer set ===========");
			logger_->error("  RenderObject: {}", debugName_);
			assert(false && "RenderObject::Draw: No vertex buffer set");
			return;
		}
		if (data.ibv.BufferLocation == 0) {
			logger_->error("=========== Draw || No index buffer set ===========");
			logger_->error("  RenderObject: {}", debugName_);
			assert(false && "RenderObject::Draw: No index buffer set");
			return;
		}

		auto cmdList = window->GetCommandObject()->GetCommandList();

		//パイプラインステートの設定
		device_->SetPSO(cmdList, psoConfig_);

		//頂点バッファの設定
		cmdList->IASetVertexBuffers(0, UINT(data.vbv.size()), data.vbv.data());
		cmdList->IASetIndexBuffer(&data.ibv);

		//bufferの設定
		UINT rootIndex = 0;
		for (size_t i = 0; i < cbvAddresses_.size(); ++i) {
			cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbvAddresses_[i][index_]);
		}
		for (size_t i = 0; i < srvHandles_.size(); ++i) {
			cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srvHandles_[i][index_]->GetGPU());
		}
		if (psoConfig_.rootConfig.useTexture) {
			auto textureHandle = device_->GetSRVManager()->GetStartPtr();
			cmdList->SetGraphicsRootDescriptorTable(rootIndex++, textureHandle);
		}

		//描画コマンド
		cmdList->DrawIndexedInstanced(data.indexNum, instanceNum_, 0, 0, 0);
	}
}

void RenderObject::SetUseTexture(bool useTexture) {
	psoConfig_.rootConfig.useTexture = useTexture;
}
