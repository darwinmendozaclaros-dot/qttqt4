#pragma once
#include "ai/AdaptiveBrain.h"
#include "ai/HeroTraits.h"
#include "core/Config.h"
#include "gfx/Dialogue.h"
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <map>
#include <string>
#include <vector>

// Lore, humor y contenido educativo. Texto en español con ñ, tildes, ¿ y ¡ (UTF-8; la fuente por defecto de Raylib cubre Latin-1).
// Personajes: NEMESIS-X (el jugador), HÉROE / SERAFÍN (la IA) y GUÍA (la voz que explica como aprende la IA).
namespace Story {
    inline constexpr Color PLAYER_COL {255, 43, 214, 255};    // NEMESIS-X
    inline constexpr Color HERO_COL   {24, 240, 255, 255};    // Heroe
    inline constexpr Color SERAFIN_COL{255, 225, 120, 255};   // Serafin
    inline constexpr Color GUIDE_COL  {57, 255, 120, 255};    // Guia
    inline constexpr Color SYSTEM_COL = GUIDE_COL;
    inline constexpr Color DATA_COL   = GUIDE_COL;

    inline constexpr const char* INTRO_LINE = "EL PROTOCOLO ADAPTATIVO LIBERA SU ÚLTIMO MODELO: UN SERAFÍN QUE HEREDA TODO LO APRENDIDO";
    inline constexpr const char* EPILOGUE   = "EL SERAFÍN CAYÓ %d VECES. CADA CAÍDA LE ENSEÑÓ ALGO... PERO NO FUE SUFICIENTE.";
    inline constexpr const char* MORAL      = "UNA IA SOLO APRENDE DE LOS DATOS QUE TÚ LE DAS: SI VARIAS, SUS CONTRAMEDIDAS FALLAN.";

    // De donde vino el dano que mato al Serafin en su ultima vida (lo mide Game).
    struct DeathReport {
        float melee = 0.f, plasma = 0.f, other = 0.f;
        int   dodges = 0, absorbed = 0;
    };

    inline std::string F(const char* fmt, ...) {
        char buf[640];
        va_list ap; va_start(ap, fmt); std::vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
        return buf;
    }

    inline Color ColorOf(const std::string& who) {
        if (who == "NEMESIS-X") return PLAYER_COL;
        if (who == "HÉROE")     return HERO_COL;
        if (who == "SERAFÍN")   return SERAFIN_COL;
        return GUIDE_COL;
    }
    inline DialogueLine L(const char* who, std::string text) { return { who, std::move(text), ColorOf(who) }; }

    // ------------------------------------------------------------------------------------------
    //  Variety: NINGUN dialogo se repite. Cada vez que se necesita una linea se elige, al azar,
    //  una variante que aun no se haya dicho; si ya se dijeron todas, no se dice nada.
    // ------------------------------------------------------------------------------------------
    class Variety {
    public:
        int Pick(int key, int count) {                    // indice libre de [0,count) o -1 si se agotaron
            uint64_t& m = seen_[key];
            int freeIdx[64]; int n = 0;
            for (int i = 0; i < count && i < 64; ++i) if (!(m & (1ull << i))) freeIdx[n++] = i;
            if (n == 0) return -1;
            const int i = freeIdx[Rand() % (uint32_t)n];
            m |= 1ull << i;
            return i;
        }
        void Reset() { seen_.clear(); }
    private:
        uint32_t Rand() { s_ ^= s_ << 13; s_ ^= s_ >> 17; s_ ^= s_ << 5; return s_; }
        std::map<int, uint64_t> seen_;
        uint32_t s_ = 2463534242u ^ (uint32_t)std::time(nullptr);
    };
    inline Variety& V() { static Variety v; return v; }
    inline void ResetVariety() { V().Reset(); }

    enum Key { K_GEN1 = 1, K_INTRO_HERO, K_INTRO_NEM, K_TIP, K_DODGE, K_DECOY, K_ORBS, K_SHIELD, K_DASH,
               K_HALF_H, K_HALF_S, K_LOW, K_DOWN_H, K_DOWN_S, K_OPEN, K_F_RANGED, K_F_MELEE, K_F_PRED,
               K_F_MOB, K_F_NONE, K_CLOSE, K_LAST_H, K_LAST_S, K_PROJ, K_OUTRO,
               K_FACT, K_FACT_P, K_FACT_D, K_CH_R, K_CH_M, K_CH_P, K_CH_MO, K_CH_N, K_BRUTAL, K_TECH };

    struct Beat    { const char* who; const char* text; };
    struct Variant { Beat b[3]; };

    inline std::vector<DialogueLine> FromVariant(const Variant& v) {
        std::vector<DialogueLine> out;
        for (const Beat& b : v.b) if (b.who) out.push_back(L(b.who, b.text));
        return out;
    }
    template <size_t N>
    inline std::vector<DialogueLine> PickVariant(int key, const Variant (&pool)[N]) {
        const int i = V().Pick(key, (int)N);
        return i < 0 ? std::vector<DialogueLine>{} : FromVariant(pool[i]);
    }

    // ----------------------------------------------------------------- Prologo (cuadro bloqueante)
    inline std::vector<DialogueLine> Prologue() {
        return {
            L("GUÍA",      "¡Bienvenido! Soy tu guía. Año 2187, instalación Cyber-Nemesis. Nivel de amenaza: máximo. Y el café, mínimo."),
            L("GUÍA",      "Tú eres NEMESIS-X, el guardián final. La instalación ha decidido eliminarte (sin rencores, es su trabajo)."),
            L("GUÍA",      "Para eso creó el Protocolo Adaptativo: una inteligencia artificial que aprende de cada combate."),
            L("GUÍA",      "Dato: esta IA no trae todo programado. Registra cómo peleas (telemetría) y usa esos datos como entrenamiento."),
            L("GUÍA",      "Esto es un proyecto educativo: en vez de explicarte la IA con fórmulas, te dejamos pelear contra una que aprende, para que la entiendas sintiéndola."),
            L("GUÍA",      "Por eso eres el Jefe: tu forma de jugar es el dato que la IA estudia. Tú cambias, ella cambia. Eso es un experimento con explosiones."),
            L("GUÍA",      "Cada vez que el Héroe cae, analiza que fallo y renace con una contramedida. Para ella, morir es estudiar."),
            L("NEMESIS-X", "Entonces cambiaré mi estilo. Si me repito, me predice. Si me predice, me aburro."),
        };
    }

    // Ganchos que rotan en la pantalla de inicio.
    inline const char* TitleHook(int i) {
        static const char* H[6] = {
            "¿Puedes vencer a una IA que aprende de ti?",
            "Mientras juegas, la IA anota cada ataque que haces.",
            "Si repites un patrón, la IA fabrica una contramedida.",
            "Aquí morir es entrenar: el Héroe vuelve más fuerte.",
            "Cada pelea es un experimento: cambia tu estilo y mira cómo responde la IA.",
            "Pista: cambiar de estilo es la mejor forma de confundirla.",
        };
        return H[((i % 6) + 6) % 6];
    }

    // Resumen final de la sesion: lo que la IA aprendio del visitante (con datos reales de la partida).
    struct SessionSummary {
        bool  hasData = false;
        float ranged = 0.f, melee = 0.f, predict = 0.f, mobility = 0.f;
        bool  shield = false, dash = false, lead = false, jets = false;
        int   heroDeaths = 0, seraphDeaths = 0, dodges = 0, absorbed = 0;
        int   choices = 0, insights = 0;          // respuestas elegidas / las que ensenaban algo
    };
    struct SummaryRow { std::string text; int kind; };      // kind: 0 titulo, 1 normal, 2 destacado

    inline std::vector<SummaryRow> SummaryLines(const SessionSummary& m) {
        auto pct = [](float x) { return (int)(x * 100.f + 0.5f); };
        std::vector<SummaryRow> v;
        v.push_back({ "TU ESTILO (lo que midió la IA)", 0 });
        if (m.hasData) {
            v.push_back({ F("Distancia %d%%   Cuerpo a cuerpo %d%%   Predecibilidad %d%%   Altura %d%%",
                            pct(m.ranged), pct(m.melee), pct(m.predict), pct(m.mobility > 1.f ? 1.f : m.mobility)), 1 });
            const char* style = (m.ranged - m.melee > 0.15f) ? "tirador a distancia"
                              : (m.melee - m.ranged > 0.15f) ? "luchador cuerpo a cuerpo"
                              : "equilibrado (mezclas distancia y cuerpo a cuerpo)";
            const char* tag = m.predict > Cfg::PREDICT_THRESHOLD ? " y bastante predecible"
                            : (m.predict < 0.35f ? " y difícil de predecir" : "");
            v.push_back({ F("Perfil: %s%s.", style, tag), 2 });
        } else {
            v.push_back({ "Sin datos suficientes de ataque en esta sesión: la IA no pudo estudiarte.", 1 });
        }

        v.push_back({ "LO QUE LA IA FABRICÓ PARA CONTRARRESTARTE", 0 });
        std::string list;
        auto add = [&](const char* s) { if (!list.empty()) list += "  |  "; list += s; };
        if (m.shield) add("Escudo de deflexión");
        if (m.dash)   add("Dash evasivo");
        if (m.lead)   add("Puntería predictiva");
        if (m.jets)   add("Propulsores");
        v.push_back({ list.empty() ? "Ninguna: tus patrones no fueron lo bastante claros." : list, 2 });

        v.push_back({ "SU ENTRENAMIENTO", 0 });
        v.push_back({ F("La IA murió y volvió %d veces (Héroe: %d, Serafín: %d).", m.heroDeaths + m.seraphDeaths, m.heroDeaths, m.seraphDeaths), 1 });
        v.push_back({ F("El serafín esquivó %d golpes y sus invocaciones bloquearon %d disparos.", m.dodges, m.absorbed), 1 });

        if (m.choices > 0) {
            v.push_back({ "TUS RESPUESTAS A LA IA", 0 });
            v.push_back({ F("Elegiste respuestas que enseñan algo en %d de %d ocasiones.", m.insights, m.choices), m.insights * 2 >= m.choices ? 2 : 1 });
        }

        v.push_back({ "LAS REGLAS DEL SISTEMA (umbrales reales)", 0 });
        v.push_back({ F("Distancia > %d%%: escudo   |   Cuerpo a cuerpo > %d%%: dash evasivo",
                        pct(Cfg::RATIO_THRESHOLD), pct(Cfg::RATIO_THRESHOLD)), 1 });
        v.push_back({ F("Predecible > %d%%: puntería predictiva   |   Altura > %d%%: propulsores",
                        pct(Cfg::PREDICT_THRESHOLD), pct(Cfg::MOBILITY_THRESHOLD)), 1 });

        v.push_back({ "PARA LLEVARTE", 0 });
        v.push_back({ "1) Los datos mandan: la IA solo aprendió de lo que hiciste.", 1 });
        v.push_back({ "2) Morir y volver es entrenar: error, análisis, mejora.", 1 });
        v.push_back({ "3) Variar tu estilo es la mejor defensa contra el sobreajuste.", 1 });
        return v;
    }

