#include "remix_extra_motion.h"

#include "port_log.h"
#include "remix_extra_source.h"
#include "resource/RelocPointerTable.h"

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

namespace {

std::once_flag sMotionOnce;
std::vector<uint8_t> sMotionArena;
int sMotionReady = 0;

static void ConvertWordsBigEndianToHost(std::vector<uint8_t>& bytes)
{
    for (size_t i = 0; i + 3 < bytes.size(); i += 4) {
        const uint32_t value =
            (static_cast<uint32_t>(bytes[i + 0]) << 24) |
            (static_cast<uint32_t>(bytes[i + 1]) << 16) |
            (static_cast<uint32_t>(bytes[i + 2]) << 8) |
            static_cast<uint32_t>(bytes[i + 3]);

        bytes[i + 0] = static_cast<uint8_t>(value & 0xFFu);
        bytes[i + 1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
        bytes[i + 2] = static_cast<uint8_t>((value >> 16) & 0xFFu);
        bytes[i + 3] = static_cast<uint8_t>((value >> 24) & 0xFFu);
    }
}

static void InitMotionArena()
{
    if (!remix_extra_source_exists()) {
        port_log("SSB64 Remix: motion arena unavailable; +EXTRA source missing\n");
        return;
    }

    sMotionArena.resize(static_cast<size_t>(REMIX_EXTRA_MOTION_ARENA_SIZE));
    const size_t got = remix_extra_source_read(
        static_cast<uint64_t>(REMIX_EXTRA_MOTION_ROM_BASE),
        sMotionArena.data(), sMotionArena.size());

    if (got != sMotionArena.size()) {
        port_log("SSB64 Remix: motion arena short read got=%zu expected=%zu\n",
                 got, sMotionArena.size());
        sMotionArena.clear();
        return;
    }

    /* Motion events and the small helper data they reference are encoded as
     * N64 big-endian words. BattleShip's PORT bitfields/parsers consume native
     * little-endian u32 values, matching the byteswap done for RELOC files. */
    ConvertWordsBigEndianToHost(sMotionArena);

    if (!portRelocRegisterRawAddressRange(
            REMIX_EXTRA_MOTION_RAW_BASE,
            sMotionArena.data(),
            sMotionArena.size())) {
        port_log("SSB64 Remix: failed to register motion raw-address range\n");
        sMotionArena.clear();
        return;
    }

    sMotionReady = 1;
    port_log("SSB64 Remix: native motion arena ready raw=0x%08X..0x%08X rom=0x%08X bytes=%zu\n",
             REMIX_EXTRA_MOTION_RAW_BASE, REMIX_EXTRA_MOTION_RAW_END,
             REMIX_EXTRA_MOTION_ROM_BASE, sMotionArena.size());
}

} // namespace

extern "C" {

int remix_extra_motion_init(void)
{
    std::call_once(sMotionOnce, InitMotionArena);
    return sMotionReady;
}

int remix_extra_motion_contains(uint32_t n64_address)
{
    return (n64_address >= REMIX_EXTRA_MOTION_RAW_BASE &&
            n64_address < REMIX_EXTRA_MOTION_RAW_END) ? 1 : 0;
}

void *remix_extra_motion_resolve(uint32_t n64_address)
{
    if (!remix_extra_motion_contains(n64_address) || !remix_extra_motion_init()) {
        return nullptr;
    }
    const size_t offset = static_cast<size_t>(n64_address - REMIX_EXTRA_MOTION_RAW_BASE);
    return sMotionArena.data() + offset;
}

size_t remix_extra_motion_arena_size(void)
{
    return remix_extra_motion_init() ? sMotionArena.size() : 0;
}

} // extern "C"
