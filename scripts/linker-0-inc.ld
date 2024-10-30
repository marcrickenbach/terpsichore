/* This is a copy of linker.ld from ${ZEPHYR_BASE}/soc/arm/nxp_imx/rt/linker.ld,
 * which is the start of linker script processing. This has been changed from
 * the original to include a custom linker script rather than the original 
 * with the include changed. This is the top-level, aka start, linker script.
 */

#include <zephyr/devicetree.h>

#define IS_CHOSEN_SRAM(x) (DT_DEP_ORD(DT_NODELABEL(x)) == DT_DEP_ORD(DT_CHOSEN(zephyr_sram)))

MEMORY
     {
#if (DT_REG_SIZE(DT_NODELABEL(ocram)) > 0) && !IS_CHOSEN_SRAM(ocram)
        OCRAM  (wx) : ORIGIN = DT_REG_ADDR(DT_NODELABEL(ocram)), LENGTH = DT_REG_SIZE(DT_NODELABEL(ocram))
#endif
#if (DT_REG_SIZE(DT_NODELABEL(sdram0)) > 0) && !IS_CHOSEN_SRAM(sdram0)
        SDRAM  (wx) : ORIGIN = DT_REG_ADDR(DT_NODELABEL(sdram0)), LENGTH = DT_REG_SIZE(DT_NODELABEL(sdram0))
#endif
     }

#if 0 /* 
 * There's no way to copy the vectors to ITCM in with the default linker script 
 * so create a modified version of the original to do that. */
#include <zephyr/arch/arm/aarch32/cortex_m/scripts/linker.ld>
#else
#include "linker-1-inc.ld"
#endif