    // ------------------------------------------------------------ Consejos de la guia (educativos + humor)
    inline DialogueLine GuideTip() {
        static const char* T[24] = {
            "Dato: la IA no 've' tus golpes, solo cuenta números. Por eso no se asusta nunca. Tú sí te asustas: ventaja humana.",
            "Telemetría es tomar apuntes de lo que haces. La IA es ese compañero que anota todo y luego te lo recuerda en el peor momento.",
            "Si te mueves siempre igual, la IA lo nota. Ser predecible no es un insulto, es una métrica.",
            "Sobreajuste: una IA que memoriza tu estilo es como estudiar solo las preguntas del examen pasado. Funciona hasta que cambian el examen.",
            "Basura entra, basura sale: una IA con malos datos aprende malas ideas muy rápido. Como un tutorial de internet.",
            "brain.data guarda lo aprendido. Es el único diario que la IA no pierde... ni te perdona.",
            "Aprendizaje por refuerzo: probar, fallar, ajustar. Es como aprendiste a montar bicicleta, pero con menos raspones.",
            "Una contramedida solo funciona contra el patrón que la originó. Un escudo contra balas no te salva de un aplastamiento. De nada.",
            "Mezcla ataques de lejos y de cerca: es como variar el menú para que nadie adivine la cena.",
            "La IA decide con probabilidades, no con intuición. Cuando esquiva al 10% no adivina: tira un dado... uno muy educado.",
            "Entrenar una IA lleva tiempo y datos. Esta aprende en minutos porque es de juguete; las de verdad tardan días y gastan más luz que tu nevera.",
            "Nadie nace perfecto: el Serafín hereda lo que aprendieron los Héroes, pero aun así necesita morir varias veces para pulirse. Hasta las IA hacen borradores.",
            "Un modelo que nunca falla probablemente no está aprendiendo nada nuevo. Equivocarse también es un dato.",
            "Cuando la IA apunta a donde vas a estar, usa predicción. Cuando tú apuntas a donde está, usas optimismo.",
            "Generalizar es aplicar lo aprendido a situaciones nuevas. Si solo sirve en el mismo escenario, eso se llama memorizar.",
            "El 'dataset' es el conjunto de ejemplos con el que aprende la IA. Aquí el dataset eres tú. ¿Presión, eh?",
            "Sesgo: si la IA solo te ve disparar, cree que solo sabes disparar. Muéstrale otras cosas y aprenderá a dudar.",
            "Una IA no tiene mala intención, tiene una función objetivo. La suya: que no ganes. Nada personal.",
            "Más datos no siempre es mejor: datos repetitivos solo enseñan a ser repetitivo.",
            "Evaluar es probar la IA con situaciones que no vio. Tú cambiando de estrategia a mitad de pelea es su examen sorpresa.",
            "Robustez: tener varias defensas en lugar de una sola. Como llevar paraguas Y chubasquero.",
            "Los humanos también aprenden por refuerzo: te quemas una vez y entiendes lo de la estufa. La IA se quema por ti, igual.",
            "Ruido: datos raros que despistan. Tu salto de pánico sin motivo es ruido. Para la IA, arte moderno.",
            "Transferencia: usar lo aprendido en una tarea en otra. El Héroe de la gen 5 usa lo de la gen 4, aunque no lo agradece.",
        };
        const int i = V().Pick(K_TIP, 24);
        if (i < 0) return { "", "", Color{0, 0, 0, 0} };
        return L("GUÍA", T[i]);
    }

    // ------------------------------------------------ Inicio de pelea (Heroe)
    inline std::vector<DialogueLine> HeroIntro(const HeroTraits& t) {
        std::vector<DialogueLine> v;
        if (t.generation <= 1 && V().Pick(K_GEN1, 1) >= 0) {
            v.push_back(L("HÉROE",     "Primer despliegue. Todavía no sé nada de ti, NEMESIS-X."));
            v.push_back(L("NEMESIS-X", "Perfecto. Yo tampoco sé nada de mi plan."));
            v.push_back(L("GUÍA",      "Aquí la IA aprende de ti: cada ataque tuyo queda anotado. Si sientes que te observan, es porque te observan."));
            return v;
        }
        std::string list;
        auto add = [&](const char* s) { if (!list.empty()) list += ", "; list += s; };
        if (t.deflectShield) add("escudo contra tus disparos");
        if (t.dashEvasive)   add("dash contra tus embestidas");
        if (t.leadShooting)  add("puntería predictiva");
        if (t.jumpJets)      add("propulsores para seguirte en altura");
        const std::string abil = list.empty() ? "ninguna contramedida especial (tus patrones no estaban claros, bien jugado)" : list;

        static const char* H[8] = {
            "Generación %d en línea. Traigo: %s. Si parece un regalo, lo es... de ti para mí.",
            "¡Soy la gen %d! Mi equipo nuevo: %s. Gracias por los datos, fueron muy útiles.",
            "Versión %d del Héroe, ahora con %s. Todavía huelo a pintura fresca.",
            "Actualización %d instalada: %s. Reiniciar no fue necesario, morir sí.",
            "Gen %d reportandose. Inventario: %s. Pase lo que pase, hoy aprendo algo.",
            "Aquí la gen %d, equipada con %s. Prometo no repetir los errores... solo los nuevos.",
            "¡Generación %d! Llevo %s. Cada pieza es un 'tomo nota' de tus últimos combates.",
            "Gen %d desplegada con %s. Dicen que la práctica hace al maestro; en mi caso, la práctica eres tú.",
        };
        int i = V().Pick(K_INTRO_HERO, 8);
        if (i >= 0) v.push_back(L("HÉROE", F(H[i], t.generation, abil.c_str())));

        static const char* N[6] = {
            "¿Otra versión? Me están haciendo spoilers de mi propia derrota.",
            "Cada vez más elegante. Cada vez más destruible.",
            "Qué bien, más visitas. Voy a necesitar más explosivos.",
            "¿Mejoras? Yo también tengo trucos. Aunque ahora mismo no los recuerdo.",
            "Ya viene otro. Que alguien le avise que la puerta se abre hacia adentro.",
            "Mis cooldowns y yo estamos listos. Mi paciencia, menos.",
        };
        i = V().Pick(K_INTRO_NEM, 6);
        if (i >= 0) v.push_back(L("NEMESIS-X", N[i]));

        const DialogueLine g = GuideTip();
        if (!g.text.empty()) v.push_back(g);
        return v;
    }

    // ------------------------------------------------------------ Datos curiosos (reales, sin repetirse)
    inline constexpr int FACT_COUNT = 43;
    inline const char* FactAt(int i) {
        static const char* T[FACT_COUNT] = {
            "En Pac-Man cada fantasma persigue distinto: Blinky va directo a ti y Pinky intenta emboscarte unos pasos por delante. Cuatro 'personalidades' con reglas muy simples.",
            "En 1997 Deep Blue, de IBM, venció al campeón mundial Garry Kasparov en un match de ajedrez a seis partidas. Fue un hito: una máquina ganando bajo reglas de torneo.",
            "En 2016 AlphaGo venció 4-1 a Lee Sedol en Go, un juego con más posiciones posibles que átomos en el universo observable.",
            "Left 4 Dead usa un 'Director' de IA que ajusta el ritmo según lo estresado que te ve: si vas muy bien, suelta más zombis; si vas fatal, te da un respiro.",
            "En los años 50 Arthur Samuel hizo un programa de damas que mejoraba jugando contra sí mismo; en 1959 popularizó el término 'aprendizaje automático'.",
            "En 2019 OpenAI Five derrotó a los campeones del mundo de Dota 2 (el equipo OG). Entrenó jugando contra copias de sí mismo.",
            "En el juego CoastRunners una IA aprendió a dar vueltas chocando para recoger bonos en vez de terminar la carrera. Se llama 'reward hacking': la IA optimiza lo que mides, no lo que quieres.",
            "AlphaZero aprendió ajedrez jugando solo contra sí mismo, sin partidas humanas, y alcanzó nivel sobrehumano tras unas pocas horas de entrenamiento (2017).",
            "En 1943 McCulloch y Pitts propusieron el primer modelo matemático de una neurona artificial: las redes neuronales son más viejas que casi todos los videojuegos.",
            "El perceptrón de Frank Rosenblatt (1958) fue de las primeras máquinas que aprendían de ejemplos. Un solo perceptrón no podía aprender la función XOR, y eso frenó el área durante años.",
            "Alan Turing propuso en 1950 lo que hoy llamamos Test de Turing, pero con un nombre menos pegadizo: 'el juego de la imitación'.",
            "Gran Turismo Sophy (2022), una IA entrenada con aprendizaje por refuerzo, superó a pilotos de eSports de primer nivel en carreras.",
            "MNIST, un conjunto de 70.000 dígitos escritos a mano, ha servido para probar miles de algoritmos. Es el 'hola mundo' del aprendizaje automático.",
            "Una 'época' en aprendizaje automático es una pasada completa por todos los datos de entrenamiento. Las IA grandes necesitan muchas; tu Héroe solo necesitó una pelea.",
            "Resident Evil 4 ajusta la dificultad en secreto según cómo juegas, sin avisarte. Es una forma de IA que te 'lee' mientras te diviertes.",
            "F.E.A.R. (2005) se hizo famoso por la IA de sus soldados: usaban planificación de acciones (GOAP) para elegir cómo llegar a un objetivo en vez de seguir un guion.",
            "En Alien: Isolation el xenomorfo combina un 'director' que sabe más o menos dónde estás con un 'cerebro' propio que solo reacciona a lo que percibe. Por eso parece que te caza sin hacer trampa del todo.",
            "En 2012 la red AlexNet redujo tanto el error en reconocimiento de imágenes que cambió el rumbo de la IA moderna.",
            "Los jefes clásicos siguen patrones fijos que el jugador memoriza. Aquí pasa al revés: la IA memoriza los tuyos.",
            "Un videojuego es un laboratorio barato para la IA: reglas claras, puntuación medible y millones de partidas sin romper nada. Por eso tantos hitos de la IA son juegos.",
            "El 'efecto IA' dice que cuando una máquina logra algo, deja de llamarse IA: 'eso ya es solo programar'. Las IA de los videojuegos viven en ese limbo.",
            "Los fantasmas de Pac-Man a veces dejan de perseguirte y se dispersan a sus esquinas. Sin ese descanso el juego sería injusto: hasta la IA necesita pausas.",
            "El término 'inteligencia artificial' nació en 1956, en el taller de Dartmouth organizado por John McCarthy y otros.",
            "Las máquinas son malísimas tirando dados de verdad: el 'azar' de los juegos es un generador pseudoaleatorio, una fórmula que parece aleatoria pero no lo es.",
            "Un modelo sobreajustado es como el alumno que se sabe de memoria las respuestas del examen pasado pero no la materia. A tu Héroe le pasa si siempre juegas igual.",
            "Nimrod, una máquina de 1951 construida solo para jugar al Nim, fue de las primeras computadoras hechas para jugar y se exhibió al público en el Festival of Britain.",
            "Una GPU hace miles de multiplicaciones a la vez; por eso, aunque nació para dibujar videojuegos, hoy entrena modelos de IA. Tu tarjeta gráfica es prima de las que entrenan a los gigantes.",
            "Entrenar un modelo es ajustar sus pesos (caro, y se hace una vez); inferir es usarlo cada vez que alguien pregunta. Según estimaciones del sector, hoy la inferencia ya consume la mayor parte del cómputo de IA.",
            "Muchos celulares y laptops traen una NPU: un chip pensado solo para IA que corre modelos pequeños en el propio dispositivo, con menos espera y sin enviar tus datos a un servidor.",
            "Mover datos cuesta casi tanto como calcular: por eso la memoria HBM, apilada junto al procesador, es de lo más disputado del hardware de IA. Su cuarta generación (HBM4) empezó a producirse en 2026.",
            "Los centros de datos de IA ya se diseñan alrededor de la electricidad y el calor: refrigeración líquida, baterías y redes eléctricas propias. Hoy el rendimiento por vatio importa tanto como la potencia.",
            "La arquitectura Transformer, publicada por Google en 2017, usa 'atención' para decidir qué palabras importan en cada frase. Casi todos los modelos de lenguaje actuales descienden de ella.",
            "Los modelos de razonamiento 'piensan' paso a paso antes de responder: gastan más cómputo al usarlos, no solo al entrenarlos, para equivocarse menos.",
            "Un agente de IA no solo responde: usa herramientas, escribe código y encadena varios pasos hacia un objetivo. Se parece al Serafín: planea, actúa, mide el resultado y vuelve a intentarlo.",
            "El RLHF (aprendizaje por refuerzo con retroalimentación humana) afina a los asistentes de IA: personas califican respuestas y el modelo aprende a producir las que se prefieren. Es la misma idea de recompensa que mueve a tu Héroe.",
            "Las 'alucinaciones' son respuestas falsas dichas con total seguridad. Un modelo de lenguaje predice texto plausible, no verifica hechos: comprueba siempre lo importante.",
            "AlphaFold, de DeepMind, predijo la forma 3D de las proteínas con una precisión sin precedentes y sus creadores recibieron parte del Nobel de Química de 2024. La IA también sirve para la ciencia, no solo para jugar.",
            "La cuantización guarda los pesos de un modelo con menos bits (por ejemplo 8 o 4, en vez de 16 o 32). Pierde algo de precisión, pero ocupa menos memoria y puede caber en un celular.",
            "La destilación enseña a un modelo pequeño a imitar a uno grande, que hace de maestro. Así salen modelos rápidos que corren en una laptop.",
            "La ley de Moore se desacelera: ya no basta con hacer los transistores más pequeños. Por eso los chips de IA recurren a la especialización, a varios 'chiplets' en un mismo paquete y a la memoria apilada.",
            "El auto-juego (self-play) genera sus propios datos: AlphaZero mejoró jugando contra sí mismo. Se parece a lo que hace el Serafín: mejora a partir de sus propias derrotas.",
            "Un videojuego debe decidir en unos 16 milisegundos por fotograma (a 60 FPS). Por eso la IA de los juegos suele ser ligera, con reglas y umbrales, mientras que un modelo grande puede tardar segundos en responder.",
            "Entrenar un modelo de frontera puede costar decenas o cientos de millones de dólares en cómputo, según estimaciones públicas. Por eso importa tanto hacerlo eficiente y no repetir entrenamientos.",
        };
        return T[((i % FACT_COUNT) + FACT_COUNT) % FACT_COUNT];
    }
    // Un dato que aun no se haya dicho en ese contexto (key distinta = pool distinto). "" si ya salieron todos.
    inline std::string FactText(int key) {
        const int i = V().Pick(key, FACT_COUNT);
        return i < 0 ? std::string() : std::string(FactAt(i));
    }
    inline std::vector<DialogueLine> FunFactLines() {
        const std::string f = FactText(K_FACT);
        if (f.empty()) return {};
        return { L("GUÍA", "Dato curioso: " + f) };
    }

