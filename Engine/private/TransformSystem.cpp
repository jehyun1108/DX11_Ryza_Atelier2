#include "Enginepch.h"

static void ImGui_ShowMatrix4x4(const char* label, const _float4x4& m)
{
#ifdef USE_IMGUI
	if (ImGui::TreeNodeEx(label,ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap))
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.6f);

		float r0[4] = { m._11, m._12, m._13, m._14 };
		float r1[4] = { m._21, m._22, m._23, m._24 };
		float r2[4] = { m._31, m._32, m._33, m._34 };
		float r3[4] = { m._41, m._42, m._43, m._44 };

		ImGui::InputFloat4("Row0", r0, "%.3f");
		ImGui::InputFloat4("Row1", r1, "%.3f");
		ImGui::InputFloat4("Row2", r2, "%.3f");
		ImGui::InputFloat4("Row3", r3, "%.3f");

		ImGui::PopStyleVar();
		ImGui::PopItemFlag();
		ImGui::TreePop();
	}
#endif
}

namespace
{
	inline _vec MakeYawPitchQuat(float yawRad, float pitchRad)
	{
		const float cosYaw   = cosf(yawRad);
		const float sinYaw   = sinf(yawRad);
		const float cosPitch = cosf(pitchRad);
		const float sinPitch = sinf(pitchRad);

		_vec look = XMVectorSet(sinYaw * cosPitch, sinPitch, cosYaw * cosPitch, 0.f);

		_vec upWorld = Utility::Up();
		_vec right = XMVector3Cross(upWorld, look);

		const float rightLength = XMVectorGetX(XMVector3LengthSq(right));
		if (rightLength < 1e-12f)
		{
			_vec alt = Utility::Right();
			right = XMVector3Cross(alt, look);
		}

		right = XMVector3Normalize(right);
		_vec up = XMVector3Normalize(XMVector3Cross(look, right));

		_mat rotMat = XMMatrixIdentity();
		rotMat.r[0] = right;
		rotMat.r[1] = up;
		rotMat.r[2] = look;

		_vec quat = XMQuaternionRotationMatrix(rotMat);
		quat = XMQuaternionNormalize(quat);
		return quat;
	}

	static inline _vec QuatNormalize(_fvec quatIn)
	{
		_vec quat = quatIn;
		const float len2 = XMVectorGetX(XMVector4LengthSq(quat));
		if (len2 < 1e-20f)
			return XMQuaternionIdentity();
		return XMQuaternionNormalize(quat);
	}

	static float ComputeYawRadWorldForward(const _float3& worldForward)
	{
		return atan2f(worldForward.x, worldForward.z);
	}

	static constexpr int modelForwardSign = -1; // +Z ? -Z?
}

Handle TransformSystem::Create(EntityID owner, const TransformDesc& desc)
{
	Handle handle = CreateComp(owner);
	auto& tf = *Get(handle);
	tf = {};
	XMStoreFloat4(&tf.rot, XMQuaternionIdentity());
	XMStoreFloat4x4(&tf.world, XMMatrixIdentity());
	tf.dirty = true;

	if (desc.pos)      tf.pos = desc.pos.value();
	if (desc.scale)    tf.scale = desc.scale.value();
	if (desc.rot)      tf.rot = desc.rot.value();
	if (desc.rotSpeed) tf.rotSpeed = desc.rotSpeed.value();

	return handle;
}

void TransformSystem::Update(float dt)
{
	ForEachAliveEx([&](Handle handle, EntityID owner, TransformData& tf)
		{
			if (tf.dirty)
				UpdateWorld(tf);
		});
}

void TransformSystem::UpdateWorld(TransformData& tf)
{
	const _vec vPos   = XMLoadFloat3(&tf.pos);
    const _vec vScale = XMLoadFloat3(&tf.scale);
    const _vec vRot   = XMLoadFloat4(&tf.rot);

    const _mat mScale = XMMatrixScalingFromVector(vScale);
    const _mat mRot   = XMMatrixRotationQuaternion(vRot);
    const _mat mTrans = XMMatrixTranslationFromVector(vPos);

	XMStoreFloat4x4(&tf.world, mScale * mRot * mTrans);
	tf.dirty = false;
}

void TransformSystem::SetPos(Handle handle, _fvec pos)
{
	if (auto tf = Get(handle))
	{
		XMStoreFloat3(&tf->pos, pos);
		tf->dirty = true;
	}
}

