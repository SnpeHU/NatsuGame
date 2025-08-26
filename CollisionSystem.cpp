#include "CollisionSystem.h"
#include "MapChipField.h"
#include <algorithm>
#include <cmath>
#include <cfloat>

// Undefine Windows min/max macros that conflict with std::min/max
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

using namespace KamataEngine::MathUtility;

CollisionSystem* CollisionSystem::instance_ = nullptr;

// Collider implementation
Collider::Collider(CollisionTag tag, const Vector3& size) 
    : tag_(tag), size_(size) {
    UpdateAABB();
}

void Collider::Update(const Vector3& position) {
    position_ = position;
    UpdateAABB();
}

void Collider::UpdateAABB() {
    Vector3 halfSize = size_ * 0.5f;
    aabb_.min = position_ - halfSize;
    aabb_.max = position_ + halfSize;
}

CollisionInfo Collider::CheckCollision(const Collider& other) const {
    CollisionInfo info;
    
    if (!isActive_ || !other.isActive_) {
        return info;
    }
    
    AABB thisAABB = GetAABB();
    AABB otherAABB = other.GetAABB();
    
    info.hasCollision = thisAABB.Intersects(otherAABB);
    
    if (info.hasCollision) {
        // Calculate penetration depth
        Vector3 penetration;
        penetration.x = std::min(thisAABB.max.x - otherAABB.min.x, otherAABB.max.x - thisAABB.min.x);
        penetration.y = std::min(thisAABB.max.y - otherAABB.min.y, otherAABB.max.y - thisAABB.min.y);
        penetration.z = std::min(thisAABB.max.z - otherAABB.min.z, otherAABB.max.z - thisAABB.min.z);
        
        info.penetration = penetration;
        
        // Calculate collision normal (direction to resolve collision)
        Vector3 centerDiff = otherAABB.GetCenter() - thisAABB.GetCenter();
        info.distance = sqrt(centerDiff.x * centerDiff.x + centerDiff.y * centerDiff.y + centerDiff.z * centerDiff.z);
        
        // Find the axis with minimum penetration (collision normal)
        if (penetration.x <= penetration.y && penetration.x <= penetration.z) {
            info.normal = {centerDiff.x > 0 ? -1.0f : 1.0f, 0.0f, 0.0f};
        } else if (penetration.y <= penetration.x && penetration.y <= penetration.z) {
            info.normal = {0.0f, centerDiff.y > 0 ? -1.0f : 1.0f, 0.0f};
        } else {
            info.normal = {0.0f, 0.0f, centerDiff.z > 0 ? -1.0f : 1.0f};
        }
    }
    
    return info;
}

void Collider::OnCollision(Collider* other) {
    for (auto& callback : callbacks_) {
        callback(this, other);
    }
}

// CollisionSystem implementation
CollisionSystem* CollisionSystem::GetInstance() {
    if (!instance_) {
        instance_ = new CollisionSystem();
    }
    return instance_;
}

void CollisionSystem::Initialize() {
    colliders_.clear();
}

void CollisionSystem::Update() {
    // Check collisions between all registered colliders
    for (size_t i = 0; i < colliders_.size(); ++i) {
        auto& colliderA = colliders_[i];
        if (!colliderA || !colliderA->IsActive()) continue;
        
        for (size_t j = i + 1; j < colliders_.size(); ++j) {
            auto& colliderB = colliders_[j];
            if (!colliderB || !colliderB->IsActive()) continue;
            
            CollisionInfo info = colliderA->CheckCollision(*colliderB);
            if (info.hasCollision) {
                // Trigger callbacks
                colliderA->OnCollision(colliderB.get());
                colliderB->OnCollision(colliderA.get());
            }
        }
    }
}

void CollisionSystem::Clear() {
    colliders_.clear();
}

void CollisionSystem::RegisterCollider(std::shared_ptr<Collider> collider) {
    if (collider) {
        colliders_.push_back(collider);
    }
}

void CollisionSystem::UnregisterCollider(std::shared_ptr<Collider> collider) {
    colliders_.erase(
        std::remove(colliders_.begin(), colliders_.end(), collider),
        colliders_.end()
    );
}

std::vector<std::shared_ptr<Collider>> CollisionSystem::GetCollidersWithTag(CollisionTag tag) {
    std::vector<std::shared_ptr<Collider>> result;
    for (auto& collider : colliders_) {
        if (collider && collider->IsActive() && collider->GetTag() == tag) {
            result.push_back(collider);
        }
    }
    return result;
}

