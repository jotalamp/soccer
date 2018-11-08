#ifndef REFEREE_H
#define REFEREE_H

#include "Player.h"

class Referee : public Player
{
public:
    //Referee(Team team, String name, Vector3 position, User* user, FieldPosition fieldPosition, float areaRadius=20.0f,
        //Color arrowColor = Color::TRANSPARENT) : Player(team, name, position, user, fieldPosition, 20.0f){};
    Referee(Team team, String name, Vector3 position, User* user, FieldPosition fieldPosition, float areaRadius=20.0f,
        Color arrowColor = Color::TRANSPARENT);

    void Whistle();

    void Update(UI* ui, float dt);

    SoundSource3D* soundSource_;
    Sound* soundWhistle_;
    ResourceCache* cache_;
};

#endif // REFEREE_H
