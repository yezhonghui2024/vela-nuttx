/****************************************************************************
 * include/nuttx/power/battery_charger.h
 * NuttX Battery Charger Interfaces
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __INCLUDE_NUTTX_POWER_BATTERY_CHARGER_H
#define __INCLUDE_NUTTX_POWER_BATTERY_CHARGER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/semaphore.h>

#include <stdbool.h>

#ifdef CONFIG_BATTERY_CHARGER

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* CONFIG_BATTERY_CHARGER - Upper half battery charger driver support
 *
 * Specific, lower-half drivers will have other configuration requirements
 * such as:
 *
 *   CONFIG_I2C - I2C support *may* be needed
 *   CONFIG_I2C_BQ2425X - The BQ2425x driver must be explicitly selected.
 *   CONFIG_I2C_BQ2429X - The BQ2429x driver must be explicitly selected.
 */

/* IOCTL Commands ***********************************************************/

/* The upper-half battery charger driver provides a character driver
 * "wrapper" around the lower-half battery charger driver that does all of
 * the real work.
 * Since there is no real data transfer to/or from a battery, all of the
 * driver interaction is through IOCTL commands.  The IOCTL commands
 * supported by the upper-half driver simply provide calls into the
 * lower half as summarized below:
 *
 * BATIOC_STATE - Return the current state of the battery (see
 *   enum battery_charger_status_e).
 *   Input value:  A pointer to type int.
 * BATIOC_HEALTH - Return the current health of the battery (see
 *   enum battery_charger_health_e).
 *   Input value:  A pointer to type int.
 * BATIOC_ONLINE - Return 1 if the battery is online; 0 if offline.
 *   Input value:  A pointer to type bool.
 * BATIOC_VOLTAGE - Define the wished charger voltage to charge the battery.
 *   Input value:  An int defining the voltage value.
 * BATIOC_CURRENT - Define the wished charger current to charge the battery.
 *   Input value:  An int defining the current value.
 * BATIOC_INPUT_CURRENT - Define the input current limit of power supply.
 *   Input value:  An int defining the input current limit value.
 * BATIOC_OPERATE - Perform miscellaneous, device-specific charger operation.
 *   Input value:  An uintptr_t that can hold a pointer to struct
 *                 batio_operate_msg_s.
 */

/* Special input values for BATIOC_INPUT_CURRENT that may optionally
 * be supported by lower-half driver:
 */

#define BATTERY_INPUT_CURRENT_EXT_LIM   (-1) /* External input current limit */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Battery status */

enum battery_charger_status_e
{
  BATTERY_UNKNOWN = 0, /* Battery state is not known */
  BATTERY_FAULT,       /* Charger reported a fault, get health for more info */
  BATTERY_IDLE,        /* Not full, not charging, not discharging */
  BATTERY_FULL,        /* Full, not discharging */
  BATTERY_CHARGING,    /* Not full, charging */
  BATTERY_DISCHARGING  /* Probably not full, discharging */
};

/* Battery Health status */

enum battery_charger_health_e
{
  BATTERY_HEALTH_UNKNOWN = 0,  /* Battery health state is not known */
  BATTERY_HEALTH_GOOD,         /* Battery is in good condiction */
  BATTERY_HEALTH_DEAD,         /* Battery is dead, nothing we can do */
  BATTERY_HEALTH_OVERHEAT,     /* Battery is over recommended temperature */
  BATTERY_HEALTH_OVERVOLTAGE,  /* Battery voltage is over recommended level */
  BATTERY_HEALTH_UNSPEC_FAIL,  /* Battery charger reported an unspected failure */
  BATTERY_HEALTH_COLD,         /* Battery is under recommended temperature */
  BATTERY_HEALTH_WD_TMR_EXP,   /* Battery WatchDog Timer Expired */
  BATTERY_HEALTH_SAFE_TMR_EXP, /* Battery Safety Timer Expired */
  BATTERY_HEALTH_DISCONNECTED  /* Battery is not connected */
};

  /* This structure defines the lower half battery interface */

struct battery_charger_dev_s;
struct battery_charger_operations_s
{
  /* Return the current battery state (see enum battery_charger_status_e) */

  int (*state)(struct battery_charger_dev_s *dev, int *status);

  /* Return the current battery health (see enum battery_charger_health_e) */

  int (*health)(struct battery_charger_dev_s *dev, int *health);

  /* Return true if the battery is online */

  int (*online)(struct battery_charger_dev_s *dev, bool *status);

  /* Set the wished battery voltage for charging */

  int (*voltage)(struct battery_charger_dev_s *dev, int value);

  /* Set the wished current rate used for charging */

  int (*current)(struct battery_charger_dev_s *dev, int value);

  /* Set the input current limit of power supply */

  int (*input_current)(struct battery_charger_dev_s *dev, int value);

  /* Do device specific operation */

  int (*operate)(struct battery_charger_dev_s *dev, uintptr_t param);
};

/* This structure defines the battery driver state structure */

struct battery_charger_dev_s
{
  /* Fields required by the upper-half driver */

  FAR const struct battery_charger_operations_s *ops; /* Battery operations */

  sem_t batsem;  /* Enforce mutually exclusive access */

