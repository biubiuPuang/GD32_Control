#ifndef __RF_APPLY_H_
#define __RF_APPLY_H_

#include "stdio.h"
#include "rf_config.h"
#include "radio_tx.h"
#include "radio_rx.h"

uint8_t rf_config_apply(const rf_factory_config_t *cfg);

void rf_test_apply_config(void);


#endif /*__RF_APPLY_H_ */
