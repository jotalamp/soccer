#pragma once

#include "Sample.h"
//#include "Player.h"

const int NUM_SOUNDS = 1;

bool showMenu = false;
bool resetPositions = true;
bool slowMotion = false;
bool autoZoom = false;
bool doAnimationTest = false;
bool showRibbontrail = false;
bool playerChanged = false;
bool takeBall = false;
bool isDragging = false;
bool subscribedA = false;
bool subscribedB = false;
bool showDebugGeometry = false;

Node* freeCameraNode; // Camera parent Node
Node* cameraTargetNode; // Node to "follow"

RigidBody* draggedBody;

int lastPlayerWhoTouchedBallID;

JoystickState* joystick;

Camera* camera;
Text* instructionText;

Node* mouseNode;

//static GameEvent gameEvent = GameEvent::GE_START_CENTER_HOME;
UI* ui;

float cameraSpeed = 0.05f;
float zoomSpeed = 0.02f;
float targetZoom = 2.5f;
float averageDeltaTime = 30.0f;
float cameraYaw     = 0.0f;
float cameraPitch   = 0.0f;

SoundSource* soundSourceCrowd;
//SoundSource* soundSourceMusic;
//Sound* sound;
//Sound* soundKick01;
Sound* soundGoal01;
//Sound* soundWhistle;
//Sound* sounds[NUM_SOUNDS];

Billboard* billboardArrow;

Timer* goalTimer;
Timer* matchTime;
bool afterGoal = false;

unsigned goalsA = 0;
unsigned goalsB = 0;

Urho3D::Image* image;

ResourceCache* cache;


//Field field;

namespace Urho3D
{

class Node;
class Scene;

}

class Character;
//class Touch;
class Player;






/// Soccer game
class CharacterDemo : public Sample
{
    URHO3D_OBJECT(CharacterDemo, Sample);

public:
    /// Construct.
    explicit CharacterDemo(Context* context);

    /// Destruct.
    ~CharacterDemo() override;

    void Setup() override;

    /// Setup after engine initialization and before running the main loop.
    void Start() override;
/*
protected:
    /// Return XML patch instructions for screen joystick layout for a specific sample app, if any.
    String GetScreenJoystickPatchString() const override { return
        "<patch>"
        "</patch>";
    }*/

private:
    void HandleSceneDrawableUpdateFinished(StringHash /*eventType*/, VariantMap& eventData);
    void HandleASyncLoadFinished(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    //void InitControls();
    //void InitWindow();
    void HandleClosePressed(StringHash eventType, VariantMap& eventData);
    void ResumeGame(StringHash eventType, VariantMap& eventData);
    void ExitGame(StringHash eventType, VariantMap& eventData);
    void HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData);
    //void HandleControlClicked(StringHash eventType, VariantMap& eventData);

    //void SpawnObject();
    /// Create static scene content.

    //void CreateGameController();
    void CreateUI();
    void CreateScene();
    /// Create controllable character.
    //void CreateCharacter();
    //Player* CreateCharacter(Team team, String name, Vector3 position, int joystickID=-1);
    /// Construct an instruction text to the UI.
    void CreateInstructions();
    /// Subscribe to necessary events.
    void SubscribeToEvents();
    void CreateRagdollBone(Node* rootNode, const String& boneName, ShapeType type, const Vector3& size, const Vector3& position,
        const Quaternion& rotation);
    void SpawnObject(int objtype);

    /// Test if some pointers are null
    bool CheckPointers();
    /// Test animations
    void AnimationTest();
    /// Update FPS counter
    void UpdateFPS();
    /// Update time text
    void UpdateTimeText();
    /// If triggers are not subscribed, subscribe them
    void CheckTriggers();
    /// Handle game events
    void HandleGameEvents();
    /// Selecting camera, zooming etc..
    void UpdateCamera();
    /// Handle application update. Set controls to character.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle application post-update. Update camera position after character has moved.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    /// Draws vector from startPoint to direction
    void DrawVector( Vector3 startPoint, Vector3 direction, Color color );
    /// Kick ball
    void KickBall(Vector3 kickDirection, float kickForce, float curl);

    void ChangePlayer(int oldSelectedPlayerID, int newSelectedPlayerID);

    /// Touch utility object.
    //SharedPtr<Touch> touch_;
    /// First person camera flag.
    bool firstPerson_;
    /// The Window.
    //SharedPtr<Window> window_;
    /// The UI's root UIElement.
    //SharedPtr<UIElement> uiRoot_;
};