void TransformSystem::SetPos(Handle handle, float x, float y, float z)
{
	if (auto tf = Get(handle))
	{
		tf->pos = _float3(x, y, z);
		tf->dirty = true;
	}
}

void TransformSystem::SetPos(Handle handle, _float3 pos)
{
	if (auto tf = Get(handle))
	{
		tf->pos = pos;
		tf->dirty = true;
	}
}

_float3 TransformSystem::GetPos(Handle handle) const
{
	if (auto tf = Get(handle))
		return tf->pos;
	return {};
}

void TransformSystem::SetScale(Handle handle, _fvec scale)
{
	if (auto tf = Get(handle))
	{
		XMStoreFloat3(&tf->scale, scale);
		tf->dirty = true;
	}
}

void TransformSystem::SetScale(Handle handle, float x, float y, float z)
{
	if (auto tf = Get(handle))
	{
		tf->scale = _float3(x, y, z);
		tf->dirty = true;
	}
}

void TransformSystem::SetEuler(Handle handle, float pitch, float yaw, float roll)
{
	if (auto tf = Get(handle))
	{
     	const float rPitch = XMConvertToRadians(pitch);
		const float rYaw   = XMConvertToRadians(yaw);
		const float rRoll  = XMConvertToRadians(roll);
		
		_vec quat =  XMQuaternionRotationRollPitchYaw(rPitch, rYaw, rRoll);
		quat = XMQuaternionNormalize(quat);
		XMStoreFloat4(&tf->rot, quat);
		tf->dirty = true;
	}
}

void TransformSystem::SetRotation(Handle handle, float yawRad, float pitchRad)
{
	if (auto tf = Get(handle))
	{
		const _vec quat = MakeYawPitchQuat(yawRad, pitchRad);
		XMStoreFloat4(&tf->rot, quat);
		tf->dirty = true;
	}
}

void TransformSystem::SetQuat(Handle handle, _fvec quat)
{
	if (auto tf = Get(handle))
	{
		_vec q = QuatNormalize(quat);
		XMStoreFloat4(&tf->rot, q);
		tf->dirty = true;
	}
}

void TransformSystem::SetForwardXZ(Handle handle, const _float3& worldPos)
{
	if (auto tf = Get(handle))
	{
		_float3 forwardXZ{ worldPos.x, 0.f, worldPos.z };
		const float lenSq = forwardXZ.x * forwardXZ.x + forwardXZ.z * forwardXZ.z;
		if (lenSq < 1e-8f) return;
		const float invLen = 1.f / sqrtf(lenSq);
		forwardXZ = { forwardXZ.x * invLen, 0.f, forwardXZ.z * invLen };

		const _float3 upWorld{ 0.f, 1.f, 0.f };

		_vec vForward = XMLoadFloat3(&forwardXZ);
		vForward = XMVectorScale(vForward, static_cast<float>(modelForwardSign));
		_vec vUp = XMLoadFloat3(&upWorld);
		_vec vRight = XMVector3Normalize(XMVector3Cross(vUp, vForward));

		if (XMVectorGetX(XMVector3LengthSq(vRight)) < 1e-12f)
		{
			_vec vAlt = Utility::Right();
			vRight = XMVector3Normalize(XMVector3Cross(vAlt, vForward));
		}
		_vec vUpOrtho = XMVector3Normalize(XMVector3Cross(vForward, vRight));

		_mat rotMat = XMMatrixIdentity();
		rotMat.r[0] = vRight;
		rotMat.r[1] = vUpOrtho;
		rotMat.r[2] = vForward;

		_vec quat = XMQuaternionNormalize(XMQuaternionRotationMatrix(rotMat));
		XMStoreFloat4(&tf->rot, quat);
		tf->dirty = true;
	}
}

