#include "remix_fighters.h"
#include "fighter_registry.h"
#include "port_log.h"

#include <cstring>

namespace {

/* Vanilla FTKind order from SSB64: Jigglypuff/Purin is slot 10.
 * Meta Knight's +EXTRA 0.5.0 config declares JIGGLYPUFF as its base
 * character, so using that descriptor is the safest bring-up path while
 * its native FTData/status resources are being ported.
 */
constexpr int kVanillaFKindJigglypuff = 10;

static const char kMetaKnightResultsName[] = "Meta Knight";

void SeedMetaKnightBringup()
{
    const FighterDescriptor* parent = port_fighter_descriptor(kVanillaFKindJigglypuff);
    if (parent == nullptr) {
        port_log("SSB64 Remix: cannot seed Meta Knight; Jigglypuff descriptor missing\n");
        return;
    }

    FighterDescriptor desc = *parent;

    /* +EXTRA 0.5.0 metadata from extra_characters/MetaKnight/config.yaml.
     * The combat/resource fields intentionally inherit Jigglypuff for this
     * first native bring-up. This keeps FTKind 0x6D valid without pretending
     * Meta Knight's actual moveset has already been ported.
     */
    desc.costume_count = 6;
    desc.results_name = kMetaKnightResultsName;
    desc.results_name_lx = 20.0f;
    desc.results_name_scale = 0.55f;
    desc.results_wins_lx = 180.0f;

    /* The inherited vanilla team costumes are marked as "no synth override"
     * by ftport.c, which is correct until +EXTRA's six-costume mapping is
     * imported explicitly. */

    port_fighter_register(PORT_REMIX_FKIND_META_KNIGHT, &desc);
    port_log("SSB64 Remix: seeded Meta Knight bring-up descriptor (fkind=0x%02X, parent=Jigglypuff)\n",
             PORT_REMIX_FKIND_META_KNIGHT);
}

} // namespace

extern "C" void port_remix_seed_fighters(void)
{
    SeedMetaKnightBringup();
}
