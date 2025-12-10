#pragma once

NS_BEGIN(Engine)

enum class CBufferID { Camera, Light, Obj, Sky, UI, Material, Post, Debug, Distortion, Trail, End };
static constexpr _uint CBufferCount = ENUM(CBUFFERSLOT::END);

class ENGINE_DLL CBufferBank
{
public:
	void     Init(ID3D11Device* dev, ID3D11DeviceContext* ctx, const array<_uint, CBufferCount>& size);
	void     UpdateRaw(CBufferID id, const void* p, _uint bytes);
	void     Bind(CBufferID id, SHADER stages, CBUFFERSLOT slot);
	CBuffer* Get(CBufferID id) const;

	template<typename T>
	void Update(CBufferID id, const T& data)
	{
		auto& b = buffers[ENUM(id)];
		assert(b);
		b->SetData(&data, sizeof(T));
		b->Update();
	}

	template<typename T>
	void UpdateBindT(CBufferID id, const T& data, SHADER stages, CBUFFERSLOT slot)
	{
		Update(id, data);
		Bind(id, stages, slot);
	}

private:
	ID3D11Device*        device{};
	ID3D11DeviceContext* context{};
	array<shared_ptr<CBuffer>, CBufferCount> buffers{};
};

NS_END