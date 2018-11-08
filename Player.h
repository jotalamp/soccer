#ifndef PLAYER_H
#define PLAYER_H

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

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Engine/Console.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/DebugNew.h>

#include "Character.h"

enum FieldPosition { FP_GK, FP_CB, FP_STRIKER };
enum KickType { KT_SHORT_PASS, KT_LONG_PASS, KT_SHORT_SHOT, KT_LONG_SHOT, KT_SHORT_CROSS, KT_LONG_CROSS, KT_LOW_THROUGH_PASS, KT_HIGH_THROUGH_PASS };
enum GameEvent { GE_GAME_ON, GE_CORNER_KICK, GE_FREE_KICK, GE_PENALTY_KICK, GE_GOAL_KICK_HOME, GE_GOAL_KICK_AWAY, GE_SIDE_THROW, GE_START_CENTER_HOME, GE_START_CENTER_AWAY };
enum GameState { MENU, GAME };
enum CameraOrientation { SIDE_BALL, SIDE_PLAYER, ORTHO_SIDE, TOP_BALL, TOP_PLAYER, CO_FREE, CO_FORWARD, N };
enum Animations { WALK1, WALK2, RUN1, RUN2, KICK1, KICK2, AN };
enum SButton {SELECT, L3, R3, START, UP, RIGHT, DOWN, LEFT, L2, R2, L1, R1, TRIANGLE, CIRCLE, CROSS, SQUARE, PS};
enum SixaxisButton {  SB_SELECT, SB_LEFTSTICK, SB_RIGHTSTICK, SB_START, SB_DPAD_UP, SB_DPAD_RIGHT, SB_DPAD_DOWN, SB_DPAD_LEFT, SB_L2, SB_R2, SB_L1, SB_R1, SB_TRIANGLE, SB_CIRCLE, SB_CROSS, SB_SQUARE, SB_PS };

static const int NUM_PLAYERS = 2;

class User;
class Ball;
class Player;

struct Field
{
    Vector2 size_;
    Node* goalA_;
    Node* goalB_;
    Ball* ball_;
};

struct Team
{
    int id;
    String name;
    String shortName;
    Color color;
    String clothesMaterial;
    bool homeTeam;
    bool attackRight;
};

class Player
{
    //URHO3D_OBJECT(Character, LogicComponent);
public:
    //
    Player(Team team, String name, Vector3 position, User* user, FieldPosition fieldPosition, float areaRadius=20.0f);
    //
    void CreateRagdollBone(const String& boneName, ShapeType type, const Vector3& size, const Vector3& position,const Quaternion& rotation);
    //
public:
    Player* NearestPlayer(bool needsToBeSameTeam, bool preferUpperLevel);
public:
    static Player* NearestPlayerToBall(int teamID, bool noGoalKeeper);

    Player* PlayerInDirection(bool needsToBeSameTeam, bool noGoalKeeper);

    Vector3 GetPosition();

    void ChangePlayer(int oldSelectedPlayerID, int newSelectedPlayerID);

    float BallsDistanceToAreaCenter();

    Vector3 MoveTargetVector();

    Vector3 KickTargetPosition();

    Vector3 KickTargetVector();

    float DistanceToMoveTarget();

    float AngleToMoveTarget();

    float AngleToKickTarget();

    Vector3 BallVector();

    float KickAnimationTime();

    Vector3 DirectionVector();

    float Angle();

    void HandleCPUPlayer(UI* ui);

    void ChargeKick(UI* ui);

    bool AnimationInRange(String animationName, float rangeMin, float rangeMax);

    void TakeBallInHands();

    void FootToGround();

    void KickBall(Vector3 kickVector);

    void Update(UI* ui, float dt);

    static constexpr float CPUReachBallRadius_ = 0.9f;
    static Player* players_[NUM_PLAYERS];
    static constexpr float kickPower_ = 30.0f;
    static constexpr float maxKickPower_ = 28.0f;
    static constexpr float minKickPower_ = 15.0f;
    static constexpr float playerReachBallRadius_ = 1.5f;
    static constexpr float playerKicksBallRadius_ = 0.3f;
    static constexpr float CPUKicksBallRadius_ = 0.5f;
    static int nextID_;
    static bool playerChanged_;
    static bool showDebugGeometry_;
    static bool takeBall_;
    static Context* context_;
    static Field field_;
    static Scene* scene_;
    static UI* ui_;
    static Ball* ball_;
    static Player* lastPlayerWhoTouchedBall_;
    static String kickAnimation01Url_;
    static Camera* camera_;
    static Node* cameraNode_;
    static GameEvent gameEvent_;
    static Timer* gameEventTimer_;

    int ID_;
    Node* node_;
    RigidBody* body_;
    AnimatedModel* animModel_;
    AnimatedModel* animModel2_;
    AnimationController* animController_;
    Character* character_;
    Team team_;
    String name_;
    Node* kickTarget_;
    Vector3 kickTargetPosition_;
    Node* moveTarget_;
    Vector3 moveTargetPosition_;
    Node* areaCenter_;
    float areaRadius_;
    Node* targetGoal_;
    FieldPosition fieldPosition_;
    User* user_;
    KickType kickType_;
    Sprite* radarSpot_;
    /// Inverse kinematic right effector.
    IKEffector* rightHandEffector_;
    IKEffector* rightFootEffector_;
    IKEffector* leftFootEffector_;
    /// Inverse kinematic solver.
    IKSolver* solverRightHand_;
    IKSolver* solverFoot_;
    /// Need references to these nodes to calculate foot angles and offsets.
    Node* rightHand_;
    Node* leftFoot_;
    Node* rightFoot_;
};

#endif // PLAYER_H
