#include "Ball.h"
#include "Player.h"

Ball::Ball(Context* context, Scene* scene, float curlFactor)
{
    context_ = context;
    UI* ui = context->GetSubsystem<UI>();

    auto* cache = context->GetSubsystem<ResourceCache>();
    cache_ = cache;

    scene_ = scene;
    node_ = scene_->GetChild("SoccerBall2");

    if(node_)
        soundSource_ = node_->CreateComponent<SoundSource3D>();

    if(soundSource_)
        soundSource_->SetSoundType(SOUND_EFFECT);  // optional

    //soundKick_ = cache->GetResource<Sound>("Sounds/Soccer/whistle01.ogg");
    soundKick_ = cache_->GetResource<Sound>("Sounds/Soccer/37156__volivieri__soccer-kick-01_edited.wav");

    // Radar spot
    auto* radarSpotTexture = cache->GetResource<Texture2D>("Textures/Soccer/SoccerRadarSpot01.png");
    radarSpot_ = new Sprite(context);
    radarSpot_->SetTexture(radarSpotTexture);
    radarSpot_->SetFullImageRect();
    radarSpot_->SetSize(IntVector2(8, 8));
    radarSpot_->SetHotSpot(IntVector2(4, 4));
    radarSpot_->SetBlendMode(BLEND_ALPHA);
    radarSpot_->SetColor(Color(0.5f,0.5f,0.5f,0.9f));
    ui->GetRoot()->AddChild(radarSpot_);
}

Vector3 Ball::GetPosition()
{
    return body_->GetPosition();
}

void Ball::SetPosition(Vector3 position)
{
    body_->SetPosition(position);
}

void Ball::ResetPosition(Vector3 position)
{
    SetPosition(position);
    body_->SetLinearVelocity(Vector3::ZERO);
    body_->SetAngularVelocity(Vector3::ZERO);
}

bool Ball::InField()
{
    return true;
}

void Ball::KickSound(float gain)
{
    if(soundSource_ and soundKick_)
    {
    }
    else if(node_)
        soundSource_ = node_->CreateComponent<SoundSource3D>();

    if(!soundSource_->IsPlaying())
    {
        soundSource_->SetGain(gain);
        //soundSource_->SetGain(2.0f);
        soundSource_->Play(soundKick_);
    }
}

void Ball::Update(float dt)
{
    UI* ui = context_->GetSubsystem<UI>();

    float horizontalSpin = body_->GetAngularVelocity().y_;
    Vector3 linearVelocity = body_->GetLinearVelocity();
    Vector3 curlDirection = horizontalSpin * Vector3(linearVelocity.z_, 0.0f, linearVelocity.x_);
    body_->ApplyForce(curlFactor_ * curlDirection * linearVelocity.Length() * dt);

    // Update radar spot position
    Vector3 position = GetPosition();
    auto radar = (Sprite*)ui->GetRoot()->GetChild("Radar",true);
    IntVector2 radarPosition = radar->GetScreenPosition() +radar->GetHotSpot();
    radarSpot_->SetPosition(Vector2(radarPosition.x_+2*position.x_,radarPosition.y_-2*position.z_));
}

Vector3 Ball::resetPosition_ = Vector3(0.0f, 1.0f, 0.0f);
