#pragma once

#include <Lib/IO.H>

void PICInit();
void PICEOI(u8 irq);
void PICClearMask(u8 irq);
