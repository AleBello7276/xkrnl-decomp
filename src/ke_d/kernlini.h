#pragma once

// hv stuff
void HvxGetVersions(void);
void HvxStartupProcessors(void);
void HvxSetTimeBaseToZero();
void HvxEnableTimebase();

void KiInitializeKernel(void);
void KiInitializeKernel2(void);
void KiInitializeNonBootProcessor(void);
