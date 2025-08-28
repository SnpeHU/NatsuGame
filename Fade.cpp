#include "Fade.h"
#include "algorithm"
Fade::~Fade() { 
	delete fadeSprite_;
	fadeSprite_ = nullptr;
}

void Fade::Initialize() { 
	
	fadeSprite_ = Sprite::Create(0, {0,0});
	fadeSprite_->SetSize(Vector2(1280.0f, 720.0f));
	fadeSprite_->SetColor(Vector4(0, 0, 0, 1.0f));

}

void Fade::Update() {

	switch (status_) {
	case Fade::Status::kNone:
		break;
	case Fade::Status::kFadeIn:
		
		counter_ += 1.0f / 60.0f; // タイマー更新
		fadeSprite_->SetColor(Vector4(0, 0, 0, std::clamp(1 - counter_ / duration_, 0.0f, 1.0f)));
		if (counter_ >= duration_) {
			counter_ = duration_;    // タイマーリセット
		}
		
		break;
	case Fade::Status::kFadeOut:
		
		counter_ += 1.0f / 60.0f; // タイマー更新
		fadeSprite_->SetColor(Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		if (counter_ >= duration_) {
			counter_ = duration_;    // タイマーリセット

		}
		
		break;
	default:
		break;
	}
}

void Fade::Draw() { 
	if (status_ == Status::kNone) {
		return;
	}
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Sprite::PreDraw(dxCommon->GetCommandList());
	fadeSprite_->Draw();
	Sprite::PostDraw();
}

bool Fade::isFinished() const { 
	
	switch (status_) {
	case Fade::Status::kNone:
		break;
	case Fade::Status::kFadeIn:
		if (counter_ >= duration_) {
			return true; 
		} else {
			return false;
		}
		break;
	case Fade::Status::kFadeOut:
		if (counter_ >= duration_) {
			return true;
		}else {
			return false;
		}
		break;
	default:
		break;
	}
	return true;
}

void Fade::Start(Status status, float duration) { 
	status_ = status; 
	duration_ = duration;
	counter_ = 0.0f;

}

void Fade::Stop() {
	status_ = Status::kNone;
}

