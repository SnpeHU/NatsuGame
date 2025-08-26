#include "CameraController.h"
#include "Player.h"
#include "Easing.h"
#include <algorithm>
#include <cmath>
using namespace KamataEngine;

// Define static constants
const float CameraController::kZoomScalingFactor = 0.6f;
const float CameraController::kMinValidZoom = 0.1f;
const float CameraController::kMaxValidZoom = 10.0f;

void CameraController::Initialize() {
	mainCamera_.Initialize();
	// Initialize camera position
	targetPosition_ = mainCamera_.translation_;
	targetPosition_.z = -cameraDistance_;
	
	// Store the base field of view angle
	baseFovAngleY_ = mainCamera_.fovAngleY;
	
	// Initialize cache
	invalidateViewportCache_ = true;
}

void CameraController::SetMovableArea(const Rect& area) {
	movableArea_ = area;
	hasValidMovableArea_ = IsValidMovableArea(area);
}

void CameraController::SetFollowSpeed(float speed) {
	if (speed < 0.001f) {
		followSpeed_ = 0.001f;
	} else if (speed > 1.0f) {
		followSpeed_ = 1.0f;
	} else {
		followSpeed_ = speed;
	}
}

void CameraController::SetCameraDistance(float distance) {
	if (distance > 0.0f) {
		cameraDistance_ = distance;
		invalidateViewportCache_ = true;
	}
}

void CameraController::SetDeadZone(float width, float height) {
	deadZoneWidth_ = (width > 0.0f) ? width : 0.0f;
	deadZoneHeight_ = (height > 0.0f) ? height : 0.0f;
}

void CameraController::SetZoom(float zoom) {
	float newZoom = zoom;
	if (newZoom < minZoom_) newZoom = minZoom_;
	if (newZoom > maxZoom_) newZoom = maxZoom_;
	
	if (std::abs(zoom_ - newZoom) > 0.001f) {
		zoom_ = newZoom;
		UpdateCameraZoom();
	}
}

void CameraController::SetZoomRange(float minZoom, float maxZoom) {
	if (minZoom > 0.0f && maxZoom > minZoom) {
		minZoom_ = minZoom;
		maxZoom_ = maxZoom;
	}
}

Vector2 CameraController::CalculateViewportSize(float distance) const {
	// Calculate viewport size based on camera's field of view and aspect ratio
	float halfFov = mainCamera_.fovAngleY * 0.5f;
	float viewportHeight = 2.0f * distance * tanf(halfFov);
	float viewportWidth = viewportHeight * mainCamera_.aspectRatio;
	
	return Vector2(viewportWidth, viewportHeight);
}

Vector2 CameraController::GetCachedViewportSize() {
	// Only recalculate if cache is invalid or distance changed
	if (invalidateViewportCache_ || std::abs(cachedDistance_ - cameraDistance_) > 0.001f) {
		cachedViewportSize_ = CalculateViewportSize(cameraDistance_);
		cachedDistance_ = cameraDistance_;
		invalidateViewportCache_ = false;
	}
	return cachedViewportSize_;
}

bool CameraController::IsValidMovableArea(const Rect& area) const {
	return (area.left != 0.0f || area.right != 0.0f || 
			area.top != 0.0f || area.bottom != 0.0f) &&
		   (area.right > area.left) && (area.top > area.bottom);
}

void CameraController::UpdateCameraZoom() {
	// Clamp zoom to valid range
	if (zoom_ < minZoom_) zoom_ = minZoom_;
	if (zoom_ > maxZoom_) zoom_ = maxZoom_;
	
	// Update field of view based on zoom
	// Smaller zoom = larger FOV (zoomed out), larger zoom = smaller FOV (zoomed in)
	mainCamera_.fovAngleY = baseFovAngleY_ / zoom_;
	
	// Invalidate viewport cache since FOV changed
	invalidateViewportCache_ = true;
}