  /* Data fields specific to the lower-half driver may follow */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: battery_charger_register
 *
 * Description:
 *   Register a lower half battery driver with the common, upper-half
 *   battery driver.
 *
 * Input Parameters:
 *   devpath - The location in the pseudo-filesystem to create the driver.
 *     Recommended standard is "/dev/bat0", "/dev/bat1", etc.
 *   dev - An instance of the battery state structure .
 *
 * Returned Value:
 *    Zero on success or a negated errno value on failure.
 *
 ****************************************************************************/

int battery_charger_register(FAR const char *devpath,
                             FAR struct battery_charger_dev_s *dev);

/****************************************************************************
 * Name: bq2425x_initialize
 *
 * Description:
 *   Initialize the BQ2425X battery driver and return an instance of the
 *   lower-half interface that may be used with battery_charger_register().
 *
 *   This driver requires:
 *
 *   CONFIG_BATTERY_CHARGER - Upper half battery charger driver support
 *   CONFIG_I2C - I2C support
 *   CONFIG_I2C_BQ2425X - And the driver must be explicitly selected.
 *
 * Input Parameters:
 *   i2c       - An instance of the I2C interface to use to communicate with
 *               the BQ2425X
 *   addr      - The I2C address of the BQ2425X (Better be 0x6A).
 *   frequency - The I2C frequency
 *   current   - The input current our power-supply can offer to charger
 *
 * Returned Value:
 *   A pointer to the initialized battery driver instance.  A NULL pointer
 *   is returned on a failure to initialize the BQ2425X lower half.
 *
 ****************************************************************************/

#if defined(CONFIG_I2C) && defined(CONFIG_I2C_BQ2425X)

struct i2c_master_s;
FAR struct battery_charger_dev_s *bq2425x_initialize(
                                    FAR struct i2c_master_s *i2c,
                                     uint8_t addr,
                                     uint32_t frequency,
                                     int current);
#endif

/****************************************************************************
 * Name: bq2429x_initialize
 *
 * Description:
 *   Initialize the BQ2429X (BQ24series LiIon Charger with USB OTG boost 5V)
 *   battery driver and return an instance of the lower-half interface that
 *   may be used with battery_charger_register().
 *
 * This is for:
 *   BQ24296M VQFN24
 *   BQ24296 VQFN24
 *   BQ24297
 *   BQ24298
 * Possibly similar:
 *   BQ24262
 *   BQ24259
 *   BQ24292I BQ24295 B
 * Possibly the following:
 *   BQ24260/1/2   Vin-14V
 *   BQ24190       Vin=17V
 *
 *   This driver requires:
 *
 *   CONFIG_BATTERY_CHARGER - Upper half battery charger driver support
 *   CONFIG_I2C - I2C support
 *   CONFIG_I2C_BQ2429X - And the driver must be explicitly selected.
 *
 * Input Parameters:
 *   i2c       - An instance of the I2C interface to use to communicate with
 *               the BQ2429X
 *   addr      - The I2C address of the BQ2429X (Better be 0x6B).
 *   frequency - The I2C frequency
 *   current   - The input current our power-supply can offer to charger
 *
 * Returned Value:
 *   A pointer to the initialized battery driver instance.  A NULL pointer
 *   is returned on a failure to initialize the BQ2429X lower half.
 *
 ****************************************************************************/

#if defined(CONFIG_I2C) && defined(CONFIG_I2C_BQ2429X)

struct i2c_master_s;
FAR struct battery_charger_dev_s *bq2429x_initialize(
                                     FAR struct i2c_master_s *i2c,
                                     uint8_t addr,
                                     uint32_t frequency,
                                     int current);
#endif

/****************************************************************************
 * Name: bq25618_initialize
 *
 * Description:
 *   Initialize the BQ25618 battery driver and return an instance of the
 *   lower-half interface that may be used with battery_charger_register().
 *
 *   This driver requires:
 *
 *   CONFIG_BATTERY_CHARGER - Upper half battery charger driver support
 *   CONFIG_I2C - I2C support
 *   CONFIG_I2C_BQ25618 - And the driver must be explicitly selected.
 *
 * Input Parameters:
 *   i2c       - An instance of the I2C interface to use to communicate with
 *               the BQ25618
 *   addr      - The I2C address of the BQ25618 (Better be 0x6A).
 *   frequency - The I2C frequency
 *   current   - The input current our power-supply can offer to charger
 *
 * Returned Value:
 *   A pointer to the initialized battery driver instance.  A NULL pointer
 *   is returned on a failure to initialize the BQ25618 lower half.
 *
 ****************************************************************************/

#if defined(CONFIG_I2C) && defined(CONFIG_I2C_BQ25618)

struct i2c_master_s;
FAR struct battery_charger_dev_s *bq25618_initialize(
                                    FAR struct i2c_master_s *i2c,
                                     uint8_t addr,
                                     uint32_t frequency,
                                     int current);
#endif

/****************************************************************************
 * Name: sc8551_initialize
 *
 * Description:
 *   Initialize the SC8551 (pump charger) charger driver and return
 *   an instance of the lower-half interface that may be used with
 *   battery_charger_register().
 *
 * This is for:
 *   SC8551
 *
 *   This driver requires:
 *
 *   CONFIG_BATTERY_CHARGER - Upper half battery charger driver support
 *   CONFIG_I2C - I2C support
 *   CONFIG_I2C_SC8551 - And the driver must be explicitly selected.
 *
 * Input Parameters:
 *   i2c       - An instance of the I2C interface to use to communicate with
 *               the SC8551
 *   addr      - The I2C address of the SC8551 (Better be 0x66).
 *   frequency - The I2C frequency
 *   current   - The input current our power-supply can offer to charger
 *
 * Returned Value:
 *   A pointer to the initialized battery driver instance.  A NULL pointer
 *   is returned on a failure to initialize the SC8551 lower half.
 *
 ****************************************************************************/

#if defined(CONFIG_I2C) && defined(CONFIG_I2C_SC8551)

struct i2c_master_s;
FAR struct battery_charger_dev_s *
  sc8551_initialize(FAR struct i2c_master_s *i2c, uint8_t addr,
                    uint32_t frequency, int current);
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* CONFIG_BATTERY_CHARGER */
#endif /* __INCLUDE_NUTTX_POWER_BATTERY_H */