void TransformSystem::SetForward(Handle handle, const _float3& worldPos, const _float3& upWorld)
{
	if (auto tf = Get(handle))
	{
		_vec vForward = XMLoadFloat3(&const_cast<_float3&>(worldPos));
		vForward = XMVectorScale(vForward, static_cast<float>(modelForwardSign));
		const float fLen = XMVectorGetX(XMVector3Length(vForward));
		if (fLen < 1e-8f) return;
		vForward = XMVectorScale(vForward, 1.f / fLen);

		_vec vUp = XMLoadFloat3(&const_cast<_float3&>(upWorld));
		const float uLen = XMVectorGetX(XMVector3Length(vUp));
		vUp = (uLen < 1e-8f) ? Utility::Up() : XMVectorScale(vUp, 1.f / uLen);

		_vec vRight = XMVector3Normalize(XMVector3Cross(vUp, vForward));
		if (XMVectorGetX(XMVector3LengthSq(vRight)) < 1e-12f)
		{
			_vec vAlt = Utility::Right();
			vRight = XMVector3Normalize(XMVector3Cross(vAlt, vForward));
		}
		_vec vUpOrtho = XMVector3Normalize(XMVector3Cross(vForward, vRight));

		_mat rotMat = XMMatrixIdentity();
		rotMat.r[0] = vRight;
		rotMat.r[1] = vUpOrtho;
		rotMat.r[2] = vForward;

		_vec quat = XMQuaternionNormalize(XMQuaternionRotationMatrix(rotMat));
		XMStoreFloat4(&tf->rot, quat);
		tf->dirty = true;
	}
}

_float2 TransformSystem::GetForwardXZ(Handle handle) const
{
	const _float3 fwd = GetForward(handle);
	_float2 fwdXZ{ fwd.x, fwd.z };

	const float lenSq = fwdXZ.x * fwdXZ.x + fwdXZ.y * fwdXZ.y;
	if (lenSq < 1e-8f)
		return _float2(0.f, 1.f); 

	const float invLen = 1.f / sqrtf(lenSq);
	return _float2{ fwdXZ.x * invLen, fwdXZ.y * invLen };
}

_float3 TransformSystem::GetForward(Handle handle) const
{
	const auto tf = Get(handle);
	if (!tf)
		return _float3(0.f, 0.f, 1.f);

	const _vec quat = XMLoadFloat4(&tf->rot);
	_vec worldLook = XMVector3Rotate(Utility::Look(), quat);
	worldLook = XMVectorScale(worldLook, static_cast<float>(modelForwardSign));

	_float3 forward{};
	XMStoreFloat3(&forward, worldLook);
	return forward; 
}

_float4 TransformSystem::GetRot(Handle handle) const
{
	if (auto tf = Get(handle))
		return tf->rot;
	return {};
}

void TransformSystem::AddWorldOffset(Handle handle, const _float3& dtWorld)
{
	if (auto tf = Get(handle))
	{
		_vec pos = XMLoadFloat3(&tf->pos);
		_vec deltaVec = XMLoadFloat3(&dtWorld);
		pos = XMVectorAdd(pos, deltaVec);
		XMStoreFloat3(&tf->pos, pos);
		tf->dirty = true;
	}
}

void TransformSystem::AddLocalOffset(Handle handle, const _float3& dtLocal)
{
	if (auto tf = Get(handle))
	{
		_vec vLocalDt = XMLoadFloat3(&const_cast<_float3&>(dtLocal));
		_vec vRot     = XMLoadFloat4(&tf->rot);
		_vec vWorldDt = XMVector3Rotate(vLocalDt, vRot);

		_vec vPos = XMLoadFloat3(&tf->pos);
		vPos      = XMVectorAdd(vPos, vWorldDt);

		XMStoreFloat3(&tf->pos, vPos);
		tf->dirty = true;
	}
}

void TransformSystem::LookAt(Handle handle, _fvec targetPos)
{
	if (auto tf = Get(handle))
	{
		const _vec vPos  = XMLoadFloat3(&tf->pos);
		const _vec vLook = XMVector3Normalize(targetPos - vPos);

		_vec worldUp = Utility::Up();
		_vec vRight  = XMVector3Cross(worldUp, vLook);

		float length = XMVectorGetX(XMVector3LengthSq(vRight));
		if (length < 1e-12f)
		{
			worldUp = Utility::Right();
			vRight  = XMVector3Cross(worldUp, vLook);
		}

		vRight         = XMVector3Normalize(vRight);
		const _vec vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

		_mat rotMat = XMMatrixIdentity();
		rotMat.r[0] = vRight;
		rotMat.r[1] = vUp;
		rotMat.r[2] = vLook;

		XMStoreFloat4(&tf->rot, XMQuaternionRotationMatrix(rotMat));
		tf->dirty = true;
	}
}

