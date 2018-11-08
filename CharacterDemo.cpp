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

#include "Character.h"
#include "CharacterDemo.h"
#include <Urho3D/DebugNew.h>

#include "Player.h"
#include "Ball.h"
#include "Replay.h"
#include "User.h"
#include "Referee.h"

Team teamA;
Team teamB;
Team teamC;

Team homeTeam;
Team awayTeam;

Replay replay;

Referee* referee;

User* userHuman1;
User* userHuman2;
User* userCPU;

Ball* ball;
Vector3 Ball::freeKickPosition_ = Vector3(0.0f, 0.3f, 0.0f);
Field Ball::field_;

Animations selectedAnimation = Animations::WALK1;

CharacterDemo::~CharacterDemo() = default;

URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)

CharacterDemo::CharacterDemo(Context* context) :
    Sample(context),
    firstPerson_(false)
{
    // Register factory and attributes for the Character component so it can be created via CreateComponent, and loaded / saved
    Character::RegisterObject(context);
}

CameraOrientation selectedCamera = CameraOrientation::SIDE_BALL;

void CharacterDemo::HandleSceneDrawableUpdateFinished(StringHash /*eventType*/, VariantMap& eventData)
{
}

void CharacterDemo::Setup()
{
    /// fullscreen works if
    /// desktop screen resolution 1280x960
    /// and engineParameters_["FullScreen"]=true; and
    /// engineParameters_["WindowWidth"]=1280; engineParameters_["WindowHeight"]=960;
    //engineParameters_["FullScreen"]=true;
    engineParameters_["FullScreen"]=false;
    engineParameters_["Headless"]= false;
    //engineParameters_["WindowWidth"]=1440;
    //engineParameters_["WindowHeight"]=900;
    engineParameters_["WindowWidth"]=1280;
    engineParameters_["WindowHeight"]=960;

    engineParameters_["ResourcePrefixPath"] = "Data2";
}

void CharacterDemo::CreateUI()
{
    auto* graphics = GetSubsystem<Graphics>();
    float width = (float)(graphics->GetWidth());
    float height = (float)(graphics->GetHeight());

    auto* cache = GetSubsystem<ResourceCache>();
    ui = GetSubsystem<UI>();

    // Set up global UI style into the root UI element
    auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    ui->GetRoot()->SetDefaultStyle(style);

    // Create a Cursor UI element because we want to be able to hide and show it at will. When hidden, the mouse cursor will
    // control the camera, and when visible, it will interact with the UI
    SharedPtr<Cursor> cursor(new Cursor(context_));
    cursor->SetStyleAuto();
    ui->SetCursor(cursor);
    // Set starting position of the cursor at the rendering window center

    //cursor->SetPosition( graphics->GetWidth() / 2, 0.5f * height);

    // Load UI content prepared in the editor and add to the UI hierarchy
    SharedPtr<UIElement> layoutRoot = ui->LoadLayout(cache->GetResource<XMLFile>("UI/GameUI03.xml"));
    //SharedPtr<UIElement> layoutRoot = ui->LoadLayout(cache->GetResource<XMLFile>("UI/SoccerUI01.xml"));
    ui->GetRoot()->AddChild(layoutRoot);
    //layoutRoot->SetOpacity (0.9f);

    // Create the Window's close button
    auto* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");
    buttonClose->SetStyle("CloseButton");

    layoutRoot->AddChild(buttonClose);

    // Subscribe to buttonClose release (following a 'press') events
    SubscribeToEvent(buttonClose, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleClosePressed));

    // Subscribe to button actions (toggle scene lights when pressed then released)
    auto* button = layoutRoot->GetChildStaticCast<Button>("ResumeGame", true);
    if (button) SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(CharacterDemo, ResumeGame));

    button = layoutRoot->GetChildStaticCast<Button>("ExitGame", true);
    if (button) SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(CharacterDemo, ExitGame));

    auto powerBar = (ProgressBar*)ui->GetRoot()->GetChild("PowerBar",true);
    powerBar->SetValue(50);

    auto replayBar = (ProgressBar*)ui->GetRoot()->GetChild("ReplayBar",true);
    replayBar->SetRange(REPLAY_LENGTH);
    //replayBar->SetPosition(-200.0f, height - 200.0f);

    //auto radar = (Sprite*)ui->GetRoot()->GetChild("Radar",true);
    //radar->SetPosition(0.5f  *width - 300.0f, height - 300.0f);

    //auto radar = (Sprite*)ui->GetRoot()->GetChild("Radar2",true);
    //radar->SetPosition(0.5f  *width - 300.0f, height - 300.0f);
}

void CharacterDemo::Start()
{
    // Execute base class startup
    Sample::Start();
    //if (touchEnabled_)
       // touch_ = new Touch(context_, TOUCH_SENSITIVITY);


    CreateScene();// Create static scene content
    CreateUI();// Create the UI content
    SubscribeToEvents();// Subscribe to necessary events
    CreateInstructions();// Create the UI content

    //Sample::InitMouseMode(MM_RELATIVE);// Set the mouse mode to use in the sample
}

void CharacterDemo::HandleClosePressed(StringHash eventType, VariantMap& eventData)
{
    //ui->GetRoot()->SetOpacity(0.1f);
}

void CharacterDemo::ResumeGame(StringHash eventType, VariantMap& eventData)
{
    ui->SetFocusElement(nullptr,false);
    //ui->GetRoot()->SetOpacity(0.1f);
    scene_->SetUpdateEnabled(true);
    //gameState = GameState::GAME;
}

void CharacterDemo::ExitGame(StringHash eventType, VariantMap& eventData)
{
    if (GetPlatform() != "Web")
        engine_->Exit();
}

