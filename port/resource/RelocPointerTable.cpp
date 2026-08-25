#include "RelocPointerTable.h"

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <spdlog/spdlog.h>

/**
 * Per-slot generational handle table mapping 32-bit token ↔ 64-bit pointer.
 *
 * Token layout: [12 bits slot-generation][20 bits slot-index]
 *   - Slot generations are PER-SLOT, not global. Each slot starts at gen 1
 *     and bumps on every release (reuse). A token resolves only if its
 *     embedded gen matches the slot's current gen — stale tokens fail decode.
 *   - 1M indices, 4096 generations per slot.
 *
 * A second, deliberately small mapping handles persistent raw N64 virtual
 * address ranges. Smash Remix generated action tables retain absolute N64
 * pointers; those values must map to native materialized data instead of being
 * interpreted as generational tokens on 64-bit hosts.
 */

namespace {

struct Slot {
    void *ptr;
    uint32_t gen;
};

struct RawAddressRange {
    uint32_t raw_base;
    uint64_t raw_end;
    uint8_t *host_base;
};

constexpr uint32_t TOKEN_GENERATION_SHIFT = 20;
constexpr uint32_t TOKEN_INDEX_MASK       = 0x000FFFFFu;
constexpr uint32_t TOKEN_GENERATION_MAX   = 0xFFFu;
constexpr uint32_t INITIAL_CAPACITY       = 256 * 1024;

static Slot     *sSlots       = nullptr;
static uint32_t  sNextIndex   = 1;
static uint32_t  sCapacity    = 0;
static std::vector<uint32_t> sFreeIndices;
static std::vector<RawAddressRange> sRawAddressRanges;

static void ensureCapacity(void)
{
    if (sSlots == nullptr) {
        sCapacity = INITIAL_CAPACITY;
        sSlots = (Slot *)calloc(sCapacity, sizeof(Slot));
        if (sSlots != nullptr) {
            const char *seed_env = getenv("SSB64_RELOC_GEN_SEED");
            if (seed_env != nullptr) {
                uint32_t seed = (uint32_t)strtoul(seed_env, nullptr, 0) & TOKEN_GENERATION_MAX;
                for (uint32_t i = 0; i < sCapacity; i++) {
                    sSlots[i].gen = seed;
                }
                spdlog::warn("RelocPointerTable: DIAG generation seed = {}", seed);
            }
        }
        return;
    }
    if (sNextIndex >= sCapacity) {
        uint32_t newCapacity = sCapacity * 2;
        if (newCapacity > TOKEN_INDEX_MASK) {
            spdlog::error("RelocPointerTable: token index capacity exhausted");
            abort();
        }
        spdlog::info("RelocPointerTable: growing {} -> {} entries",
                     sCapacity, newCapacity);
        Slot *grown = (Slot *)realloc(sSlots, newCapacity * sizeof(Slot));
        if (grown == nullptr) {
            spdlog::error("RelocPointerTable: out of memory growing to {} entries", newCapacity);
            abort();
        }
        sSlots = grown;
        memset(sSlots + sCapacity, 0, (newCapacity - sCapacity) * sizeof(Slot));
        sCapacity = newCapacity;
    }
}

static uint32_t bumpSlotGeneration(uint32_t gen)
{
    if (gen == 0) return 1;
    if (gen >= TOKEN_GENERATION_MAX) return 1;
    return gen + 1;
}

static uint32_t makeToken(uint32_t index, uint32_t gen)
{
    return (gen << TOKEN_GENERATION_SHIFT) | (index & TOKEN_INDEX_MASK);
}

static bool decodeToken(uint32_t token, uint32_t *outIndex)
{
    uint32_t tokenGen   = token >> TOKEN_GENERATION_SHIFT;
    uint32_t tokenIndex = token & TOKEN_INDEX_MASK;
    if (token == 0 || tokenGen == 0 || tokenIndex == 0 || tokenIndex >= sNextIndex) {
        return false;
    }
    if (sSlots == nullptr) {
        return false;
    }
    if (sSlots[tokenIndex].gen != tokenGen) {
        return false;
    }
    *outIndex = tokenIndex;
    return true;
}

static void *resolveRawAddress(uint32_t raw_address)
{
    const uint64_t address = static_cast<uint64_t>(raw_address);
    for (const RawAddressRange& range : sRawAddressRanges) {
        if (address >= range.raw_base && address < range.raw_end) {
            return range.host_base + static_cast<size_t>(address - range.raw_base);
        }
    }
    return nullptr;
}

} /* namespace */

