#include "Goal.h"

void Goal::Update() {
	worldTransform_.MakeAffineMatrix4x4();
	worldTransform_.TransferMatrix();
}
