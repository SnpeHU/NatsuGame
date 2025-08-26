#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;
class Player;
struct Rect {
	float left;
	float right;
	float top;
	float bottom;
};
class CameraController {
public:
	CameraController() = default;
	~CameraController() = default;
	void Initialize();
	void Update();

	void SetTarget(Player* target) { target_ = target; }
	const Camera& GetCamera() { return mainCamera_; }
	void SetMovableArea(const Rect& area);
	
	// Calculate the viewport size at a given Z distance
	Vector2 CalculateViewportSize(float distance) const;
	
	// Set camera follow parameters
	void SetFollowSpeed(float speed);
	void SetCameraDistance(float distance);
	
	// New methods for improved camera behavior
	void SetMapBoundsPriority(bool enabled) { mapBoundsPriority_ = enabled; }
	void SetDeadZone(float width, float height);
	void SetInitialPosition(const Vector3& position) { targetPosition_ = position; mainCamera_.translation_ = position; }
	
	// Camera scaling methods
	void SetZoom(float zoom);
	void SetAutoZoomByMapWidth(float mapWidth, float referenceMapWidth = 20.0f);
	void SetZoomRange(float minZoom, float maxZoom);
	
	// Configuration constants
	static const float kZoomScalingFactor;
	static const float kMinValidZoom;
	static const float kMaxValidZoom;
	
private:
	void UpdateCameraZoom();
	bool IsValidMovableArea(const Rect& area) const;
	Vector2 GetCachedViewportSize();
	
	Rect movableArea_; // カメラが移動可能な範囲
	KamataEngine::Camera mainCamera_; // メインカメラ
	Player* target_ = nullptr; // 追従対象のプレイヤー
	Vector3 targetPosition_ = {0.0f, 0.0f, -50.0f}; // カメラの目標位置（スムージング用）
	float followSpeed_ = 0.1f; // カメラの追従速度
	float cameraDistance_ = 50.0f; // カメラとゲーム平面との距離
	
	// New members for improved camera behavior
	bool mapBoundsPriority_ = true; // Prioritize map bounds over character following
	float deadZoneWidth_ = 4.0f; // Dead zone width for camera following
	float deadZoneHeight_ = 4.0f; // Dead zone height for camera following
	
	// Camera zoom/scaling members
	float zoom_ = 1.0f; // Current zoom level (1.0 = default)
	float baseFovAngleY_ = 45.0f * 3.141592654f / 180.0f; // Base field of view angle
	float minZoom_ = 0.3f; // Minimum zoom (more zoomed out)
	float maxZoom_ = 3.0f; // Maximum zoom (more zoomed in)
	
	// Performance optimization members
	bool hasValidMovableArea_ = false; // Cache for movable area validity
	bool invalidateViewportCache_ = true; // Flag to recalculate viewport size
	Vector2 cachedViewportSize_ = {0.0f, 0.0f}; // Cached viewport size
	float cachedDistance_ = 0.0f; // Distance used for cached viewport
};