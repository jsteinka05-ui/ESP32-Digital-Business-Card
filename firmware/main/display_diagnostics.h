#ifndef DISPLAY_DIAGNOSTICS_H
#define DISPLAY_DIAGNOSTICS_H

#include "esp_err.h"

// Run once after bus and panel initialization, then leave the pattern visible.
// Checks bounds, fills, and repeated panel initialization. Visual checks are manual.
esp_err_t display_diagnostics_run(void);

#endif // DISPLAY_DIAGNOSTICS_H
