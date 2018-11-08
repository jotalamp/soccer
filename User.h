#ifndef USER_H
#define USER_H

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>

using namespace Urho3D;

enum UserType { UT_HUMAN, UT_CPU };

class Player;

class User
{
public:
    User();
    UserType userType_;
    Color color_;
    JoystickState* joystick_;
    Player* player_;
    String arrowMaterialUrl_;
};

#endif // USER_H
