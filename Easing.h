#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;
static inline Vector3 Lerp(const Vector3& start, const Vector3& end, float t) { return {start.x + (end.x - start.x) * t, start.y + (end.y - start.y) * t, start.z + (end.z - start.z) * t}; }

static inline float EaseOut(float start, float end, float t) {
	// イージング関数（Ease Out）
	return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t));
}