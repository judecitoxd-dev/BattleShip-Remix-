#include "remix_extra_vpk0.h"

#include <cstdint>
#include <vector>

namespace {

struct BitStream {
    const uint8_t* data = nullptr;
    size_t size = 0;
    size_t pos = 0;
    uint64_t bits = 0;
    unsigned avail = 0;
    bool ok = true;

    bool refill(unsigned need) {
        while (avail < need) {
            if (pos >= size || avail > 56) {
                ok = false;
                return false;
            }
            bits = (bits << 8) | data[pos++];
            avail += 8;
        }
        return true;
    }

    uint32_t read(unsigned count) {
        if (count == 0) return 0;
        if (count > 32 || !refill(count)) return 0;
        avail -= count;
        const uint64_t mask = (count == 32) ? 0xFFFFFFFFULL : ((1ULL << count) - 1ULL);
        return static_cast<uint32_t>((bits >> avail) & mask);
    }
};

struct HuffNode {
    int left = -1;
    int right = -1;
    uint8_t value = 0;
};

bool buildTree(BitStream& bs, std::vector<HuffNode>& nodes, int& root) {
    std::vector<int> stack;
    stack.reserve(256);
    nodes.clear();
    nodes.reserve(511);

    for (unsigned guard = 0; guard < 4096 && bs.ok; ++guard) {
        const uint32_t bit = bs.read(1);
        if (!bs.ok) return false;

        if (bit != 0 && stack.size() < 2) {
            if (stack.size() != 1) return false;
            root = stack.back();
            return true;
        }

        HuffNode node;
        if (bit != 0) {
            if (stack.size() < 2) return false;
            node.left = stack[stack.size() - 2];
            node.right = stack[stack.size() - 1];
            nodes.push_back(node);
            stack.resize(stack.size() - 2);
            stack.push_back(static_cast<int>(nodes.size() - 1));
        } else {
            node.value = static_cast<uint8_t>(bs.read(8));
            if (!bs.ok) return false;
            nodes.push_back(node);
            stack.push_back(static_cast<int>(nodes.size() - 1));
        }
    }
    return false;
}

bool treeDecode(BitStream& bs, const std::vector<HuffNode>& nodes, int root, uint8_t& out) {
    if (root < 0 || static_cast<size_t>(root) >= nodes.size()) return false;
    int index = root;
    for (unsigned guard = 0; guard < 512; ++guard) {
        const HuffNode& node = nodes[static_cast<size_t>(index)];
        if (node.left < 0) {
            out = node.value;
            return true;
        }
        const uint32_t bit = bs.read(1);
        if (!bs.ok) return false;
        index = bit ? node.right : node.left;
        if (index < 0 || static_cast<size_t>(index) >= nodes.size()) return false;
    }
    return false;
}

uint32_t readBe32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) |
            static_cast<uint32_t>(p[3]);
}

} // namespace

extern "C" size_t remix_extra_vpk0_decoded_size(const void* src, size_t src_size) {
    if (src == nullptr || src_size < 9) return 0;
    const auto* p = static_cast<const uint8_t*>(src);
    if (p[0] != 'v' || p[1] != 'p' || p[2] != 'k' || p[3] != '0') return 0;
    return static_cast<size_t>(readBe32(p + 4));
}

extern "C" size_t remix_extra_vpk0_decode(const void* src, size_t src_size,
                                            void* dst, size_t dst_capacity) {
    if (src == nullptr || dst == nullptr || src_size < 9) return 0;
    const auto* input = static_cast<const uint8_t*>(src);
    auto* output = static_cast<uint8_t*>(dst);

    const size_t decodedSize = remix_extra_vpk0_decoded_size(src, src_size);
    if (decodedSize == 0 || decodedSize > dst_capacity) return 0;

    // The SSB64 stream begins at byte 4. The first 32 bits repeat the decoded
    // size, followed by the sample method byte and two prefix-coded trees.
    BitStream bs{ input + 4, src_size - 4 };
    (void)bs.read(16);
    (void)bs.read(16);
    const uint32_t sampleMethod = bs.read(8);
    if (!bs.ok) return 0;

    std::vector<HuffNode> offsetNodes;
    std::vector<HuffNode> lengthNodes;
    int offsetRoot = -1;
    int lengthRoot = -1;
    if (!buildTree(bs, offsetNodes, offsetRoot) || !buildTree(bs, lengthNodes, lengthRoot)) {
        return 0;
    }

    size_t outPos = 0;
    while (outPos < decodedSize && bs.ok) {
        const uint32_t flag = bs.read(1);
        if (!bs.ok) return 0;

        if (flag == 0) {
            output[outPos++] = static_cast<uint8_t>(bs.read(8));
            if (!bs.ok) return 0;
            continue;
        }

        uint8_t extraBits = 0;
        if (!treeDecode(bs, offsetNodes, offsetRoot, extraBits) || extraBits > 31) return 0;
        uint32_t value = extraBits ? bs.read(extraBits) : 0;
        if (!bs.ok) return 0;

        int64_t copySource = 0;
        if (sampleMethod != 0) {
            uint32_t subOffset = 0;
            if (value <= 2) {
                subOffset = value + 1;
                if (!treeDecode(bs, offsetNodes, offsetRoot, extraBits) || extraBits > 31) return 0;
                value = extraBits ? bs.read(extraBits) : 0;
                if (!bs.ok) return 0;
            }
            copySource = static_cast<int64_t>(outPos) -
                         static_cast<int64_t>(value) * 4 -
                         static_cast<int64_t>(subOffset) + 8;
        } else {
            copySource = static_cast<int64_t>(outPos) - static_cast<int64_t>(value);
        }

        uint8_t lengthBits = 0;
        if (!treeDecode(bs, lengthNodes, lengthRoot, lengthBits) || lengthBits > 31) return 0;
        const uint32_t length = lengthBits ? bs.read(lengthBits) : 0;
        if (!bs.ok || length == 0) return 0;
        if (outPos + static_cast<size_t>(length) > decodedSize) return 0;

        for (uint32_t i = 0; i < length; ++i) {
            if (copySource < 0 || static_cast<size_t>(copySource) >= outPos) return 0;
            output[outPos++] = output[static_cast<size_t>(copySource++)];
        }
    }

    return (bs.ok && outPos == decodedSize) ? outPos : 0;
}