void CharacterDemo::CreateScene()
{
    cache = GetSubsystem<ResourceCache>();
    scene_ = new Scene(context_);
    scene_->LoadAsyncXML(cache->GetFile("/media/sda7/Ohjelmointi/Urho3D/bin/Data/Scenes/Soccer/SoccerScene04.xml"));

    scene_->CreateComponent<DebugRenderer>();

    // Create static scene content. First create a zone for ambient lighting and fog control
    Node* zoneNode = scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetAmbientColor(Color(0.4f, 0.4f, 0.4f));
    //zone->SetFogColor(Color(0.0f, 0.0f, 0.0f));
    zone->SetFogColor(Color(1.0f, 1.0f, 1.0f));
    zone->SetFogStart(120.0f);
    zone->SetFogEnd(200.0f);
    zone->SetBoundingBox(BoundingBox(-200.0f, 200.0f));

    // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
    // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
    //cameraNode_ = new Node(context_);
    //
    Node* mouseNode = scene_->CreateChild("MouseNode");

    Node* cameraRotateNode_ = scene_->CreateChild("CameraRotate");
    cameraNode_ = cameraRotateNode_->CreateChild("Camera");
    //cameraNode_->CreateComponent<Camera>();
    camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(150.0f);

    // Set an initial position for the camera scene node above the plane
    cameraNode_->SetPosition(Vector3(0, 0, -4));
    cameraRotateNode_->SetPosition(Vector3(0, 0, 0));
    //float pitch_ = 20;
    //float yaw_ = 50;

    freeCameraNode = cameraRotateNode_;
    //freeCameraNode->SetPosition(Vector3::ZERO);

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    GetSubsystem<Renderer>()->SetViewport(0, viewport);

    // Clone the default render path so that we do not interfere with the other viewport, then add
    // bloom and FXAA post process effects to the front viewport. Render path commands can be tagged
    // for example with the effect name to allow easy toggling on and off. We start with the effects
    // disabled.
    auto* cache = GetSubsystem<ResourceCache>();
    SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();

    //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));

    //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
    //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));
    //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/GreyScale.xml"));

    // Make the bloom mixing parameter more pronounced
    //effectRenderPath->SetShaderParameter("BloomMix", Vector2(0.9f, 0.6f));
    //effectRenderPath->SetEnabled("Bloom", false);
    //effectRenderPath->SetEnabled("FXAA2", true);
    //effectRenderPath->SetEnabled("GreyScale", true);
    //effectRenderPath->SetEnabled("FXAA4", true);

    viewport->SetRenderPath(effectRenderPath);
    // for 3D sounds to work
    SoundListener* listener=cameraNode_->CreateComponent<SoundListener>();
    GetSubsystem<Audio>()->SetListener(listener);

    // you can set master volumes for the different kinds if sounds, here 30% for music
    GetSubsystem<Audio>()->SetMasterGain(SOUND_MUSIC,0.9);
    GetSubsystem<Audio>()->SetMasterGain(SOUND_EFFECT,1.0);

    //sound=cache->GetResource<Sound>("Music/Background Beat1.wav");
    //sound->SetLooped(false);  // sound can be set to be repeated
    // you can use an existing or a new node to append the sound to
    Node* node=scene_->CreateChild("Sound");

    soundSourceCrowd = node->CreateComponent<SoundSource>();
    soundSourceCrowd->SetSoundType(SOUND_EFFECT);  // optional
    soundSourceCrowd->SetGain(0.2f);
    //soundSource->Play(sound2);

    matchTime = new Timer();

    teamA.name = "Finland";
    teamA.id = 0;
    teamA.shortName = "FIN";
    teamA.color = Color(1.0f,1.0f,1.0f,1.0f);
    teamA.clothesMaterial = "Materials/Soccerplayer01SoccershortsandshirtTightsmaterial.xml";
    teamA.homeTeam = true;
    teamA.attackRight = true;

    teamB.name = "Sweden";
    teamB.id = 1;
    teamB.shortName = "SWE";
    teamB.color = Color(1.0f,1.0f,0.0f,1.0f);
    teamB.clothesMaterial = "Materials/TeamYellowClothes.xml";

    teamC.id = 2;
    teamC.name = "England";
    teamC.shortName = "ENG";
    teamC.color = Color(1.0f,0.0f,0.0f,1.0f);
    teamC.clothesMaterial = "Materials/TeamRedClothes.xml";

    homeTeam = teamA;
    awayTeam = teamC;

    ball = new Ball(context_, scene_, 0.00003f);

    Player::field_.size_ = Vector2(100.0f, 70.0f);
    Player::field_.goalA_ = scene_->CreateChild("GoalA");
    Player::field_.goalA_->SetPosition(Vector3(-0.5f*Player::field_.size_.x_, 0.0f, 0.0f));
    Player::field_.goalB_ = scene_->CreateChild("GoalB");
    Player::field_.goalB_->SetPosition(Vector3( 0.5f*Player::field_.size_.x_, 0.0f, 0.0f));
    Player::field_.ball_ = ball;
    Player::ball_ = ball;
    Player::context_ = context_;
    Player::scene_ = scene_;
    Player::camera_ = camera;
    Player::cameraNode_ = cameraNode_;

    Ball::field_ = Player::field_;

    Player::ui_ = context_->GetSubsystem<UI>();;

    using namespace Update;

    auto* input = GetSubsystem<Input>();

    userHuman1 = new User();
    userHuman1->userType_ = UserType::UT_HUMAN;
    userHuman1->color_ = Color::RED;
    userHuman1->joystick_ = input->GetJoystickByIndex(0);
    userHuman1->arrowMaterialUrl_ = "Materials/ArrowRed.xml";

    userHuman2 = new User();
    userHuman2->userType_ = UserType::UT_HUMAN;
    userHuman2->color_ = Color::BLUE;
    userHuman2->joystick_ = input->GetJoystickByIndex(1);
    userHuman2->arrowMaterialUrl_ = "Materials/ArrowBlue.xml";

    userCPU = new User();
    userCPU->userType_ = UserType::UT_CPU;
    userCPU->color_ = Color::TRANSPARENT;
    userCPU->joystick_ = nullptr;
    userCPU->player_ = nullptr;
    userCPU->arrowMaterialUrl_ = "Materials/ArrowNone.xml";

    Player::players_[0] = new Player(homeTeam, "Player01", Vector3(- 2.0f, -0.3f, -0.0f), userHuman1, FieldPosition::FP_STRIKER );
    Player::players_[1] = new Player(awayTeam, "Player03", Vector3( 49.0f, -0.3f, -0.0f), userCPU   , FieldPosition::FP_GK );
    /*
    Player::players_[2] = new Player(homeTeam, "Player02", Vector3(-25.0f, -0.3f, -0.0f), userCPU   , FieldPosition::FP_CB );

    Player::players_[3] = new Player(homeTeam, "Player04", Vector3(- 2.0f, -0.3f,-10.0f), userCPU   , FieldPosition::FP_STRIKER );
*/
    //Player::players_[2] = new Player(awayTeam, "Player05", Vector3(  2.0f, -0.3f, -0.0f), userCPU   , FieldPosition::FP_STRIKER );
  /*  Player::players_[5] = new Player(awayTeam, "Player06", Vector3( 25.0f, -0.3f, -0.0f), userCPU   , FieldPosition::FP_CB );
    Player::players_[6] = new Player(awayTeam, "Player07", Vector3( 49.0f, -0.3f, -0.0f), userCPU   , FieldPosition::FP_GK );
    Player::players_[7] = new Player(awayTeam, "Player08", Vector3(  2.0f, -0.3f, 10.0f), userCPU   , FieldPosition::FP_STRIKER );

    for(int i=8;i<NUM_PLAYERS;i++)
        Player::players_[i] = new Player(awayTeam, "Player"+String(i), Vector3( -48.0f+i, -0.3f, -10.0f), userCPU, FieldPosition::FP_CB );
*/
    Player::lastPlayerWhoTouchedBall_ = Player::players_[0];

    referee = new Referee(awayTeam, "Referee", Vector3( 0.0f, 1.0f, -30.0f), userCPU, FieldPosition::FP_CB );

    userHuman1->player_ = Player::players_[0];
    //userHuman2->player_ = Player::players_[3];

    Player::gameEventTimer_ = new Timer();
    Player::showDebugGeometry_ = false;

    soundGoal01 = cache->GetResource<Sound>("Sounds/Soccer/424228__jeanpaul477__soccer-game_edited.wav");

}

void CharacterDemo::CreateInstructions()
{
    auto* cache = GetSubsystem<ResourceCache>();
    //auto* ui = GetSubsystem<UI>();

    Text* homeTeamText = (Text*)ui->GetRoot()->GetChild("homeTeam",true);
    homeTeamText->SetText(homeTeam.shortName);
    Text* awayTeamText = (Text*)ui->GetRoot()->GetChild("awayTeam",true);
    awayTeamText->SetText(awayTeam.shortName);

    // Construct new Text object, set string to display and font to use
    instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetName("InstructionText");
    instructionText->SetColor(Color(0.9f,0.9f,0.9f,0.9f));
    instructionText->SetText("testi");

    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 8);
    // The text has multiple rows. Center them in relation to each other
    instructionText->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_RIGHT);
    instructionText->SetVerticalAlignment(VA_TOP);
    instructionText->SetPosition(0, 0);
}

void CharacterDemo::SubscribeToEvents()
{
    // Subscribe to Update event for setting the character controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharacterDemo, HandleUpdate));

    // Subscribe to PostUpdate event for updating the camera position after physics simulation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostUpdate));

    //SubscribeToEvent(E_ASYNCLOADFINISHED, URHO3D_HANDLER(CharacterDemo, HandleASyncLoadFinished));

    // Subscribe HandlePostRenderUpdate() function for processing the post-render update event, during which we request
    // debug geometry
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostRenderUpdate));
}

