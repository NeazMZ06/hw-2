#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "CameraComponent.h"
#include "CameraFollowerComponent.h"
#include "CameraShakeComponent.h"
#include "CameraSystem.h"
#include "CollisionComponent.h"
#include "CollisionLateSystem.h"
#include "CollisionSystem.h"
#include "ECSEngine.h"
#include "EntityManager.h"
#include "GravityComponent.h"
#include "GravitySystem.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "LocationComponent.h"
#include "MovementComponent.h"
#include "MovementSystem.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "ScoreSystem.h"
#include "SpawnComponent.h"
#include "SpawnSystem.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SpriteSystem.h"

std::string gResourcePath = "../assets/";
#define COMPONENTS                                                                             \
    ECSEngine::LocationComponent, ECSEngine::MovementComponent, ECSEngine::CollisionComponent, \
        ECSEngine::SpriteComponent, ECSEngine::SpawnComponent, ECSEngine::CameraComponent,     \
        ECSEngine::CameraFollower, ECSEngine::InputComponent, ECSEngine::GravityComponent,     \
        ECSEngine::CameraShake, ECSEngine::ScoreComponent, ECSEngine::RespawnComponent
using Engine = ECSEngine::ECSEngine<COMPONENTS>;

namespace {

std::string TrimCarriage(std::string line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

void AddSprite(Engine& engine, ECSEngine::EntityID id, ECSEngine::SpriteID texture, ECSEngine::Rect bounds,
               bool worldSpace) {
    ECSEngine::SpriteComponent sprite;
    sprite.textureID = texture;
    sprite.bounds = bounds;
    sprite.worldSpace = worldSpace;
    engine.GetEntityManager().AddComponent(id, sprite);
}

void AddStar(Engine& engine, ECSEngine::Point2D at, ECSEngine::SpriteID texture, ECSEngine::Point2D velocity) {
    auto& entities = engine.GetEntityManager();
    const ECSEngine::EntityID star = entities.CreateEntity("star");
    ECSEngine::LocationComponent loc;
    loc.loc = at;
    entities.AddComponent(star, loc);
    AddSprite(engine, star, texture, ECSEngine::MakeRect(0.f, 0.f, 48.f, 48.f), true);
    entities.AddComponent(star, ECSEngine::MovementComponent{velocity});
    ECSEngine::CollisionComponent collision{};
    collision.staticEntity = 0;
    collision.collisionBox = ECSEngine::MakeRect(6.f, 6.f, 36.f, 36.f);
    collision.previousCollisionBox = ECSEngine::WorldRect(loc.loc, collision.collisionBox);
    entities.AddComponent(star, collision);
}

}  // namespace

void LoadMap(Engine& engine, const std::string& path, const std::string& file,
             ECSEngine::SpriteID starSprite) {
    std::ifstream in(path + file);
    if (!in) {
        std::cerr << "Failed to open map " << path + file << '\n';
        return;
    }

    std::string keyword;
    int dictType = 1;
    in >> keyword >> dictType;
    std::string leftover;
    std::getline(in, leftover);

    struct TileDef {
        ECSEngine::SpriteID sprite = 0;
        bool solid = false;
        ECSEngine::Rect bounds{};
    };
    std::unordered_map<char, TileDef> dict;

    std::string line;
    while (std::getline(in, line)) {
        line = TrimCarriage(line);
        if (line.rfind("map ", 0) == 0) {
            break;
        }
        if (line.empty()) {
            continue;
        }
        std::istringstream row(line);
        char symbol = 0;
        std::string image;
        float sx = 0.f;
        float sy = 0.f;
        float sw = 0.f;
        float sh = 0.f;
        if (!(row >> symbol >> image >> sx >> sy >> sw >> sh)) {
            continue;
        }
        TileDef def;
        def.sprite = engine.GetSpriteManager().RegisterTexture(path + image, ECSEngine::MakeRect(sx, sy, sw, sh));
        def.bounds = ECSEngine::MakeRect(0.f, 0.f, sw, sh);
        if (dictType == 2) {
            float bx = 0.f;
            float by = 0.f;
            float bw = 0.f;
            float bh = 0.f;
            if (row >> bx >> by >> bw >> bh) {
                def.solid = true;
                def.bounds = ECSEngine::MakeRect(bx, by, bw, bh);
            }
        }
        dict[symbol] = def;
    }

    std::istringstream header(line);
    std::string word;
    float originX = 0.f;
    float originY = 0.f;
    float tileW = 0.f;
    float tileH = 0.f;
    int gridW = 0;
    int gridH = 0;
    header >> word >> word >> originX >> originY >> word >> tileW >> tileH >> word >> gridW >> gridH;

    auto& entities = engine.GetEntityManager();
    int rowIndex = 0;
    while (rowIndex < gridH && std::getline(in, line)) {
        line = TrimCarriage(line);
        if (line.empty()) {
            continue;
        }
        for (int col = 0; col < gridW && col < static_cast<int>(line.size()); ++col) {
            const char symbol = line[static_cast<size_t>(col)];
            if (symbol == '.' || symbol == ' ') {
                continue;
            }
            const auto found = dict.find(symbol);
            if (found == dict.end()) {
                continue;
            }
            const TileDef& def = found->second;
            const float x = originX + static_cast<float>(col) * tileW;
            const float y = originY + static_cast<float>(rowIndex) * tileH;
            const ECSEngine::EntityID id = entities.CreateEntity(std::string(1, symbol));
            ECSEngine::LocationComponent loc;
            loc.loc = ECSEngine::Point2D(x, y);
            entities.AddComponent(id, loc);
            AddSprite(engine, id, def.sprite, ECSEngine::MakeRect(0.f, 0.f, tileW, tileH), true);
            if (def.solid) {
                ECSEngine::CollisionComponent collision{};
                collision.staticEntity = 1;
                collision.collisionBox = def.bounds;
                collision.previousCollisionBox = ECSEngine::WorldRect(loc.loc, def.bounds);
                entities.AddComponent(id, collision);
            }
            if (symbol == 'S') {
                ECSEngine::SpawnComponent spawn;
                spawn.entityID = id;
                spawn.spawnType = "star";
                spawn.spawnTextureID = starSprite;
                spawn.timeToNextSpawn = 90;
                spawn.timeBetweenSpawns = 150;
                spawn.totalSpawns = 20;
                entities.AddComponent(id, spawn);
            }
        }
        ++rowIndex;
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && argv[1] == std::string("-path")) {
        gResourcePath = argv[2];
        if (!gResourcePath.empty() && gResourcePath.back() != '/' && gResourcePath.back() != '\\') {
            gResourcePath.push_back('/');
        }
    }
    std::cout << "Usage: " << argv[0] << " [-path resource_path]\n";
    std::cout << "Using resource path: " << gResourcePath << "\n";

    Engine engine(1024, 768, "Platformer");
    auto& sounds = engine.GetSoundManager();
    sounds.RegisterSound(gResourcePath + "sfx_jump.ogg", "jump");
    sounds.RegisterSound(gResourcePath + "sfx_bump.ogg", "land");
    sounds.RegisterSound(gResourcePath + "sfx_bump.ogg", "wall");
    sounds.RegisterSound(gResourcePath + "sfx_coin.ogg", "coin");
    sounds.RegisterSound(gResourcePath + "sfx_hurt.ogg", "hurt");

    auto& sprites = engine.GetSpriteManager();
    const ECSEngine::SpriteID starSprite = sprites.RegisterTexture(
        gResourcePath + "spritesheet-tiles-default.png", ECSEngine::MakeRect(195.f, 455.f, 64.f, 64.f));
    const float digitX[10] = {845.f, 910.f, 975.f, 1040.f, 1105.f, 0.f, 65.f, 130.f, 195.f, 260.f};
    const float digitY[10] = {195.f, 195.f, 195.f, 195.f, 195.f, 260.f, 260.f, 260.f, 260.f, 260.f};
    ECSEngine::SpriteID digits[10];
    for (int i = 0; i < 10; ++i) {
        digits[i] = sprites.RegisterTexture(gResourcePath + "spritesheet-tiles-default.png",
                                            ECSEngine::MakeRect(digitX[i], digitY[i], 64.f, 64.f));
    }
    const ECSEngine::SpriteID playerSprite = sprites.RegisterTexture(
        gResourcePath + "spritesheet-characters-default.png", ECSEngine::MakeRect(645.f, 0.f, 128.f, 128.f));

    LoadMap(engine, gResourcePath, "sky.map", starSprite);
    LoadMap(engine, gResourcePath, "world.map", starSprite);

    auto& entities = engine.GetEntityManager();
    const ECSEngine::Point2D start(128.f, 752.f);
    const ECSEngine::EntityID player = entities.CreateEntity("player");
    entities.AddComponent(player, ECSEngine::LocationComponent{start});
    AddSprite(engine, player, playerSprite, ECSEngine::MakeRect(0.f, 0.f, 72.f, 72.f), true);
    ECSEngine::CollisionComponent playerHit{};
    playerHit.staticEntity = 0;
    playerHit.collisionBox = ECSEngine::MakeRect(18.f, 12.f, 36.f, 56.f);
    playerHit.previousCollisionBox = ECSEngine::WorldRect(start, playerHit.collisionBox);
    entities.AddComponent(player, playerHit);
    entities.AddComponent(player, ECSEngine::MovementComponent{});
    entities.AddComponent(player, ECSEngine::GravityComponent(ECSEngine::Point2D(0.f, 1.15f)));
    entities.AddComponent(player, ECSEngine::InputComponent(player));
    entities.GetComponent<ECSEngine::InputComponent>(player).lastDash = ECSEngine::kDashCooldownFrames;
    entities.AddComponent(player, ECSEngine::RespawnComponent{start});

    // Sitting in the first vertical shaft, so reaching it takes a wall jump.
    AddStar(engine, ECSEngine::Point2D(33.f * 64.f + 8.f, 3.f * 64.f), starSprite,
            ECSEngine::Point2D(1.5f, 1.f));

    const float mapHeight = 14.f * 64.f;
    const float viewScale = mapHeight / 768.f;
    const ECSEngine::Point2D cameraStart(start.x + 36.f, mapHeight * 0.5f);
    const ECSEngine::EntityID camera = entities.CreateEntity("camera");
    ECSEngine::CameraComponent cameraComponent;
    cameraComponent.position = cameraStart;
    cameraComponent.worldUnitsPerPixel = viewScale;
    cameraComponent.speed = 0.12f;
    entities.AddComponent(camera, cameraComponent);
    entities.AddComponent(camera, ECSEngine::CameraFollower{player});
    entities.AddComponent(camera, ECSEngine::CameraShake{});
    engine.GetWindowManager().SetWorldScale(viewScale);
    engine.GetWindowManager().SetWorldCenter(cameraStart);

    const ECSEngine::EntityID scoreId = entities.CreateEntity("score");
    ECSEngine::ScoreComponent score;
    score.score = 0;
    for (int i = 0; i < 10; ++i) {
        score.digitTextureID[i] = digits[i];
    }
    for (int place = 0; place < 3; ++place) {
        const ECSEngine::EntityID digit = entities.CreateEntity("digit");
        ECSEngine::LocationComponent digitLoc;
        digitLoc.loc = ECSEngine::Point2D(16.f + static_cast<float>(place) * 42.f, 12.f);
        entities.AddComponent(digit, digitLoc);
        AddSprite(engine, digit, digits[0], ECSEngine::MakeRect(0.f, 0.f, 36.f, 36.f), false);
        score.digitID[place] = digit;
    }
    entities.AddComponent(scoreId, score);

    auto& sm = engine.GetSystemManager();
    sm.AddSystem(std::make_unique<ECSEngine::CollisionLateSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::InputSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::GravitySystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::MovementSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CollisionSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::ScoreSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CameraSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpriteSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpawnSystem<COMPONENTS>>());

    engine.Run();
    return 0;
}
