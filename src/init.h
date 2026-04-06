#pragma once

typedef struct HardwareInfo {
    int m_something;
    char pad[0xc];
} HardwareInfo;

extern HardwareInfo XboxHardwareInfo;
