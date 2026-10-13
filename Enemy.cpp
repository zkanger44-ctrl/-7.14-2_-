#include "Enemy.h"
#include <iostream>

std::vector<EnemyData> CreateAllEnemies()
{
	return {
		{1, {"スライムA", 200, 0, 30, 5, 8}, 0.05},
		{2, {"ゴブリン", 300, 0, 45, 10, 12}, 0.10},
		{3, {"オーガ", 800, 0, 80, 30, 6}, 0.02},
		{4, {"ドラゴン", 1500, 200, 150, 70, 20}, 0.20}
	};
}

void ShowEnemyData(const EnemyData& e)
{
	std::cout << "ID:" << e.ID
		<< ", Name:" << e.Settings.Name
		<< ", HP:" << e.Settings.HP
		<< ", ATK:" << e.Settings.ATK
		<< ", DEF:" << e.Settings.DEF
		<< ", SPD:" << e.Settings.SPD
		<< ", criticalRate:" << e.criticalRate
		<< std::endl;
}