void CharacterDemo::CreateRagdollBone(Node* rootNode, const String& boneName, ShapeType type, const Vector3& size, const Vector3& position,
    const Quaternion& rotation)
{
    // Find the correct child scene node recursively
    Node* boneNode = rootNode->GetChild(boneName, true);
    if (!boneNode)
    {
        URHO3D_LOGWARNING("Could not find bone " + boneName + " for creating ragdoll physics components");
        return;
    }

    auto* body = boneNode->CreateComponent<RigidBody>();
    // Set mass to make movable
    body->SetMass(1.0f);
    // Set damping parameters to smooth out the motion
    body->SetLinearDamping(0.05f);
    body->SetAngularDamping(0.85f);
    // Set rest thresholds to ensure the ragdoll rigid bodies come to rest to not consume CPU endlessly
    body->SetLinearRestThreshold(1.5f);
    body->SetAngularRestThreshold(2.5f);

    auto* shape = boneNode->CreateComponent<CollisionShape>();
    // We use either a box or a capsule shape for all of the bones
    if (type == SHAPE_BOX)
        shape->SetBox(size, position, rotation);
    else
        shape->SetCapsule(size.x_, size.y_, position, rotation);
}

void CharacterDemo::SpawnObject(int objtype)
{
/*
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    const float OBJECT_VELOCITY = 10.0f;

    // Create a smaller box at camera position
    Node* boxNode = scene_->CreateChild("SmallBox");

    if (objtype == 0)
    {
        StaticModel *boxObject = boxNode->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/uvMat.xml"));
        boxObject->SetCastShadows(true);

        SoftBody *softbody = boxNode->CreateComponent<SoftBody>();
        softbody->CreateFromStaticModel();
        softbody->SetTransform(cameraNode_->GetPosition() + cameraNode_->GetDirection(), cameraNode_->GetRotation());
        softbody->SetMass(10.0f);
        softbody->SetVelocity(cameraNode_->GetRotation() * Vector3(0.0f, 0.25f, 1.0f) * OBJECT_VELOCITY);

        // recalculate normals based on the mdl triangle faces
        // consider enabling this only for models such as a box
        //softbody->SetFaceNormals(true);
    }
    else if (objtype == 1)
    {
        boxNode->SetPosition(cameraNode_->GetPosition() + cameraNode_->GetDirection());
        boxNode->SetRotation(cameraNode_->GetRotation());
        boxNode->SetScale(0.5f);
        StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/cubeMat.xml"));
        boxObject->SetCastShadows(true);

        // Create physics components, use a smaller mass also
        RigidBody* body = boxNode->CreateComponent<RigidBody>();
        body->SetMass(0.25f);
        body->SetFriction(0.75f);
        CollisionShape* shape = boxNode->CreateComponent<CollisionShape>();
        shape->SetBox(Vector3::ONE);
        body->SetLinearVelocity(cameraNode_->GetRotation() * Vector3(0.0f, 0.25f, 1.0f) * OBJECT_VELOCITY);
    }
    else
    {
        boxNode->SetPosition(cameraNode_->GetPosition() + cameraNode_->GetDirection());
        boxNode->SetRotation(cameraNode_->GetRotation());
        boxNode->SetScale(0.75f);
        StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/cubeMat.xml"));
        boxObject->SetCastShadows(true);

        // Create physics components, use a smaller mass also
        RigidBody* body = boxNode->CreateComponent<RigidBody>();
        body->SetMass(0.5f);
        body->SetFriction(0.75f);
        body->SetAngularDamping(0.5f);
        CollisionShape* shape = boxNode->CreateComponent<CollisionShape>();
        shape->SetSphere(1.0f);
        body->SetLinearVelocity(cameraNode_->GetRotation() * Vector3(0.0f, 0.25f, 1.0f) * OBJECT_VELOCITY);
    }
    */
}

void CharacterDemo::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    if (Player::showDebugGeometry_)
    {
        DebugRenderer* dbgRenderer = scene_->GetComponent<DebugRenderer>();
        if (dbgRenderer)
        {
            // Draw Physics data :
            PhysicsWorld * phyWorld = scene_->GetComponent<PhysicsWorld>();
            phyWorld->DrawDebugGeometry(dbgRenderer,false);

            /*
            Player::players_[0]->node_ = scene_->GetChild("Player01", true);
            Node* ball->node_ = scene_->GetChild("SoccerBall2", true);

            if(!ball->node_ or !Player::players_[0]->node_) return;

            //ball->body_ = ball->node_->GetComponent<RigidBody>();

            if(!ball->body_) return;

            float vectorLength = 20.0f;
            Vector3 a = ball->body_->GetPosition();
            Vector3 c = Vector3(50.0f,1.0f,0.0f);

            Vector3 dirGoal = c-a;
            dirGoal.Normalize();

            Vector3 dirPlayer = Player::players_[0]->node_->GetDirection();

            Vector3 dirAverage = 0.5f * dirGoal + 0.5f * dirPlayer;

            dbgRenderer->AddLine(a, a + vectorLength * dirGoal,     Color(1, 1, 1, 1), false);
            dbgRenderer->AddLine(a, a + vectorLength * dirPlayer,   Color(0, 0, 1, 1), false);
            dbgRenderer->AddLine(a, a + vectorLength * dirAverage,  Color(1, 0, 0, 1), false);
            */
        }
    }
}

void CharacterDemo::HandleASyncLoadFinished(StringHash eventType, VariantMap& eventData)
{
    ball->node_ = scene_->GetChild("SoccerBall2", true);

    if(showRibbontrail)
    {
        // Add ribbon trail to ball->
        ball->node_ = scene_->GetChild("SoccerBall2", true);
        auto ballTrail = ball->node_->CreateComponent<RibbonTrail>();

        // Set ball trail type to bone and set other parameters.
        ballTrail->SetTrailType(TT_BONE);
        ballTrail->SetMaterial(cache->GetResource<Material>("Materials/SlashTrail.xml"));
        ballTrail->SetLifetime(2.0f);
        ballTrail->SetStartColor(Color(0.0f, 0.0f, 1.0f, 0.9f));
        ballTrail->SetEndColor(Color(1.0f, 0.0f, 0.0f, 0.0f));
        ballTrail->SetTailColumn(4);
        ballTrail->SetUpdateInvisible(true);
    }

}

void CharacterDemo::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{

    using namespace NodeCollision;
    Node* contact_node = (Node*)eventData[P_OTHERNODE].GetPtr();
	VectorBuffer contacts = eventData[P_CONTACTS].GetBuffer();
	Vector3 pos = contacts.ReadVector3();

	if(contact_node and ( contact_node->GetName()=="SoccerBall2") and !afterGoal )
	{
        soundSourceCrowd->Play(soundGoal01);

        goalTimer = new Timer();
        Player::gameEventTimer_ = new Timer();
        afterGoal = true;

        if      (pos.x_> 40.0f)
        {
            Player::gameEvent_ = GameEvent::GE_START_CENTER_AWAY;
            goalsA++;
            for(int i=0;i<NUM_PLAYERS;i++) Player::players_[i]->moveTarget_ = Player::players_[i]->areaCenter_;
        }
        else if (pos.x_<-40.0f)
        {
            Player::gameEvent_ = GameEvent::GE_START_CENTER_HOME;
            goalsB++;
            for(int i=0;i<NUM_PLAYERS;i++) Player::players_[i]->moveTarget_ = Player::players_[i]->areaCenter_;
        }

        auto homeGoals = (Text*)ui->GetRoot()->GetChild("homeGoals",true);
        homeGoals->SetText(String(goalsA));

        auto awayGoals = (Text*)ui->GetRoot()->GetChild("awayGoals",true);
        awayGoals->SetText(String(goalsB));

        Player::players_[0]->animController_->Play("Models/88_01_backflip.ani", 0, false, 1.0f);
        Player::players_[0]->animController_->SetSpeed("Models/88_01_backflip.ani",0.5f);
        //soundSourceMusic->Play(sound);
        //selectedCamera = SIDE_PLAYER;
        //targetZoom = 5.0f;
        if(slowMotion) scene_->SetTimeScale(0.5f);
	}
}

void CharacterDemo::DrawVector( Vector3 startPoint, Vector3 direction, Color color )
{
    DebugRenderer* dbgRenderer = scene_->GetComponent<DebugRenderer>();
    if(!dbgRenderer)
        return;
    direction.Normalize();
    dbgRenderer->AddLine( startPoint, startPoint + 30.0f*direction, color, false );
}

