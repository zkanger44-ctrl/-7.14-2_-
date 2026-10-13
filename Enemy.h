#pragma once

#include <vector>
#include "PlayerSettings.h"

struct EnemyData
{
	int ID;
	PlayerSettings Settings;
	double criticalRate; // クリティカル率 (0.0 - 1.0)
};

std::vector<EnemyData> CreateAllEnemies();
void ShowEnemyData(const EnemyData& e);

