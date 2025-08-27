#include "Skydome.h"

void Skydome::Update() {
	worldTransform_.MakeAffineMatrix4x4();
	worldTransform_.TransferMatrix();
}
