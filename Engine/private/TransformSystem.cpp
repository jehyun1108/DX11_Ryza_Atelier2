#include "Enginepch.h"

static void ImGui_ShowMatrix4x4(const char* label, const _float4x4& m)
{
#ifdef USE_IMGUI
	if (ImGui::TreeNodeEx(label,ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap))
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
	struct MoveInfo { _uint axis; float sign; };
	constexpr MoveInfo moveDir[(size_t)MOVE::END] = 
	{
		{0,  1.f}, {0, -1.f}, // Forward, Back
		{1, -1.f}, {1,  1.f}, // Left, Right
		{2,  1.f}, {2, -1.f}, // Up , Down
	};

	inline _vec GetAxisVec(const TransformData& tf, _uint axis)
	{
		const _vec quat = XMLoadFloat4(&tf.rot);
		if (axis == 0) return XMVector3Rotate(Utility::Look(),  quat);
		if (axis == 1) return XMVector3Rotate(Utility::Right(), quat);
		               return XMVector3Rotate(Utility::Up(),    quat);
	}
	
	inline _vec MakeYawPitchQuat(float yawRad, float pitchRad)
	{
		// 1. Yaw/Pitch로 부터 Look 벡터 직접 생성
		const float cosYaw   = cosf(yawRad);
		const float sinYaw   = sinf(yawRad);
		const float cosPitch = cosf(pitchRad);
		const float sinPitch = sinf(pitchRad);

		_vec look = XMVectorSet(sinYaw * cosPitch, sinPitch, cosYaw * cosPitch, 0.f);

		// 2. 직교 기저 재구성 : Right, Up
		_vec upWorld = Utility::Up();
		_vec right = XMVector3Cross(upWorld, look);

		// 3. 특이점 근처 (look ~ upWorld 평행) 보정
		const float rightLength = XMVectorGetX(XMVector3LengthSq(right));
		if (rightLength < 1e-12f)
		{
			// upWorld가 거의 평행이면, world Right를 사용해 보정
			_vec alt = Utility::Right();
			right = XMVector3Cross(alt, look);
		}

		right = XMVector3Normalize(right);
		_vec up = XMVector3Normalize(XMVector3Cross(look, right));

		// 4. 회전 행렬 재구성
		_mat rotMat = XMMatrixIdentity();
		rotMat.r[0] = right;
		rotMat.r[1] = up;
		rotMat.r[2] = look;

		// 5. 행렬 -> 쿼터니언 정규화
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
	if (desc.speed)    tf.speed = desc.speed.value();
	if (desc.rotSpeed) tf.rotSpeed = desc.rotSpeed.value();

	return handle;
}

void TransformSystem::Update(float dt)
{
	ForEachAlive([&](_uint, TransformData& tf) 
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

void TransformSystem::AddRotation(Handle handle, _fvec axis, float radian)
{
	if (auto tf = Get(handle))
	{
		_vec vAxis = axis;
		const float len2 = XMVectorGetX(XMVector3LengthSq(vAxis));

		if (len2 < 1e-20f || !isfinite(radian)) return;

		vAxis      = XMVector3Normalize(vAxis);
		_vec delta = XMQuaternionRotationAxis(vAxis, radian);

		_vec cur = QuatNormalize(XMLoadFloat4(&tf->rot));
		_vec out = XMQuaternionNormalize(XMQuaternionMultiply(delta, cur)); 
		XMStoreFloat4(&tf->rot, out);
		tf->dirty = true;
	}
}

void TransformSystem::AddRotation(Handle handle, _fvec deltaQuat)
{
	if (auto tf = Get(handle))
	{
		_vec quat = QuatNormalize(deltaQuat);
		_vec cur = QuatNormalize(XMLoadFloat4(&tf->rot));
		_vec out = XMQuaternionNormalize(XMQuaternionMultiply(quat, cur));
		XMStoreFloat4(&tf->rot, out);
		tf->dirty = true;
	}
}

void TransformSystem::SetSpeed(Handle handle, float speed)
{
	if (auto tf = Get(handle))
		tf->speed = speed;
}

void TransformSystem::Move(Handle handle, MOVE move, float dt)
{
	if (auto tf = Get(handle))
	{
		const auto info = moveDir[(size_t)move];
		const _vec dir = GetAxisVec(*tf, info.axis);
		_vec pos = XMLoadFloat3(&tf->pos);
		pos = XMVectorMultiplyAdd(dir, XMVectorReplicate(tf->speed * dt * info.sign), pos);
		XMStoreFloat3(&tf->pos, pos);
		tf->dirty = true;
	}
}

void TransformSystem::LookAt(Handle handle, _fvec targetPos)
{
	if (auto tf = Get(handle))
	{
		const _vec vPos = XMLoadFloat3(&tf->pos);
		const _vec vLook = XMVector3Normalize(targetPos - vPos);

		_vec worldUp = Utility::Up();
		_vec vRight = XMVector3Cross(worldUp, vLook);

		float length = XMVectorGetX(XMVector3LengthSq(vRight));
		if (length < 1e-12f)
		{
			worldUp = Utility::Right();
			vRight = XMVector3Cross(worldUp, vLook);
		}

		vRight = XMVector3Normalize(vRight);
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

			const ImGuiTreeNodeFlags flags =
				ImGuiTreeNodeFlags_DefaultOpen |
				ImGuiTreeNodeFlags_Framed |
				ImGuiTreeNodeFlags_AllowItemOverlap;

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