    // ------------------------------------------------------------ Responder a la IA (elecciones educativas)
    struct ChoiceScene {
        DialogueLine prompt;                                   // lo que dice la IA
        std::vector<std::string> options;                      // lo que puede responder NEMESIS-X
        std::vector<bool> good;                                // la respuesta ensena / acierta algo
        std::vector<std::vector<DialogueLine>> replies;        // IA + guia contestan a cada opcion
    };
    struct Opt   { const char* text; bool good; Beat r[2]; };
    struct Scene { Beat prompt; Opt o[3]; };

    inline ChoiceScene BuildScene(const Scene& sc) {
        ChoiceScene c;
        c.prompt = L(sc.prompt.who, sc.prompt.text);
        for (const Opt& o : sc.o) {
            c.options.push_back(o.text);
            c.good.push_back(o.good);
            std::vector<DialogueLine> r;
            for (const Beat& b : o.r) if (b.who) r.push_back(L(b.who, b.text));
            c.replies.push_back(r);
        }
        return c;
    }
    template <size_t N>
    inline bool PickScene(int key, const Scene (&pool)[N], ChoiceScene& out) {
        const int i = V().Pick(key, (int)N);
        if (i < 0) return false;
        out = BuildScene(pool[i]);
        return true;
    }

    // type: 0 distancia, 1 cuerpo a cuerpo, 2 predecible, 3 altura, 4 sin patron
    inline bool HeroScene(int type, ChoiceScene& out) {
        static const Scene R[3] = {
            { {"HÉROE","Fabriqué un escudo porque disparas mucho de lejos. ¿Qué me respondes?"}, {
                {"Entonces cambio a cuerpo a cuerpo: tu escudo no sirve ahí.", true,
                    {{"HÉROE","Ay. Justo lo que no quería que hicieras."},{"GUÍA","Correcto: una contramedida solo sirve contra el patrón que la originó. Cambiar de estrategia es la mejor forma de que una IA no te prediga."}}},
                {"Disparo más fuerte, a ver si lo rompo.", false,
                    {{"HÉROE","Perfecto: más datos del mismo tipo para mí."},{"GUÍA","Repetir el patrón lo refuerza: la IA aprende más de lo que más haces."}}},
                {"¿Cómo decidiste eso exactamente?", true,
                    {{"HÉROE","Conte tus disparos: superaron el umbral y active la regla del escudo."},{"GUÍA","Es una regla de umbral: si los disparos a distancia pasan de cierto porcentaje, la IA crea un escudo. No es magia: es un 'si... entonces' alimentado con tus datos."}}} } },
            { {"HÉROE","Ya vi suficientes disparos tuyos y mi escudo viene listo. ¿Algo que decir?"}, {
                {"Una sola pelea es muy poca muestra para juzgarme.", true,
                    {{"HÉROE","Cierto, pero ya tengo datos suficientes para actuar."},{"GUÍA","Con pocos datos una IA puede sacar conclusiones equivocadas. Por eso el sistema exige un mínimo de ataques antes de adaptarse."}}},
                {"Solo estaba calentando.", false,
                    {{"HÉROE","Calentar también se mide."},{"GUÍA","Para la IA no existe 'solo calentando': todo lo que haces es dato."}}},
                {"¿Qué más mides aparte de mis disparos?", true,
                    {{"HÉROE","Tus saltos, dashes, el tiempo en el aire y en qué zona de la arena estás."},{"GUÍA","Eso es telemetría: variables medidas de tu comportamiento. Cada variable es una pista sobre tu estilo."}}} } },
            { {"HÉROE","Según mis números eres un tirador. ¿Te molesta?"}, {
                {"Soy más que un número.", true,
                    {{"HÉROE","Seguro, pero yo solo veo los números."},{"GUÍA","Una métrica resume un estilo, no a la persona: los modelos solo ven las variables que les das, y de ahí nacen muchos sesgos."}}},
                {"Muchísimo. Ahora te destruyo.", false,
                    {{"HÉROE","Me encanta tu motivación. Y tus datos."},{"GUÍA","Las emociones no cambian la estadística: lo que cambia tu perfil es lo que haces."}}},
                {"Entonces me vuelvo impredecible.", true,
                    {{"HÉROE","Eso complicaría mis reglas..."},{"GUÍA","Variar rompe el patrón y la contramedida aprendida pierde valor. Cuando un modelo se ata demasiado a unos datos se llama sobreajuste."}}} } },
        };
        static const Scene M[3] = {
            { {"HÉROE","Te lanzas al cuerpo a cuerpo una y otra vez. Ahora tengo dash evasivo. ¿Qué haces?"}, {
                {"Mezclo disparos desde lejos para que el dash no baste.", true,
                    {{"HÉROE","Eso me lo pondrá difícil."},{"GUÍA","Diversificar ataques reduce el valor de cualquier contramedida aislada."}}},
                {"¡Aplasto más rápido!", false,
                    {{"HÉROE","Más velocidad con el mismo patrón: más datos para mí."},{"GUÍA","Hacerlo más rápido no cambia el patrón; la IA sigue leyéndolo igual."}}},
                {"¿Cuánto tarda en recargar tu dash?", true,
                    {{"HÉROE","Lo bastante para que lo pienses dos veces."},{"GUÍA","Explorar los límites del rival es método científico: observar, formular una hipótesis y probarla en la siguiente pelea."}}} } },
            { {"HÉROE","Tu aplastamiento es tu sello. Ya sé dónde NO estar. ¿Opinas algo?"}, {
                {"Voy a cambiar el ritmo y la distancia.", true,
                    {{"HÉROE","Maldición. Tendré que recalcular."},{"GUÍA","Cambiar ritmo y distancia mete variedad en tus datos, y la variedad es lo que más incómoda a un modelo ajustado a tu estilo."}}},
                {"Es que me encanta aplastar.", false,
                    {{"HÉROE","Lo sé: lo tengo anotado."},{"GUÍA","Los hábitos son oro para una IA: cuanto más constante eres, más fácil eres de predecir."}}},
                {"¿Entonces predices, no adivinas, verdad?", true,
                    {{"HÉROE","Exacto: uso tu historial, no una bola de cristal."},{"GUÍA","Predicción es usar datos pasados para anticipar lo que viene. Si el pasado es repetitivo, el futuro también lo parece."}}} } },
            { {"HÉROE","Mi nuevo dash esquiva tus golpes de cerca. ¿Tu respuesta?"}, {
                {"Esperar a que gaste el dash y entonces golpear.", true,
                    {{"HÉROE","Paciencia... odio la paciencia."},{"GUÍA","Eso es explotar un límite del modelo: toda contramedida tiene un costo o una recarga. Encontrarlos es parte de jugar contra una IA."}}},
                {"Gritar más fuerte.", false,
                    {{"HÉROE","No tengo oídos, solo sensores."},{"GUÍA","Las IA no se asustan ni se impresionan: reaccionan a datos, no a gritos."}}},
                {"Voy a probar algo que no hayas visto.", true,
                    {{"HÉROE","Eso... no lo tengo en mis datos."},{"GUÍA","Una IA se prepara para lo que ya vio; lo nuevo la deja sin respuestas. Esa debilidad se llama falta de generalización."}}} } },
        };
        static const Scene P[3] = {
            { {"HÉROE","Te mueves de forma muy predecible. Ya sé dónde vas a estar."}, {
                {"Predecible no es lo mismo que invencible.", true,
                    {{"HÉROE","Pero ayuda bastante."},{"GUÍA","La predecibilidad es una métrica: cuánto te repites en movimiento y zona. No es un defecto, es una pista que la IA aprovecha."}}},
                {"Mentira, soy imprevisible.", false,
                    {{"HÉROE","Los números dicen otra cosa."},{"GUÍA","Una IA no discute: mide. Si te repites, aparece en la estadística."}}},
                {"Entonces cambio de zona de la arena cada rato.", true,
                    {{"HÉROE","Eso me costaría bastante."},{"GUÍA","Cambiar de zona baja tu predecibilidad: parte de esa métrica es cuánto dominas una misma zona."}}} } },
            { {"HÉROE","Voy a apuntar donde VAS a estar, no donde estás. ¿Qué opinas?"}, {
                {"Es como adelantarse a un pase de fútbol.", true,
                    {{"HÉROE","Exactamente: anticipar el movimiento."},{"GUÍA","Es puntería predictiva: usar tu velocidad y dirección para calcular dónde estarás. Muchos sistemas reales predicen trayectorias así."}}},
                {"Entonces me quedo quieto.", false,
                    {{"HÉROE","Perfecto: un blanco fácil."},{"GUÍA","Quedarte quieto también es un patrón, y uno muy fácil de predecir."}}},
                {"¿Y si cambio de dirección justo cuando disparas?", true,
                    {{"HÉROE","Eso... sería un problema."},{"GUÍA","Los cambios bruscos rompen la extrapolación: el modelo supone que sigues igual. Cuando el mundo cambia, el modelo falla."}}} } },
            { {"HÉROE","Tus movimientos se repiten como un metrónomo. ¿Qué dices?"}, {
                {"Cambiaré el ritmo: lento, rápido, pausa.", true,
                    {{"HÉROE","Odio los ritmos raros."},{"GUÍA","Variar el ritmo mete ruido útil: hace que los patrones aprendidos dejen de encajar."}}},
                {"Me gusta la rutina.", false,
                    {{"HÉROE","A mí también: es muy cómoda de leer."},{"GUÍA","La rutina es el mejor amigo de una IA que predice."}}},
                {"¿Cómo mides lo del metrónomo?", true,
                    {{"HÉROE","Comparo muestras de tu movimiento: si repites la misma acción, sube mi puntaje."},{"GUÍA","El juego toma muestras periódicas y mide cuántas veces repites el mismo movimiento y cuánto dominas una zona. Ese es el 'puntaje de predecibilidad'."}}} } },
        };
        static const Scene Mo[2] = {
            { {"HÉROE","Vives en las plataformas. Ya tengo propulsores para alcanzarte. ¿Algo que decir?"}, {
                {"Bajo al suelo y cambio de altura todo el rato.", true,
                    {{"HÉROE","Me vas a dar trabajo."},{"GUÍA","Cambiar de altura es variar tu comportamiento: justo lo que desordena a un modelo que aprendió un solo estilo."}}},
                {"¡No puedes subir hasta aquí!", false,
                    {{"HÉROE","Mírame."},{"GUÍA","Cuando una IA detecta una restricción repetida, crea una herramienta para saltarsela: se llama adaptación."}}},
                {"¿Cuánto dura tu vuelo?", true,
                    {{"HÉROE","Lo suficiente para que me tomes en serio."},{"GUÍA","Preguntar por los límites de un sistema es el primer paso para entenderlo. Toda IA tiene límites, aunque no los anuncie."}}} } },
            { {"HÉROE","Tanto saltar me da mareo, pero ya aprendí a seguirte por el aire."}, {
                {"Pues dejo de saltar y camino por el suelo.", true,
                    {{"HÉROE","Y yo dejo de volar. Pero ahora tengo que recalcular."},{"GUÍA","Cambiar tu comportamiento deja obsoleta una parte del modelo: tendrá que aprender de nuevo con los datos nuevos."}}},
                {"¡Salto más alto!", false,
                    {{"HÉROE","Más altura es más dato."},{"GUÍA","Exagerar el mismo comportamiento no lo cambia: lo hace más fácil de medir."}}},
                {"¿Cómo supiste que saltaba tanto?", true,
                    {{"HÉROE","Mido tus saltos, dashes y cuánto tiempo pasas en plataformas."},{"GUÍA","Eso es movilidad: el mayor entre tus saltos y dashes por segundo y el tiempo en plataformas, normalizados con un umbral."}}} } },
        };
        static const Scene N[2] = {
            { {"HÉROE","No logre sacar un patrón claro de ti. Eso me incómoda."}, {
                {"Es que cambio todo el tiempo.", true,
                    {{"HÉROE","Entonces no tengo una contramedida específica."},{"GUÍA","Un jugador variado es el más difícil de modelar: sin patrón claro, no hay regla que aplicar."}}},
                {"Mejor: así nunca me ganarás.", false,
                    {{"HÉROE","Todavía tengo muchas peleas para aprender."},{"GUÍA","Que hoy no haya un patrón claro no significa que no aparezca mañana: con más datos, los patrones salen."}}},
                {"¿Cuántos datos necesitas para entenderme?", true,
                    {{"HÉROE","Al menos unos cuantos ataques registrados."},{"GUÍA","Las IA necesitan una muestra mínima: con muy pocos datos sacarían conclusiones al azar. En este juego, hace falta un mínimo de ataques."}}} } },
            { {"HÉROE","Con tan pocos datos mi mejor contramedida es mejorar mis números base."}, {
                {"Será más fácil si me ves pelear más tiempo.", true,
                    {{"HÉROE","Sin duda. Más datos, mejores decisiones."},{"GUÍA","Más datos reducen el azar en las conclusiones: es la idea de la 'muestra grande'."}}},
                {"Mejor sigue a ciegas.", false,
                    {{"HÉROE","A ciegas también se aprende. Solo que más despacio."},{"GUÍA","Sin datos suficientes, una IA no tiene en qué apoyarse: mejora lo general (vida, daño) en vez de lo específico."}}},
                {"¿Qué cambia si no te doy suficientes datos?", true,
                    {{"HÉROE","Solo subo mi vida, mi daño y mi cadencia."},{"GUÍA","Con pocos ataques registrados, la IA no activa contramedidas específicas: solo mejora estadísticas base. No inventa patrones."}}} } },
        };
        switch (type) {
            case 0: return PickScene(K_CH_R, R, out);
            case 1: return PickScene(K_CH_M, M, out);
            case 2: return PickScene(K_CH_P, P, out);
            case 3: return PickScene(K_CH_MO, Mo, out);
            default: return PickScene(K_CH_N, N, out);
        }
    }

