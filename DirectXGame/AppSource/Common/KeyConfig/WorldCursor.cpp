#include "WorldCursor.h"

Vector3 GetWorldCursor(Camera* camera, Vector2 cursorPos) {
	// 1. スクリーン座標をNDC（正規化デバイス座標）に変換
	// NDCは[-1, 1]の範囲（左下が(-1,-1)、右上が(1,1)）
	Vector2 ndc;
	constexpr float screenWidth = 1280.0f;
	constexpr float screenHeight = 720.0f;
	ndc.x = (cursorPos.x / screenWidth) * 2.0f - 1.0f;
	ndc.y = 1.0f - (cursorPos.y / screenHeight) * 2.0f; // Y軸を反転して変換

	// 2. ビュー行列とプロジェクション行列の逆行列を取得
	Matrix4x4 viewMatrix = camera->GetTransformMatrix();

	Matrix4x4 invView = viewMatrix.Inverse();
	Matrix4x4 invProj = camera->GetProjectionMatrix().Inverse();

	// 3. NDCからビュー空間への変換（Near平面とFar平面）
	Vector4 nearPoint = Vector4(ndc.x, ndc.y, 0.0f, 1.0f); // Near平面
	Vector4 farPoint = Vector4(ndc.x, ndc.y, 1.0f, 1.0f);  // Far平面

	// プロジェクション逆変換
	Vector4 nearView = {
		nearPoint.x * invProj.m[0][0] + nearPoint.y * invProj.m[1][0] + nearPoint.z * invProj.m[2][0] + nearPoint.w * invProj.m[3][0],
		nearPoint.x * invProj.m[0][1] + nearPoint.y * invProj.m[1][1] + nearPoint.z * invProj.m[2][1] + nearPoint.w * invProj.m[3][1],
		nearPoint.x * invProj.m[0][2] + nearPoint.y * invProj.m[1][2] + nearPoint.z * invProj.m[2][2] + nearPoint.w * invProj.m[3][2],
		nearPoint.x * invProj.m[0][3] + nearPoint.y * invProj.m[1][3] + nearPoint.z * invProj.m[2][3] + nearPoint.w * invProj.m[3][3]
	};
	nearView = nearView / nearView.w; // 同次座標を正規化

	Vector4 farView = {
		farPoint.x * invProj.m[0][0] + farPoint.y * invProj.m[1][0] + farPoint.z * invProj.m[2][0] + farPoint.w * invProj.m[3][0],
		farPoint.x * invProj.m[0][1] + farPoint.y * invProj.m[1][1] + farPoint.z * invProj.m[2][1] + farPoint.w * invProj.m[3][1],
		farPoint.x * invProj.m[0][2] + farPoint.y * invProj.m[1][2] + farPoint.z * invProj.m[2][2] + farPoint.w * invProj.m[3][2],
		farPoint.x * invProj.m[0][3] + farPoint.y * invProj.m[1][3] + farPoint.z * invProj.m[2][3] + farPoint.w * invProj.m[3][3]
	};
	farView = farView / farView.w; // 同次座標を正規化

	// 4. ビュー空間からワールド空間への変換
	Vector3 nearWorld = {
		nearView.x * invView.m[0][0] + nearView.y * invView.m[1][0] + nearView.z * invView.m[2][0] + invView.m[3][0],
		nearView.x * invView.m[0][1] + nearView.y * invView.m[1][1] + nearView.z * invView.m[2][1] + invView.m[3][1],
		nearView.x * invView.m[0][2] + nearView.y * invView.m[1][2] + nearView.z * invView.m[2][2] + invView.m[3][2]
	};

	Vector3 farWorld = {
		farView.x * invView.m[0][0] + farView.y * invView.m[1][0] + farView.z * invView.m[2][0] + invView.m[3][0],
		farView.x * invView.m[0][1] + farView.y * invView.m[1][1] + farView.z * invView.m[2][1] + invView.m[3][1],
		farView.x * invView.m[0][2] + farView.y * invView.m[1][2] + farView.z * invView.m[2][2] + invView.m[3][2]
	};

	// 5. レイの方向を計算
	Vector3 rayDirection = (farWorld - nearWorld).Normalize();

	// 6. レイとY=0平面の交点を求める
	// 平面の方程式: Y = 0
	// レイの方程式: P = nearWorld + t * rayDirection
	// 交点: nearWorld.y + t * rayDirection.y = 0
	// t = -nearWorld.y / rayDirection.y

	if (std::abs(rayDirection.y) < 0.0001f) {
		return { 0.0f, 0.0f, 0.0f }; // レイが平面と平行な場合、適当な値を設定
	}

	float t = -nearWorld.y / rayDirection.y;
	Vector3 intersectionPoint = nearWorld + rayDirection * t;
	return intersectionPoint;
}
