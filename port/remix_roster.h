#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PortRemixRosterGroup {
    PORT_REMIX_ROSTER_BASE = 0,
    PORT_REMIX_ROSTER_EXTRA = 1,
    PORT_REMIX_ROSTER_POLYGON = 2,
} PortRemixRosterGroup;

typedef struct PortRemixRosterInfo {
    int fkind;
    int parent_fkind;
    const char* source_name;
    PortRemixRosterGroup group;
} PortRemixRosterInfo;

/* Generated Character rows in the verified Remix 2.0.1 + EXTRA 0.5.0 build.
 * This excludes RANDOM (0x1B) and the 0x1C filler row. It includes the 48
 * Smash Remix rows, 19 EXTRA rows and 20 Remix polygon rows: 87 total. */
int port_remix_roster_count(void);
const PortRemixRosterInfo* port_remix_roster_at(int index);
const PortRemixRosterInfo* port_remix_roster_info(int fkind);
int port_remix_roster_is_generated(int fkind);
int port_remix_roster_group_count(int group);

#ifdef __cplusplus
}
#endif