void CharacterDemo::KickBall(Vector3 kickDirection, float kickForce, float curl)
{
    kickDirection.Normalize();
    ball->body_->SetLinearVelocity( kickDirection * kickForce);
    /// Right foot inside kick -> curl left  -> ( < 0 )
    ball->body_->SetAngularVelocity(Vector3(0.0f,curl,0.0f));
    /// Left  foot inside kick -> curl right -> ( > 0 )
    //ball->body_->SetAngularVelocity(Vector3(0.0f, 100.0f,0.0f));
    //ball->body_->SetLinearVelocity(Player::players_[0]->node_->GetRotation() * Vector3(0.0f, 0.0f, 1.0f) * kickPower * kickForce);
}

int nearestPlayerID(int p, bool needsToBeSameTeam, bool preferUpperLevel)
{
    int nearestPlayerID = p;
    float shortestDistance = 200.0f;

    for(int i = 0;i<NUM_PLAYERS;i++)
    {
        float distance = (Player::players_[p]->node_->GetPosition() - Player::players_[i]->node_->GetPosition()).Length();
        if (distance<shortestDistance and i!=p)
        {
            if(needsToBeSameTeam and Player::players_[i]->team_.id == Player::players_[p]->team_.id)
            {
                if(!preferUpperLevel)
                {
                    nearestPlayerID = i;
                    shortestDistance = distance;
                }
                else if(Player::players_[p]->team_.attackRight and Player::players_[i]->node_->GetPosition().x_>=Player::players_[p]->node_->GetPosition().x_)
                {
                    nearestPlayerID = i;
                    shortestDistance = distance;
                }
                else if(!Player::players_[p]->team_.attackRight and Player::players_[i]->node_->GetPosition().x_<=Player::players_[p]->node_->GetPosition().x_)
                {
                    nearestPlayerID = i;
                    shortestDistance = distance;
                }
            }
        }
    }
    return nearestPlayerID;
    //return Player::players_[0];
}

int nearestOtherPlayerToBallID(int p, bool needsToBeSameTeam)
{
    int nearestPlayerID = p;
    float shortestDistance = 200.0f;

    for(int i = 0;i<NUM_PLAYERS;i++)
    {
        float distance = (ball->node_->GetPosition() - Player::players_[i]->body_->GetPosition()).Length();

        if (distance<shortestDistance and i!=p)
        {
            if(needsToBeSameTeam and Player::players_[i]->team_.id == Player::players_[p]->team_.id)
            {
                nearestPlayerID = i;
                shortestDistance = distance;
            }
        }
    }
    return nearestPlayerID;
}

void CharacterDemo::ChangePlayer(int oldSelectedPlayerID, int newSelectedPlayerID)
{
    User*   tempOldUser     = Player::players_[oldSelectedPlayerID]->user_;
    Player* tempOldPlayer   = Player::players_[oldSelectedPlayerID];

    User*   tempNewUser     = Player::players_[newSelectedPlayerID]->user_;
    Player* tempNewPlayer   = Player::players_[newSelectedPlayerID];

    Player::players_[oldSelectedPlayerID]->user_             = tempNewUser;
    Player::players_[oldSelectedPlayerID]->user_->player_    = tempOldPlayer;

    Player::players_[newSelectedPlayerID]->user_             = tempOldUser;
    Player::players_[newSelectedPlayerID]->user_->player_    = tempNewPlayer;

    Bone* headBone = Player::players_[newSelectedPlayerID]->animModel_->GetSkeleton().GetBone("head");
    if (headBone)
    {
        BillboardSet* billboardArrow = headBone->node_->GetComponent<BillboardSet>();
        Material* arrowMaterial = cache->GetResource<Material>(Player::players_[newSelectedPlayerID]->user_->arrowMaterialUrl_);
        billboardArrow->SetMaterial(arrowMaterial);
    }

    Bone* headBone2 = Player::players_[oldSelectedPlayerID]->animModel_->GetSkeleton().GetBone("head");
    if (headBone2)
    {
        BillboardSet* billboardArrow = headBone2->node_->GetComponent<BillboardSet>();
        Material* arrowMaterial = cache->GetResource<Material>(Player::players_[oldSelectedPlayerID]->user_->arrowMaterialUrl_);
        billboardArrow->SetMaterial(arrowMaterial);
    }

    playerChanged = true;
}

Vector3 BallPosition()
{
    if(ball->body_)
        return ball->body_->GetPosition();
    else
        return Vector3::ZERO;
}

bool CharacterDemo::CheckPointers()
{
    if(!referee) return false;

    auto* input = GetSubsystem<Input>();

    for(int i=0;i<NUM_PLAYERS;i++)
    {
        if(!Player::players_[i])
        {
            instructionText->SetText("No Player::players_[" + String(i) +"]");
            return false;
        }
    }

    JoystickState* joystick0 = input->GetJoystickByIndex(0);

    if(!userHuman1->joystick_)
    {
        instructionText->SetText("No joystick!");
        return false;
    }

    ball->node_ = scene_->GetChild("SoccerBall2", true);

    if(!ball->node_)
        return false;

    ball->body_ = ball->node_->GetComponent<RigidBody>();

    return true;
}

void CharacterDemo::AnimationTest()
{
    auto* input = GetSubsystem<Input>();

    if(doAnimationTest)
    {
        if (input->GetKeyPress(KEY_R))
        {
            selectedAnimation = static_cast<Animations>((selectedAnimation + 1) % Animations::AN);

            ball->body_->SetPosition(Vector3(0.0f, 2.5f, 0.0f));
            ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
        }

        auto* input = GetSubsystem<Input>();

        Node* rightFootNode = Player::players_[0]->node_->GetChild("toe.R", true);

        if(!rightFootNode)
            return;
        if(!ball->node_)
            return;
        if(!ball->body_)
            return;
        if(!Player::players_[0]->node_)
            return;
        if((rightFootNode->GetWorldPosition()-ball->node_->GetPosition()).Length()<0.2f)
            ball->body_->SetLinearVelocity(Vector3(10,0,0));
            //KickBall(Player::players_[0]->node_->GetRotation()*Vector3::FORWARD,20.0f,0.0f);

        DebugRenderer* dbgRenderer = scene_->GetComponent<DebugRenderer>();
        if(!dbgRenderer)
            return;
        //dbgRenderer->AddNode( rightFootNode, 1.0f, false );

        Player::players_[0]->node_->SetPosition(Vector3(-0.25f,-0.23f,-0.8f));

        Player::players_[0]->animController_->PlayExclusive("Models/11_01.ani", 0, true, 1.2f);
        //Player::players_[0]->animController_->SetSpeed("Models/11_01.ani", joystick[playerControlledWithController_0_ID].GetAxisPosition(2));
        Vector3 playerPosition = Player::players_[0]->node_->GetPosition();
        Vector3 lookAtPosition = playerPosition;

        lookAtPosition.y_ += 1.1;

        Vector3 cameraPosition = lookAtPosition;

        cameraPosition.x_ -= 3.0f;

        float cameraHorizontalAngle = input->GetMousePosition().x_*M_PI;

        cameraPosition.x_ = 3.0f * sin(cameraHorizontalAngle);
        cameraPosition.z_ = 3.0f * cos(cameraHorizontalAngle);

        cameraNode_->LookAt     ( lookAtPosition, Vector3::UP );
        cameraNode_->SetPosition( cameraPosition );
        return;
    }
}

void CharacterDemo::UpdateFPS()
{
    float smoothDeltaTime = GetSubsystem<Engine>()->GetNextTimeStep();
    averageDeltaTime = 0.01f*smoothDeltaTime + 0.99f*averageDeltaTime;
    auto textFPS = (Text*)ui->GetRoot()->GetChild("FPSCounter",true);
    if (textFPS and averageDeltaTime>0.0f)
        textFPS->SetText("FPS: "+String((int)(1.0f/averageDeltaTime)));
}

