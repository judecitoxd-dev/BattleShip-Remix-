#include "remix_fighters.h"
#include "fighter_registry.h"
#include "port_log.h"

#include <cstddef>

namespace {

/*
 * Exact Smash Remix + EXTRA 0.5.0 roster metadata.
 *
 * Parent and costume values come from the 0.5.0 extra_characters/*/config.yaml
 * files, not current/main +EXTRA. This matters: several parent selections
 * changed in later releases.
 *
 * main_file_id / character_file_id stay zero until their exact 0.5.0 RELOC
 * IDs are verified. Meta Knight is already verified against the target ROM.
 */
static const PortRemixExtraFighterInfo kExtraFighters[] = {
    { PORT_REMIX_FKIND_BIRDO,        PORT_VANILLA_FKIND_JIGGLYPUFF, "Birdo",       "Birdo",            6,  "Birdo",      45.0f, 0.75f, 180.0f, 0, 0 },
    { PORT_REMIX_FKIND_CB_KNUCKLES,  PORT_VANILLA_FKIND_FOX,        "CBKnuckles",  "Knuckles",         6,  "Knuckles",   25.0f, 0.75f, 180.0f, 0, 0 },
    { PORT_REMIX_FKIND_CBM_KNUCKLES, PORT_VANILLA_FKIND_FOX,        "CBMKnuckles", "Knuckles",         6,  "Knuckles",   25.0f, 0.75f, 180.0f, 0, 0 },
    { PORT_REMIX_FKIND_CLOUD,        PORT_VANILLA_FKIND_LINK,       "Cloud",       "Cloud",            6,  "Cloud",      30.0f, 1.00f, 170.0f, 0, 0 },
    { PORT_REMIX_FKIND_DK_ULT,       PORT_VANILLA_FKIND_DONKEY,     "DKUlt",       "Ultimate DK",      5,  "DK ULT",     30.0f, 1.00f, 180.0f, 0, 0 },
    { PORT_REMIX_FKIND_KAZUYA,       PORT_VANILLA_FKIND_CAPTAIN,    "Kazuya",      "Kazuya",           6,  "Kazuya",     25.0f, 0.78f, 175.0f, 0, 0 },
    { PORT_REMIX_FKIND_KEN,          PORT_VANILLA_FKIND_CAPTAIN,    "Ken",         "Ken",              8,  "Ken",        50.0f, 1.00f, 160.0f, 0, 0 },
    { PORT_REMIX_FKIND_KNUCKLES,     PORT_VANILLA_FKIND_FOX,        "Knuckles",    "Knuckles",         6,  "Knuckles",   25.0f, 0.75f, 180.0f, 0, 0 },
    { PORT_REMIX_FKIND_M_KNUCKLES,   PORT_VANILLA_FKIND_FOX,        "MKnuckles",   "Knuckles",         6,  "Knuckles",   25.0f, 0.75f, 180.0f, 0, 0 },
    { PORT_REMIX_FKIND_MR_GAW,       PORT_VANILLA_FKIND_MARIO,      "MRGAW",       "Mr. Game & Watch", 7,  "Mr. G&W",     25.0f, 0.80f, 180.0f, 0, 0 },
    { PORT_REMIX_FKIND_MR_GAW_PLUS,  PORT_VANILLA_FKIND_MARIO,      "MRGAWPLUS",   "Mr. Game & Watch", 7,  "Mr. G&W",     25.0f, 0.50f, 180.0f, 0, 0 },
    { PORT_REMIX_FKIND_MR_GAW_3D,    PORT_VANILLA_FKIND_MARIO,      "MRGAWTHREED", "Mr. Game & Watch", 12, "Mr. G&W",     25.0f, 0.40f, 180.0f, 0, 0 },
    { PORT_REMIX_FKIND_META_KNIGHT,  PORT_VANILLA_FKIND_JIGGLYPUFF, "MetaKnight",  "Meta Knight",      6,  "Meta Knight", 20.0f, 0.55f, 180.0f, PORT_REMIX_FILE_META_KNIGHT_MAIN, PORT_REMIX_FILE_META_KNIGHT_CHARACTER },
    { PORT_REMIX_FKIND_REBECCA,      PORT_VANILLA_FKIND_FOX,        "Rebecca",     "Rebecca",          5,  "Rebecca",    20.0f, 0.90f, 185.0f, 0, 0 },
    { PORT_REMIX_FKIND_RYU,          PORT_VANILLA_FKIND_CAPTAIN,    "Ryu",         "Ryu",              8,  "Ryu",        50.0f, 1.00f, 160.0f, 0, 0 },
    { PORT_REMIX_FKIND_SNAKE,        PORT_VANILLA_FKIND_CAPTAIN,    "Snake",       "Snake",            7,  "Snake",      30.0f, 1.00f, 170.0f, 0, 0 },
    { PORT_REMIX_FKIND_SPIDERMAN,    PORT_VANILLA_FKIND_CAPTAIN,    "Spiderman",   "Spider-Man",       8,  "Spider-Man", 20.0f, 0.65f, 185.0f, 0, 0 },
    { PORT_REMIX_FKIND_TERRY,        PORT_VANILLA_FKIND_CAPTAIN,    "Terry",       "Terry",            7,  "Terry",      30.0f, 1.00f, 170.0f, 0, 0 },
    { PORT_REMIX_FKIND_YOUNG_ZELDA,  PORT_VANILLA_FKIND_FOX,        "YZelda",      "Young Zelda",      9,  "Young Zelda",20.0f, 0.55f, 180.0f, 0, 0 },
};

constexpr int kExtraFighterCount =
    static_cast<int>(sizeof(kExtraFighters) / sizeof(kExtraFighters[0]));

void SeedBringupRow(const PortRemixExtraFighterInfo& info)
{
    const FighterDescriptor* parent = port_fighter_descriptor(info.parent_fkind);
    if (parent == nullptr) {
        port_log("SSB64 Remix: cannot seed %s (fkind=0x%02X); parent %d is missing\n",
                 info.display_name, info.fkind, info.parent_fkind);
        return;
    }

    FighterDescriptor desc = *parent;

    /*
     * Parent-clone mode is deliberately conservative. The row gets a unique
     * synth FTKind and safe result-screen identity immediately, but model,
     * FTData, status functions and costume frames remain the vanilla parent
     * until that fighter's exact native resources are bound.
     *
     * In particular, do NOT copy declared_costume_count into costume_count
     * yet: a 12-costume synth backed by Mario's temporary model would let CSS
     * select nonexistent material-animation frames.
     */
    desc.costume_count = 0; /* accessor falls back to safe vanilla mapping */
    desc.default_costumes = nullptr;
    desc.default_costumes_count = 0;
    for (unsigned char& costume : desc.team_costume) {
        costume = 0xFF;
    }

    /* Never present the parent's results identity for a synth. Announcer and
     * emblem stay disabled until the corresponding Remix audio/model assets
     * are wired, while text geometry can already use the exact 0.5.0 values. */
    desc.results_announce_fgm = 0;
    desc.results_name = info.results_name;
    desc.results_name_lx = info.results_name_lx;
    desc.results_name_scale = info.results_name_scale;
    desc.results_wins_lx = info.results_wins_lx;
    desc.results_emblem_valid = 0;
    desc.results_emblem_dobjdesc = 0;
    desc.results_emblem_mobjsub = 0;
    desc.results_emblem_matanim = 0;

    port_fighter_register(info.fkind, &desc);

    port_log("SSB64 Remix: seeded %-16s fkind=0x%02X parent=%d costumes=%d assets=%d/%d\n",
             info.display_name, info.fkind, info.parent_fkind,
             info.declared_costume_count, info.main_file_id, info.character_file_id);
}

} // namespace