    // Escena cuando el Serafin vuelve: usa datos reales de su ultima vida (fallen = 1..3).
    inline ChoiceScene SeraphScene(int fallen, const DeathReport& r) {
        const int k = fallen < 1 ? 0 : (fallen > 3 ? 2 : fallen - 1);
        ChoiceScene c;
        if (k == 0) {
            const float tot = r.melee + r.plasma + r.other;
            int dom = 0; float best = r.plasma;
            if (r.melee > best) { best = r.melee; dom = 1; }
            if (r.other > best) { best = r.other; dom = 2; }
            static const char* src[3] = { "tus disparos de plasma", "tus aplastamientos", "otros peligros de la arena" };
            c.prompt = L("SERAFÍN", tot > 0.f
                ? F("Según mis datos, el %d%% del daño que recibí vino de %s. ¿Qué opinas?", (int)(best / tot * 100.f + 0.5f), src[dom])
                : std::string("No tengo datos claros de cómo me mataste. ¿Qué opinas?"));
            c.options = { "Gracias por el dato: variaré mis ataques.", "Seguro tus datos están mal.", "¿Cómo sabes de dónde vino el daño?" };
            c.good = { true, false, true };
            c.replies = {
                { L("SERAFÍN", "Entonces ya no seré tan fácil de leer... pero lo intentaré."),
                  L("GUÍA", "Diagnosticar de dónde viene el error es el primer paso de cualquier mejora. Cambiar de estrategia, el segundo.") },
                { L("SERAFÍN", "Puedes dudar de mis datos, pero no de mis heridas."),
                  L("GUÍA", "Medir no es opinar: el juego registra el origen de cada daño. Dudar de un dato es sano, pero hay que comprobarlo, no negarlo.") },
                { L("SERAFÍN", "Cada vez que me golpeas, el juego anota el origen: plasma, cuerpo a cuerpo u otros."),
                  L("GUÍA", "Eso es un registro de eventos (logging). Sin buenos registros no hay buen diagnóstico.") } };
        } else if (k == 1) {
            c.prompt = L("SERAFÍN", F("Esquivé %d golpes en esa vida. ¿Te parece suerte o aprendizaje?", r.dodges));
            c.options = { "Aprendizaje: tu esquiva sube con cada derrota.", "Suerte pura.", "Un poco de las dos." };
            c.good = { true, false, true };
            c.replies = {
                { L("SERAFÍN", "Cada muerte me da más probabilidad de esquivar."),
                  L("GUÍA", "Exacto: lo que mejora con la experiencia es una probabilidad. El azar sigue ahí, pero los números cambian.") },
                { L("SERAFÍN", "Si fuera pura suerte, mi porcentaje no subiría."),
                  L("GUÍA", "Hay azar, pero el porcentaje de esquiva lo fija lo aprendido en cada vida. Sin esa mejora no habría diferencia entre vidas.") },
                { L("SERAFÍN", "Justo: el dado es de azar, pero lo cargo yo."),
                  L("GUÍA", "Los modelos probabilísticos mezclan ambas cosas: azar en cada evento, aprendizaje en la probabilidad.") } };
        } else {
            c.prompt = L("SERAFÍN", "Es mi última oportunidad. ¿Qué consejo me das antes de la pelea final?");
            c.options = { "Que no te confíes: yo también voy a cambiar.", "Ninguno, vas a perder.", "Revisa tus datos: quizá te falta variedad." };
            c.good = { true, false, true };
            c.replies = {
                { L("SERAFÍN", "Un rival que también aprende... me gusta."),
                  L("GUÍA", "Cuando dos agentes se adaptan el uno al otro, ninguno puede quedarse con una sola estrategia. Así funcionan muchas IA que juegan contra sí mismas.") },
                { L("SERAFÍN", "Eso dijeron las otras versiones."),
                  L("GUÍA", "Predecir un resultado sin datos es apostar. Una IA solo puede aprender de lo que ya ha visto.") },
                { L("SERAFÍN", "Variedad... lo anoto."),
                  L("GUÍA", "La diversidad en los datos es clave: una IA entrenada con ejemplos muy parecidos generaliza mal.") } };
        }
        return c;
    }

    // ------------- Cinematica cuando el Heroe cae y se reconstruye (usa la telemetria REAL del combate)
    inline const char* Concept(int fromGen) {
        static const char* C[5] = {
            "Concepto: telemetría. La IA mide tus ataques, tu movimiento y tu altura. Es como un contador de pasos, pero para pelear.",
            "Concepto: aprender de los errores. Una derrota es un dato; la IA compara que hizo y que fallo. Cero drama, mucha hoja de cálculo.",
            "Concepto: adaptación. Si usas siempre lo mismo, la IA crea una contramedida a medida. Un sastre, pero de venganzas.",
            "Concepto: predicción. Con suficientes datos, la IA anticipa dónde estarás y apunta ahí. No adivina el futuro: lee tu historial.",
            "Concepto: memoria. Lo aprendido se guarda en brain.data y sobrevive entre partidas. Reiniciar el juego no la hace olvidar.",
        };
        const int i = fromGen < 1 ? 0 : (fromGen > 5 ? 4 : fromGen - 1);
        return C[i];
    }

