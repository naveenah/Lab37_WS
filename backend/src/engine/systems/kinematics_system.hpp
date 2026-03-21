#pragma once

#include "interfaces/i_robot_kinematics.hpp"
#include <entt/entt.hpp>

namespace teleop {

class KinematicsSystem {
public:
    explicit KinematicsSystem(IRobotKinematics& kinematics);
    void tick(entt::registry& registry, float dt);

private:
    IRobotKinematics& kinematics_;
};

} // namespace teleop
