/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * Adapted for BEETLE BLUE by SikkenWorks, 2026.
 * UC25HQ80 compatibility reported by Sven, The Firmware Hub.
 */
#pragma once
/* Both parts: 8 Mbit/1 MiB; supported standard 0x9F JEDEC identification.
 * Deny unknown IDs for WRITE only, retaining read-only backup paths. */
#define FL_JEDEC_PUYA_P25Q80    0x856014u
#define FL_JEDEC_UCUN_UC25HQ80  0xB36014u
#define FL_JEDEC_OK(id) (((id) == FL_JEDEC_PUYA_P25Q80) || ((id) == FL_JEDEC_UCUN_UC25HQ80))
