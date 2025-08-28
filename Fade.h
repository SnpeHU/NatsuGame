#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;
class Fade {
public:
	enum class Status {
		kNone,
		kFadeIn,
		kFadeOut,
	};
	Fade() = default;
	~Fade();
	void Initialize();
	void Update();
	void Draw();
	bool isFinished() const;

	void Start(Status status,float duration_);
	void Stop();


	private:
		Sprite* fadeSprite_ = nullptr;
	    Status status_ = Status::kNone;

		float duration_ = 1.0f; // フェードの持続時間
	    float counter_ = 0.0f;   // タイマー

};