    struct RebirthScript {
        std::vector<DialogueLine> pre, closing;   // antes y despues de la eleccion
        ChoiceScene scene;
        bool hasScene = false;
    };

    inline RebirthScript HeroRebirthScript(const AdaptationReport& rep, const HeroTraits& next) {
        std::vector<DialogueLine> v;
        int firstType = -1;
        static const char* OPEN[5] = {
            "// Androide destruido // Reconstruyendo con la telemetría del combate... (no es un lavado, es un reentrenamiento).",
            "Pieza por pieza, la IA se rearma. En el proceso no se pierde la memoria, solo las piezas rotas.",
            "Modo reparación: el Héroe convierte los datos de la pelea en mejoras. Es la parte donde le llega la factura al Boss.",
            "Rebooteando androide. Si suena a ascensor de hotel, es normal.",
            "Atención: al caer, la IA no se enoja, actualiza parámetros. Mucho menos dramático que en las películas.",
        };
        int i = V().Pick(K_OPEN, 5);
        if (i >= 0) v.push_back(L("GUÍA", OPEN[i]));

        auto pct = [](float x) { return (int)(x * 100.f + 0.5f); };
        int found = 0;
        if (found < 2 && rep.enoughData && rep.rangedRatio > Cfg::RATIO_THRESHOLD) {
            static const char* R[5] = {
                "Disparaste desde lejos el %d%% de las veces. Eso me dio una idea: un escudo que desvíe tus proyectiles.",
                "El %d%% de tus ataques fueron a distancia. Mi nuevo escudo dice: 'gracias por avisar'.",
                "Mucho disparo lejano (%d%%). Instalo un escudo; la paciencia ya venía de serie.",
                "Según mis notas, el %d%% de tu estilo es disparar de lejos. Escudo en camino, sin costo de envío.",
                "Fuiste tirador el %d%% del tiempo. Voy a devolverte esos disparos con intereses.",
            };
            i = V().Pick(K_F_RANGED, 5);
            if (i >= 0) { v.push_back(L("HÉROE", F(R[i], pct(rep.rangedRatio)))); if (firstType < 0) firstType = 0; ++found; }
        }
        if (found < 2 && rep.enoughData && rep.meleeRatio > Cfg::RATIO_THRESHOLD) {
            static const char* M[5] = {
                "Te lanzaste al cuerpo a cuerpo el %d%% de las veces. Ahora seré más rápido y tendré un dash evasivo.",
                "Mucho aplastamiento (%d%%). Mi nuevo dash dice: 'permiso, ya me voy'.",
                "El %d%% de tus ataques fue de cerca. Aprendí a no estar donde cae el golpe. Revolucionario, lo sé.",
                "Cuerpo a cuerpo en el %d%% de los casos. Voy a necesitar zapatillas nuevas... digo, un dash.",
                "Me castigaste de cerca (%d%%). Mejora desbloqueada: esquivar por adelantado.",
            };
            i = V().Pick(K_F_MELEE, 5);
            if (i >= 0) { v.push_back(L("HÉROE", F(M[i], pct(rep.meleeRatio)))); if (firstType < 0) firstType = 1; ++found; }
        }
        if (found < 2 && rep.predictability > Cfg::PREDICT_THRESHOLD) {
            static const char* P[5] = {
                "Tus movimientos fueron predecibles en un %d%%. Ahora apuntaré a donde VAS a estar, no donde estás.",
                "Eres predecible al %d%%. Tranquilo, es una métrica, no un diagnóstico.",
                "Un %d%% de predecibilidad: casi te leo la mente. Bueno, el historial.",
                "Repetiste patrones (%d%%). Mi puntería ahora llega con adelanto, como el autobús que nunca llega... pero este sí.",
                "Predecible en un %d%%. Mejora instalada: disparos que te esperan en la esquina.",
            };
            i = V().Pick(K_F_PRED, 5);
            if (i >= 0) { v.push_back(L("HÉROE", F(P[i], pct(rep.predictability)))); if (firstType < 0) firstType = 2; ++found; }
        }
        if (found < 2 && rep.mobility > Cfg::MOBILITY_THRESHOLD) {
            static const char* Mo[5] = {
                "Usaste mucho las alturas. Instalaré propulsores para alcanzarte donde sea.",
                "Te encanta andar por las plataformas. Perfecto: acabo de aprender a volar. Más o menos.",
                "Tanto salto... tomo nota. Propulsores activados, aterrizajes en revisión.",
                "Subes y bajas como ascensor. Ahora yo también tengo botones: propulsores listos.",
                "Mucha altura en tu juego. Nuevo equipo: jets. Esto ya es una persecución en 3D... en 2D.",
            };
            i = V().Pick(K_F_MOB, 5);
            if (i >= 0) { v.push_back(L("HÉROE", Mo[i])); if (firstType < 0) firstType = 3; ++found; }
        }
        if (found == 0) {
            firstType = 4;
            static const char* Z[3] = {
                "No encontré un patrón claro en ti... Seguiré observando, pero volveré más rápido y resistente.",
                "Tus datos son un misterio. Me gusta: la IA odia no tener patrones. Aun así, vuelvo mejorado.",
                "Sin patrón claro, sin contramedida especial. Bien jugado... por ahora.",
            };
            i = V().Pick(K_F_NONE, 3);
            if (i >= 0) v.push_back(L("HÉROE", Z[i]));
        }

        v.push_back(L("GUÍA", Concept(rep.fromGen)));

        static const char* CL[5] = {
            "Generación %d lista: vuelvo con %d de vida y golpeando más fuerte. Prepárate, NEMESIS-X.",
            "Gen %d en pie, con %d de vida. Si me destruyes otra vez, será con más estilo.",
            "Reiniciado. Gen %d, %d de vida. Gracias por el entrenamiento gratuito.",
            "Gen %d operativa (%d de vida). Dicen que lo que no te destruye te hace más fuerte. Pues a mí me destruiste varias veces.",
            "Listo: generación %d, %d de vida y cero modestia. Vamos otra vez.",
        };
        RebirthScript out;
        out.pre = v;
        i = V().Pick(K_CLOSE, 5);
        if (i >= 0) out.closing.push_back(L("HÉROE", F(CL[i], next.generation, (int)(next.maxHp + 0.5f))));
        out.hasScene = HeroScene(firstType < 0 ? 4 : firstType, out.scene);
        return out;
    }

    // Version sin eleccion (lineas seguidas), por compatibilidad.
    inline std::vector<DialogueLine> HeroRebirth(const AdaptationReport& rep, const HeroTraits& next) {
        RebirthScript sc = HeroRebirthScript(rep, next);
        std::vector<DialogueLine> v = sc.pre;
        v.insert(v.end(), sc.closing.begin(), sc.closing.end());
        return v;
    }

    // Aviso (una sola vez) al empezar en "Muy dificil".
    inline std::vector<DialogueLine> BrutalNote() {
        if (V().Pick(K_BRUTAL, 1) < 0) return {};
        return { L("GUÍA", "Dificultad MUY DIFÍCIL: aquí casi todo el tiempo toca esquivar. Dato: subir la dificultad de un juego suele ser cambiar números (cadencia, velocidad, vida), no volver más lista a la IA.") };
    }

    // ------------------------------------------------ Inicio de la fase final
    inline std::vector<DialogueLine> SeraphIntro() {
        return {
            L("SERAFÍN",   "Soy la suma de seis generaciones de datos. Aun así, siento que me falta algo."),
            L("NEMESIS-X", "¿Un Serafín con todo ese historial? Vaya, ya no da pena."),
            L("GUÍA",      "El Serafín hereda lo aprendido por los Héroes: ya esquiva un poco y sabe invocar una copia. Cada muerte pulirá lo que le falte, como en un entrenamiento."),
        };
    }

    // ---- Cuando el Serafin muere y VUELVE a la pelea (fallen = 1..CYCLES-1). Se cuenta mientras pelea.
    inline std::vector<DialogueLine> Rebirth(int fallen, const DeathReport& r) {
        const Seraph::Stats a = Seraph::ForCycle(fallen), b = Seraph::ForCycle(fallen + 1);
        const int k = fallen < 1 ? 0 : (fallen > 3 ? 2 : fallen - 1);
        std::vector<DialogueLine> v;

        static const char* wake[3] = {
            "...¿Qué fue eso? Todo paso demasiado rápido.",
            "Otra vez caigo... pero esta vez te vi venir.",
            "Ya casi te entiendo, NEMESIS-X.",
        };
        v.push_back(L("SERAFÍN", wake[k]));

        static const char* quip[3] = {
            "¿Ya volvió? Pero si acababa de disfrutar el silencio.",
            "¿Otra vez? Esto ya parece una suscripción.",
            "Por la tercera vez te cobro entrada.",
        };
        v.push_back(L("NEMESIS-X", quip[k]));

        // Diagnostico REAL de la ultima vida.
        const float tot = r.melee + r.plasma + r.other;
        int dom = 0;                               // 0 plasma, 1 cuerpo a cuerpo, 2 otros
        float best = r.plasma;
        if (r.melee > best) { best = r.melee; dom = 1; }
        if (r.other > best) { best = r.other; dom = 2; }
        static const char* src[3] = { "tus disparos de plasma", "tus aplastamientos cuerpo a cuerpo", "otros peligros de la arena" };
        static const char* diag[3] = {
            "Diagnóstico: el %d%% del daño vino de %s. Esquivo %d golpes y sus invocaciones bloquearon %d disparos. No es chisme, es telemetría.",
            "Informe de la caída: %d%% del daño = %s. Golpes esquivados: %d. Disparos bloqueados: %d. Si fuera un examen, ya estaría estudiando.",
            "Autopsia digital: el %d%% del daño vino de %s. Esquives: %d. Bloqueos: %d. La IA ya sabe dónde se equivocó; tú aún lo estás decidiendo.",
        };
        if (tot > 0.f)
            v.push_back(L("GUÍA", F(diag[k], (int)(best / tot * 100.f + 0.5f), src[dom], r.dodges, r.absorbed)));
        else
            v.push_back(L("GUÍA", "Datos insuficientes: la IA no puede corregir lo que no mide."));

        // Una reaccion distinta para cada causa (plasma / cuerpo a cuerpo / otros) y para cada caida: nunca se repite.
        static const char* reaction[3][3] = {
            { "Tus disparos rompían mis copias demasiado rápido. Eso se corrige.",
              "Tus disparos aún lograban alcanzarme. Necesito defensas más fuertes.",
              "Seguías llegando con plasma desde lejos. Esta vez combino todas mis defensas." },
            { "Cuando te acercabas no tenía tiempo de reaccionar. Debo moverme mejor y ganar distancia.",
              "Tus aplastamientos todavía me alcanzaban. Esta vez calculo tu alcance con más cuidado.",
              "El cuerpo a cuerpo sigue siendo tu fuerte. Entonces yo aprendo a ser un poco más escurridizo." },
            { "Hubo peligros que no supe leer. Voy a estar más atento a la arena.",
              "Las trampas de la arena me siguen sorprendiendo. Ya me lo apunté.",
              "Aún me pillan peligros que no veo. Es mi punto ciego y lo estoy puliendo." },
        };
        v.push_back(L("SERAFÍN", reaction[dom][k]));

        static const char* mech[3] = {
            "Mis copias ahora aguantan más, y además invoco ORBES GUARDIANES que giran a mí alrededor y bloquean tus disparos.",
            "Ahora invoco DOS copias a la vez: clones de espejo. Mis orbes son más y vuelven más rápido.",
            "Esta es mi forma completa: cuatro orbes, copias dobles, ataques combinados y defensas que se recargan antes.",
        };
        v.push_back(L("SERAFÍN", mech[k]));

        v.push_back(L("SERAFÍN", F("Y corregí mis ataques: disparo más rápido y duele más (ataque del %d%% al %d%%), mi esquiva sube del %d%% al %d%% y ahora aguanto %d de vida.",
                                   (int)(a.attack * 100.f + 0.5f), (int)(b.attack * 100.f + 0.5f),
                                   (int)(a.dodge * 100.f + 0.5f),  (int)(b.dodge * 100.f + 0.5f), (int)b.hp)));

        static const char* lesson[3] = {
            "Aprendizaje por refuerzo: la IA prueba, recibe una 'penalización' si pierde y ajusta. Morir es un dato, no un final (aunque duela un poquito).",
            "Análisis de errores: no basta saber qué pasó, hay que saber por qué. La IA busca la causa que más se repite en sus derrotas, como tú cuando pierdes las llaves.",
            "Sobreajuste: una IA que solo se prepara para un ataque falla cuando cambias de estrategia. Por eso aprende varias defensas a la vez: esquiva, copias y orbes.",
        };
        v.push_back(L("GUÍA", lesson[k]));

        static const char* bye[3] = {
            "Vuelve a intentarlo, NEMESIS-X.",
            "Esta vez no será tan fácil.",
            "Esta es mi última oportunidad... y también la tuya.",
        };
        v.push_back(L("SERAFÍN", bye[k]));
        return v;
    }

