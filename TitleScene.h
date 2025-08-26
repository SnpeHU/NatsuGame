#pragma once
#include "IScene.h"
class TitleScene : public IScene {
	public:
	TitleScene() = default;
	~TitleScene() = default;
	void Initialize() override;
	void OnEnter() override;
	void Update() override;
	void Draw() override;
	void OnExit() override;

	private:


};
