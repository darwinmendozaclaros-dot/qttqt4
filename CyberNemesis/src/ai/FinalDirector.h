#pragma once
#include "raylib.h"
#include "core/Config.h"
#include <string>
#include <vector>

class Hero;
class Boss;
class Hazards;
class BulletField;

class FinalDirector {
public:
    enum class Pattern { None, Ring, Spiral, Rain, Fan, Sine, Blasters, Walls, Waves, Bounce, Pillars };

    // cycle = Eco actual (1..Seraph::CYCLES); st = estadisticas de ese Eco.
    void Start(const Seraph::Stats& st, const DiffParams& dp);
    void Update(float dt, Hero& hero, const Boss& boss, BulletField& bf, Hazards& hz);

    // --- Fases del Heroe: los patrones del Serafin tambien aparecen en el combate normal ---
    // Cada fase suma un patron: fase 1 = 1, fase 2 = 2 a la vez, fase 3 = 3 a la vez.
    void HeroStart(int generation, const DiffParams& dp);
    void HeroPhase(int phase);                    // 0..2: llamar al empezar cada fase
    void HeroStop() { heroOn_ = false; active_.clear(); }
    void HeroUpdate(float dt, Hero& hero, const Boss& boss, BulletField& bf, Hazards& hz);
    bool ConsumeHeroCycle() { const bool r = heroNew_; heroNew_ = false; return r; }
    const std::string& HeroLabel() const { return heroLabel_; }

    const char* PatternName() const;
    bool IsFlyingMode() const { return flying_; }
    bool InRest() const { return phase_ == Phase::Rest; }
    int  ConsumeBursts() { int r = bursts_; bursts_ = 0; return r; }

private:
    enum class Phase { Intro, Turn, Rest };
    struct Active {
        Pattern p = Pattern::None;
        float t = 0.f, dur = 8.f, tick = 0.f, ang = 0.f, aux = 0.f, scale = 1.f;
        int   n = 0;
    };

    void BeginTurn(const Boss& boss);
    void HeroBeginCycle();
    void Run(Active& a, float dt, Hero& hero, const Boss& boss, BulletField& bf, Hazards& hz);
    void SteerTurn(Hero& hero, const Boss& boss) const;
    void SteerRest(Hero& hero, const Boss& boss) const;
    Pattern PickPattern() const;
    static const char* NameOf(Pattern p);

    Phase  phase_ = Phase::Intro;
    float  phaseT_ = 0.f, clock_ = 0.f, restDur_ = 2.2f;
    bool   flying_ = true;
    int    tier_ = 0, turns_ = 0, bursts_ = 0;   // tier_ = intensidad fija del Eco (no cambia durante la pelea)
    float  restMul_ = 1.f, turnMul_ = 1.f;   // dificultad: descansos y turnos
    float  dmgMul_ = 1.f, speedMul_ = 1.f, density_ = 1.f;   // dano, velocidad y cantidad de sus proyectiles
    float  atk_ = 1.f, rate_ = 1.f;      // conocimiento de ataque (0.5..1) y cadencia relativa
    bool   heroOn_ = false, heroNew_ = false, heroStack_ = true;
    int    heroPhase_ = 0;
    float  heroT_ = 0.f, heroRest_ = 5.f;
    Pattern heroOrder_[3] = { Pattern::Ring, Pattern::Fan, Pattern::Rain };
    std::string heroLabel_;
    bool   relentless_ = false;   // dificultad "Muy difícil": sin pausas reales
    float  dripT_ = 0.f;          // temporizador del goteo de balas durante el descanso
    bool   knowsWalls_ = true, knowsBlasters_ = true, knowsRain_ = true, knowsCombos_ = true;
    Pattern last_ = Pattern::None, main_ = Pattern::None;
    std::vector<Active> active_;
};
