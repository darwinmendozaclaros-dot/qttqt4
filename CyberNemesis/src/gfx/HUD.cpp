#include "gfx/HUD.h"
#include "entities/Boss.h"
#include "entities/Hero.h"
#include "core/Config.h"
#include "core/Util.h"
#include "core/Story.h"
#include <algorithm>
#include <cmath>

void HUD::Text(const char* txt, int x, int y, int size, Color c) {
    DrawText(txt, x + 1, y + 1, size, Fade(BLACK, 0.8f));
    DrawText(txt, x, y, size, c);
}

void HUD::TextCentered(const char* txt, int cx, int y, int size, Color c) {
    Text(txt, cx - MeasureText(txt, size) / 2, y, size, c);
}

void HUD::Bar(int x, int y, int w, int h, float v, float ghost, Color fill) {
    DrawRectangle(x, y, w, h, Color{12, 10, 28, 220});
    DrawRectangle(x, y, (int)(w * Util::Clamp01(ghost)), h, Fade(WHITE, 0.55f));
    DrawRectangle(x, y, (int)(w * Util::Clamp01(v)), h, fill);
    DrawRectangleLines(x - 1, y - 1, w + 2, h + 2, Fade(fill, 0.9f));
}

void HUD::MiniBar(int x, int y, int w, int h, float v, Color fill) {
    DrawRectangle(x, y, w, h, Color{12, 10, 28, 220});
    DrawRectangle(x, y, (int)(w * Util::Clamp01(v)), h, fill);
    DrawRectangleLines(x, y, w, h, Fade(fill, 0.6f));
}

void HUD::Reset(const Boss& boss, const Hero& hero) {
    bossShown_ = bossGhost_ = boss.hp / boss.maxHp;
    heroShown_ = heroGhost_ = hero.hp / hero.maxHp;
}

void HUD::Update(float dt, const Boss& boss, const Hero& hero) {
    const float b = boss.hp / boss.maxHp;
    const float h = hero.hp / hero.maxHp;
    bossShown_ = b; heroShown_ = h;
    bossGhost_ = (bossGhost_ < b) ? b : std::max(b, bossGhost_ - dt * 0.4f);
    heroGhost_ = (heroGhost_ < h) ? h : std::max(h, heroGhost_ - dt * 0.4f);
}

void HUD::DrawBattle(const Boss& boss, const Hero& hero, const Telemetry& tel, float hintAlpha) const {
    const int W = Cfg::VIEW_W, H = Cfg::VIEW_H;

    Text("NEMESIS-X  [TÚ]", 10, 5, 10, Theme::NEON_MAGENTA);
    Bar(10, 17, 250, 10, bossShown_, bossGhost_, Theme::NEON_MAGENTA);

    // Cooldowns: solo se muestran mientras recargan (si no hay nada, esta listo).
    if (boss.MeleeCooldownFrac() > 0.f) { Text("J",  10, 32, 10, Theme::NEON_YELLOW); MiniBar(22, 34, 40, 6, 1.f - boss.MeleeCooldownFrac(), Theme::NEON_YELLOW); }
    if (boss.ShotCooldownFrac()  > 0.f) { Text("K",  70, 32, 10, Theme::NEON_CYAN);   MiniBar(82, 34, 40, 6, 1.f - boss.ShotCooldownFrac(),  Theme::NEON_CYAN); }
    if (boss.DashCooldownFrac()  > 0.f) { Text("SH", 130, 32, 10, Theme::NEON_GREEN); MiniBar(150, 34, 40, 6, 1.f - boss.DashCooldownFrac(), Theme::NEON_GREEN); }
    if (!boss.AirJumpAvailable()) DrawCircle(204, 38, 3.f, Fade(GRAY, 0.4f));
    if (boss.IsOvercharged() && std::fmod(GetTime() * 6.0, 1.0) < 0.75)
        Text("SOBRECARGA", 214, 32, 10, Theme::NEON_YELLOW);

    const HeroTraits& t = hero.Traits();
    const int hx = W - 10 - 250;
    if (final_) Text(sCycle_ > 1 ? TextFormat("SERAFÍN - CAÍDAS: %d", sCycle_ - 1) : "SERAFÍN", hx, 5, 10, Color{255, 225, 120, 255});
    else        Text(TextFormat("ANDROIDE  GEN %02d", t.generation), hx, 5, 10, Theme::NEON_CYAN);
    Bar(hx, 17, 250, 10, heroShown_, heroGhost_, Theme::NEON_CYAN);
    if (!final_) {   // el Serafin no tiene fases: solo mejora al renacer
        DrawRectangle(hx + (int)(250 * Cfg::PHASE1_FRAC), 16, 1, 12, Fade(WHITE, 0.75f));
        DrawRectangle(hx + (int)(250 * Cfg::PHASE2_FRAC), 16, 1, 12, Fade(WHITE, 0.75f));
    }

    if (final_) {
        const Color gold{255, 225, 120, 255};
        if (sDecoy_ >= 0.f && sDecoy_ < 1.f) {      // recarga de las invocaciones: solo mientras recargan
            Text("SEÑUELO", hx, 31, 10, Fade(gold, 0.85f));
            MiniBar(hx + 56, 33, 40, 6, sDecoy_, sDecoy_ >= 1.f ? gold : Fade(gold, 0.5f));
        }
        if (sGuard_ >= 0.f && sGuard_ < 1.f) {
            Text("ORBES", hx + 112, 31, 10, Fade(gold, 0.85f));
            MiniBar(hx + 154, 33, 40, 6, sGuard_, sGuard_ >= 1.f ? gold : Fade(gold, 0.5f));
        }
    }

    (void)tel;

    if (hintAlpha > 0.f) {
        Text(final_ ? "ESQUIVA: SHIFT DASH   J APLASTAR (BORRA BALAS)   K/CLIC DISPARAR   P PAUSA"
                    : "A/D MOVER   W SALTAR   SHIFT DASH   S BAJAR   J APLASTAR   K/CLIC DISPARAR   P PAUSA", 10, H - 14, 10,
             Fade(WHITE, hintAlpha));
    }
}