extern "C" {

uint32_t portRelocRegisterPointer(void *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    ensureCapacity();

    uint32_t index;
    uint32_t gen;
    if (!sFreeIndices.empty()) {
        index = sFreeIndices.back();
        sFreeIndices.pop_back();
        if (sSlots[index].gen == 0) {
            sSlots[index].gen = 1;
        }
        gen = sSlots[index].gen;
    } else {
        index = sNextIndex++;
        ensureCapacity();
        gen = bumpSlotGeneration(sSlots[index].gen);
        sSlots[index].gen = gen;
    }
    sSlots[index].ptr = ptr;
    return makeToken(index, gen);
}

int portRelocRegisterRawAddressRange(uint32_t raw_base,
                                     void *host_base,
                                     size_t size)
{
    if (raw_base == 0 || host_base == nullptr || size == 0) {
        return 0;
    }

    const uint64_t start = static_cast<uint64_t>(raw_base);
    const uint64_t end = start + static_cast<uint64_t>(size);
    if (end > 0x100000000ULL || end <= start) {
        spdlog::error("RelocPointerTable: invalid raw range base=0x{:08X} size={}",
                      raw_base, size);
        return 0;
    }

    for (RawAddressRange& range : sRawAddressRanges) {
        const bool same = (range.raw_base == raw_base && range.raw_end == end);
        if (same) {
            range.host_base = static_cast<uint8_t *>(host_base);
            return 1;
        }

        const bool overlaps = (start < range.raw_end) &&
                              (end > static_cast<uint64_t>(range.raw_base));
        if (overlaps) {
            spdlog::error(
                "RelocPointerTable: refusing overlapping raw ranges 0x{:08X}..0x{:08X} and 0x{:08X}..0x{:08X}",
                raw_base, static_cast<uint32_t>(end - 1),
                range.raw_base, static_cast<uint32_t>(range.raw_end - 1));
            return 0;
        }
    }

    sRawAddressRanges.push_back({
        raw_base,
        end,
        static_cast<uint8_t *>(host_base)
    });

    spdlog::info("RelocPointerTable: raw alias 0x{:08X}..0x{:08X} -> {} bytes",
                 raw_base, static_cast<uint32_t>(end - 1), size);
    return 1;
}

void *portRelocResolvePointer(uint32_t token)
{
    return portRelocResolvePointerDebug(token, nullptr, 0);
}

void *portRelocResolvePointerDebug(uint32_t token, const char *file, int line)
{
    if (token == 0) {
        return nullptr;
    }

    /* Raw N64 aliases take precedence. A value in a registered patch window
     * is an address, even if its bit pattern could also decode as a very old
     * generational token after thousands of slot reuses. */
    if (void *raw = resolveRawAddress(token); raw != nullptr) {
        return raw;
    }

    uint32_t index = 0;
    if (!decodeToken(token, &index)) {
        static uint32_t sStaleLogCount = 0;
        if ((sStaleLogCount++ & 0x3FF) == 0) {
            uint32_t tokenGen   = token >> TOKEN_GENERATION_SHIFT;
            uint32_t tokenIndex = token & TOKEN_INDEX_MASK;
            uint32_t slotGen    = (sSlots && tokenIndex < sNextIndex) ? sSlots[tokenIndex].gen : 0;
            if (file != nullptr) {
                spdlog::error("RelocPointerTable: invalid/stale token 0x{:08X} "
                              "(token_gen=0x{:03X} slot_gen=0x{:03X} index={} max={}, caller={}:{}, miss_count={})",
                              token, tokenGen, slotGen, tokenIndex, sNextIndex - 1, file, line, sStaleLogCount);
            } else {
                spdlog::error("RelocPointerTable: invalid/stale token 0x{:08X} "
                              "(token_gen=0x{:03X} slot_gen=0x{:03X} index={} max={}, miss_count={})",
                              token, tokenGen, slotGen, tokenIndex, sNextIndex - 1, sStaleLogCount);
            }
        }
        return nullptr;
    }
    return sSlots[index].ptr;
}

void *portRelocTryResolvePointer(uint32_t token)
{
    if (token == 0) {
        return nullptr;
    }
    if (void *raw = resolveRawAddress(token); raw != nullptr) {
        return raw;
    }

    uint32_t index = 0;
    if (!decodeToken(token, &index)) {
        return nullptr;
    }
    return sSlots[index].ptr;
}

void portRelocInvalidateRange(const void *base, size_t size)
{
    if (sSlots == nullptr || base == nullptr || size == 0) {
        return;
    }
    uintptr_t lo = reinterpret_cast<uintptr_t>(base);
    uintptr_t hi = lo + size;
    for (uint32_t i = 1; i < sNextIndex; ++i) {
        if (sSlots[i].gen == 0) continue;
        uintptr_t p = reinterpret_cast<uintptr_t>(sSlots[i].ptr);
        if (p >= lo && p < hi) {
            sSlots[i].ptr = nullptr;
            sSlots[i].gen = bumpSlotGeneration(sSlots[i].gen);
            sFreeIndices.push_back(i);
        }
    }
}

void portRelocResetPointerTable(void)
{
    if (sSlots != nullptr) {
        memset(sSlots, 0, sCapacity * sizeof(Slot));
    }
    sNextIndex = 1;
    sFreeIndices.clear();
    /* Persistent raw aliases intentionally survive: their caller-owned backing
     * storage is process-lifetime data, not scene-arena memory. */
}

} /* extern "C" */
