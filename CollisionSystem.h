#pragma once
#include <KamataEngine.h>
#include <functional>
#include <vector>
#include <memory>
#include "math/MathUtility.h"

// Prevent Windows min/max macro conflicts
#ifndef NOMINMAX
#define NOMINMAX
#endif

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

// Forward declarations
class CollisionSystem;
class Collider;

// Collision callback type
using CollisionCallback = std::function<void(Collider*, Collider*)>;

// Collision tags for different object types
enum class CollisionTag {
    Player,
    Block,
    Enemy,
    Pickup,
    Trigger
};

// AABB structure for collision detection
struct AABB {
    Vector3 min;
    Vector3 max;
    
    AABB() = default;
    AABB(const Vector3& center, const Vector3& size) {
        Vector3 halfSize = size * 0.5f;
        min = center - halfSize;
        max = center + halfSize;
    }
    
    // Check if this AABB intersects with another
    bool Intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }
    
    // Get center point
    Vector3 GetCenter() const {
        return (min + max) * 0.5f;
    }
    
    // Get size
    Vector3 GetSize() const {
        return max - min;
    }
};

// Collision info structure
struct CollisionInfo {
    bool hasCollision = false;
    Vector3 penetration = {0.0f, 0.0f, 0.0f};  // How much objects overlap
    Vector3 normal = {0.0f, 0.0f, 0.0f};       // Collision normal
    float distance = 0.0f;                      // Distance between centers
};

// Base collider class
class Collider {
public:
    Collider(CollisionTag tag, const Vector3& size = {2.0f, 2.0f, 2.0f});
    virtual ~Collider() = default;
    
    // Update collider position
    virtual void Update(const Vector3& position);
    
    // Get AABB for collision detection
    virtual AABB GetAABB() const { return aabb_; }
    
    // Collision detection
    virtual CollisionInfo CheckCollision(const Collider& other) const;
    
    // Setters and getters
    void SetTag(CollisionTag tag) { tag_ = tag; }
    CollisionTag GetTag() const { return tag_; }
    
    void SetSize(const Vector3& size) { size_ = size; }
    Vector3 GetSize() const { return size_; }
    
    void SetPosition(const Vector3& position) { position_ = position; UpdateAABB(); }
    Vector3 GetPosition() const { return position_; }
    
    void SetActive(bool active) { isActive_ = active; }
    bool IsActive() const { return isActive_; }
    
    void SetTrigger(bool isTrigger) { isTrigger_ = isTrigger; }
    bool IsTrigger() const { return isTrigger_; }
    
    // Callback management
    void AddCollisionCallback(CollisionCallback callback) { callbacks_.push_back(callback); }
    void OnCollision(Collider* other);

protected:
    void UpdateAABB();
    
    Vector3 position_ = {0.0f, 0.0f, 0.0f};
    Vector3 size_ = {2.0f, 2.0f, 2.0f};
    AABB aabb_;
    CollisionTag tag_;
    bool isActive_ = true;
    bool isTrigger_ = false;
    std::vector<CollisionCallback> callbacks_;
};

// Collision system manager
class CollisionSystem {
public:
    static CollisionSystem* GetInstance();
    
    void Initialize();
    void Update();
    void Clear();
    
    // Collider management
    void RegisterCollider(std::shared_ptr<Collider> collider);
    void UnregisterCollider(std::shared_ptr<Collider> collider);
    
    // Collision queries
    std::vector<std::shared_ptr<Collider>> GetCollidersWithTag(CollisionTag tag);
    std::vector<CollisionInfo> CheckCollisions(const Collider& collider);
    CollisionInfo CheckCollision(const Collider& a, const Collider& b);
    
    // Map collision detection
    CollisionInfo CheckMapCollision(const Vector3& position, const Vector3& size, class MapChipField* mapField);
    
private:
    CollisionSystem() = default;
    ~CollisionSystem() = default;
    CollisionSystem(const CollisionSystem&) = delete;
    CollisionSystem& operator=(const CollisionSystem&) = delete;
    
    std::vector<std::shared_ptr<Collider>> colliders_;
    static CollisionSystem* instance_;
};