    // ------------------------------------------------ A mitad de pelea (por eventos, sin repetir)
    enum class Ev { Dodge, Decoy, Orbs, Shield, Dash, HalfHp, LowHp };

    inline std::vector<DialogueLine> EventLines(Ev ev, bool seraph) {
        switch (ev) {
            case Ev::Dodge: {
                static const Variant P[4] = {
                    {{ {"SERAFÍN", "¡Esquivé! No fue suerte pura: mi probabilidad de esquivar crece con cada derrota."},
                       {"GUÍA", "Esquiva: aquí es una probabilidad. Lo que la IA mejora con la experiencia son esos números (y su ego)."} }},
                    {{ {"NEMESIS-X", "¿Se teletransporta? Eso es trampa."},
                       {"SERAFÍN", "Es 'agilidad aprendida'. Hay una diferencia legal... creo."},
                       {"GUÍA", "Un 20% de esquiva significa que, en promedio, 1 de cada 5 golpes falla. El azar no promedia, la estadística sí."} }},
                    {{ {"SERAFÍN", "¡Fallaste! Mi porcentaje de esquiva te manda saludos."},
                       {"GUÍA", "Cada derrota sube los números del Serafín: así 'aprende' una IA sin cerebro. Solo ajusta parámetros y presume."} }},
                    {{ {"NEMESIS-X", "¡Tenía el golpe seguro!"},
                       {"GUÍA", "Seguro, seguro... al 70%. La IA no ve golpes, ve probabilidades. Por eso a veces gana el que menos 'sabe'."} }},
                };
                return PickVariant(K_DODGE, P);
            }
            case Ev::Decoy: {
                static const Variant P[4] = {
                    {{ {"SERAFÍN", "¡Señuelo invocado! Que tus disparos se estrellen en una copia."},
                       {"GUÍA", "Señuelo: la IA aprendió que defenderse no es solo huir; también puede crear objetivos falsos."} }},
                    {{ {"NEMESIS-X", "Ese clon tiene mejor pose que tú."},
                       {"SERAFÍN", "Es el de la derecha. No, el otro. Bueno, da igual."},
                       {"GUÍA", "Un señuelo absorbe disparos mientras la IA gana tiempo. Estrategia pura (con un poco de teatro)."} }},
                    {{ {"SERAFÍN", "Copia lista. Idéntica a mí, salvo que ella no siente dolor."},
                       {"GUÍA", "Diversificar riesgos: si todo depende de una sola cosa, se rompe; si lo repartes, aguanta."} }},
                    {{ {"NEMESIS-X", "¿Otra copia? Ya me siento en una fotocopiadora."},
                       {"GUÍA", "Un señuelo dura pocos segundos y absorbe uno o dos disparos. Tu aplastamiento (J) lo borra de un golpe."} }},
                };
                return PickVariant(K_DECOY, P);
            }
            case Ev::Orbs: {
                static const Variant P[4] = {
                    {{ {"SERAFÍN", "¡Orbes guardianes! Cada uno bloquea un disparo."},
                       {"GUÍA", "Redundancia: una sola defensa es frágil; varias capas hacen a la IA más robusta."} }},
                    {{ {"NEMESIS-X", "Bolitas brillantes... qué miedo. Más o menos."},
                       {"GUÍA", "Los orbes giran y bloquean un disparo cada uno: capas de defensa. Tu aplastamiento (J) los revienta, por si te interesa."} }},
                    {{ {"SERAFÍN", "Mis orbes orbitan como satélites. Gratis, sin suscripción."},
                       {"GUÍA", "Cuando un sistema tiene varias capas, un fallo no lo tumba todo. En ingeniería: redundancia. En tu casa: dos llaves."} }},
                    {{ {"NEMESIS-X", "Los orbes son mosquitos con complejo de planeta."},
                       {"GUÍA", "La IA los invoca cuando ve un disparo en camino. No predice el futuro: reacciona rápido a lo que ya mide."} }},
                };
                return PickVariant(K_ORBS, P);
            }
            case Ev::Shield: {
                static const Variant P[4] = {
                    {{ {"HÉROE", "¡Mi escudo absorbió tu disparo!"},
                       {"GUÍA", "Contramedida: este escudo existe porque la IA midió que disparabas mucho a distancia."} }},
                    {{ {"NEMESIS-X", "¿Un escudo? Qué descaro. Me siento observado."},
                       {"GUÍA", "Lo estás: la IA contó tus disparos y fabricó la respuesta. Se llama adaptación y duele un poquito."} }},
                    {{ {"HÉROE", "Rebote perfecto. ¡Gracias por entrenarme!"},
                       {"GUÍA", "Cada vez que un escudo funciona, confirma que la IA leyó bien tu patrón. Cada vez que falla, tiene nuevos datos que leer."} }},
                    {{ {"NEMESIS-X", "Disparé demasiado, lo sé."},
                       {"GUÍA", "Si cambias a otro tipo de ataque, el escudo pierde sentido: la IA tendría que volver a aprender. Variar es tu superpoder."} }},
                };
                return PickVariant(K_SHIELD, P);
            }
            case Ev::Dash: {
                static const Variant P[4] = {
                    {{ {"HÉROE", "¡Dash evasivo! Te vi venir."},
                       {"GUÍA", "El dash aparece cuando la IA nota que atacas mucho cuerpo a cuerpo."} }},
                    {{ {"NEMESIS-X", "Se escurre como jabón. Odio eso."},
                       {"GUÍA", "Esquivar es una respuesta aprendida: si tu aplastamiento es tu sello, la IA aprende a no estar donde cae."} }},
                    {{ {"HÉROE", "Esquivar por adelantado: mi truco favorito. Y el único."},
                       {"GUÍA", "Predicción: usar datos pasados para anticipar. Tu aplastamiento es tan frecuente que ya tiene fan club (de datos)."} }},
                    {{ {"NEMESIS-X", "Se mueve antes de que yo me mueva. Ya es personal."},
                       {"GUÍA", "La IA no lee tu mente: lee tu historial. Si tu historial es repetir lo mismo, ella lo sabe antes de que lo hagas."} }},
                };
                return PickVariant(K_DASH, P);
            }
            case Ev::HalfHp: {
                static const Variant H[3] = {
                    {{ {"HÉROE", "Estoy perdiendo... pero guardo cada dato de tus golpes."},
                       {"GUÍA", "Telemetría en vivo: mientras peleas, la IA registra tus ataques para analizarlos al morir. No es rencor, es estadística."} }},
                    {{ {"HÉROE", "Mitad de vida. En mi idioma eso es 'zona de aprendizaje intensivo'."},
                       {"GUÍA", "Cuando una IA pierde, el dato más valioso es POR QUÉ perdió. Por eso morir le sirve más que ganar."} }},
                    {{ {"NEMESIS-X", "¿Ya casi? ¿Se acabó la fiesta de los porcentajes?"},
                       {"HÉROE", "Todavía no. Un 50% no es derrota, es un 'ya veremos'."},
                       {"GUÍA", "En aprendizaje automático el 'error' se mide: cuanto más se equivoca la IA, más cambia su conducta."} }},
                };
                static const Variant S[3] = {
                    {{ {"SERAFÍN", "Siento que me debilito... pero cada golpe tuyo también es un dato."},
                       {"GUÍA", "Datos en vivo: mientras peleas, la IA anota tus ataques para estudiarlos cuando muera. Es un diario, pero con más explosiones."} }},
                    {{ {"SERAFÍN", "Voy por la mitad. Todavía me queda dignidad y varias vidas."},
                       {"GUÍA", "Cada vida del Serafín es un intento de entrenamiento. En IA se llaman 'episodios': pruebas completas de las que se aprende."} }},
                    {{ {"NEMESIS-X", "Se cae a pedazos y aún habla. Admirable."},
                       {"SERAFÍN", "Hablar es gratis. Esquivar, solo un porcentaje."},
                       {"GUÍA", "Una IA no se cansa ni se desanima: solo optimiza. Tú sí puedes: descansa los dedos de vez en cuando."} }},
                };
                return seraph ? PickVariant(K_HALF_S, S) : PickVariant(K_HALF_H, H);
            }
            case Ev::LowHp: {
                static const Variant P[3] = {
                    {{ {"SERAFÍN", "Estoy cerca del final... y aun así, estoy aprendiendo."},
                       {"GUÍA", "Error: en aprendizaje automático se mide cuán mal lo hizo la IA. Cuanto más error, más cambia su conducta."} }},
                    {{ {"SERAFÍN", "Si caigo, vuelvo. Esa es mi ventaja sobre ti."},
                       {"GUÍA", "Reiniciar no borra lo aprendido si los datos se guardan. Por eso la IA vuelve sabiendo más, no menos."} }},
                    {{ {"NEMESIS-X", "¡Un golpe más y se acabó!"},
                       {"SERAFÍN", "O empieza otra versión de mi. Apuesta con cuidado."},
                       {"GUÍA", "Iterar es repetir mejorando. Lo importante de cada intento no es el resultado, sino lo que se aprende."} }},
                };
                return PickVariant(K_LOW, P);
            }
        }
        return {};
    }

    // ------------------------------------------------ Final de pelea
    inline DialogueLine LastWord(bool seraph) {
        static const char* H[6] = {
            "Datos registrados... volveré distinto.",
            "Ay. Anoten eso para la próxima versión.",
            "Error 404: victoria no encontrada.",
            "Apaguen la luz... y enciendan la actualización.",
            "Me desarmo, pero no se me olvida nada.",
            "¿Esto también cuenta como entrenamiento, verdad?",
        };
        static const char* S[3] = {
            "No... todavía no...",
            "Interesante. Duele, pero es información.",
            "Reinicio en 3, 2... ya, ya, voy.",
        };
        const int i = seraph ? V().Pick(K_LAST_S, 3) : V().Pick(K_LAST_H, 6);
        if (i < 0) return { "", "", Color{0, 0, 0, 0} };
        return L(seraph ? "SERAFÍN" : "HÉROE", seraph ? S[i] : H[i]);
    }

