#include "Referee.h"

Referee::Referee(Team team, String name, Vector3 position, User* user, FieldPosition fieldPosition, float areaRadius,
    Color arrowColor) : Player(team, name, position, user, fieldPosition, 20.0f)
{
    auto* cache_ = context_->GetSubsystem<ResourceCache>();

    soundSource_ = node_->CreateComponent<SoundSource3D>();
    soundWhistle_ = cache_->GetResource<Sound>("Sounds/Soccer/whistle01.ogg");

    //node_ = scene_->GetChild("SoccerBall2");

    if(node_)
        soundSource_ = node_->CreateComponent<SoundSource3D>();

    if(soundSource_)
    {
        //soundSource_->SetSoundType(SOUND_EFFECT);  // optional
        //soundSource_->SetNearDistance(10);
        soundSource_->SetFarDistance(200);
        soundSource_->SetGain(1.0f);
    }

    //soundKick_ = cache->GetResource<Sound>("Sounds/Soccer/whistle01.ogg");
    //soundKick_ = cache_->GetResource<Sound>("Sounds/Soccer/37156__volivieri__soccer-kick-01_edited.wav");

    animModel_->SetMaterial(0, cache_->GetResource<Material>("Materials/RefereeClothes.xml"));
}

void Referee::Whistle()
{
    if(soundSource_ and soundWhistle_)
        soundSource_->Play(soundWhistle_);
    else
    {
        if(node_)
            soundSource_ = node_->CreateComponent<SoundSource3D>();

        if(soundSource_)
        {
            //soundSource_->SetSoundType(SOUND_EFFECT);  // optional
            //soundSource_->SetNearDistance(100);
            //soundSource_->SetFarDistance(300);
        }

        soundWhistle_ = cache_->GetResource<Sound>("Sounds/Soccer/whistle01.ogg");

        if(soundSource_ and soundWhistle_)
            soundSource_->Play(soundWhistle_);
    }
}
void Referee::Update(UI* ui, float dt)
{
    Player::Update(ui, dt);
}
