#include "sl_component_catalog.h"
#ifdef SL_CATALOG_KERNEL_PRESENT
#include "app_rta_internal.h"
#else // SL_CATALOG_KERNEL_PRESENT
#include "app_rta_internal_bm.h"
#endif // SL_CATALOG_KERNEL_PRESENT
#include "sli_bt_app_ota_dfu.h"

void app_rta_init_contributors(void)
{
  sli_bt_app_ota_dfu_init();
}

void app_rta_ready(void)
{
  sli_bt_app_ota_dfu_rta_ready();
}