void TransformSystem::SetWorld(Handle handle, _fmat world)
{
	if (auto tf = Get(handle))
	{
		_vec outScale, outRot, outTrans;
		if (XMMatrixDecompose(&outScale, &outRot, &outTrans, world))
		{
			XMStoreFloat3(&tf->scale, outScale);
			XMStoreFloat3(&tf->pos, outTrans);
			outRot = QuatNormalize(outRot);
			XMStoreFloat4(&tf->rot, outRot);
		}
		XMStoreFloat4x4(&tf->world, world);
		tf->dirty = false;
	}
}

void TransformSystem::SetWorld(Handle handle, const _float4x4& world)
{
	SetWorld(handle, XMLoadFloat4x4(&world));
}

_float3 TransformSystem::GetScale(Handle handle) const
{
	const auto tf = Get(handle);
	return tf ? tf->scale : _float3(1, 1, 1);
}

const _float4x4* TransformSystem::GetWorld(Handle handle) const
{
	auto tf = Get(handle);
	return tf ? &tf->world : nullptr;
}

PlanarBasisXZ TransformSystem::GetPlanarBasisXZ(Handle handle) const
{
	PlanarBasisXZ out{};
	const auto tf = Get(handle);
	if (!tf) return out;

	const _vec vRot   = XMLoadFloat4(&tf->rot);
	_vec       vRight = XMVector3Rotate(Utility::Right(), vRot);
	_vec       vLook  = XMVector3Rotate(Utility::Look(), vRot);

	_float3 right, forward;
	XMStoreFloat3(&right, vRight);
	XMStoreFloat3(&forward, vLook);

	_float2 rightXZ   = { right.x, right.z };
	_float2 forwardXZ = { forward.x, forward.z };

	rightXZ   = Utility::Normalize(rightXZ);
	forwardXZ = Utility::Normalize(forwardXZ);

	if (forwardXZ.x * forwardXZ.x + forwardXZ.y * forwardXZ.y <= 1e-12f)
		forwardXZ = _float2{ -rightXZ.y, rightXZ.x };

	{
		const float dot2 = forwardXZ.x * rightXZ.x + forwardXZ.y * rightXZ.y;
		forwardXZ = _float2{ forwardXZ.x - dot2 * rightXZ.x, forwardXZ.y - dot2 * rightXZ.y };
		forwardXZ = Utility::Normalize(forwardXZ);

		if (forwardXZ.x * forwardXZ.x + forwardXZ.y * forwardXZ.y <= 1e-12f)
			forwardXZ = _float2{ -rightXZ.y, rightXZ.x };
	}

	out.rightXZ   = rightXZ;
	out.forwardXZ = forwardXZ;
	
	return out;
}

_vec TransformSystem::GetRight(Handle handle) const
{
	const auto tf = Get(handle);
	if (!tf) return Utility::Right();
	const _vec quat = XMLoadFloat4(&tf->rot);
	return XMVector3Rotate(Utility::Right(), quat);
}

_vec TransformSystem::GetUp(Handle handle) const
{
	const auto tf = Get(handle);
	if (!tf) return Utility::Up();
	const _vec quat = XMLoadFloat4(&tf->rot);
	return XMVector3Rotate(Utility::Up(), quat);
}

_vec TransformSystem::GetLook(Handle handle) const
{
	const auto tf = Get(handle);
	if (!tf) return Utility::Look();
	const _vec quat = XMLoadFloat4(&tf->rot);
	return XMVector3Rotate(Utility::Look(), quat);
}

void TransformSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
	ForEachOwned(id, [&](Handle handle, TransformData& tf)
		{
			ImGui::PushID((int)handle.idx);

			const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap;

			if (ImGui::TreeNodeEx("Transform", flags))
			{
				bool changed = false;

				changed |= ImGui::DragFloat3("Pos", &tf.pos.x, 0.1f);

				_float3 eulerDeg = Utility::ToEuler(tf.rot);
				_float3 edited = eulerDeg;

				if (ImGui::DragFloat3("Rot", &edited.x, 1.f, -360.f, 360.f))
					SetEuler(handle, edited.x, edited.y, edited.z);

				changed |= ImGui::DragFloat3("Scale", &tf.scale.x, 0.05f);
				if (changed)
					tf.dirty = true;

				ImGui_ShowMatrix4x4("World Matrix", tf.world);

				ImGui::TreePop();
			}
			ImGui::PopID();
		});
#endif
}