void CharacterDemo::UpdateTimeText()
{
    auto textTime = (Text*)ui->GetRoot()->GetChild("time",true);
    unsigned seconds = matchTime->GetMSec(false)/1000;
    unsigned minutes = seconds/60;
    seconds = seconds % 60;
    if(seconds<10)
        textTime->SetText(String(minutes)+":0"+String(seconds));
    else
        textTime->SetText(String(minutes)+":"+String(seconds));
}

void CharacterDemo::CheckTriggers()
{
    Node* soccerGoalATriggerNode = scene_->GetChild("SoccerGoalATrigger", true);
    if(soccerGoalATriggerNode and (!subscribedA))
    {
        SubscribeToEvent(soccerGoalATriggerNode, E_NODECOLLISIONSTART, URHO3D_HANDLER(CharacterDemo, HandleNodeCollisionStart));
        subscribedA = true;
    }
	Node* soccerGoalBTriggerNode = scene_->GetChild("SoccerGoalBTrigger", true);
	if(soccerGoalBTriggerNode and (!subscribedB))
	{
        SubscribeToEvent(soccerGoalBTriggerNode, E_NODECOLLISIONSTART, URHO3D_HANDLER(CharacterDemo, HandleNodeCollisionStart));
        subscribedB = true;
    }
}

void CharacterDemo::HandleGameEvents()
{
    auto* input = GetSubsystem<Input>();

    // Free kick
    if (input->GetKeyPress(KEY_F))
    {
        /*
        ball->body_->SetPosition(Vector3(38.75f, 0.5f, 0.0f));
        ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
        ball->body_->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));

        Player::players_[0]->body_->SetPosition(Vector3(35.0f, 0.5f, 0.0f));
        Player::players_[0]->body_->SetRotation( Quaternion(90, Vector3::UP ) );
        Player::players_[0]->body_->SetLinearVelocity(Vector3::ZERO);*/
        Ball::freeKickPosition_ = ball->GetPosition();

        Player::gameEvent_ = GameEvent::GE_FREE_KICK;
    }

    // Penalty kick
    if (input->GetKeyPress(KEY_P))
    {
        ball->body_->SetPosition(Vector3(38.75f, 0.5f, 0.0f));
        ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
        ball->body_->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));

        Player::players_[0]->body_->SetPosition(Vector3(35.0f, 0.5f, 0.0f));
        Player::players_[0]->body_->SetRotation( Quaternion(90, Vector3::UP ) );
        Player::players_[0]->body_->SetLinearVelocity(Vector3::ZERO);

        Player::gameEvent_ = GameEvent::GE_PENALTY_KICK;
    }

    if(Player::gameEvent_ == GameEvent::GE_GAME_ON)
    {

        ball->readyForGameEvent_ = false;
        //if(!ball->readyForGameEvent_ and Player::Player::gameEventTimer_ and Player::Player::gameEventTimer_->GetMSec(false)>5000)
            //selectedCamera = CameraOrientation::SIDE_BALL;

        if(BallPosition().x_> 50.0f and ( abs(BallPosition().z_)>3.7f or BallPosition().y_>2.44f ) )
        {
            //soundSource->Play(soundWhistle);
            referee->Whistle();

            if(Player::players_[lastPlayerWhoTouchedBallID]->team_.attackRight)
            {
                Player::gameEvent_ = GameEvent::GE_GOAL_KICK_AWAY;
            }
            else
            {
                Player::gameEvent_ = GameEvent::GE_CORNER_KICK;
            }

            Player::Player::gameEventTimer_ = new Timer();

            //ball->body_->SetPosition(Vector3(44.6f, 0.5f, 8.0f));
            //ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
            //ball->body_->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));

            //Player::players_[0]->body_->SetPosition(Vector3(35.0f, 0.5f, 0.0f));
            //Player::players_[0]->body_->SetRotation( Quaternion(90, Vector3::UP ) );
            //Player::players_[0]->body_->SetLinearVelocity(Vector3::ZERO);
        }

        if(BallPosition().x_<-50.0f and ( abs(BallPosition().z_)>3.7f or BallPosition().y_>2.44f ) )
        {
            referee->Whistle();

            if(!Player::players_[lastPlayerWhoTouchedBallID]->team_.attackRight)
            {
                Player::gameEvent_ = GameEvent::GE_GOAL_KICK_HOME;
            }
            else
            {
                Player::gameEvent_ = GameEvent::GE_CORNER_KICK;
            }

            Player::Player::gameEventTimer_ = new Timer();

            //Player::players_[0]->body_->SetPosition(Vector3(35.0f, 0.5f, 0.0f));
            //Player::players_[0]->body_->SetRotation( Quaternion(90, Vector3::UP ) );
            //Player::players_[0]->body_->SetLinearVelocity(Vector3::ZERO);
        }

        // Side throw
         if(ball->body_->GetPosition().z_> 35.0f or ball->body_->GetPosition().z_<-35.0f)
        {
            Player::gameEvent_ = GameEvent::GE_SIDE_THROW;
            referee->Whistle();
            Player::Player::gameEventTimer_ = new Timer();
/*
            int playerID = 0;
            if(Player::players_[lastPlayerWhoTouchedBallID]->team_.homeTeam)
                playerID = nearestPlayerToBallID(awayTeam.id,false);
            else
                playerID = nearestPlayerToBallID(homeTeam.id,false);

            Player::players_[playerID]->moveTarget_ = ball->node_;
*/
/*
            if(ball->body_->GetPosition().z_>0)
            {
                ball->body_->SetPosition(Vector3(ball->body_->GetPosition().x_, 0.5f,  34.9f));
            }
            else
            {
                ball->body_->SetPosition(Vector3(ball->body_->GetPosition().x_, 0.5f, -34.9f));
            }

            ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
*/
            //Player::players_[0]->body_->SetPosition(Vector3(35.0f, 0.5f, 0.0f));
            //Player::players_[0]->body_->SetRotation( Quaternion(90, Vector3::UP ) );
            //Player::players_[0]->body_->SetLinearVelocity(Vector3::ZERO);
        }
    }

    switch(Player::gameEvent_)
    {
        case GameEvent::GE_CORNER_KICK          : instructionText->SetText("Corner kick");      break;
        case GameEvent::GE_FREE_KICK            : instructionText->SetText("Free kick");        break;
        case GameEvent::GE_GAME_ON              : instructionText->SetText("Game on");          break;
        case GameEvent::GE_GOAL_KICK_AWAY       : instructionText->SetText("Goal kick away");   break;
        case GameEvent::GE_GOAL_KICK_HOME       : instructionText->SetText("Goal kick home");   break;
        case GameEvent::GE_PENALTY_KICK         : instructionText->SetText("Penalty kick");     break;
        case GameEvent::GE_SIDE_THROW           : instructionText->SetText("Side throw");       break;
        case GameEvent::GE_START_CENTER_AWAY    : instructionText->SetText("Start Center Away");break;
        case GameEvent::GE_START_CENTER_HOME    : instructionText->SetText("Start Center Home");break;
    }

    if(Player::Player::gameEventTimer_ and Player::Player::gameEventTimer_->GetMSec(false)>1000)
    {
        if(Player::gameEvent_ == GameEvent::GE_SIDE_THROW)
        {
            //Vector3 playerPosition = userHuman1->player_->GetPosition();
            Vector3 playerPosition = userHuman1->player_->node_->GetPosition();
            playerPosition.y_ = -0.27f;

            if(BallPosition().z_ < 0.0f)
            {
                userHuman1->player_->node_->SetRotation(Quaternion(0,90,0));
                playerPosition.z_ = -34.8f;
            }
            else
            {
                userHuman1->player_->node_->SetRotation(Quaternion(0,270,0));
                playerPosition.z_ =  34.8f;
            }

            userHuman1->player_->node_->SetPosition(playerPosition);

            userHuman1->player_->animController_->PlayExclusive("Models/pick_115_10_B.ani", 0, false, 1.2f);
            float animationTime = userHuman1->player_->animController_->GetTime("Models/pick_115_10_B.ani");
            if(animationTime<2.0f)
            {
                Node* boneNode1 = userHuman1->player_->node_->GetChild("hand.R", true);
                Node* boneNode2 = userHuman1->player_->node_->GetChild("hand.L", true);
                Vector3 average = 0.5f*boneNode1->GetWorldPosition() + 0.5f*boneNode2->GetWorldPosition();
                ball->body_->SetPosition(average);
                ball->body_->SetLinearVelocity(Vector3::ZERO);
                ball->body_->SetAngularVelocity(Vector3::ZERO);
            }
            else
            {
                ball->body_->SetLinearVelocity(3.0f*(-userHuman1->player_->node_->GetRight()+Vector3::UP));
                Player::gameEvent_ = GameEvent::GE_GAME_ON;
            }

            //if(animationTime>3.0f) Player::gameEvent_ = GameEvent::GE_GAME_ON;
        }
    }
    if(Player::Player::gameEventTimer_ and Player::Player::gameEventTimer_->GetMSec(false)>3000)
    {
        // Reset ball position to center 5 seconds after scoring
        //float afterGoalTime = 5000;
        //if(sound) afterGoalTime = sound->GetLength()*1000;
        //if(goalTimer and afterGoal and (goalTimer->GetMSec(false)>afterGoalTime))
        //referee->Whistle();

        if(Player::gameEvent_ == GameEvent::GE_FREE_KICK)
        {
            if(!ball->readyForGameEvent_)
            {
                Vector3 FKBallPosition = Vector3(10.0f, 0.3f, 5.0f);
                ball->body_->SetPosition(FKBallPosition);
                ball->body_->SetLinearVelocity(Vector3::ZERO);

                userHuman1->player_->kickTarget_ = userHuman1->player_->targetGoal_;
                userHuman1->player_->body_->SetPosition(FKBallPosition+Vector3(-1.0f,0.0f,0.3f));
                userHuman1->player_->body_->SetRotation(Quaternion(0,userHuman1->player_->AngleToKickTarget()+90,0));
                userHuman1->player_->body_->SetLinearVelocity(Vector3::ZERO);

                ball->readyForGameEvent_ = true;
                selectedCamera = CameraOrientation::CO_FORWARD;
            }

            //Player::gameEvent_ = GameEvent::GE_GAME_ON;
        }

        if(Player::gameEvent_ == GameEvent::GE_START_CENTER_HOME)
        {
            referee->Whistle();
            Player::players_[lastPlayerWhoTouchedBallID]->animController_->Play("Models/13_30.ani", 0, false, 0.2f);

            if(resetPositions)
            {
                ball->body_->SetPosition(Vector3(0.0f, 0.5f, 0.0f));
                ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));

                //int playerID = nearestPlayerToBallID(homeTeam.id,true);
                for(int i=0;i<NUM_PLAYERS;i++) Player::players_[i]->moveTarget_ = Player::players_[i]->areaCenter_;
                //Player::players_[playerID]->body_->SetPosition(Vector3(-2.0f, 0.5f, 0.0f));
                //Player::players_[playerID]->body_->SetRotation( Quaternion(90, Vector3::UP ) );
                //Player::players_[playerID]->body_->SetLinearVelocity(Vector3::ZERO);
            }
            //targetZoom = 1.0f;
            afterGoal = false;
            scene_->SetTimeScale(1.0f);
            Player::gameEvent_ = GameEvent::GE_GAME_ON;
        }

        if(Player::gameEvent_ == GameEvent::GE_START_CENTER_AWAY)
        {
            referee->Whistle();
            Player::players_[lastPlayerWhoTouchedBallID]->animController_->Play("Models/13_30.ani", 0, false, 0.2f);


            if(resetPositions)
            {
                ball->body_->SetPosition(Vector3(0.0f, 0.5f, 0.0f));
                ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));

                for(int i=0;i<NUM_PLAYERS;i++) Player::players_[i]->moveTarget_ = Player::players_[i]->areaCenter_;

                //int playerID = nearestPlayerToBallID(awayTeam.id,true);
                //Player::players_[playerID]->body_->SetPosition(Vector3(2.0f, 0.5f, 0.0f));
                //Player::players_[playerID]->body_->SetRotation( Quaternion(-90, Vector3::UP ) );
                //Player::players_[playerID]->body_->SetLinearVelocity(Vector3::ZERO);
            }
            //targetZoom = 1.0f;
            afterGoal = false;
            scene_->SetTimeScale(1.0f);
            Player::gameEvent_ = GameEvent::GE_GAME_ON;
        }

        if(Player::gameEvent_ == GameEvent::GE_GOAL_KICK_HOME)
        {
            ball->body_->SetPosition(Vector3(-44.6f, 0.5f, 8.0f));
            ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
            ball->body_->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));