extern "C" {

void port_remix_seed_fighters(void)
{
    int seeded = 0;
    for (const PortRemixExtraFighterInfo& info : kExtraFighters) {
        SeedBringupRow(info);
        if (port_fighter_descriptor(info.fkind) != nullptr) {
            ++seeded;
        }
    }

    port_log("SSB64 Remix: +EXTRA 0.5.0 registry ready (%d/%d synth rows)\n",
             seeded, kExtraFighterCount);
}

int port_remix_extra_fighter_count(void)
{
    return kExtraFighterCount;
}

const PortRemixExtraFighterInfo* port_remix_extra_fighter_at(int index)
{
    if (index < 0 || index >= kExtraFighterCount) {
        return nullptr;
    }
    return &kExtraFighters[index];
}

const PortRemixExtraFighterInfo* port_remix_extra_fighter_info(int fkind)
{
    if (fkind < PORT_REMIX_FKIND_BIRDO || fkind > PORT_REMIX_FKIND_YOUNG_ZELDA) {
        return nullptr;
    }

    const int index = fkind - PORT_REMIX_FKIND_BIRDO;
    if (index < 0 || index >= kExtraFighterCount) {
        return nullptr;
    }

    const PortRemixExtraFighterInfo* info = &kExtraFighters[index];
    return (info->fkind == fkind) ? info : nullptr;
}

int port_remix_extra_is_fighter(int fkind)
{
    return port_remix_extra_fighter_info(fkind) != nullptr ? 1 : 0;
}

} // extern "C"