std::vector<std::string> HUD::Wrap(const std::string& s, int maxW, int size) {
    std::vector<std::string> out;
    std::string cur, word;
    auto flush = [&]() {
        if (word.empty()) return;
        const std::string t = cur.empty() ? word : cur + " " + word;
        if (!cur.empty() && MeasureText(t.c_str(), size) > maxW) { out.push_back(cur); cur = word; }
        else cur = t;
        word.clear();
    };
    for (char ch : s) { if (ch == ' ') flush(); else word.push_back(ch); }
    flush();
    if (!cur.empty()) out.push_back(cur);
    return out;
}

void HUD::DrawCodex(int page, float t) const {
    const int W = Cfg::VIEW_W, H = Cfg::VIEW_H;
    DrawRectangle(0, 0, W, H, Fade(BLACK, 0.80f));
    for (int y = 0; y < H; y += 4) DrawRectangle(0, y, W, 1, Fade(Theme::NEON_CYAN, 0.04f));

    TextCentered("// ARCHIVO: CÓMO APRENDE LA IA //", W / 2, 14, 20, Theme::NEON_CYAN);
    const Story::CodexPage& pg = Story::Codex(page);

    DrawRectangle(40, 52, W - 80, 250, Fade(BLACK, 0.55f));
    DrawRectangleLines(40, 52, W - 80, 250, Fade(Theme::NEON_CYAN, 0.6f));
    Text(pg.title, 60, 68, 20, Theme::NEON_YELLOW);
    DrawRectangle(60, 96, W - 120, 1, Fade(Theme::NEON_CYAN, 0.5f));

    const std::vector<std::string> rows = Wrap(pg.body, W - 120, 10);
    const int shown = (int)(t * 90.f);          // aparece como si se estuviera escribiendo
    int remaining = shown;
    for (size_t i = 0; i < rows.size() && remaining > 0; ++i) {
        const int n = std::min((int)rows[i].size(), remaining);
        Text(Util::Utf8Head(rows[i], (size_t)n).c_str(), 60, 112 + (int)i * 15, 10, WHITE);
        remaining -= n;
    }

    for (int i = 0; i < Story::CODEX_PAGES; ++i)
        DrawRectangle(W / 2 - Story::CODEX_PAGES * 8 + i * 16, 312, 12, 4, i == page ? Theme::NEON_CYAN : Fade(WHITE, 0.25f));
    TextCentered("[A / D o FLECHAS] CAMBIAR PÁGINA     [ENTER / ESC] VOLVER", W / 2, 330, 10, Fade(WHITE, 0.75f));
}
