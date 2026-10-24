#pragma once

#include "CoreMinimal.h"

// Project log category. Per-hit / per-frame / per-spawn traces use Verbose so they are hidden by default
// (enable with "log LogTowerDefense Verbose"); real configuration problems use Warning or Error.
DECLARE_LOG_CATEGORY_EXTERN(LogTowerDefense, Log, All);
