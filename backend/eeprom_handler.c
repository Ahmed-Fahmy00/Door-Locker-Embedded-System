#include "eeprom_handler.h"

// TivaWare includes
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "components/motor.h"

// Initialize password in EEPROM
int initialize_password(uint32_t new_password)
{
    uint32_t data[1];
    data[0] = new_password;
    
    // Program the new password into EEPROM
    if (EEPROMProgram(data, PASSWORD_OFFSET, sizeof(data)) != 0)
    {
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

// Authenticate the candidate password with the stored password
// NOTE: Does NOT open door - caller must handle that after sending UART response
int authenticate(uint32_t candidate_password, uint32_t mode)
{
    uint32_t stored_password[1];
    (void)mode;  /* Mode handled by caller (uart_handler) */
    
    // Read the stored password from EEPROM
    EEPROMRead(stored_password, PASSWORD_OFFSET, sizeof(stored_password));
    
    // Compare candidate password with the stored password
    if (stored_password[0] == candidate_password)
    {
        return STATUS_OK;  // Password matches
    }
    return STATUS_AUTH_FAIL;  // Password does not match
}

// Change password in EEPROM
int change_password(uint32_t new_password)
{  
    // Set the new password
    return initialize_password(new_password);
}

// Get auto timeout value from EEPROM
int get_auto_timeout(uint32_t* timeout)
{
    uint32_t data[1];
    
    // Read the timeout value from EEPROM
    EEPROMRead(data, TIMEOUT_OFFSET, sizeof(data));
    
    *timeout = data[0];  // Return the timeout value
    return STATUS_OK;
}

// Change auto timeout value in EEPROM
int change_auto_timeout(uint32_t new_timeout)
{
    uint32_t data[1];
    data[0] = new_timeout;
    
    // Program the new timeout value into EEPROM
    if (EEPROMProgram(data, TIMEOUT_OFFSET, sizeof(data)) != 0)
    {
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

// Set default auto timeout if no value is present
int set_default_auto_timeout()
{
    // Check if the timeout is already set
    // EEPROM erased state is 0xFFFFFFFF, so check for that AND 0
    uint32_t timeout;
    int result = get_auto_timeout(&timeout);
    
    // If reading was OK AND the value is valid (not erased/uninitialized), keep it
    if (result == STATUS_OK && timeout != 0 && timeout != 0xFFFFFFFF) 
    {
        return STATUS_OK;
    }
    
    // Set default timeout
    return change_auto_timeout(DEFAULT_TIMEOUT);
}

// Get potentiometer value from EEPROM
int get_potentiometer_value(uint32_t* value)
{
    uint32_t data[1];
    
    // Read the potentiometer value from EEPROM
    EEPROMRead(data, POTENTIOMETER_OFFSET, sizeof(data));
    
    *value = data[0];  // Return the potentiometer value
    return STATUS_OK;
}

// Set potentiometer value in EEPROM
int set_potentiometer_value(uint32_t value)
{
    uint32_t data[1];
    data[0] = value;
    
    // Program the new potentiometer value into EEPROM
    if (EEPROMProgram(data, POTENTIOMETER_OFFSET, sizeof(data)) != 0)
    {
        return STATUS_ERROR;
    }
    return STATUS_OK;
}