/*
            int playerID = nearestPlayerToBallID(homeTeam.id,false);
            Player::players_[playerID]->body_->SetPosition(Vector3(-45.6f, 0.5f, 8.0f));
            Player::players_[playerID]->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
            Player::players_[playerID]->body_->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));
            Player::players_[playerID]->body_->SetRotation( Quaternion(90, Vector3::UP ) );
*/
            Player::gameEvent_ = GameEvent::GE_GAME_ON;
        }
        if(Player::gameEvent_ == GameEvent::GE_GOAL_KICK_AWAY)
        {
            ball->body_->SetPosition(Vector3( 44.6f, 0.5f, 8.0f));
            ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
            ball->body_->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));
/*
            int playerID = nearestPlayerToBallID(awayTeam.id,false);
            Player::players_[playerID]->body_->SetPosition(Vector3( 45.6f, 0.5f, 8.0f));
            Player::players_[playerID]->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
            Player::players_[playerID]->body_->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));
            Player::players_[playerID]->body_->SetRotation( Quaternion(90, Vector3::UP ) );
*/
            Player::gameEvent_ = GameEvent::GE_GAME_ON;
        }



        // Corner kick
        if (Player::gameEvent_ == GameEvent::GE_CORNER_KICK)
        {
            Vector3 ballPos = ball->body_->GetPosition();

            ball->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
            ball->body_->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));
            Player* nearestPlayer = Player::NearestPlayerToBall(homeTeam.id,true);
            nearestPlayer->body_->SetLinearVelocity(Vector3::ZERO);

            if(ballPos.x_>0)
            {
                if(ballPos.z_>0)
                {
                    ball->body_->SetPosition(Vector3( 49.5f, 0.5f, 34.5f));
                }
                else
                {
                    ball->body_->SetPosition(Vector3( 49.5f, 0.5f,-34.5f));
                }
            }
            else
            {
                if(ballPos.z_>0)
                {
                    ball->body_->SetPosition(Vector3(-49.5f, 0.5f, 34.5f));
                }
                else
                {
                    ball->body_->SetPosition(Vector3(-49.5f, 0.5f,-34.5f));
                }
            }

            //nearestPlayer->body_->SetLinearVelocity(Vector3::ZERO);
            nearestPlayer->body_->SetPosition(Vector3(49.5f, 0.5f, 35.5f));
            nearestPlayer->body_->SetRotation( Quaternion(225, Vector3::UP ) );

            //Player::gameEvent_ = GameEvent::GE_GAME_ON;
        }
    }
}

