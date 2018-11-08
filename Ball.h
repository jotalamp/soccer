#ifndef BALL_H
#define BALL_H

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/RibbonTrail.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/ToolTip.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/ProgressBar.h>
#include <Urho3D/IK/IKEffector.h>
#include <Urho3D/IK/IKSolver.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Graphics/BillboardSet.h>

#include <Urho3D/UI/Sprite.h>

using namespace Urho3D;

class Field;

class Ball
{
public:
    Ball(Context* context, Scene* scene, float curlFactor);

    bool InField();

    void KickSound(float gain);

    void Update(float dt);

    Vector3 GetPosition();

    void SetPosition(Vector3 position);

    void ResetPosition(Vector3 position = resetPosition_);

    static Vector3 freeKickPosition_;
    static Vector3 resetPosition_;
    static Field field_;
    Context* context_;
    float curlFactor_;
    Scene* scene_;
    Node* node_;
    RigidBody* body_;
    SoundSource3D* soundSource_;
    Sound* soundKick_;
    ResourceCache* cache_;
    Sprite* radarSpot_;
    bool readyForGameEvent_ = false;
};

#endif // BALL_H