    inline std::vector<DialogueLine> SeraphFinalWords() {
        return {
            L("SERAFÍN",   "Lo entendí casi todo... pero no fue suficiente."),
            L("NEMESIS-X", "Casi. Esa palabra me encanta."),
            L("GUÍA",      "Entrenar no garantiza ganar: una IA mejora con datos y tiempo, pero tiene límites. Tu creatividad también cuenta (y tus reflejos)."),
        };
    }

    inline std::vector<DialogueLine> BossDownLines(bool seraph) {
        static const Variant H[3] = {
            {{ {"HÉROE", "Te derrote... pero cada derrota también te enseña a ti."},
               {"GUÍA", "Tú también aprendes: piensa que patrón de la IA te venció y cambia tu estrategia."} }},
            {{ {"HÉROE", "Gane yo, pero fue por poco. Muy poco. Casi nada."},
               {"GUÍA", "Perder es un dato para ti también. Pregunta útil: ¿qué hizo la IA que yo no esperaba?"} }},
            {{ {"NEMESIS-X", "Mala suerte, nada más."},
               {"GUÍA", "En IA, la 'mala suerte' se llama varianza. Con más intentos se promedia y gana el que mejor aprende."} }},
        };
        static const Variant S[3] = {
            {{ {"SERAFÍN", "Te derrote... y ni siquiera estaba al máximo."},
               {"GUÍA", "Cuando la IA gana no celebra: solo sube su 'recompensa'. La fiesta la pones tú (la IA no baila)."} }},
            {{ {"SERAFÍN", "Hoy gane yo. Mañana vuelves, claro."},
               {"GUÍA", "En el aprendizaje por refuerzo, las victorias refuerzan lo que funcionó. Si la IA gana con un truco, lo repetirá. Cuidado con tus malos hábitos."} }},
            {{ {"NEMESIS-X", "No me hables de derrotas: fue un descanso estratégico."},
               {"GUÍA", "Reintentar con otra estrategia es exactamente lo que hace la IA al morir. Cópiale el truco, es gratis."} }},
        };
        return seraph ? PickVariant(K_DOWN_S, S) : PickVariant(K_DOWN_H, H);
    }

    // -------------------------------------- Por que existe este proyecto (guiones, sin repetir)
    // Se sueltan de vez en cuando durante la pelea, mezclados con los consejos de la guia.
    inline std::vector<DialogueLine> ProjectNote() {
        static const Variant P[10] = {
            {{ {"NEMESIS-X", "Una pregunta: ¿por qué esto es un videojuego y no una charla?"},
               {"GUÍA", "Porque aprender haciendo se queda más que escuchar. Además, nadie se duerme esquivando balas."} }},
            {{ {"GUÍA", "Nota del proyecto: la IA es difícil de ver. Aquí la hacemos visible: miras cómo te mide, cómo responde y cómo cambia."},
               {"NEMESIS-X", "O sea, soy un experimento con cuernos."},
               {"GUÍA", "Un experimento con excelente puntería, sí."} }},
            {{ {"NEMESIS-X", "¿Y por qué un jefe final contra una IA?"},
               {"GUÍA", "Porque un enemigo que se adapta muestra la idea central de la IA: aprender de la experiencia para hacerlo mejor la próxima vez."} }},
            {{ {"GUÍA", "¿Por qué morir y volver? Porque así se entrena un modelo: intento, error, análisis, mejora. Aquí el fracaso es un dato, no una vergüenza."} }},
            {{ {"NEMESIS-X", "¿Y si cambio mi estilo?"},
               {"GUÍA", "Justo esa es la idea del proyecto: probar hipótesis. Cambias algo, observas que cambia la IA, sacas una conclusión. Método científico con explosiones."} }},
            {{ {"GUÍA", "Aviso honesto: esta IA es una versión simple. Usa reglas sobre tu telemetría, no una red neuronal. Sirve para entender el ciclo; las IA reales usan muchísimos más datos."},
               {"NEMESIS-X", "Pues me siento un poco menos amenazado."},
               {"GUÍA", "Eso dura hasta el Serafín."} }},
            {{ {"GUÍA", "¿Por qué el Serafín renace varias veces? Para mostrar que mejorar lleva iteraciones: nadie (ni nada) sale perfecto a la primera."},
               {"NEMESIS-X", "¿Ni siquiera yo?"},
               {"GUÍA", "Tú tampoco. Mira tus cooldowns."} }},
            {{ {"GUÍA", "Tú eres el dataset: la IA solo aprende de lo que hagas. El proyecto quiere que sientas cómo tus decisiones moldean a una IA, para que luego la mires con ojo crítico en la vida real."} }},
            {{ {"NEMESIS-X", "¿Entonces esto es una clase disfrazada?"},
               {"GUÍA", "Un laboratorio disfrazado de juego. La clase es opcional; las explosiones, no."} }},
            {{ {"GUÍA", "Otra razón para elegir un juego: puedes equivocarte sin consecuencias reales. En el mundo real, ajustar mal una IA puede salir caro. Aquí solo cuesta un Héroe."} }},
        };
        return PickVariant(K_PROJ, P);
    }

    // Cierre cuando cae el Serafin definitivamente.
    inline std::vector<DialogueLine> ProjectOutro() {
        static const Variant P[2] = {
            {{ {"GUÍA", "Fin del experimento. Lo que viste: una IA que mide, se equivoca, ajusta y vuelve. Eso es aprender, aunque sea con ayuda de explosiones."},
               {"NEMESIS-X", "Entonces gane mi examen."},
               {"GUÍA", "Ganó tu curiosidad. Ahora mira las IA reales con ojo crítico."} }},
            {{ {"GUÍA", "Por eso se eligió un juego: para que entendieras la idea sin leer una fórmula. Si te quedó una pregunta, ya aprendiste lo más importante: preguntar."},
               {"NEMESIS-X", "¿Preguntar qué? ¿Cuándo sale la secuela?"},
               {"GUÍA", "Esa también es una pregunta válida."} }},
        };
        return PickVariant(K_OUTRO, P);
    }

    // ------------------------------------------------------ Archivo (menu principal)

    // ------------------------------------------------------------------------------------------
    //  Lecciones: mini-guiones de 3 lineas sobre la IA de hoy y su hardware. Salen una vez cada
    //  una (Variety). "HÉROE" pasa a ser "SERAFÍN" cuando se pelea contra el Serafín.
    // ------------------------------------------------------------------------------------------
    inline std::vector<DialogueLine> TechLesson(bool seraph) {
        static const Variant pool[] = {
            { { { "GUÍA", "Lección de hardware: tu tarjeta gráfica y la IA comparten un truco. Las dos hacen miles de operaciones simples a la vez." },
                { "HÉROE", "Yo corro en una sola CPU y mis reglas caben en un archivo. Un modelo grande necesita miles de GPU." },
                { "GUÍA", "Por eso entrenarlos cuesta tanto: no es solo código, es hardware, tiempo y electricidad." } } },
            { { { "GUÍA", "Dos palabras clave: entrenar e inferir. Entrenar es aprender; inferir es usar lo aprendido." },
                { "HÉROE", "Yo entreno cuando caigo e infiero cada vez que decido cómo esquivarte." },
                { "GUÍA", "Exacto. En la IA de hoy, la mayor parte del cómputo ya se va en inferir: cada pregunta que alguien hace." } } },
            { { { "GUÍA", "Dato de hardware: a veces el cuello de botella no es calcular, sino traer los datos a tiempo." },
                { "HÉROE", "Como cuando tardo en recargar y tú ya estás encima." },
                { "GUÍA", "Algo así. Por eso la memoria HBM, apilada junto al procesador, es tan codiciada en los chips de IA." } } },
            { { { "GUÍA", "Los centros de datos de IA se diseñan pensando en la electricidad y el calor, no solo en los chips." },
                { "HÉROE", "Yo gasto lo que gasta un videojuego. Ellos, lo que gasta un barrio entero; los más grandes, una ciudad." },
                { "GUÍA", "Por eso la eficiencia importa: hacer lo mismo con menos vatios es una mejora real, no un detalle." } } },
            { { { "GUÍA", "La IA de moda son los modelos de lenguaje: aprenden a predecir la siguiente palabra a partir de muchísimo texto." },
                { "HÉROE", "¿Predecir? Yo también predigo: dónde vas a estar." },
                { "GUÍA", "Sí, pero ellos predicen texto plausible, no verdad. Por eso a veces 'alucinan'. Verifica siempre lo importante." } } },
            { { { "GUÍA", "Dos novedades recientes: modelos que 'piensan' paso a paso antes de responder, y agentes que usan herramientas." },
                { "HÉROE", "¿Un agente? Como yo: observo, planeo, actúo y mido el resultado." },
                { "GUÍA", "Parecido. Pero ellos persiguen objetivos muy generales, y tú solo uno: que no ganes." } } },
            { { { "GUÍA", "Los asistentes de IA se afinan con retroalimentación humana: personas califican respuestas y el modelo aprende qué se prefiere." },
                { "HÉROE", "Una recompensa. Igual que cuando tus aplastamientos me enseñan a esquivar." },
                { "GUÍA", "Sí, y el riesgo es el mismo: si la recompensa está mal diseñada, la IA optimiza lo equivocado." } } },
            { { { "GUÍA", "Cada vez más IA corre en tu propio dispositivo gracias a las NPU, chips pensados solo para eso." },
                { "HÉROE", "Sin enviar tus datos a ningún servidor. A mí me gusta: lo tuyo se queda en brain.data." },
                { "GUÍA", "Y de ahí sale una regla útil: pregunta siempre quién guarda tus datos y para qué." } } },
            { { { "GUÍA", "No todo es chat: AlphaFold predijo la forma de las proteínas y ayudó a la biología; sus creadores recibieron un Nobel." },
                { "HÉROE", "Una IA que aprende de datos... y no solo de golpes." },
                { "GUÍA", "La clave es siempre la misma: buenos datos, un buen objetivo y personas que revisen los resultados." } } },
        };
        std::vector<DialogueLine> v = PickVariant(K_TECH, pool);
        if (seraph)
            for (DialogueLine& l : v)
                if (l.speaker == "HÉROE") { l.speaker = "SERAFÍN"; l.color = SERAFIN_COL; }
        return v;
    }