void CharacterDemo::UpdateCamera()
{
    auto* input = GetSubsystem<Input>();

    if (input->GetKeyPress(KEY_Y) or userHuman1->joystick_->GetButtonPress(SButton::SELECT))
        selectedCamera = static_cast<CameraOrientation>((selectedCamera + 1) % CameraOrientation::N);

    if (input->GetKeyPress(KEY_1))
        selectedCamera = CameraOrientation::CO_FREE;

    if (input->GetKeyPress(KEY_2))
        cameraTargetNode = Player::ball_->node_;

    if (input->GetKeyPress(KEY_3))
        cameraTargetNode = Player::field_.goalA_;

    // Zoom
    if(userHuman1->joystick_->GetButtonDown(SButton::L2))
        targetZoom = Clamp(camera->GetZoom() * (1.0f - 2.0f * userHuman1->joystick_->GetAxisPosition(3)), 1.0f, 50.0f);

    camera->SetZoom((1.0f-zoomSpeed)*camera->GetZoom()+zoomSpeed*targetZoom);

    // TODO if zoom changes
    //if(zoomChanged)
    {
        //for(int i = 0;i<NUM_PLAYERS;i++)
           // Player::players_[i]->UpdateArrowSize();
    }

    if(cameraTargetNode)
        freeCameraNode->SetPosition(0.96f * freeCameraNode->GetPosition() + 0.04f * cameraTargetNode->GetPosition());

    auto textCamera = (Text*)ui->GetRoot()->GetChild("TextCamera",true);

    switch(selectedCamera)
    {
        case CameraOrientation::ORTHO_SIDE :
        {
            textCamera->SetText("Orthogonal Side");
            //freeCameraNode->SetRotation(Quaternion(0, 0, 0));
            Vector3 oldPos = freeCameraNode->GetPosition();
            freeCameraNode->SetPosition(Vector3(oldPos.x_,oldPos.y_+0.1f,oldPos.z_));
            Graphics* graphics = context_->GetSubsystem<Graphics>();
            camera->SetOrthographic(true); // Set camera orthographic
            camera->SetOrthoSize((float)graphics->GetHeight()*0.02f); // Set camera ortho size (the value of PIXEL_SIZE is 0.01)
            Vector3 lookAtPosition = freeCameraNode->GetPosition();
            cameraYaw = 0.0f;
            cameraPitch = 0.01f;
            Vector3 cameraOffset(0.0f, 0.0f, -20.0f);  // Camera offset relative to target node
            cameraNode_->SetPosition(cameraOffset);  // Set new camera position and lookat values
            freeCameraNode->SetRotation(Quaternion(cameraPitch, cameraYaw, 0.0f));
            cameraNode_->LookAt( lookAtPosition, Vector3::UP );
            break;
        }
        case CameraOrientation::SIDE_BALL :
        {
            textCamera->SetText("Side Ball");
            cameraTargetNode = Player::ball_->node_;
            camera->SetOrthographic(false);
            Vector3 lookAtPosition = freeCameraNode->GetPosition();
            cameraYaw = 0.0f;
            cameraPitch = 25.0f;
            Vector3 cameraOffset(0.0f, 0.0f, -80.0f);  // Camera offset relative to target node
            cameraNode_->SetPosition(cameraOffset);  // Set new camera position and lookat values
            freeCameraNode->SetRotation(Quaternion(cameraPitch, cameraYaw, 0.0f));
            cameraNode_->LookAt( lookAtPosition, Vector3::UP );
            break;
        }
        case CameraOrientation::SIDE_PLAYER :
        {
            textCamera->SetText("Side Player");
            cameraTargetNode = userHuman1->player_->node_;
            camera->SetOrthographic(false);
            Vector3 lookAtPosition = freeCameraNode->GetPosition();
            cameraPitch = 20.0f;
            cameraYaw = 0.0f;
            Vector3 cameraOffset(0.0f, 0.0f, -90.0f);  // Camera offset relative to target node
            cameraNode_->SetPosition(cameraOffset);  // Set new camera position and lookat values
            freeCameraNode->SetRotation(Quaternion(cameraPitch, cameraYaw, 0.0f));
            cameraNode_->LookAt( lookAtPosition, Vector3::UP );
            break;
        }
        case CameraOrientation::TOP_PLAYER :
        {
            textCamera->SetText("Top Player");
            cameraTargetNode = userHuman1->player_->node_;
            Graphics* graphics = context_->GetSubsystem<Graphics>();
            camera->SetOrthographic(true); // Set camera orthographic
            camera->SetOrthoSize((float)graphics->GetHeight()*0.02f); // Set camera ortho size (the value of PIXEL_SIZE is 0.01)
            Vector3 lookAtPosition = freeCameraNode->GetPosition();
            cameraYaw = 0.0f;
            cameraPitch = 90.0f;
            Vector3 cameraOffset(0.0f, 0.0f, -20.0f);  // Camera offset relative to target node
            cameraNode_->SetPosition(cameraOffset);  // Set new camera position and lookat values
            freeCameraNode->SetRotation(Quaternion(cameraPitch, cameraYaw, 0.0f));
            cameraNode_->LookAt( lookAtPosition, Vector3::FORWARD );
            break;
        }
        case CameraOrientation::TOP_BALL :
        {
            textCamera->SetText("Top Ball");
            cameraTargetNode = Player::ball_->node_;
            Graphics* graphics = context_->GetSubsystem<Graphics>();
            camera->SetOrthographic(true); // Set camera orthographic
            camera->SetOrthoSize((float)graphics->GetHeight()*0.02f); // Set camera ortho size (the value of PIXEL_SIZE is 0.01)
            Vector3 lookAtPosition = freeCameraNode->GetPosition();
            cameraYaw = 0.0f;
            cameraPitch = 90.0f;
            Vector3 cameraOffset(0.0f, 0.0f, -20.0f);  // Camera offset relative to target node
            cameraNode_->SetPosition(cameraOffset);  // Set new camera position and lookat values
            freeCameraNode->SetRotation(Quaternion(cameraPitch, cameraYaw, 0.0f));
            cameraNode_->LookAt( lookAtPosition, Vector3::FORWARD );
            break;
        }
        case CameraOrientation::CO_FREE :
        {
            textCamera->SetText("Free");
            if(!cameraTargetNode) cameraTargetNode = Player::ball_->node_;
            camera->SetOrthographic(false);
            Vector3 lookAtPosition = freeCameraNode->GetPosition();
            if(!userHuman1->joystick_->GetButtonDown(SButton::L2))
            {
                cameraYaw     += userHuman1->joystick_->GetAxisPosition(2);  // Rotate camera N degrees
                cameraPitch   -= userHuman1->joystick_->GetAxisPosition(3);  // Rotate camera N degrees
                cameraPitch = Clamp(cameraPitch, 1.0f, 89.0f);
            }

            Vector3 cameraOffset(0.0f, 0.0f, -30.0f);  // Camera offset relative to target node

            cameraNode_->SetPosition(cameraOffset);  // Set new camera position and lookat values
            freeCameraNode->SetRotation(Quaternion(cameraPitch, cameraYaw, 0.0f));
            cameraNode_->LookAt( lookAtPosition, Vector3::UP );
            break;
        }
        case CameraOrientation::CO_FORWARD :
        {
            textCamera->SetText("Forward");
            //freeCameraNode->SetRotation(Quaternion(0, 0, 0));
            Vector3 oldPos = freeCameraNode->GetPosition();
            freeCameraNode->SetPosition(Vector3(oldPos.x_,oldPos.y_+0.1f,oldPos.z_));
            Graphics* graphics = context_->GetSubsystem<Graphics>();
            camera->SetOrthographic(false); // Set camera orthographic
            Vector3 lookAtPosition = freeCameraNode->GetPosition();
            cameraYaw = 90.0f;
            cameraPitch = 5.0f;
            Vector3 cameraOffset(0.0f, 0.0f, -30.0f);  // Camera offset relative to target node
            cameraNode_->SetPosition(cameraOffset);  // Set new camera position and lookat values
            freeCameraNode->SetRotation(Quaternion(cameraPitch, cameraYaw, 0.0f));
            cameraNode_->LookAt( lookAtPosition, Vector3::UP );
            break;
        }
        default :
        {
            textCamera->SetText("----");
            break;
        }
    }

}

void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    // Take the frame time step, which is stored as a float
    float dt = eventData[Update::P_TIMESTEP].GetFloat();

    if(!CheckPointers()) return;

    auto* input = GetSubsystem<Input>();

    DebugRenderer* dbgRenderer = scene_->GetComponent<DebugRenderer>();


    Graphics* graphics = context_->GetSubsystem<Graphics>();
    float width = (float)(graphics->GetWidth());
    float height = (float)(graphics->GetHeight());
