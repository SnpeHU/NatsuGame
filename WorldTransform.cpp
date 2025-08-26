#include"KamataEngine.h"
#include "3d/WorldTransform.h"
using namespace KamataEngine;
using namespace MathUtility;
void WorldTransform::MakeAffineMatrix4x4() {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale_);
	Matrix4x4 rotationMatrixX = MakeRotateXMatrix(rotation_.x);
	Matrix4x4 rotationMatrixY = MakeRotateYMatrix(rotation_.y);
	Matrix4x4 rotationMatrixZ = MakeRotateZMatrix(rotation_.z);
	Matrix4x4 rotationMatrix = rotationMatrixX * rotationMatrixY * rotationMatrixZ;
	Matrix4x4 translationMatrix = MakeTranslateMatrix(translation_);
	Matrix4x4 localMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	if (parent_) {
		matWorld_ = localMatrix * parent_->matWorld_;
	} else {
		matWorld_ = localMatrix;
	}
}