void CameraController::SetAutoZoomByMapWidth(float mapWidth, float referenceMapWidth) {
	// Input validation
	if (mapWidth <= 0.0f || referenceMapWidth <= 0.0f) {
		return;
	}
	
	// Calculate zoom based on map width
	// If map is wider than reference, zoom out (smaller zoom value)
	// If map is narrower than reference, zoom in (larger zoom value)
	float autoZoom = referenceMapWidth / mapWidth;
	
	// Apply scaling factor to make the adjustment more reasonable
	autoZoom = std::pow(autoZoom, kZoomScalingFactor); // Use configurable constant
	
	SetZoom(autoZoom);
}

void CameraController::Update() { 
	if (!target_) {
		mainCamera_.UpdateMatrix(); 
		return;
	}
	
	// Get player position
	Vector3 playerPosition = target_->GetTranslation();
	
	// Start with current camera position
	Vector3 desiredPosition = targetPosition_;
	desiredPosition.z = -cameraDistance_; // Keep camera at a configurable distance from the game plane
	
	// Use cached viewport size for better performance
	Vector2 viewportSize = GetCachedViewportSize();
	float halfViewportWidth = viewportSize.x * 0.5f;
	float halfViewportHeight = viewportSize.y * 0.5f;
	
	// Calculate dead zone bounds around current camera position
	float halfDeadZoneWidth = deadZoneWidth_ * 0.5f;
	float halfDeadZoneHeight = deadZoneHeight_ * 0.5f;
	
	float deadZoneLeft = desiredPosition.x - halfDeadZoneWidth;
	float deadZoneRight = desiredPosition.x + halfDeadZoneWidth;
	float deadZoneBottom = desiredPosition.y - halfDeadZoneHeight;
	float deadZoneTop = desiredPosition.y + halfDeadZoneHeight;
	
	// Check if player is outside dead zone and adjust camera accordingly
	if (playerPosition.x < deadZoneLeft) {
		desiredPosition.x = playerPosition.x + halfDeadZoneWidth;
	} else if (playerPosition.x > deadZoneRight) {
		desiredPosition.x = playerPosition.x - halfDeadZoneWidth;
	}
	
	if (playerPosition.y < deadZoneBottom) {
		desiredPosition.y = playerPosition.y + halfDeadZoneHeight;
	} else if (playerPosition.y > deadZoneTop) {
		desiredPosition.y = playerPosition.y - halfDeadZoneHeight;
	}
	
	// Apply map bounds constraints with priority (optimized condition check)
	if (mapBoundsPriority_ && hasValidMovableArea_) {
		// Calculate effective camera bounds considering viewport
		float effectiveLeft = movableArea_.left + halfViewportWidth;
		float effectiveRight = movableArea_.right - halfViewportWidth;
		float effectiveBottom = movableArea_.bottom + halfViewportHeight;
		float effectiveTop = movableArea_.top - halfViewportHeight;
		
		// Ensure effective bounds are valid (handle cases where map is smaller than viewport)
		if (effectiveLeft > effectiveRight) {
			float center = (movableArea_.left + movableArea_.right) * 0.5f;
			effectiveLeft = effectiveRight = center;
		}
		if (effectiveBottom > effectiveTop) {
			float center = (movableArea_.bottom + movableArea_.top) * 0.5f;
			effectiveBottom = effectiveTop = center;
		}
		
		// Strictly enforce map bounds - this takes priority over character following
		if (desiredPosition.x < effectiveLeft) desiredPosition.x = effectiveLeft;
		if (desiredPosition.x > effectiveRight) desiredPosition.x = effectiveRight;
		if (desiredPosition.y < effectiveBottom) desiredPosition.y = effectiveBottom;
		if (desiredPosition.y > effectiveTop) desiredPosition.y = effectiveTop;
	}
	
	// Smooth camera movement using lerp
	targetPosition_ = Lerp(targetPosition_, desiredPosition, followSpeed_);
	
	// Update camera position
	mainCamera_.translation_ = targetPosition_;
	
	mainCamera_.UpdateMatrix(); 
}