std::vector<CollisionInfo> CollisionSystem::CheckCollisions(const Collider& collider) {
    std::vector<CollisionInfo> results;
    
    for (auto& other : colliders_) {
        if (other && other.get() != &collider && other->IsActive()) {
            CollisionInfo info = collider.CheckCollision(*other);
            if (info.hasCollision) {
                results.push_back(info);
            }
        }
    }
    
    return results;
}

CollisionInfo CollisionSystem::CheckCollision(const Collider& a, const Collider& b) {
    return a.CheckCollision(b);
}

CollisionInfo CollisionSystem::CheckMapCollision(const Vector3& position, const Vector3& size, MapChipField* mapField) {
    CollisionInfo info;
    
    if (!mapField) {
        return info;
    }
    
    // Create AABB for the object
    AABB objectAABB(position, size);
    
    // Get the range of map tiles to check
    IndexSet minIndex = mapField->GetMapChipIndexByPosition(objectAABB.min);
    IndexSet maxIndex = mapField->GetMapChipIndexByPosition(objectAABB.max);
    
    // Ensure indices are within valid range
    uint32_t mapWidth = mapField->GetNumBlockHorizontal();
    uint32_t mapHeight = mapField->GetNumBlockVertical();
    
    // Clamp indices to valid range
    uint32_t startX = std::max(0u, std::min(minIndex.xIndex, mapWidth - 1));
    uint32_t endX = std::max(0u, std::min(maxIndex.xIndex, mapWidth - 1));
    uint32_t startY = std::max(0u, std::min(minIndex.yIndex, mapHeight - 1));
    uint32_t endY = std::max(0u, std::min(maxIndex.yIndex, mapHeight - 1));
    
    // Track the best collision (with minimum penetration)
    CollisionInfo bestCollision;
    float minPenetration = FLT_MAX;
    
    // Check each potentially colliding map tile
    for (uint32_t y = startY; y <= endY; ++y) {
        for (uint32_t x = startX; x <= endX; ++x) {
            MapChipType chipType = mapField->GetMapChipTypeByIndex(x, y);
            
            if (chipType == MapChipType::kBlock) {
                // Get the tile's AABB
                Vector3 tileCenter = mapField->GetMapChipPositionByIndex(x, y);
                AABB tileAABB(tileCenter, {MapChipField::kBlockWidth, MapChipField::kBlockHeight, 2.0f});
                
                if (objectAABB.Intersects(tileAABB)) {
                    CollisionInfo currentCollision;
                    currentCollision.hasCollision = true;
                    
                    // Calculate penetration depth
                    Vector3 penetration;
                    penetration.x = std::min(objectAABB.max.x - tileAABB.min.x, tileAABB.max.x - objectAABB.min.x);
                    penetration.y = std::min(objectAABB.max.y - tileAABB.min.y, tileAABB.max.y - objectAABB.min.y);
                    penetration.z = std::min(objectAABB.max.z - tileAABB.min.z, tileAABB.max.z - objectAABB.min.z);
                    
                    currentCollision.penetration = penetration;
                    
                    // Calculate collision normal based on minimum penetration axis
                    Vector3 centerDiff = objectAABB.GetCenter() - tileAABB.GetCenter();
                    
                    // Find the axis with minimum penetration for the collision normal
                    float minAxisPenetration = std::min(penetration.x, std::min(penetration.y, penetration.z));
                    
                    if (penetration.x == minAxisPenetration) {
                        // X-axis collision
                        currentCollision.normal = {centerDiff.x > 0 ? 1.0f : -1.0f, 0.0f, 0.0f};
                    } else if (penetration.y == minAxisPenetration) {
                        // Y-axis collision  
                        currentCollision.normal = {0.0f, centerDiff.y > 0 ? 1.0f : -1.0f, 0.0f};
                    } else {
                        // Z-axis collision
                        currentCollision.normal = {0.0f, 0.0f, centerDiff.z > 0 ? 1.0f : -1.0f};
                    }
                    
                    // Use the collision with the smallest penetration depth
                    if (minAxisPenetration < minPenetration) {
                        minPenetration = minAxisPenetration;
                        bestCollision = currentCollision;
                    }
                }
            }
        }
    }
    
    return bestCollision;
}