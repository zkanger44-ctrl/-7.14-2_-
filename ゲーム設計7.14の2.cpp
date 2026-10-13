#include "GameDesign.h"
#include <iostream>
#include <numeric>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

void Show(const Player& player)
{
    std::cout << "Name:" << player.Name << ", HP:" << player.Settings.HP
        << ", MP:" << player.Settings.MP << ", ATK:" << player.Settings.ATK
        << ", DEF:" << player.Settings.DEF << ", SPD:" << player.Settings.SPD << '\n';
}

void ConsoleRpg::Attack(Player& attacker, Player& defender)
{
    std::bernoulli_distribution critical(attacker.CriticalRate);
    const bool isCritical = critical(random_);
    const int damage = CalculateDamage(attacker.Settings, defender.Settings) * (isCritical ? 2 : 1);
    defender.Settings.HP = std::max(0, defender.Settings.HP - damage);
    std::cout << attacker.Name << "の攻撃！ ダメージ:" << damage
        << (isCritical ? " (CRITICAL)" : "") << " " << defender.Name
        << "の残HP:" << defender.Settings.HP << '\n';
}

void ConsoleRpg::Encounter()
{
    enemy_ = CharacterFactory::CreateRandomEnemy(pool_, random_);
    if (!enemy_) throw std::runtime_error("Enemy pool is full");
    std::cout << "\n第" << victories_ + 1 << "戦 / " << Goal << "戦\n";
    Show(*hero_);
    Show(*enemy_);
    bool heroFirst = hero_->Settings.SPD > enemy_->Settings.SPD;
    if (hero_->Settings.SPD == enemy_->Settings.SPD)
        heroFirst = std::bernoulli_distribution(0.5)(random_);
    state_ = heroFirst ? GameState::HeroTurn : GameState::EnemyTurn;
    std::cout << (heroFirst ? "勇者が先攻\n" : "敵が先攻\n");
}

void ConsoleRpg::Run()
{
    const auto& enemies = GameDataTable::Instance().Enemies();
    const int totalHP = std::accumulate(enemies.begin(), enemies.end(), 0,
        [](int sum, const EnemyData& enemy) { return sum + enemy.Settings.HP; });
    const double averageATK = std::accumulate(enemies.begin(), enemies.end(), 0.0,
        [](double sum, const EnemyData& enemy) { return sum + enemy.Settings.ATK; }) / enemies.size();
    std::cout << "全敵 HP合計: " << totalHP << ", 平均 ATK: " << averageATK << '\n';
    for (const auto& enemy : enemies) ShowEnemyData(enemy);
    std::cout << "\nコンソールRPG開始！ 3連勝でクリア\n";

    while (state_ != GameState::Finished)
    {
        switch (state_)
        {
        case GameState::Encounter:
            Encounter();
            break;
        case GameState::HeroTurn:
            Attack(*hero_, *enemy_);
            state_ = enemy_->IsAlive() ? GameState::EnemyTurn : GameState::Victory;
            break;
        case GameState::EnemyTurn:
            Attack(*enemy_, *hero_);
            state_ = hero_->IsAlive() ? GameState::HeroTurn : GameState::Defeat;
            break;
        case GameState::Victory:
            ++victories_;
            std::cout << "\n勇者の勝利！ 残HP:" << hero_->Settings.HP << '\n';
            pool_.Release(enemy_);
            enemy_ = nullptr;
            if (victories_ == Goal)
            {
                std::cout << "3連勝！ ゲームクリア！\n";
                state_ = GameState::Finished;
            }
            else
            {
                const int before = hero_->Settings.HP;
                hero_->Settings.HP = std::min(GameDataTable::Instance().Hero().HP, before + 200);
                std::cout << "休憩でHPが" << hero_->Settings.HP - before << "回復\n";
                state_ = GameState::Encounter;
            }
            break;
        case GameState::Defeat:
            std::cout << "\n敵の勝利！ 敵残HP:" << enemy_->Settings.HP
                << "\nゲームオーバー。勝利数:" << victories_ << '\n';
            pool_.Release(enemy_);
            enemy_ = nullptr;
            state_ = GameState::Finished;
            break;
        case GameState::Finished:
            break;
        }
    }
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try
    {
        ConsoleRpg game;
        game.Run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "エラー: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

