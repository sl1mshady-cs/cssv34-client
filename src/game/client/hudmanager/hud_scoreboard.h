#pragma once
#include "uihudmgr.h"
#include <string>
#include <vector>


struct ScoreboardPlayer
{
    std::string name;
    int score = 0;
    int deaths = 0;
    int latency = 0;
    bool isBot = false;
};

struct ScoreboardTeam
{
    std::string name;
    int score = 0;
    std::vector<ScoreboardPlayer> players;
};



class CHudScoreboard
{
public:
    void Initialize();
    void Think();
    void Shutdown();

private:
    void UpdatePlayerList();
};