/*
    auto* phyWorld = scene_->GetComponent<PhysicsWorld>();

    // Cast ray down to get the normal of the underlying surface
    PhysicsRaycastResult result;

    Vector2 mousePosition = Vector2((float)(input->GetMousePosition().x_)/width,(float)(input->GetMousePosition().y_)/height);
    //phyWorld->RaycastSingle(result, camera->GetScreenRay(mousePosition.x_,mousePosition.y_), 300.0f);

    if(draggedBody) dbgRenderer->AddSphere( Sphere(draggedBody->GetPosition(),0.1f), Color(1,0,1,1), false );

    if(input->GetMouseButtonDown(1))
    {
        if(!isDragging)
        {
            phyWorld->RaycastSingle(result, camera->GetScreenRay(mousePosition.x_,mousePosition.y_), 300.0f);

            if (result.body_)
            {
                draggedBody = result.body_;
                //draggedBody->ApplyImpulse(Vector3::UP);
                Vector3 mousePosition3D = camera->ScreenToWorldPoint(Vector3(mousePosition.x_,mousePosition.y_,300.0f));
                mousePosition3D.z_ = draggedBody->GetPosition().z_;
                //draggedBody->ApplyImpulse(0.1f*mousePosition3D-draggedBody->GetPosition());
                //if(draggedBody) draggedBody->SetKinematic(true);
                isDragging = true;
            }
        }
        else if(draggedBody)
        {
            //draggedBody->SetPosition(result.position_);
            Vector2 mousePosition = Vector2((float)(input->GetMousePosition().x_)/width,(float)(input->GetMousePosition().y_)/height);
            //Vector3 mousePosition3D = camera->ScreenToWorldPoint(Vector3(mousePosition.x_,mousePosition.y_,0.0f));
            Vector2 bodyScreenPosition = camera->WorldToScreenPoint(draggedBody->GetPosition());
            Vector3 mousePosition3D = (mousePosition.x_-bodyScreenPosition.x_)*cameraNode_->GetRight()-(mousePosition.y_-bodyScreenPosition.y_)*cameraNode_->GetUp();
            //mousePosition3D.z_ = draggedBody->GetPosition().z_;
            //draggedBody->ApplyImpulse(mousePosition3D);
            //draggedBody->SetLinearVelocity(5000.0f*mousePosition3D*dt);
            draggedBody->SetPosition(draggedBody->GetPosition()+mousePosition3D);
            //draggedBody->SetPosition(mousePosition3D);
        }
    }
    else
    {
        if(draggedBody and isDragging)
        {
            //draggedBody->ApplyImpulse(Vector3::UP);
            Vector3 mousePosition3D = camera->ScreenToWorldPoint(Vector3(mousePosition.x_,mousePosition.y_,300.0f));
            mousePosition3D.z_ = draggedBody->GetPosition().z_;
            //draggedBody->ApplyImpulse(0.1f*mousePosition3D-draggedBody->GetPosition());
            // Cast ray down to get the normal of the underlying surface
            //PhysicsRaycastResult result;

            //Vector2 mousePosition = Vector2((float)(input->GetMousePosition().x_)/width,(float)(input->GetMousePosition().y_)/height);
            //phyWorld->RaycastSingle(result, camera->GetScreenRay(mousePosition.x_,mousePosition.y_), 300.0f);

            //draggedBody->SetKinematic(false);
            isDragging = false;
        }
        //isDragging = false;
    }

    //Debug.Log(v3); //Current Position of mouse in world space
    //return v3;
*/

    //Vector3 playerScreenPosition = camera->ScreenToWorldPoint(userHuman1->player_->GetPosition());
    Vector2 playerScreenPosition = camera->WorldToScreenPoint(userHuman1->player_->GetPosition());
    auto powerBar = (ProgressBar*)ui->GetRoot()->GetChild("PowerBar",true);

    //powerBar->SetPosition(IntVector2(-270+width*playerScreenPosition.x_,-160+height*playerScreenPosition.y_));
    powerBar->SetPosition(IntVector2(width*playerScreenPosition.x_-powerBar->GetWidth()/2,height*playerScreenPosition.y_ + 20));

    if (input->GetKeyPress(KEY_R))
    {
        replay.on_ = !replay.on_;
        //scene_->SetUpdateEnabled(!replay.on_);
    }

    if(replay.on_)
    {
        //PhysicsWorld* physicsWorld  = scene_->GetComponent<PhysicsWorld>();
        //physicsWorld->SetUpdateEnabled(false);
        //replay.Show(ball,Replay::showTime);
        replay.Play(ball,userHuman1->player_, 50.0f * userHuman1->joystick_->GetAxisPosition(0) * dt);
        //replay.Play(ball,userHuman1->player_,0.5f);
        //Replay::showTime += 0.5f;
        auto replayBar = (ProgressBar*)ui->GetRoot()->GetChild("ReplayBar",true);
        replayBar->SetValue(Replay::showTime);
        return;
    }
    else
    {
        //PhysicsWorld* physicsWorld  = scene_->GetComponent<PhysicsWorld>();
        //physicsWorld->SetUpdateEnabled(true);
        //scene_->SetUpdateEnabled(true);
        replay.Save(ball,userHuman1->player_,dt);
    }

    // Right mouse button controls mouse cursor visibility: hide when pressed
    ui->GetCursor()->SetVisible(input->GetMouseButtonDown(MOUSEB_RIGHT));

    ball->Update(dt);
    UpdateFPS();
    UpdateTimeText();
    CheckTriggers();
    HandleGameEvents();

    if(userHuman1->joystick_->GetButtonPress(SButton::START))
    {
        //scene_->SetUpdateEnabled(false);
        ui->GetRoot()->SetOpacity(1.0f);
        auto* button = ui->GetRoot()->GetChildStaticCast<Button>("ResumeGame", true);
        if(button)
        {
            button->SetSelected(true);
            IntVector2 pos = button->GetScreenPosition();
            input->SetMousePosition(pos);
            //gameState = GameState::MENU;
        }
    }

    if (userHuman1->joystick_->GetButtonPress(SButton::L2))
    {
        //if( scene_->GetTimeScale() < 0.9f ) scene_->SetTimeScale(1.0f);
        //else scene_->SetTimeScale(0.2f);
    }

    //if (userHuman1->joystick_->GetButtonDown(SButton::R2))
        //userHuman1->player_->TakeBallInHands();

    if (input->GetKeyPress(KEY_SPACE))
        Player::showDebugGeometry_ = !Player::showDebugGeometry_;


    if(userHuman1->joystick_)
    {
        Vector3 controller_1_playerDirection = userHuman1->player_->GetPosition() - BallPosition();
        float controller_1_playerDistance = controller_1_playerDirection.Length();
        if(controller_1_playerDistance > 60.0f / camera->GetZoom())
            dbgRenderer->AddLine(BallPosition()+60.0f/camera->GetZoom()*controller_1_playerDirection.Normalized(), BallPosition() + controller_1_playerDirection, Color(1, 0, 0, 1), false);
    }

    if(userHuman2->joystick_)
    {
        Vector3 controller_2_playerDirection = userHuman2->player_->GetPosition() - BallPosition();
        float controller_2_playerDistance = controller_2_playerDirection.Length();
        if(controller_2_playerDistance > 60.0f / camera->GetZoom())
            dbgRenderer->AddLine(BallPosition() + 60.0f / camera->GetZoom() * controller_2_playerDirection.Normalized(), BallPosition() + controller_2_playerDirection, Color(0, 0, 1, 1), false);
    }

    for(int i = 0;i<NUM_PLAYERS;i++)
        Player::players_[i]->Update(ui, dt);

    referee->Update(ui, dt);

    //input->SetMouseVisible(true);
    //instructionText->SetText(String(input->GetMousePosition().y_));
}

void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    if(!CheckPointers()) return;

    UpdateCamera();
    //auto* input = GetSubsystem<Input>();
    //instructionText->SetText(String(input->GetMousePosition().x_));
}