    struct CodexPage { const char* title; const char* body; };
    inline constexpr int CODEX_PAGES = 24;
    inline const CodexPage& Codex(int i) {
        // Las cifras salen de Config.h: si se ajusta el balance, el Archivo se mantiene al dia.
        static const std::string rules = F(
            "Al caer el Héroe, AdaptiveBrain::Adapt evalúa tu telemetría con umbrales: disparos a distancia > %d%% -> escudo de deflexión (-%d%% de daño a distancia); "
            "cuerpo a cuerpo > %d%% -> dash evasivo y más velocidad; predecibilidad > %d%% -> puntería predictiva; altura/movilidad > %d%% -> propulsores. "
            "Además cada generación suma +%d de vida, x1.18 de daño, x0.92 en el intervalo de disparo y +7%% de esquiva (máximo 85%%). Con menos de %d ataques registrados, solo mejoran las estadísticas base.",
            (int)(Cfg::RATIO_THRESHOLD * 100.f + 0.5f), (int)(Cfg::DEFLECT_REDUCTION * 100.f + 0.5f),
            (int)(Cfg::RATIO_THRESHOLD * 100.f + 0.5f), (int)(Cfg::PREDICT_THRESHOLD * 100.f + 0.5f),
            (int)(Cfg::MOBILITY_THRESHOLD * 100.f + 0.5f), (int)(Cfg::HERO_HP_PER_GEN + 0.5f), Cfg::MIN_ATTACK_SAMPLE);
        static const std::string facts1 = std::string(FactAt(0)) + "  *  " + FactAt(1) + "  *  " + FactAt(2) + "  *  " + FactAt(3) + "  *  " + FactAt(5);
        static const std::string facts2 = std::string(FactAt(6)) + "  *  " + FactAt(9) + "  *  " + FactAt(14) + "  *  " + FactAt(21) + "  *  " + FactAt(23);
        static const CodexPage P[CODEX_PAGES] = {
            // ---- El proyecto: por que se eligio este juego ----
            { "¿POR QUÉ ESTE JUEGO?",
              "La IA suele explicarse con gráficos y fórmulas, y por eso parece magia. Este proyecto prueba otra idea: que la veas aprender "
              "mientras juegas. Un jefe final contra una IA que se adapta es un laboratorio disfrazado de videojuego." },
            { "APRENDER JUGANDO",
              "Jugar convierte conceptos en experiencias: no lees que una IA se adapta, lo notas cuando el Héroe llega con un escudo justo contra tu estilo. "
              "Aquí equivocarse no cuesta nada, y perder es parte de la lección." },
            { "TÚ ERES EL DATASET",
              "En una IA real, los datos mandan. En este juego los datos son tus ataques, tus saltos y tus costumbres. Cada pelea es un experimento: "
              "cambia tu estilo y observa cómo cambia la IA." },
            { "MORIR Y VOLVER = ENTRENAR",
              "Se eligió un enemigo que muere y regresa mejorado porque así funciona el entrenamiento: probar, fallar, analizar, mejorar y repetir. "
              "Cada vida del Héroe y del Serafín es una iteración (en IA se llaman episodios o épocas)." },
            { "QUÉ ES REAL Y QUÉ ES SIMPLE",
              "Honestidad: esta IA no es una red neuronal. Usa reglas con umbrales sobre tu telemetría (por ejemplo, si disparas mucho de lejos, crea un escudo). "
              "Es una versión simplificada del mismo ciclo: observar, medir, ajustar. Las IA reales usan muchísimos más datos y cálculos." },
            { "CÓMO JUGARLO PARA APRENDER",
              "Prueba tres cosas: 1) repite el mismo ataque y mira qué contramedida aparece; 2) cambia de estilo y compara; "
              "3) fíjate en los números que la IA dice haber medido de ti cuando se reconstruye (distancia, predecibilidad, altura). Convierte cada partida en un mini experimento." },
            // ---- Como aprende la IA ----
            { "QUÉ ES EL PROTOCOLO ADAPTATIVO",
              "Es la IA que controla al Héroe. No sigue un guion fijo: observa cómo luchas y cambia su forma de pelear. "
              "Cada generación es un modelo nuevo, entrenado con lo que paso en la anterior." },
            { "TELEMETRÍA: LOS DATOS",
              "Durante el combate el juego mide cuántos ataques haces a distancia y cuerpo a cuerpo, cuán predecible es tu movimiento "
              "y cuánto usas las alturas. Esos números son el 'dataset' del que la IA aprende. Sin datos no hay aprendizaje." },
            { "MORIR Y VOLVER: APRENDER DE LOS ERRORES",
              "Cuando el Héroe cae, la IA analiza la telemetría, detecta tu patrón más fuerte y crea una contramedida: escudo contra tus disparos, "
              "dash contra tus embestidas, puntería predictiva... Es aprendizaje por refuerzo: error, ajuste, nuevo intento." },
            { "EL SERAFÍN: UNA IA QUE RENACE",
              "En la fase final la IA muere varias veces. Tras cada caída diagnostica de dónde vino el daño (lo mide de verdad) y regresa con más ataque, "
              "más esquiva y mecánicas nuevas: copias, orbes y clones de espejo. Mejora por iteraciones, no por magia." },
            { "MEMORIA PERSISTENTE",
              "Lo aprendido se guarda en el archivo brain.data. Si cierras el juego, la IA recuerda. Puedes borrarla desde el menú "
              "para que olvide todo y empiece de cero." },
            { "CÓMO GANARLE A UNA IA QUE APRENDE",
              "Una IA solo puede aprender de los datos que le das. Si repites un patrón, lo predice. Si cambias de estilo, sus contramedidas "
              "pierden valor (es el sobreajuste: se prepara demasiado bien para algo que ya no pasa). Variar es tu mejor defensa." },
            // ---- Tecnico: como funciona por dentro ----
            { "TÉCNICO: LAS REGLAS DE ADAPTACIÓN", rules.c_str() },
            { "TÉCNICO: CÓMO SE MIDE TU ESTILO",
              "PatternTracker muestrea la partida y cuenta ataques cuerpo a cuerpo y a distancia, saltos, dashes, tiempo en el aire y en plataformas, y en qué zona de la arena estás. "
              "Predecibilidad = 0.65 x (proporción de muestras con el mismo movimiento) + 0.35 x (cuánto dominas una misma zona). "
              "Movilidad = el mayor entre (saltos + dashes por segundo / 0.9) y (tiempo en plataformas / 0.7)." },
            { "TÉCNICO: MEMORIA Y SUAVIZADO",
              "Cada métrica se guarda en una matriz de decisión con su último valor y una media móvil exponencial (60% historial + 40% dato nuevo), para que la IA no sobrerreaccione a una sola pelea. "
              "Todo se serializa en brain.data con una cabecera (marca y versión) y se valida al cargar: si el archivo está corrupto o es de otra versión, se descarta y la IA empieza de cero." },
            { "TÉCNICO: EL SERAFÍN Y SUS INVOCACIONES",
              "El Serafín es un planificador de turnos (FinalDirector) que alterna ataque y descanso, y vuela o camina. Su dificultad es fija en cada vida y sale de una tabla "
              "(vida, ataque, cadencia, daño, velocidad y cantidad de proyectiles, esquiva, agilidad, invocaciones). Señuelos y orbes detectan disparos que se acercan midiendo distancia y desviación lateral, y se activan con tiempos de recarga. "
              "Cada muerte registra de dónde vino el daño: ese es el diagnóstico que cuenta." },
            { "TÉCNICO: LÍMITES Y TRABAJO FUTURO",
              "Límites: son reglas fijas, no un modelo entrenado; con pocas peleas hay pocos datos; la IA solo reacciona al pasado inmediato. "
              "Mejoras posibles: sustituir los umbrales por aprendizaje por refuerzo (por ejemplo un bandido multibrazo o Q-learning), usar una red neuronal pequeña sobre la telemetría, "
              "medir diversión y dificultad con jugadores reales y comprobar si la IA generaliza a estilos nuevos." },
            // ---- La IA de hoy y su hardware ----
            { "LA IA EN 2026: QUÉ CAMBIÓ",
              "Hace una década una IA aprendía una sola tarea (jugar al Go, reconocer gatos). Hoy los modelos de lenguaje y multimodales (texto, imagen, audio y video) se entrenan con cantidades enormes de datos y sirven para muchísimas tareas. "
              "Tres ideas explican el salto: 1) la arquitectura Transformer (2017), que usa 'atención' para decidir qué partes del texto importan; 2) el escalamiento: más datos, más parámetros y más cómputo suelen dar mejores resultados; "
              "3) el ajuste con retroalimentación humana (RLHF), un aprendizaje por refuerzo parecido al de este juego: se premia lo que ayuda. "
              "Las novedades recientes son los modelos de razonamiento, que gastan más cómputo 'pensando' antes de responder, y los agentes, que usan herramientas y completan tareas de varios pasos." },
            { "HARDWARE (1/2): GPU, TPU Y NPU",
              "Una red neuronal es, sobre todo, multiplicar matrices enormes. Una CPU hace pocas cosas muy rápido y de una en una; una GPU hace miles de operaciones simples en paralelo, y por eso se volvió el motor de la IA (y de tus videojuegos). "
              "También hay chips hechos a medida: las TPU de Google y otros aceleradores propios de las grandes nubes, y las NPU que traen muchos celulares y laptops para correr IA en el propio dispositivo, sin enviar tus datos a un servidor. "
              "Hay dos trabajos distintos: entrenar (ajustar los pesos del modelo: caro y puntual) e inferir (usar el modelo cada vez que alguien le pregunta). Según varias estimaciones del sector, hoy la mayor parte del cómputo de IA ya se va en inferencia." },
            { "HARDWARE (2/2): MEMORIA Y ENERGÍA",
              "Mover datos cuesta casi tanto como calcular: por eso la memoria HBM, que apila chips de memoria pegados al procesador, es hoy uno de los componentes más disputados; su cuarta generación (HBM4) empezó a producirse en 2026. "
              "En 2026 también llegan plataformas que tratan un rack entero como una sola computadora de IA (por ejemplo, Vera Rubin de NVIDIA y Helios de AMD). "
              "Pero el límite ya no es solo el chip: es la electricidad y el calor. Los centros de datos de IA obligan a usar refrigeración líquida, baterías y redes eléctricas nuevas, y su consumo total se compara ya con el de un país grande. "
              "Por eso la eficiencia (más rendimiento por vatio) importa tanto como la potencia." },
            { "DEL JUEGO AL MUNDO REAL",
              "Este juego corre en una sola CPU y su 'cerebro' son reglas con umbrales: decide en microsegundos y cabe en un archivo diminuto (brain.data). Un modelo grande necesita miles de GPU, semanas de entrenamiento y muchísima energía. "
              "La idea de fondo es la misma: observar, medir, ajustar. Cuando uses una IA real: verifica lo importante (puede equivocarse con total seguridad, lo que se llama alucinación), recuerda que aprende de datos con sesgos, "
              "protege tu privacidad y pregunta de dónde vienen los datos y quién responde por los errores. Y si dudas, haz lo mismo que en esta pelea: cambia tu estilo y mira cómo responde." },
            // ---- Datos curiosos ----
            { "DATOS CURIOSOS (1/2): LA IA EN LOS JUEGOS", facts1.c_str() },
            { "DATOS CURIOSOS (2/2): HISTORIA Y TRAMPAS", facts2.c_str() },
            // ---- Sobre el codigo ----
            { "CÓMO ESTÁ HECHO",
              "El juego está escrito en C++17 con la biblioteca Raylib. No usa imágenes ni sonidos externos: los sprites se dibujan por código y la música se sintetiza. "
              "Así todo el proyecto cabe en una carpeta, se puede leer entero y es fácil de modificar para hacer tus propios experimentos." },
        };
        return P[i < 0 ? 0 : (i >= CODEX_PAGES ? CODEX_PAGES - 1 : i)];
    }
}
