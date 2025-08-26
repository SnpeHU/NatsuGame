# 碰撞检测系统使用说明

## 系统概述

我们实现了一个完整的碰撞检测系统，包括：

1. **AABB（轴对齐包围盒）碰撞检测**
2. **地图与角色碰撞检测**
3. **物体间碰撞检测**
4. **碰撞回调系统**
5. **平台跳跃移动系统**

## 核心组件

### 1. CollisionSystem（碰撞系统管理器）

- **单例模式**：全局唯一的碰撞系统实例
- **碰撞器管理**：注册、注销、查询碰撞器
- **碰撞检测**：每帧检测所有碰撞器之间的碰撞
- **地图碰撞**：专门处理角色与地图瓦片的碰撞

### 2. Collider（碰撞器）

- **AABB包围盒**：基于轴对齐包围盒的碰撞检测
- **碰撞标签**：区分不同类型的对象（Player, Enemy, Pickup, Block, Trigger）
- **触发器模式**：支持触发器碰撞（不阻挡移动）
- **回调系统**：碰撞发生时触发自定义回调函数

### 3. Player（玩家角色）

- **平台跳跃机制**：
  - 重力系统
  - 变高度跳跃（根据按键时长）
  - 土狼时间（离开平台后短时间内仍可跳跃）
  - 跳跃缓冲（提前按跳跃键会在着地时自动跳跃）
- **碰撞响应**：
  - 地图碰撞：阻挡移动并正确解析碰撞
  - 敌人碰撞：击退效果
  - 道具碰撞：触发收集

## 使用方法

### 1. 初始化碰撞系统

```cpp
// 在GameScene::Initialize()中
CollisionSystem::GetInstance()->Initialize();
```

### 2. 创建碰撞器

```cpp
// 为对象创建碰撞器
collider_ = std::make_shared<Collider>(CollisionTag::Player, Vector3{width, height, depth});
collider_->SetPosition(worldTransform_.translation_);

// 注册碰撞回调
collider_->AddCollisionCallback([this](Collider* self, Collider* other) {
    OnCollisionEnter(self, other);
});

// 注册到碰撞系统
CollisionSystem::GetInstance()->RegisterCollider(collider_);
```

### 3. 更新碰撞系统

```cpp
// 在GameScene::Update()中，在其他对象更新之前
CollisionSystem::GetInstance()->Update();
```

### 4. 处理碰撞回调

```cpp
void Player::OnCollisionEnter(Collider* self, Collider* other) {
    switch (other->GetTag()) {
        case CollisionTag::Enemy:
            // 处理敌人碰撞
            break;
        case CollisionTag::Pickup:
            // 处理道具碰撞
            break;
        // ... 其他碰撞类型
    }
}
```

## 示例对象

### Enemy（敌人）
- **巡逻AI**：在两点间来回移动
- **碰撞检测**：与玩家和地图块碰撞
- **标签**：CollisionTag::Enemy

### Pickup（道具）
- **动画效果**：上下浮动和旋转
- **触发器碰撞**：不阻挡移动，仅触发收集
- **自动销毁**：被收集后自动从场景中移除
- **标签**：CollisionTag::Pickup

## 调试功能

在Debug模式下，可以：

1. **查看碰撞器信息**：在ImGui窗口中查看活跃的碰撞器数量和属性
2. **动态生成对象**：使用按钮在玩家位置生成敌人和道具
3. **实时监控**：查看玩家的移动状态、碰撞状态等

## 控制说明

### 玩家控制
- **A/D 或 左/右箭头**：水平移动
- **空格/W/上箭头**：跳跃
- **松开跳跃键**：提前结束跳跃（变高度跳跃）

### 调试控制
- **T键**：切换调试摄像机
- **右箭头**：切换到标题场景

## 技术特性

### 平台跳跃机制
1. **土狼时间（Coyote Time）**：离开平台后0.1秒内仍可跳跃
2. **跳跃缓冲（Jump Buffer）**：着地前0.1秒按跳跃键会在着地时自动跳跃
3. **变高度跳跃**：根据按键时长调整跳跃高度
4. **重力和最大下落速度**：模拟真实的重力效果

### 碰撞检测
1. **AABB碰撞检测**：高效的轴对齐包围盒碰撞
2. **碰撞解析**：正确计算碰撞法线和穿透深度
3. **分离轴碰撞**：分别处理X轴和Y轴碰撞
4. **地图瓦片碰撞**：针对网格化地图的优化碰撞检测

### 回调系统
1. **灵活的碰撞响应**：每个对象可以注册多个碰撞回调
2. **类型安全**：基于碰撞标签的类型识别
3. **事件驱动**：碰撞发生时自动触发相应的处理逻辑

## 扩展性

这个系统设计为可扩展的：

1. **新的碰撞标签**：在CollisionTag枚举中添加新类型
2. **自定义碰撞形状**：可以扩展Collider类支持其他形状
3. **物理效果**：可以添加弹性、摩擦力等物理属性
4. **空间优化**：可以添加空间分割算法（如四叉树）提高性能

## 文件结构

```
CollisionSystem.h/cpp    - 碰撞系统核心
Player.h/cpp            - 玩家角色（集成碰撞和平台跳跃）
Enemy.h/cpp             - 敌人示例（巡逻AI + 碰撞）
Pickup.h/cpp            - 道具示例（触发器碰撞）
GameScene.h/cpp         - 场景管理（集成碰撞系统）
```

这个碰撞检测系统提供了完整的2D平台游戏所需的碰撞功能，包括角色移动、敌人AI、道具收集等常见游戏机制。