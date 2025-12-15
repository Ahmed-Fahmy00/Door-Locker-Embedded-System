#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// --- EEPROM Offsets (Aligned to 4 bytes) ---
#define PASSWORD_OFFSET 0x00    // Offset for the password in EEPROM
#define TIMEOUT_OFFSET 0x04     // Offset for the timeout in EEPROM
#define POTENTIOMETER_OFFSET 0x08 // Offset for the potentiometer value in EEPROM
#define DEFAULT_TIMEOUT 11      // Default timeout value (seconds)

// --- Status Codes for function results ---
#define STATUS_OK 0
#define STATUS_ERROR 1
#define STATUS_AUTH_FAIL 2

/**
 * @brief Initialize password in EEPROM.
 * * @param new_password The 32-bit password to store.
 * @return int STATUS_OK on success, STATUS_ERROR on EEPROM failure.
 */
int initialize_password(uint32_t new_password);

/**
 * @brief Authenticate the candidate password with the stored password.
 *
 * @param candidate_password The 32-bit password to check.
 * @param mode Operation mode: 0x0 = authenticate only, 0x1 = authenticate and open door.
 * @return int STATUS_OK if match, STATUS_AUTH_FAIL if no match.
 */
int authenticate(uint32_t candidate_password, uint32_t mode);

/**
 * @brief Change password in EEPROM after authenticating the old one.
 * * @param old_password The current 32-bit password.
 * @param new_password The new 32-bit password to store.
 * @return int STATUS_OK on success, STATUS_AUTH_FAIL on authentication failure, STATUS_ERROR on EEPROM failure.
 */
int change_password(uint32_t new_password);

/**
 * @brief Get auto timeout value from EEPROM.
 *
 * @param timeout Pointer to store the read 32-bit timeout value.
 * @return int STATUS_OK.
 */
int get_auto_timeout(uint32_t* timeout);

/**
 * @brief Change auto timeout value in EEPROM.
 *
 * @param new_timeout The new 32-bit timeout value (seconds) to store.
 * @return int STATUS_OK on success, STATUS_ERROR on EEPROM failure.
 */
int change_auto_timeout(uint32_t new_timeout);

/**
 * @brief Set default auto timeout if no value is present (value is 0).
 *
 * @return int STATUS_OK on success or if already set, STATUS_ERROR on EEPROM failure.
 */
int set_default_auto_timeout();

/**
 * @brief Get potentiometer value from EEPROM.
 *
 * @param value Pointer to store the read 32-bit potentiometer value.
 * @return int STATUS_OK on success.
 */
int get_potentiometer_value(uint32_t* value);

/**
 * @brief Set potentiometer value in EEPROM.
 *
 * @param value The 32-bit potentiometer value to store.
 * @return int STATUS_OK on success, STATUS_ERROR on EEPROM failure.
 */
int set_potentiometer_value(uint32_t value);

#endif // EEPROM_CONFIG_H