#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// void redrawPattern(void);
int32_t periodToNote(int32_t period); // 0 = no note, 1 = illegal note, 2..37 = note

#ifdef __cplusplus
}
#endif
