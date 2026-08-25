#include "remix_roster.h"

namespace {

static const PortRemixRosterInfo kRoster[] = {
    { 0x1D, 1, "FALCO", PORT_REMIX_ROSTER_BASE },
    { 0x1E, 7, "GND", PORT_REMIX_ROSTER_BASE },
    { 0x1F, 5, "YLINK", PORT_REMIX_ROSTER_BASE },
    { 0x20, 0, "DRM", PORT_REMIX_ROSTER_BASE },
    { 0x21, 0, "WARIO", PORT_REMIX_ROSTER_BASE },
    { 0x22, 3, "DSAMUS", PORT_REMIX_ROSTER_BASE },
    { 0x23, 5, "ELINK", PORT_REMIX_ROSTER_BASE },
    { 0x24, 3, "JSAMUS", PORT_REMIX_ROSTER_BASE },
    { 0x25, 11, "JNESS", PORT_REMIX_ROSTER_BASE },
    { 0x26, 11, "LUCAS", PORT_REMIX_ROSTER_BASE },
    { 0x27, 5, "JLINK", PORT_REMIX_ROSTER_BASE },
    { 0x28, 7, "JFALCON", PORT_REMIX_ROSTER_BASE },
    { 0x29, 1, "JFOX", PORT_REMIX_ROSTER_BASE },
    { 0x2A, 0, "JMARIO", PORT_REMIX_ROSTER_BASE },
    { 0x2B, 4, "JLUIGI", PORT_REMIX_ROSTER_BASE },
    { 0x2C, 2, "JDK", PORT_REMIX_ROSTER_BASE },
    { 0x2D, 9, "EPIKA", PORT_REMIX_ROSTER_BASE },
    { 0x2E, 10, "JPUFF", PORT_REMIX_ROSTER_BASE },
    { 0x2F, 10, "EPUFF", PORT_REMIX_ROSTER_BASE },
    { 0x30, 8, "JKIRBY", PORT_REMIX_ROSTER_BASE },
    { 0x31, 6, "JYOSHI", PORT_REMIX_ROSTER_BASE },
    { 0x32, 9, "JPIKA", PORT_REMIX_ROSTER_BASE },
    { 0x33, 3, "ESAMUS", PORT_REMIX_ROSTER_BASE },
    { 0x34, 6, "BOWSER", PORT_REMIX_ROSTER_BASE },
    { 0x35, 6, "GBOWSER", PORT_REMIX_ROSTER_BASE },
    { 0x36, 0, "PIANO", PORT_REMIX_ROSTER_BASE },
    { 0x37, 1, "WOLF", PORT_REMIX_ROSTER_BASE },
    { 0x38, 1, "CONKER", PORT_REMIX_ROSTER_BASE },
    { 0x39, 6, "MTWO", PORT_REMIX_ROSTER_BASE },
    { 0x3A, 7, "MARTH", PORT_REMIX_ROSTER_BASE },
    { 0x3B, 1, "SONIC", PORT_REMIX_ROSTER_BASE },
    { 0x3C, 7, "SANDBAG", PORT_REMIX_ROSTER_BASE },
    { 0x3D, 1, "SSONIC", PORT_REMIX_ROSTER_BASE },
    { 0x3E, 7, "SHEIK", PORT_REMIX_ROSTER_BASE },
    { 0x3F, 7, "MARINA", PORT_REMIX_ROSTER_BASE },
    { 0x40, 7, "DEDEDE", PORT_REMIX_ROSTER_BASE },
    { 0x41, 0, "GOEMON", PORT_REMIX_ROSTER_BASE },
    { 0x42, 1, "PEPPY", PORT_REMIX_ROSTER_BASE },
    { 0x43, 1, "SLIPPY", PORT_REMIX_ROSTER_BASE },
    { 0x44, 7, "BANJO", PORT_REMIX_ROSTER_BASE },
    { 0x45, 4, "MLUIGI", PORT_REMIX_ROSTER_BASE },
    { 0x46, 0, "EBI", PORT_REMIX_ROSTER_BASE },
    { 0x47, 7, "DRAGONKING", PORT_REMIX_ROSTER_BASE },
    { 0x48, 0, "CRASH", PORT_REMIX_ROSTER_BASE },
    { 0x49, 1, "PEACH", PORT_REMIX_ROSTER_BASE },
    { 0x4A, 7, "ROY", PORT_REMIX_ROSTER_BASE },
    { 0x4B, 4, "DRL", PORT_REMIX_ROSTER_BASE },
    { 0x4C, 0, "LANKY", PORT_REMIX_ROSTER_BASE },
    { 0x4D, 10, "Birdo", PORT_REMIX_ROSTER_EXTRA },
    { 0x4E, 1, "CBKnuckles", PORT_REMIX_ROSTER_EXTRA },
    { 0x4F, 1, "CBMKnuckles", PORT_REMIX_ROSTER_EXTRA },
    { 0x50, 5, "Cloud", PORT_REMIX_ROSTER_EXTRA },
    { 0x51, 2, "DKUlt", PORT_REMIX_ROSTER_EXTRA },
    { 0x52, 7, "Kazuya", PORT_REMIX_ROSTER_EXTRA },
    { 0x53, 7, "Ken", PORT_REMIX_ROSTER_EXTRA },
    { 0x54, 1, "Knuckles", PORT_REMIX_ROSTER_EXTRA },
    { 0x55, 1, "MKnuckles", PORT_REMIX_ROSTER_EXTRA },
    { 0x56, 0, "MRGAW", PORT_REMIX_ROSTER_EXTRA },
    { 0x57, 0, "MRGAWPLUS", PORT_REMIX_ROSTER_EXTRA },
    { 0x58, 0, "MRGAWTHREED", PORT_REMIX_ROSTER_EXTRA },
    { 0x59, 10, "MetaKnight", PORT_REMIX_ROSTER_EXTRA },
    { 0x5A, 1, "Rebecca", PORT_REMIX_ROSTER_EXTRA },
    { 0x5B, 7, "Ryu", PORT_REMIX_ROSTER_EXTRA },
    { 0x5C, 7, "Snake", PORT_REMIX_ROSTER_EXTRA },
    { 0x5D, 7, "Spiderman", PORT_REMIX_ROSTER_EXTRA },
    { 0x5E, 7, "Terry", PORT_REMIX_ROSTER_EXTRA },
    { 0x5F, 1, "YZelda", PORT_REMIX_ROSTER_EXTRA },
    { 0x60, 0, "NWARIO", PORT_REMIX_ROSTER_POLYGON },
    { 0x61, 11, "NLUCAS", PORT_REMIX_ROSTER_POLYGON },
    { 0x62, 6, "NBOWSER", PORT_REMIX_ROSTER_POLYGON },
    { 0x63, 1, "NWOLF", PORT_REMIX_ROSTER_POLYGON },
    { 0x64, 0, "NDRM", PORT_REMIX_ROSTER_POLYGON },
    { 0x65, 1, "NSONIC", PORT_REMIX_ROSTER_POLYGON },
    { 0x66, 7, "NSHEIK", PORT_REMIX_ROSTER_POLYGON },
    { 0x67, 7, "NMARINA", PORT_REMIX_ROSTER_POLYGON },
    { 0x68, 1, "NFALCO", PORT_REMIX_ROSTER_POLYGON },
    { 0x69, 7, "NGND", PORT_REMIX_ROSTER_POLYGON },
    { 0x6A, 3, "NDSAMUS", PORT_REMIX_ROSTER_POLYGON },
    { 0x6B, 7, "NMARTH", PORT_REMIX_ROSTER_POLYGON },
    { 0x6C, 6, "NMTWO", PORT_REMIX_ROSTER_POLYGON },
    { 0x6D, 7, "NDEDEDE", PORT_REMIX_ROSTER_POLYGON },
    { 0x6E, 5, "NYLINK", PORT_REMIX_ROSTER_POLYGON },
    { 0x6F, 0, "NGOEMON", PORT_REMIX_ROSTER_POLYGON },
    { 0x70, 1, "NCONKER", PORT_REMIX_ROSTER_POLYGON },
    { 0x71, 7, "NBANJO", PORT_REMIX_ROSTER_POLYGON },
    { 0x72, 1, "NPEACH", PORT_REMIX_ROSTER_POLYGON },
    { 0x73, 0, "NCRASH", PORT_REMIX_ROSTER_POLYGON },
};

constexpr int kRosterCount = static_cast<int>(sizeof(kRoster) / sizeof(kRoster[0]));

} // namespace

extern "C" {

int port_remix_roster_count(void)
{
    return kRosterCount;
}

const PortRemixRosterInfo* port_remix_roster_at(int index)
{
    return (index >= 0 && index < kRosterCount) ? &kRoster[index] : nullptr;
}

const PortRemixRosterInfo* port_remix_roster_info(int fkind)
{
    if (fkind < 0x1D || fkind > 0x73) {
        return nullptr;
    }
    const int index = fkind - 0x1D;
    if (index < 0 || index >= kRosterCount) {
        return nullptr;
    }
    return (kRoster[index].fkind == fkind) ? &kRoster[index] : nullptr;
}

int port_remix_roster_is_generated(int fkind)
{
    return port_remix_roster_info(fkind) != nullptr ? 1 : 0;
}

int port_remix_roster_group_count(int group)
{
    int count = 0;
    for (const PortRemixRosterInfo& row : kRoster) {
        if (static_cast<int>(row.group) == group) {
            ++count;
        }
    }
    return count;
}

} // extern "C"
