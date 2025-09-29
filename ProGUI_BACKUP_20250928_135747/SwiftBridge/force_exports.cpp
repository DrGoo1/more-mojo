#include "ProGUIBridgeC.h"

extern "C" {
// Force-reference exported C bridge symbols so the linker keeps them in the dylib
__attribute__((used)) static const void* progui_force_exports[] = {
    (const void*) &ProGUI_CreateComponent,
    (const void*) &ProGUI_DestroyComponent,
    (const void*) &ProGUI_ResizeComponent,
    (const void*) &ProGUI_SetParameterValue,
    (const void*) &ProGUI_GetParameterValue,
    (const void*) &ProGUI_ProcessAudio,
    (const void*) &ProGUI_GetNativeView,
    (const void*) &ProGUI_SetOverlayVisible,
    (const void*) &ProGUI_SetTestSignalEnabled,
};
}
