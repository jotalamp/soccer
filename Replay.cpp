#include "Replay.h"
#include "Player.h"
#include "Ball.h"

void Replay::Save(Ball* ball, Player* player, float time)
{
    time_[nextTimePosition] = time;

    ballOrientations[nextTimePosition].position = ball->GetPosition();
    ballOrientations[nextTimePosition].rotation = ball->body_->GetRotation();

    playerOrientations[nextTimePosition].position = player->GetPosition();
    playerOrientations[nextTimePosition].rotation = player->body_->GetRotation();

    if(player->animController_->IsPlaying(Player::kickAnimation01Url_))
        playerOrientations[nextTimePosition].animationName = Player::kickAnimation01Url_;
    else if(player->animController_->IsPlaying("Models/16_35_run_shorted.ani"))
        playerOrientations[nextTimePosition].animationName = "Models/16_35_run_shorted.ani";

    playerOrientations[nextTimePosition].animationTime = player->animController_->GetTime(playerOrientations[nextTimePosition].animationName);

    nextTimePosition = (nextTimePosition+1) % REPLAY_LENGTH;
}

void Replay::Show(Ball* ball,float time)
{
    int intTime = time;
    intTime = intTime % REPLAY_LENGTH;
    ball->body_->SetPosition(ballOrientations[intTime].position);
    ball->body_->SetRotation(ballOrientations[intTime].rotation);
}

void Replay::Play(Ball* ball,Player* player,float speed)
{
    showTime += speed;
    if(showTime > REPLAY_LENGTH ) showTime -= REPLAY_LENGTH;
    if(showTime < 0             ) showTime += REPLAY_LENGTH;
    int intTime = FloorToInt(showTime);
    int intTimeNext = (intTime+1)%REPLAY_LENGTH;
    float t = showTime-intTime;
    //float t = time_[intTimeNext];
    //intTime = intTime % REPLAY_LENGTH;
    ball->body_->SetLinearVelocity(Vector3::ZERO);
    ball->body_->SetAngularVelocity(Vector3::ZERO);
    ball->body_->SetPosition(ballOrientations[intTime].position.Lerp( ballOrientations[intTimeNext].position,t));
    ball->body_->SetRotation(ballOrientations[intTime].rotation.Nlerp(ballOrientations[intTimeNext].rotation,t,true));

    player->body_->SetLinearVelocity(Vector3::ZERO);
    player->body_->SetAngularVelocity(Vector3::ZERO);
    player->body_->SetPosition(playerOrientations[intTime].position.Lerp( playerOrientations[intTimeNext].position,t));
    player->body_->SetRotation(playerOrientations[intTime].rotation.Nlerp(playerOrientations[intTimeNext].rotation,t,true));

    float animationLength = player->animController_->GetLength(playerOrientations[intTime].animationName);

    player->animController_->SetSpeed(playerOrientations[intTime].animationName, 0.0f);
    player->animController_->PlayExclusive(playerOrientations[intTime].animationName, 0, false, 5.0f);
    float timeA = playerOrientations[intTime].animationTime;
    float timeB = playerOrientations[intTimeNext].animationTime;
    if(timeB<timeA) timeB += animationLength;
    //if(timeA > animationLength) timeA -= animationLength;
    //if(timeB > animationLength) timeB -= animationLength;
    float animationTime = Lerp(timeA,timeB,t);
    if(animationTime>animationLength) animationTime -= animationLength;
    player->animController_->SetTime (playerOrientations[intTime].animationName, animationTime);
    //ball->body_->SetRotation(ballOrientations[intTime].rotation);
    //showTime =
}

bool Replay::on_ = false;
float Replay::showTime = 0.0f;
