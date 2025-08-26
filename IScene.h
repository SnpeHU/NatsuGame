#pragma once
#include <string>
using std::string;
class IScene {
public:
	IScene() = default;
	virtual ~IScene() = default;

	virtual void Initialize() = 0;
	virtual void OnEnter() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void OnExit() = 0;


protected:
	string sceneName_ = "DefaultScene";

};
