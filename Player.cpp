#include "Player.h"
#include "Ball.h"
#include "User.h"

GameEvent gameEvent = GameEvent::GE_GAME_ON;
Timer* gameEventTimer;

Player::Player(Team team, String name, Vector3 position, User* user, FieldPosition fieldPosition, float areaRadius)
{
    ui_ = context_->GetSubsystem<UI>();

    ID_ = nextID_;
    nextID_++;

    //players_[ID_] = this;

    areaCenter_ = scene_->CreateChild(name + "Area");
    areaCenter_->SetPosition(position);
    moveTarget_ = scene_->CreateChild(name + "MoveTarget");

    kickTargetPosition_ = Vector3::ZERO;
    moveTargetPosition_ = Vector3::ZERO;

    team_ = team;

    if(team_.homeTeam)
        targetGoal_ = field_.goalB_;
    else
        targetGoal_ = field_.goalA_;

    fieldPosition_ = fieldPosition;

    auto* input = context_->GetSubsystem<Input>();

    user_ = user;

    auto* cache = context_->GetSubsystem<ResourceCache>();

    name_ = name;
    node_ = scene_->CreateChild(name);
    node_->SetScale(Vector3(0.001f, 0.001f, 0.001f));
    node_->SetPosition(position);

    Node* spinNode = node_->CreateChild("SpinNode");
    spinNode->SetRotation( Quaternion(180, Vector3(0, 1, 0) ) );

    Node* kickNode = node_->CreateChild("KickNode");
    kickNode->SetPosition(1000.0f*Vector3(0.25f,-0.23f,0.8f));

    // Create the rendering component + animation controller
    animModel_ = spinNode->CreateComponent<AnimatedModel>();
    animModel_->SetModel(cache->GetResource<Model>("Models/player03.mdl"));

    if(fieldPosition_==FieldPosition::FP_GK)
        animModel_->SetMaterial(0, cache->GetResource<Material>("Materials/GoalKeeperClothes.xml"));
    else
        animModel_->SetMaterial(0, cache->GetResource<Material>(team_.clothesMaterial));

    animModel_->SetMaterial(1, cache->GetResource<Material>("Materials/Soccerplayer01High-polyEyeblue.xml"));
    animModel_->SetMaterial(2, cache->GetResource<Material>("Materials/Soccerplayer01BodyMiddleage_cauasian_male.xml"));
    animModel_->SetCastShadows(true);

    animController_ = spinNode->CreateComponent<AnimationController>();

    Node* hipsBoneNode = node_->GetChild("hips", true);

    // Create rigidbody, and set non-zero mass so that the body becomes dynamic
    body_ = node_->CreateComponent<RigidBody>();

    body_->SetCollisionLayer(1);
    body_->SetMass(1.0f);

    // Set zero angular factor so that physics doesn't turn the character on its own.
    // Instead we will control the character yaw manually
    body_->SetAngularFactor(Vector3(0.0f,0.1f,0.0f));
    body_->SetAngularDamping(0.9f);

    // Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
    body_->SetCollisionEventMode(COLLISION_ALWAYS);

    // Set a shape for collision
    auto* shape = node_->CreateComponent<CollisionShape>();
    //shape->SetCylinder(1000.0f*1.1f, 1000.0f*1.81f, Vector3(0.0f*1000.0f, 1.26f*1000.0f, -0.5f*1000.0f));
    shape->SetCylinder(1000.0f*0.1f, 1000.0f*1.81f, Vector3(0.0f*1000.0f, 1.26f*1000.0f, -0.5f*1000.0f));

    // Create the character logic component, which takes care of steering the rigidbody
    // Remember it so that we can set the controls. Use a WeakPtr because the scene hierarchy already owns it
    // and keeps it alive as long as it's not removed from the hierarchy
    character_ = node_->CreateComponent<Character>();

    Bone* headBone = animModel_->GetSkeleton().GetBone("head");

    // Create RigidBody & CollisionShape components to bones
    //CreateRagdollBone("hips", SHAPE_BOX, Vector3(0.3f, 0.2f, 0.25f), Vector3(0.0f, 0.0f, 0.0f),
        //Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("shin.R", SHAPE_CAPSULE, Vector3(0.12f, 0.6f, 0.12f), Vector3(0.0f, 0.22f, -0.02f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("shin.L", SHAPE_CAPSULE, Vector3(0.12f, 0.6f, 0.12f), Vector3(0.0f, 0.22f, -0.02f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("foot.R", SHAPE_CAPSULE, Vector3(0.1f, 0.28f, 0.1f), Vector3(0.0f, 0.09f, 0.0f),
        Quaternion(20.0f, 0.0f, 0.0f));
    CreateRagdollBone("foot.L", SHAPE_CAPSULE, Vector3(0.1f, 0.28f, 0.1f), Vector3(0.0f, 0.09f, 0.0f),
        Quaternion(20.0f, 0.0f, 0.0f));
    /*CreateRagdollBone("toe.R", SHAPE_CAPSULE, Vector3(0.1f, 0.12f, 0.1f), Vector3(0.0f, 0.01f, 0.0f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("toe.L", SHAPE_CAPSULE, Vector3(0.1f, 0.12f, 0.1f), Vector3(0.0f, 0.01f, 0.0f),
        Quaternion(0.0f, 0.0f, 0.0f));*/
    CreateRagdollBone("head", SHAPE_CAPSULE, Vector3(0.22f, 0.3f, 0.22f), Vector3(0.0f, 0.01f, -0.02f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("spine", SHAPE_CAPSULE, Vector3(0.25f, 0.6f, 0.25f), Vector3(0.0f, 0.1f, -0.02f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("chest", SHAPE_CAPSULE, Vector3(0.25f, 0.6f, 0.25f), Vector3(0.0f, 0.1f, -0.02f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("thigh.R", SHAPE_CAPSULE, Vector3(0.2f, 0.46f, 0.2f), Vector3(0.0f, 0.19f, 0.02f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("thigh.L", SHAPE_CAPSULE, Vector3(0.2f, 0.46f, 0.2f), Vector3(0.0f, 0.19f, 0.02f),
        Quaternion(0.0f, 0.0f, 0.0f));

    CreateRagdollBone("upper_arm.R", SHAPE_CAPSULE, Vector3(0.12f, 0.34f, 0.12f), Vector3( 0.01f, 0.11f, 0.02f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("upper_arm.L", SHAPE_CAPSULE, Vector3(0.12f, 0.34f, 0.12f), Vector3(-0.01f, 0.11f, 0.02f),
        Quaternion(0.0f, 0.0f, 0.0f));

    CreateRagdollBone("forearm.R", SHAPE_CAPSULE, Vector3(0.08f, 0.36f, 0.08f), Vector3( 0.0f, 0.15f, 0.0f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("forearm.L", SHAPE_CAPSULE, Vector3(0.08f, 0.36f, 0.08f), Vector3(-0.0f, 0.15f, 0.0f),
        Quaternion(0.0f, 0.0f, 0.0f));

    CreateRagdollBone("hand.R", SHAPE_CAPSULE, Vector3(0.09f, 0.24f, 0.09f), Vector3( 0.0f, 0.12f, 0.0f),
        Quaternion(0.0f, 0.0f, 0.0f));
    CreateRagdollBone("hand.L", SHAPE_CAPSULE, Vector3(0.09f, 0.24f, 0.09f), Vector3(-0.0f, 0.12f, 0.0f),
        Quaternion(0.0f, 0.0f, 0.0f));

    if (headBone)
    {
        /*
        Graphics* graphics = context_->GetSubsystem<Graphics>();
        //float width = (float)(graphics->GetWidth());
        //float height = (float)(graphics->GetHeight());
        Vector2 screenSize = (Vector2)graphics->GetSize();

        auto* textureArrow = cache->GetResource<Texture2D>("Textures/Arrow02.png");

        spriteArrow_ = new Sprite(context_);
        spriteArrow_->SetTexture(textureArrow);
        //headBone->node_->AddChild(spriteArrow_);
        ui_->GetRoot()->AddChild(spriteArrow_);
        //spriteArrow_->SetPosition(camera_->WorldToScreenPoint(headBone->node_->GetWorldPosition()));
        //spriteArrow_->SetPosition( screenSize * camera_->WorldToScreenPoint(headBone->node_->GetPosition()+Vector3(0,5,0)));
        spriteArrow_->SetPosition( screenSize * camera_->WorldToScreenPoint(headBone->node_->GetPosition()));
        //spriteArrow_->SetPosition( screenSize * Vector2(0.5f,0.5f));
        //spriteArrow_->SetPosition(500,500);
        spriteArrow_->SetSize(IntVector2(16, 16));
        spriteArrow_->SetColor(user_->color_);
        */
        auto* billboardObject = headBone->node_->CreateComponent<BillboardSet>();
        billboardObject->SetNumBillboards(1);

        Material* arrowMaterial = cache->GetResource<Material>(user_->arrowMaterialUrl_);

        if(arrowMaterial)
        {
            billboardObject->SetMaterial(arrowMaterial);
            billboardObject->SetSorted(true);
            //billboardObject->SetFixedScreenSize(true);
            Billboard* billboardArrow = billboardObject->GetBillboard(0);
            //billboardArrow->size_ = 50*Vector2(2.0f, 2.0f);
            billboardArrow->enabled_ = true;
            billboardArrow->position_ = Vector3(-0.0f, 8.0f, 0.0f);
            // After modifying the billboards, they need to be "committed" so that the BillboardSet updates its internals
            billboardObject->Commit();
        }
    }

    // Set something to kickTarget that it is not null
    kickTarget_ = node_;
    moveTarget_ = node_;

    areaRadius_ = areaRadius;

    // Radar spot
    auto* radarSpotTexture = cache->GetResource<Texture2D>("Textures/Soccer/SoccerRadarSpot01.png");
    radarSpot_ = new Sprite(context_);
    radarSpot_->SetTexture(radarSpotTexture);
    radarSpot_->SetFullImageRect();
    radarSpot_->SetSize(IntVector2(12, 12));
    radarSpot_->SetHotSpot(IntVector2(6, 6));
    radarSpot_->SetBlendMode(BLEND_REPLACE);
    radarSpot_->SetColor(team_.color);
    ui_->GetRoot()->AddChild(radarSpot_);

    rightHand_  = node_->GetChild("hand.R", true);
    rightHandEffector_ = rightHand_->CreateComponent<IKEffector>();
    rightHandEffector_->SetChainLength(2);

    // For the effectors to work, an IKSolver needs to be attached to one of
    // the parent nodes. Typically, you want to place the solver as close as
    // possible to the effectors for optimal performance. Since in this case
    // we're solving the legs only, we can place the solver at the spine.
    Node* rightClavicle = node_->GetChild("clavicle.R", true);
    solverRightHand_ = rightClavicle->CreateComponent<IKSolver>();

    // Two-bone solver is more efficient and more stable than FABRIK (but only
    // works for two bones, obviously).
    solverRightHand_->SetAlgorithm(IKSolver::TWO_BONE);

    // Disable auto-solving, which means we need to call Solve() manually
    solverRightHand_->SetFeature(IKSolver::AUTO_SOLVE, false);
    //solver_->SetFeature(IKSolver::AUTO_SOLVE, true);


    // We need to attach two inverse kinematic effectors to Jack's feet to
    // control the grounding.
    leftFoot_  = node_->GetChild("foot.L", true);
    rightFoot_ = node_->GetChild("foot.R", true);
    leftFootEffector_  = leftFoot_->CreateComponent<IKEffector>();
    rightFootEffector_ = rightFoot_->CreateComponent<IKEffector>();
    // Control 2 segments up to the hips
    leftFootEffector_->SetChainLength(2);
    rightFootEffector_->SetChainLength(2);

    // For the effectors to work, an IKSolver needs to be attached to one of
    // the parent nodes. Typically, you want to place the solver as close as
    // possible to the effectors for optimal performance. Since in this case
    // we're solving the legs only, we can place the solver at the spine.
    Node* spine = node_->GetChild("spine", true);
    solverFoot_ = spine->CreateComponent<IKSolver>();

    // Two-bone solver is more efficient and more stable than FABRIK (but only
    // works for two bones, obviously).
    solverFoot_->SetAlgorithm(IKSolver::TWO_BONE);

    // Disable auto-solving, which means we need to call Solve() manually
    solverFoot_->SetFeature(IKSolver::AUTO_SOLVE, false);
}

void Player::CreateRagdollBone(const String& boneName, ShapeType type, const Vector3& size, const Vector3& position, const Quaternion& rotation)
{
    // Find the correct child scene node recursively
    Node* boneNode = node_->GetChild(boneName, true);
    if (!boneNode)
    {
        URHO3D_LOGWARNING("Could not find bone " + boneName + " for creating ragdoll physics components");
        return;
    }

    auto* body = boneNode->CreateComponent<RigidBody>();
    // Set mass to make movable
    body->SetMass(0.1f);
    // Set damping parameters to smooth out the motion
    body->SetLinearDamping(0.05f);
    body->SetAngularDamping(0.85f);
    // Set rest thresholds to ensure the ragdoll rigid bodies come to rest to not consume CPU endlessly
    //body->SetLinearRestThreshold(1.5f);
    //body->SetAngularRestThreshold(2.5f);

    body->SetCollisionEventMode(COLLISION_ALWAYS);

    body->SetKinematic(true);

    auto* shape = boneNode->CreateComponent<CollisionShape>();

    float s = 10.0f;//scale factor

    // We use either a box or a capsule shape for all of the bones
    if (type == SHAPE_BOX)
        shape->SetBox(s*size, s*position, rotation);
    else
        shape->SetCapsule(s*size.x_, s*size.y_, s*position, rotation);
}

Player* Player::NearestPlayer(bool needsToBeSameTeam, bool preferUpperLevel)
{
    Player* nearestPlayer = this;
    float shortestDistance = 200.0f;

    for(int i = 0;i<NUM_PLAYERS;i++)
    {
        float distance = (GetPosition() - players_[i]->node_->GetPosition()).Length();

        if (distance<shortestDistance and players_[i]!=this)
        {
            if(needsToBeSameTeam and players_[i]->team_.id == team_.id)
            {
                if(!preferUpperLevel)
                {
                    nearestPlayer = players_[i];
                    shortestDistance = distance;
                }
                else if(team_.attackRight and players_[i]->node_->GetPosition().x_>=GetPosition().x_)
                {
                    nearestPlayer = players_[i];
                    shortestDistance = distance;
                }
                else if(!team_.attackRight and players_[i]->node_->GetPosition().x_<=GetPosition().x_)
                {
                    nearestPlayer = players_[i];
                    shortestDistance = distance;
                }
            }
        }
    }
    return nearestPlayer;
}

 Player* Player::NearestPlayerToBall(int teamID, bool noGoalKeeper)
{
    Player* nearestPlayer;
    float shortestDistance = 200.0f;

    for(int i = 0;i<NUM_PLAYERS;i++)
    {
        float distance = (field_.ball_->node_->GetPosition() - players_[i]->body_->GetPosition()).Length();

        if (distance<shortestDistance)
        {
            if(players_[i]->team_.id == teamID and (!noGoalKeeper or players_[i]->fieldPosition_!=FieldPosition::FP_GK))
            {
                nearestPlayer = players_[i];
                shortestDistance = distance;
            }
        }
    }
    return nearestPlayer;
}

Player* Player::PlayerInDirection(bool needsToBeSameTeam, bool noGoalKeeper)
{
    Player* playerInDirection = NearestPlayerToBall(this->team_.id, false);
    float smallestAngle = 360.0f;

    for(int i = 0;i<NUM_PLAYERS;i++)
    {
        float angle = (players_[i]->node_->GetPosition() - GetPosition()).Angle(this->node_->GetDirection());
        if (angle<smallestAngle and players_[i]!=this)
        {
            if(needsToBeSameTeam and players_[i]->team_.id == this->team_.id and (!noGoalKeeper or players_[i]->fieldPosition_!=FieldPosition::FP_GK))
            {
                playerInDirection = players_[i];
                smallestAngle = angle;
            }
            else if(!needsToBeSameTeam)
            {
                playerInDirection = players_[i];
                smallestAngle = angle;
            }
        }
    }
    return playerInDirection;
}

Vector3 Player::GetPosition()
{
    return body_->GetPosition();
}

void Player::ChangePlayer(int oldSelectedPlayerID, int newSelectedPlayerID)
{
    User*   tempOldUser     = players_[oldSelectedPlayerID]->user_;
    Player* tempOldPlayer   = players_[oldSelectedPlayerID];

    User*   tempNewUser     = players_[newSelectedPlayerID]->user_;
    Player* tempNewPlayer   = players_[newSelectedPlayerID];

    players_[oldSelectedPlayerID]->user_             = tempNewUser;
    players_[oldSelectedPlayerID]->user_->player_    = tempOldPlayer;

    players_[newSelectedPlayerID]->user_             = tempOldUser;
    players_[newSelectedPlayerID]->user_->player_    = tempNewPlayer;

    auto* cache = scene_->GetSubsystem<ResourceCache>();

    Bone* headBone = players_[newSelectedPlayerID]->animModel_->GetSkeleton().GetBone("head");
    if (headBone)
    {
        BillboardSet* billboardArrow = headBone->node_->GetComponent<BillboardSet>();
        Material* arrowMaterial = cache->GetResource<Material>(players_[newSelectedPlayerID]->user_->arrowMaterialUrl_);
        billboardArrow->SetMaterial(arrowMaterial);
    }

    Bone* headBone2 = players_[oldSelectedPlayerID]->animModel_->GetSkeleton().GetBone("head");
    if (headBone2)
    {
        BillboardSet* billboardArrow = headBone2->node_->GetComponent<BillboardSet>();
        Material* arrowMaterial = cache->GetResource<Material>(players_[oldSelectedPlayerID]->user_->arrowMaterialUrl_);
        billboardArrow->SetMaterial(arrowMaterial);
    }

    playerChanged_ = true;
}

float Player::BallsDistanceToAreaCenter()
{
    return (areaCenter_->GetPosition() - ball_->GetPosition()).Length();
}

Vector3 Player::MoveTargetVector()
{
    Vector3 moveTargetVector = Vector3::ZERO;

    if(moveTarget_ != nullptr)
        moveTargetVector = moveTarget_->GetPosition() - GetPosition();
    else
        moveTargetVector = moveTargetPosition_ - GetPosition();

    return moveTargetVector;
}

Vector3 Player::KickTargetPosition()
{
    Vector3 kickTargetPosition = Vector3::ZERO;

    if(kickTarget_ != nullptr)
        kickTargetPosition = kickTarget_->GetPosition();
    else
        kickTargetPosition = kickTargetPosition_;

    return kickTargetPosition;
}

Vector3 Player::KickTargetVector()
{
    return KickTargetPosition() - GetPosition();
}

float Player::DistanceToMoveTarget()
{
    return MoveTargetVector().Length();
}

float Player::AngleToMoveTarget()
{
    Vector3 moveTargetVector = MoveTargetVector();
    return atan2(moveTargetVector.x_, moveTargetVector.z_);
}

float Player::AngleToKickTarget()
{
    Vector3 kickTargetVector = KickTargetVector();
    return atan2(kickTargetVector.x_, kickTargetVector.z_);
}

Vector3 Player::BallVector()
{
    return ball_->GetPosition() - GetPosition();
}

float Player::KickAnimationTime()
{
    return animController_->GetTime(kickAnimation01Url_);
}

Vector3 Player::DirectionVector()
{
    return node_->GetDirection();
}

float Player::Angle()
{
    Vector3 direction = DirectionVector();
    return atan2(direction.x_,direction.z_);
}

void Player::HandleCPUPlayer(UI* ui)
{
    //return;

    if(!ui) return;

    Text* instructionText = (Text*)ui->GetRoot()->GetChild("InstructionText",true);

    //if(user_->userType_ == UserType::UT_CPU)
    if(this->user_->userType_ == UserType::UT_CPU)
    {
        //if(instructionText)
        //instructionText->SetText("CPU!!");

        //auto powerBar = (ProgressBar*)ui->GetRoot()->GetChild("PowerBar",true);

        //if(ballsDistanceToAreaCenter<areaRadius and ballPosition.x_<50.0f and distanceToMoveTarget>0.5f)
        //if(BallsDistanceToAreaCenter() < areaRadius_ and DistanceToMoveTarget() > 0.5f and this==NearestPlayerToBall(team_.id,true))
        //if(BallsDistanceToAreaCenter() < areaRadius_ and DistanceToMoveTarget() > 0.5f)
        if  (
                (DistanceToMoveTarget() > 0.5f)
                and
                (
                    this==NearestPlayerToBall(team_.id,true)
                    or
                    (
                        this->BallVector().Length()<10.0f
                            and
                        ((NearestPlayerToBall(team_.id,true)->GetPosition()-GetPosition()).Length()>4.0f)
                    )
                )
            )
            {
                character_->GetNode()->SetRotation(Quaternion(AngleToMoveTarget()*360/(2*M_PI), Vector3::UP));
                Vector3 AI_MoveDir = MoveTargetVector();
                AI_MoveDir.y_ = 0.0f;
                AI_MoveDir.Normalize();
                //body_->ApplyImpulse(AI_MoveDir*2.9f);
                //body_->ApplyImpulse(AI_MoveDir*2.5f);
                body_->ApplyImpulse(AI_MoveDir*0.9f);
                character_->controls_.Set(CTRL_FORWARD,    true);
            }

        if(gameEvent== GameEvent::GE_GAME_ON)
        {
            //if(distanceToBall<1.4f and !animController_->IsPlaying(kickAnimation01Url))
            if(BallVector().Length() < CPUReachBallRadius_ and animController_->GetTime(kickAnimation01Url_)<0.1f)
            {
                // Stop ball
                Vector3 ballVelocity = field_.ball_->body_->GetLinearVelocity();
                ballVelocity.x_ = 0.0f;
                ballVelocity.z_ = 0.0f;
                field_.ball_->body_->SetLinearVelocity(ballVelocity);
                animController_->PlayExclusive(kickAnimation01Url_, 0, false, 1.2f);
                animController_->SetTime (kickAnimation01Url_, 3.0f);
                //kickTarget_ = scene_->GetChild("GoalB", true);
                kickTarget_ = targetGoal_;
                character_->GetNode()->SetRotation(Quaternion(AngleToKickTarget() * 360 / (2 * M_PI), Vector3::UP));
            }

            // Kick ball
            if(KickAnimationTime() > 3.5f and KickAnimationTime() < 4.0f and BallVector().Length() < CPUKicksBallRadius_)
            {
                //if(!soundSource->IsPlaying())
                {
                    //soundSource->SetGain((10.0f+0.3f*distanceToKickTarget)/maxKickPower);
                    //soundSource->Play(soundKick01);
                }
                //KickBall(KickTargetVector(), 10.0f + 0.3f * KickTargetVector().Length(), 0.0f);
                //lastPlayerWhoTouchedBall_ = this;
                //KickBall(KickTargetVector()*(10.0f + 0.3f * KickTargetVector().Length()));
                KickBall(KickTargetVector()*(0.001f + 0.001f * KickTargetVector().Length()));

                //moveTarget_ = targetGoal;
            }

            if(fieldPosition_ == FieldPosition::FP_GK)
            {
                if(BallVector().Length() < 15.0f and (BallsDistanceToAreaCenter() < areaRadius_))
                {
                    //kickTarget_ = NearestPlayer(true,false)->node_;
                    kickTarget_ = targetGoal_;
                    moveTarget_ = ball_->node_;
                }
                else
                {
                    moveTarget_ = nullptr;
                    moveTargetPosition_ = 0.2f * ball_->body_->GetPosition() + 0.8f * areaCenter_->GetPosition();
                }

                Vector3 oldPos = node_->GetPosition();
                node_->SetPosition(Vector3(48.0f,oldPos.y_,oldPos.z_));
                node_->SetRotation(Quaternion(0,-90,0));
            }
            else if(fieldPosition_ == FieldPosition::FP_STRIKER)
            {
                moveTarget_ = ball_->node_;
                kickTarget_ = targetGoal_;
            }
            else
            {
            }
            //powerBar->SetValue(20);
        }
    }

    if(fieldPosition_ == FieldPosition::FP_GK)
        node_->SetRotation(Quaternion(0,-90,0));
}

void Player::ChargeKick(UI* ui)
{
    //if(!ui) return;
    //auto powerBar = (ProgressBar*)ui->GetRoot()->GetChild("PowerBar",true);
    auto powerBar = (ProgressBar*)ui_->GetRoot()->GetChild("PowerBar",true);

    powerBar->ChangeValue(4);
    animController_->PlayExclusive(kickAnimation01Url_, 0, false, 0.0f);
    animController_->SetTime (kickAnimation01Url_, 3.0f);
    //character_->GetNode()->SetRotation(Quaternion(AngleToKickTarget() * 360 / (2 * M_PI), Vector3::UP));
    Node* kickNode = node_->GetChild("KickNode");
    Vector3 kickPositionBallVector = Vector3::ZERO;
    if(kickNode)
    {
        kickPositionBallVector = kickNode->GetWorldPosition() - field_.ball_->GetPosition();
        kickPositionBallVector.y_ = 0.0f;
    }

    ball_->body_->SetLinearVelocity(2.0f * kickPositionBallVector);
    body_->SetLinearVelocity(-20.0f * kickPositionBallVector);

    float angleDifferenceToKickTarget = AngleToKickTarget() - Angle();
    if( angleDifferenceToKickTarget > M_PI ) angleDifferenceToKickTarget = M_PI - angleDifferenceToKickTarget;

    body_->SetAngularVelocity(Vector3(0, angleDifferenceToKickTarget, 0));
}

bool Player::AnimationInRange(String animationName, float rangeMin, float rangeMax)
{
    if(rangeMax<=0.0f) rangeMax = animController_->GetLength(animationName) + rangeMax;
    float animationTime = animController_->GetTime(animationName);
    if(rangeMin < animationTime and animationTime < rangeMax)
        return true;
    else
        return false;
}

void Player::TakeBallInHands()
{

    //rightEffector_->SetTargetPosition(rightHand_->GetWorldPosition()+Vector3(0,1,0));

    rightHandEffector_->SetTargetPosition(ball_->GetPosition());
    solverRightHand_->SetFeature(IKSolver::AUTO_SOLVE, true);

    /*
    animController_->PlayExclusive("Models/pick_115_10_B.ani", 0, true, 1.2f);
    //float animationTime = animController_->GetTime("Models/pick_115_10_B.ani");
    ball_->body_->SetLinearVelocity(Vector3::ZERO);
    ball_->body_->SetAngularVelocity(Vector3::ZERO);

    ball_->body_->SetUseGravity(true);

    if(AnimationInRange("Models/pick_115_10_B.ani", 0.0f, 0.5f))
    {
        //ball_->body_->SetKinematic(true);
        ball_->body_->SetUseGravity(false);
    }
    else if(AnimationInRange("Models/pick_115_10_B.ani", 0.5f, 1.5f))
    {
        ball_->body_->SetUseGravity(false);
        Node* boneNode1 = node_->GetChild("hand.R", true);
        Node* boneNode2 = node_->GetChild("hand.L", true);
        Vector3 average = 0.5f*boneNode1->GetWorldPosition() + 0.5f*boneNode2->GetWorldPosition();
        ball_->body_->SetPosition(average);
    }
    else
    {
        //ball_->body_->SetKinematic(false);
        ball_->body_->SetUseGravity(true);

    }
    */
}

void Player::FootToGround()
{
    /*
    auto* phyWorld = scene_->GetComponent<PhysicsWorld>();
    Vector3 leftFootPosition = leftFoot_->GetWorldPosition();
    Vector3 rightFootPosition = rightFoot_->GetWorldPosition();

    // Cast ray down to get the normal of the underlying surface
    PhysicsRaycastResult result;

    phyWorld->RaycastSingle(result, Ray(leftFootPosition + Vector3(0, 1, 0), Vector3(0, -1, 0)), 2);
    if (result.body_)
    {
        // Cast again, but this time along the normal. Set the target position
        // to the ray intersection
        phyWorld->RaycastSingle(result, Ray(leftFootPosition + result.normal_, -result.normal_), 2);
        // The foot node has an offset relative to the root node
        float footOffset = leftFoot_->GetWorldPosition().y_ - node_->GetWorldPosition().y_;
        leftFootEffector_->SetTargetPosition(result.position_ + result.normal_ * footOffset*2);
        // Rotate foot according to normal
        leftFoot_->Rotate(Quaternion(Vector3(0, 1, 0), result.normal_), TS_WORLD);
    }
    //leftFootEffector_->SetTargetPosition(rightFootPosition);

    // Same deal with the right foot
    phyWorld->RaycastSingle(result, Ray(rightFootPosition + Vector3(0, 1, 0), Vector3(0, -1, 0)), 2);
    if (result.body_)
    {
        phyWorld->RaycastSingle(result, Ray(rightFootPosition + result.normal_, -result.normal_), 2);
        float footOffset = rightFoot_->GetWorldPosition().y_ - node_->GetWorldPosition().y_;
        rightFootEffector_->SetTargetPosition(result.position_ + result.normal_ * footOffset*2);
        rightFoot_->Rotate(Quaternion(Vector3(0, 1, 0), result.normal_), TS_WORLD);
    }

    solverFoot_->Solve();
    */
}

void Player::KickBall(Vector3 kickVector)
{
    ball_->KickSound(kickVector.Length()/maxKickPower_);
    ball_->body_->SetLinearVelocity(kickVector);
    lastPlayerWhoTouchedBall_ = this;
    gameEvent = GameEvent::GE_GAME_ON;
    gameEventTimer = new Timer();
}

void Player::Update(UI* ui, float dt)
{
    //if(user_->userType_ == UserType::UT_CPU)
        //return;

    JoystickState* joystick =  user_->joystick_;

    if(!ui) return;

    // Update radar spot position
    Vector3 position = GetPosition();
    auto radar = (Sprite*)ui->GetRoot()->GetChild("Radar",true);
    IntVector2 radarPosition = radar->GetScreenPosition() +radar->GetHotSpot();
    radarSpot_->SetPosition(Vector2(radarPosition.x_+2*position.x_,radarPosition.y_-2*position.z_));

    auto powerBar = (ProgressBar*)ui->GetRoot()->GetChild("PowerBar",true);

    float angleDifferenceToKickTarget = AngleToKickTarget() - Angle();
    if( angleDifferenceToKickTarget > M_PI ) angleDifferenceToKickTarget = M_PI - angleDifferenceToKickTarget;

    // To nearestPlayer
    Player* nearest = NearestPlayer(true,true);
    float distanceToNearestPlayer = (nearest->node_->GetPosition() - node_->GetPosition()).Length();

    // To goal
    Vector3 vectorToGoal = Vector3(50.0f,10.0f,0.0f) - node_->GetPosition();
    float distanceToGoal = vectorToGoal.Length();
    float angleToGoal = atan2(vectorToGoal.x_,vectorToGoal.z_);
    float angleDifferenceToGoal = angleToGoal - Angle();
    if( angleDifferenceToGoal > M_PI ) angleDifferenceToGoal = M_PI - angleDifferenceToGoal;

    //float curlCompensation = M_PI/180*1;
    //angleDifference += curlCompensation;

    Node* rightFootNode = node_->GetChild("toe.R", true);
    Node*  leftFootNode = node_->GetChild("toe.L", true);

    Node* kickNode = node_->GetChild("KickNode");
    Vector3 kickPositionBallVector = Vector3::ZERO;
    if(kickNode)
    {
        kickPositionBallVector = kickNode->GetWorldPosition() - field_.ball_->GetPosition();
        kickPositionBallVector.y_ = 0.0f;
    }

    // To dribbleTarget
    Vector3 averageFootPosition = 0.5f*(rightFootNode->GetWorldPosition() + leftFootNode->GetWorldPosition());
    Vector3 targetDribblePosition = averageFootPosition + 2.0f * DirectionVector();
    targetDribblePosition.y_ = 0.0f;
    Vector3 targetDribbleVector = targetDribblePosition - field_.ball_->GetPosition();
    //difference.y_ = 0;
    float ballsDistanceToDribbleTarget = targetDribbleVector.Length();
    vectorToGoal.Normalize();
    //playerDirection.Normalize();

    // Kick
    float kickForce = 0.01f * powerBar->GetValue();
    Vector3 kickDirection = KickTargetVector();
    //kickDirection.Normalize();

    // Limit players in area
    Vector3 playerPosition = body_->GetPosition();
    float limitsFactor = 0.546;
    if(playerPosition.x_> limitsFactor*field_.size_.x_) playerPosition.x_ = limitsFactor*field_.size_.x_;
    if(playerPosition.x_<-limitsFactor*field_.size_.x_) playerPosition.x_ =-limitsFactor*field_.size_.x_;
    if(playerPosition.z_> limitsFactor*field_.size_.y_) playerPosition.z_ = limitsFactor*field_.size_.y_;
    if(playerPosition.z_<-limitsFactor*field_.size_.y_) playerPosition.z_ =-limitsFactor*field_.size_.y_;

    //playerPosition.y_ = 0.0f;
    body_->SetPosition(playerPosition);

    DebugRenderer* dbgRenderer = scene_->GetComponent<DebugRenderer>();
    if (showDebugGeometry_)
    {
        if(!dbgRenderer)
            return;
        //DrawVector( ballPosition, playerDirection,  Color(1,1,1,1) );
        //DrawVector( ballPosition, dirAverage,       Color(1,0,0,1) );
        //DrawVector( ballPosition, kickDirection,    Color(0,1,0,1) );
        //DrawVector( ballPosition, directionToGoal,  Color(0,0,1,1) );
        //DrawVector( node_->GetPosition(), testVector,  Color(1,0,0,1) );
        //dbgRenderer->AddSkeleton( animModel_->GetSkeleton(), Color(1,1,1,1), false );
        //dbgRenderer->AddNode( node_->GetChild("KickNode"), 1.0f, false );
        //dbgRenderer->AddSphere( Sphere(rightFootNode->GetWorldPosition(),0.1f), Color(1,1,1,1), false );
        //dbgRenderer->AddSphere( Sphere(kickTarget_->GetWorldPosition(),0.1f), Color(0,0,1,1), false );
        //dbgRenderer->AddCylinder (areaCenter->GetPosition(), areaRadius, 0.1f, Color(0,0,0,0.1f), false);
        //DrawVector( rightFootNode->GetWorldPosition(), rightFootNode->GetRotation()*Vector3::FORWARD,  Color(1,1,1,1) );
        //dbgRenderer->AddNode( moveTarget_, 1.0f, false );
        dbgRenderer->AddSphere( Sphere(KickTargetPosition(),0.1f), Color(0,0,1,1), true );
        //DrawVector( ball_->GetPosition(), KickTargetVector(),  Color(1,1,1,1) );
        dbgRenderer->AddLine( ball_->GetPosition(), KickTargetPosition(), Color(1,1,1,1), true );

        Vector3 kickTargetGroundPosition = KickTargetPosition();
        kickTargetGroundPosition.y_ = 0.0f;
        dbgRenderer->AddLine( ball_->GetPosition(), kickTargetGroundPosition, Color(0,0,0,1), true );
    }
    dbgRenderer->AddSphere( Sphere(KickTargetPosition(),0.1f), Color(0,0,1,1), true );

    //DebugRenderer* dbgRenderer = scene_->GetComponent<DebugRenderer>();

    HandleCPUPlayer(ui);

    if(user_->userType_ == UserType::UT_CPU)
        return;

    if(user_->joystick_ == nullptr)
        return;

    //character_->controls_.yaw_     += 8.0f*(float)user_->joystick_->GetAxisPosition(2) * YAW_SENSITIVITY;
    //character_->controls_.pitch_   += 8.0f*(float)user_->joystick_->GetAxisPosition(3) * YAW_SENSITIVITY;

    //Vector2 leftStick = Vector2(user_->joystick_->GetAxisPosition(0),user_->joystick_->GetAxisPosition(1));
    Vector3 leftStick2 = Vector3(user_->joystick_->GetAxisPosition(0),0.0f,-user_->joystick_->GetAxisPosition(1));

    //Quaternion worldRotation = Quaternion(0,180,0) * cameraNode_->GetWorldRotation();
    Quaternion worldRotation = cameraNode_->GetWorldRotation();

    leftStick2 = worldRotation * leftStick2;
    //leftStick2.y_ = 0.0f;
    leftStick2.z_ -= leftStick2.y_;

    //if (leftStick2.LengthSquared() > 1.0f) leftStick2.Normalize();

    // Player moving
    //float a = atan2(leftStick.x_,-leftStick.y_);
    float a = atan2(leftStick2.x_,leftStick2.z_);

    float angleDifference2 = a - Angle();

    if( angleDifference2 > M_PI ) angleDifference2 = M_PI - angleDifference2;

    //Vector3 moveDir = Vector3(dx,0.0f,-dy);
    //Vector3 moveDir = Vector3(leftStick.x_,0.0f,-leftStick.y_);
    Vector3 moveDir = leftStick2;

    /// Moving -------
    if(leftStick2.Length()>0.3f
        and !user_->joystick_->GetButtonDown(SButton::CIRCLE)
        and !user_->joystick_->GetButtonDown(SButton::CROSS)
        and !user_->joystick_->GetButtonDown(SButton::SQUARE)
        and (KickAnimationTime() > 4.0f or KickAnimationTime() < 0.1f)
        )
    {
        // Normalize move vector so that diagonal strafing is not faster
        if (moveDir.LengthSquared() > 1.0f) moveDir.Normalize();

        character_->GetNode()->SetRotation(Quaternion(a*360/(2*M_PI), Vector3::UP));

        moveDir.y_ = 0.0f;
        moveDir *= dt * 35.0f;
        // Fast dribble with R1 button
        if (user_->joystick_->GetButtonDown(SButton::R1))
            //body_->ApplyImpulse(moveDir * 3.1f);
            body_->ApplyForce(50.0f*moveDir * 3.1f);
        else
            //body_->ApplyImpulse(moveDir * 2.4f);
            body_->ApplyForce(50.0f*moveDir * 2.4f);

        character_->controls_.Set(CTRL_FORWARD,    true);
    }
    else
    {
        character_->controls_.Set(CTRL_FORWARD,    false);
    }

    /// Handle kick buttons -----------
    if((GetPosition() - ball_->GetPosition()).Length() < playerReachBallRadius_)
    {
        // Charging pass
        if (joystick->GetButtonDown(SButton::CROSS) or joystick->GetButtonPress(SButton::CROSS))
        {
            kickTarget_ = PlayerInDirection(true,false)->node_;
            kickType_ = KickType::KT_SHORT_PASS;
            ChargeKick(ui);
        }
        // Charging cross(high pass)
        else if (joystick->GetButtonDown(SButton::SQUARE) or joystick->GetButtonPress(SButton::SQUARE))
        {
            //kickTarget_ = PlayerInDirection(true,true)->node_;
            kickTarget_ = nullptr;
            kickTargetPosition_ = PlayerInDirection(true,true)->GetPosition() + Vector3(0.0f, 3.0f*(0.5f+0.5f*leftStick2.x_),0.0f);
            kickType_ = KickType::KT_LONG_CROSS;
            ChargeKick(ui);
        }
        // Charging shoot
        else if (joystick->GetButtonDown(SButton::CIRCLE) or joystick->GetButtonPress(SButton::CIRCLE))
        {
            kickTarget_ = nullptr;
            kickTargetPosition_ = targetGoal_->GetPosition() + Vector3(0, 4.0f*(0.5f+0.5f*leftStick2.x_), leftStick2.z_ * 5.0f);
            kickType_ = KickType::KT_LONG_SHOT;
            ChargeKick(ui);
        }
         // Charging through pass
        else if (joystick->GetButtonDown(SButton::TRIANGLE) or joystick->GetButtonPress(SButton::TRIANGLE))
        {
            kickTarget_ = PlayerInDirection(true,true)->node_;
            kickType_ = KickType::KT_HIGH_THROUGH_PASS;
            ChargeKick(ui);
        }
        // Taking ball in hands
        //else if (joystick->GetButtonDown(SButton::R2) or joystick->GetButtonPress(SButton::R2) or AnimationInRange("Models/pick_115_10_B.ani", 0.1f, 3.0f))
        else if (joystick->GetButtonDown(SButton::R2) or joystick->GetButtonPress(SButton::R2))
        {
            TakeBallInHands();
        }
        else if(AnimationInRange("Models/16_35_run_shorted.ani", 0.1f, 3.0f))
        {
            // Dribble
            if(BallVector().Length() < 1.4f)
            {
                if(KickAnimationTime() > 3.5f and KickAnimationTime() < 4.0f)
                {}
                else if(KickAnimationTime() < 3.0f and (gameEvent == GameEvent::GE_GAME_ON ) and user_->joystick_)
                {
                    Vector3 oldLinearVelocity = ball_->body_->GetLinearVelocity();
                    Vector3 newLinearVelocity = 9.0f * kickPositionBallVector;
                    kickPositionBallVector.y_ = oldLinearVelocity.y_;
                    //ball_->body_->SetLinearVelocity(newLinearVelocity);
                    ball_->body_->SetLinearVelocity(0.1f*oldLinearVelocity+0.9f*newLinearVelocity);
                }
            }
        }
    }

    if(KickAnimationTime() > 3.4f and KickAnimationTime() < 4.1f)
    {
        /// Kick
        if((rightFootNode->GetWorldPosition() - ball_->GetPosition()).Length() < playerKicksBallRadius_)
        {
            kickForce = powerBar->GetValue();

            switch(kickType_)
            {
                case KickType::KT_SHORT_PASS :
                    kickDirection.y_ = 0.05f;
                    //ChangePlayer(this->ID_, PlayerInDirection(this->ID_,true)->ID_);
                    break;
                case KickType::KT_LONG_SHOT :
                    kickDirection.y_ *= 0.06f*kickDirection.Length() + 0.0015f * kickDirection.Length()*kickDirection.Length();
                    break;
                case KickType::KT_LONG_CROSS :
                    kickDirection.y_ *= 0.06f*kickDirection.Length() + 0.0015f * kickDirection.Length()*kickDirection.Length();
                    kickDirection.y_ *= 0.5f;
                    break;
            }
            Vector3 kickVector =  kickDirection * (0.8f+kickForce);

            // Limit kickpower
            if(kickVector.Length()>maxKickPower_)
                kickVector = maxKickPower_*kickVector.Normalized();
            if(kickVector.Length()<minKickPower_)
                kickVector = minKickPower_*kickVector.Normalized();

            KickBall(kickVector);
            powerBar->SetValue(30);
            //gameEvent = GameEvent::GE_GAME_ON;
        }
    }
    else if(KickAnimationTime() < 0.1f or KickAnimationTime() > 4.2f)
    {
        /// Dribble
        if((rightFootNode->GetWorldPosition()-ball_->GetPosition()).Length()<0.3f or (leftFootNode->GetWorldPosition()-ball_->GetPosition()).Length()<0.3f)
            lastPlayerWhoTouchedBall_ = this;
    }

    if (takeBall_ and user_->joystick_->GetButtonDown(SButton::TRIANGLE))
    {
        ball_->body_->SetPosition(node_->GetPosition()+node_->GetRotation() * Vector3(0.0f, 0.5f, 1.0f));
        ball_->body_->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));

        scene_->SetTimeScale(1.0f);
    }

    FootToGround();

    position = GetPosition();
    position.y_ = -0.15f;
    //body_->SetPosition(position);
    //node_->SetPosition(position);

    /// Change player
    //int newSelectedPlayerID = playerInDirectionID(i,true,true);
    int newSelectedPlayerID = PlayerInDirection(true,true)->ID_;
    if(
        (
            (
                (players_[newSelectedPlayerID]->user_->userType_ == UserType::UT_CPU
                    and (players_[newSelectedPlayerID]->GetPosition() - ball_->GetPosition()).Length() < 4.0f )
                    and (newSelectedPlayerID == players_[newSelectedPlayerID]->NearestPlayerToBall(players_[newSelectedPlayerID]->team_.id,false)->ID_)
            )
            or user_->joystick_->GetButtonPress(SButton::L1)) and !playerChanged_)// (dont change player many times)
        ChangePlayer(this->ID_,newSelectedPlayerID);
    else
        playerChanged_ = false;

    return;
}

bool Player::playerChanged_ = false;
bool Player::showDebugGeometry_ = false;
bool Player::takeBall_ = false;
Player* Player::players_[NUM_PLAYERS];
int Player::nextID_ = 0;
Context* Player::context_;
Field Player::field_;
Scene* Player::scene_;
UI* Player::ui_;
Ball* Player::ball_;
Player* Player::lastPlayerWhoTouchedBall_;
String Player::kickAnimation01Url_ = "Models/11_01.ani";
Camera* Player::camera_;
Node* Player::cameraNode_;
GameEvent Player::gameEvent_ = GameEvent::GE_GAME_ON;
Timer* Player::gameEventTimer_;
