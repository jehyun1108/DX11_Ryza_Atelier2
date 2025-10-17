#include "Enginepch.h"

Handle SocketSystem::Create(EntityID owner, Handle childTf, Handle parentAnim, const string& boneName, const _float3& offsetPos, const _float3& offsetRot)
{
	auto& animSys = registry.Get<AnimatorSystem>();
	const _uint boneIdx = animSys.GetBoneIdxByName(parentAnim, boneName);
	assert(boneIdx != static_cast<_uint>(-1) && "SocketSystem: bone name not found");
	if (boneIdx == static_cast<_uint>(-1)) return {};

	return Create(owner, childTf, parentAnim, boneIdx, offsetPos, offsetRot);
}

Handle SocketSystem::Create(EntityID owner, Handle childTf, Handle parentAnim, _uint boneIdx, const _float3& offsetPos, const _float3& offsetRot)
{
	auto handle        = CreateComp(owner);
	auto& socket       = *Get(handle);
	socket             = {};
	socket.childTf     = childTf;
	socket.parentAnim  = parentAnim;
	socket.boneIdx     = boneIdx;

    socket.offsetPos   = offsetPos;
    socket.offsetRot   = offsetRot;

    RebuildOffset(socket);
	return handle;
}

void SocketSystem::SetOffsetMat(Handle handle, _fmat mat)
{
	if (auto socket = Get(handle))
	{
		XMStoreFloat4x4(&socket->offsetMat, mat);
		socket->offsetDirty = false;
	}
}

void SocketSystem::SetOffsetMat(Handle handle, const _float4x4& mat)
{
	if (auto socket = Get(handle))
	{
		socket->offsetMat = mat;
		socket->offsetDirty = false;
	}
}

void SocketSystem::SetOffsetPos(Handle handle, _float3 pos)
{
	if (auto socket = Get(handle))
	{
		socket->offsetPos = pos;
		socket->offsetDirty = true;
	}
}

void SocketSystem::SetOffsetRot(Handle handle, _float3 euler)
{
	if (auto socket = pool.Get(handle))
	{
		socket->offsetRot = euler;
		socket->offsetDirty = true;
	}
}

void SocketSystem::Update(float dt)
{
	auto& animSys = registry.Get<AnimatorSystem>();
	auto& tfSys   = registry.Get<TransformSystem>();

	ForEachAlive([&](auto, SocketData& socket)
		{
			if (socket.offsetDirty) RebuildOffset(socket);

			const auto pBoneWorld = animSys.GetBoneWorld(socket.parentAnim, socket.boneIdx);
			if (!pBoneWorld) return;

			const _mat boneWorld = XMLoadFloat4x4(pBoneWorld);
			const _mat offset    = XMLoadFloat4x4(&socket.offsetMat);

			const _float3 vScale = tfSys.GetScale(socket.childTf);
			const _mat scale     = XMMatrixScaling(vScale.x, vScale.y, vScale.z);

			const _mat world = scale * offset * boneWorld;
			tfSys.SetWorld(socket.childTf, world);
		});
}

void SocketSystem::RebuildOffset(SocketData& socket) const
{
	const _mat trans = XMMatrixTranslation(socket.offsetPos.x, socket.offsetPos.y, socket.offsetPos.z);
	const _mat rot = XMMatrixRotationRollPitchYaw(XMConvertToRadians(socket.offsetRot.x),
		                                          XMConvertToRadians(socket.offsetRot.y),
		                                          XMConvertToRadians(socket.offsetRot.z));
	XMStoreFloat4x4(&socket.offsetMat, rot * trans);
	socket.offsetDirty = false;
}

void SocketSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
	auto& animSys = registry.Get<AnimatorSystem>();
	auto& tfSys = registry.Get<TransformSystem>();

	struct NameBuffer { char text[128] = {}; };
	static unordered_map<_uint, NameBuffer> nameBuffers;

    ForEachOwned(id, [&](Handle handle, SocketData& socket)
        {
            ImGui::PushID((int)handle.idx);

            if (ImGui::CollapsingHeader("Socket"))
            {
                ImGui::TextDisabled("childTf: idx=%u gen=%u", socket.childTf.idx, socket.childTf.generation);
                ImGui::TextDisabled("anim   : idx=%u gen=%u", socket.parentAnim.idx, socket.parentAnim.generation);
                ImGui::Text("BoneIdx: %u", socket.boneIdx);

                {
                    NameBuffer& nb = nameBuffers[handle.idx];
                    ImGui::SetNextItemWidth(180.f);
                    ImGui::InputText("Bone Name", nb.text, IM_ARRAYSIZE(nb.text));
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Find & Set"))
                    {
                        const string boneName(nb.text);
                        const _uint foundIdx = animSys.GetBoneIdxByName(socket.parentAnim, boneName);
                        if (foundIdx != (_uint)-1)
                            socket.boneIdx = foundIdx;
                        else
                            ImGui::OpenPopup("BoneNotFound");
                    }
                    if (ImGui::BeginPopup("BoneNotFound"))
                    {
                        ImGui::TextUnformatted("Bone not found.");
                        ImGui::EndPopup();
                    }
                }

                ImGui::SeparatorText("Offset (Local to Bone)");

                {
                    float pos[3] = { socket.offsetPos.x, socket.offsetPos.y, socket.offsetPos.z };
                    if (ImGui::DragFloat3("Pos", pos, 0.1f))
                    {
                        socket.offsetPos = _float3(pos[0], pos[1], pos[2]);
                        socket.offsetDirty = true;
                    }
                }

                {
                    float euler[3] = { socket.offsetRot.x, socket.offsetRot.y, socket.offsetRot.z };
                    if (ImGui::DragFloat3("Rot (deg)", euler, 0.1f))
                    {
                        socket.offsetRot = _float3(euler[0], euler[1], euler[2]);
                        socket.offsetDirty = true;
                    }
                }

                if (ImGui::TreeNodeEx("Offset Matrix", ImGuiTreeNodeFlags_Framed))
                {
                    const _float4x4& m = socket.offsetMat;
                    ImGui::Text("%.3f %.3f %.3f %.3f", m._11, m._12, m._13, m._14);
                    ImGui::Text("%.3f %.3f %.3f %.3f", m._21, m._22, m._23, m._24);
                    ImGui::Text("%.3f %.3f %.3f %.3f", m._31, m._32, m._33, m._34);
                    ImGui::Text("%.3f %.3f %.3f %.3f", m._41, m._42, m._43, m._44);
                    ImGui::TreePop();
                }

                ImGui::Separator();
                if (ImGui::SmallButton("Rebuild Offset Now"))
                    RebuildOffset(socket);

                ImGui::SameLine();
                if (ImGui::SmallButton("Reset Offset (Zero Pos/Rot)"))
                {
                    socket.offsetPos = _float3(0, 0, 0);
                    socket.offsetRot = _float3(0, 0, 0);
                    RebuildOffset(socket);
                }

                ImGui::SameLine();
                if (ImGui::SmallButton("Capture Offset From Current"))
                {
                    const _float4x4* pChildWorld = tfSys.GetWorld(socket.childTf);
                    const _float4x4* pBoneWorld  = animSys.GetBoneWorld(socket.parentAnim, socket.boneIdx);
                    if (pChildWorld && pBoneWorld)
                    {
                        const _mat mWorld     = XMLoadFloat4x4(pChildWorld);
                        const _mat mBoneWorld = XMLoadFloat4x4(pBoneWorld);

                        const _float3 scale = tfSys.GetScale(socket.childTf);
                        const _mat mInvScale = XMMatrixScaling(
                            (fabsf(scale.x) > 1e-6f) ? 1.f / scale.x : 0.f,
                            (fabsf(scale.y) > 1e-6f) ? 1.f / scale.y : 0.f,
                            (fabsf(scale.z) > 1e-6f) ? 1.f / scale.z : 0.f
                        );

                        const _mat mInvBoneWorld = XMMatrixInverse(nullptr, mBoneWorld);
                        const _mat mOffset       = mInvScale * mWorld * mInvBoneWorld;

                        XMStoreFloat4x4(&socket.offsetMat, mOffset);

                        _vec vScale, vQuat, vTrans;
                        XMMatrixDecompose(&vScale, &vQuat, &vTrans, mOffset);

                        socket.offsetPos = _float3(XMVectorGetX(vTrans), XMVectorGetY(vTrans), XMVectorGetZ(vTrans));
                        socket.offsetRot = Utility::ToEuler(vQuat); 
                        socket.offsetDirty = false;
                    }
                }
            }

            ImGui::PopID();
        });
#endif
}