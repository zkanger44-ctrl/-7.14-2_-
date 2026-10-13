#pragma once
#include "Enemy.h"
#include <algorithm>
#include <array>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

// コンストラクタ: 初期設定を受け取り、戦闘中の状態を初期化する。
class Player
{
public:
    std::string Name;
    PlayerSettings Settings;
    double CriticalRate;

    explicit Player(const PlayerSettings& settings, double criticalRate = 0.05)
        : Name(settings.Name), Settings(settings), CriticalRate(criticalRate) {}

    void Reset(const PlayerSettings& settings, double criticalRate)
    {
        Name = settings.Name;
        Settings = settings;
        CriticalRate = criticalRate;
    }
    bool IsAlive() const { return Settings.HP > 0; }
};

// シングルトン + データテーブル: マスターデータを一度だけ作る。
// 戦闘ではコピーを使い、マスターのHPを変更しない。
class GameDataTable
{
public:
    static const GameDataTable& Instance()
    {
        static const GameDataTable instance;
        return instance;
    }
    const std::vector<EnemyData>& Enemies() const { return enemies_; }
    const PlayerSettings& Hero() const { return hero_; }
    const EnemyData& FindEnemy(int id) const
    {
        for (const auto& enemy : enemies_)
            if (enemy.ID == id) return enemy;
        throw std::out_of_range("Unknown enemy ID");
    }
    GameDataTable(const GameDataTable&) = delete;
    GameDataTable& operator=(const GameDataTable&) = delete;

private:
    GameDataTable() : enemies_(CreateAllEnemies()) {}
    const PlayerSettings hero_{ "勇者", 1000, 100, 100, 50, 10 };
    const std::vector<EnemyData> enemies_;
};

// オブジェクトプール: 敵の実体を再利用する。取得ごとにHPなどを戻す。
class EnemyPool
{
public:
    Player* Acquire(const EnemyData& data)
    {
        for (auto& slot : slots_)
        {
            if (slot.inUse) continue;
            if (!slot.player)
                slot.player = std::make_unique<Player>(data.Settings, data.criticalRate);
            else
                slot.player->Reset(data.Settings, data.criticalRate);
            slot.inUse = true;
            return slot.player.get();
        }
        return nullptr;
    }
    void Release(Player* player)
    {
        for (auto& slot : slots_)
            if (slot.player.get() == player) { slot.inUse = false; return; }
    }

private:
    struct Slot
    {
        std::unique_ptr<Player> player;
        bool inUse = false;
    };
    std::array<Slot, 4> slots_{};
};

// ファクトリーパターン: IDや乱数から敵を生成し、生成方法をまとめる。
class CharacterFactory
{
public:
    static std::unique_ptr<Player> CreateHero()
    {
        return std::make_unique<Player>(GameDataTable::Instance().Hero());
    }
    static Player* CreateEnemy(int id, EnemyPool& pool)
    {
        return pool.Acquire(GameDataTable::Instance().FindEnemy(id));
    }
    static Player* CreateRandomEnemy(EnemyPool& pool, std::mt19937& random)
    {
        const auto& table = GameDataTable::Instance().Enemies();
        std::uniform_int_distribution<std::size_t> choice(0, table.size() - 1);
        return CreateEnemy(table[choice(random)].ID, pool);
    }
};

inline int CalculateDamage(const PlayerSettings& attacker, const PlayerSettings& defender)
{
    return std::max(1, attacker.ATK - defender.DEF);
}

// 有限状態機械: 戦闘開始、各ターン、結果、終了の遷移を明示する。
enum class GameState { Encounter, HeroTurn, EnemyTurn, Victory, Defeat, Finished };

class ConsoleRpg
{
public:
    explicit ConsoleRpg(unsigned int seed = std::random_device{}())
        : random_(seed), hero_(CharacterFactory::CreateHero()) {}
    void Run();

private:
    void Attack(Player& attacker, Player& defender);
    void Encounter();
    std::mt19937 random_;
    std::unique_ptr<Player> hero_;
    EnemyPool pool_;
    Player* enemy_ = nullptr; // 所有権はpool_にある。
    GameState state_ = GameState::Encounter;
    int victories_ = 0;
    static constexpr int Goal = 3;
};
