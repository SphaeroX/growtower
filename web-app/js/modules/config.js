/**
 * BLE Service and Characteristic UUIDs
 * Must match the firmware definitions exactly
 */
export const BLE_UUIDS = {
    // Service UUID
    SERVICE: '4fafc201-1fb5-459e-8fcc-c5c9c331914b',

    // Characteristic UUIDs
    LIGHT: 'beb5483e-36e1-4688-b7f5-ea07361b26a8',
    FAN: '82563452-9477-4b78-953e-38ec6f43e592',
    FAN_MIN: 'c0b9a304-1234-4567-89ab-cdef01234567',
    FAN_MAX: 'c0b9a304-1234-4567-89ab-cdef01234568',
    LIGHT_ON: 'c0b9a304-1234-4567-89ab-cdef01234569',
    LIGHT_OFF: 'c0b9a304-1234-4567-89ab-cdef0123456a',

    // Alternative Service UUID (matches raw firmware bytes if Little Endian conversion fails)
    // 0x4b, 0x91, 0x31, 0xc3 -> 4b9131c3
    SERVICE_ALT: '4b9131c3-c9c5-cc8f-9e45-b51f01c2af4f'
};

/**
 * Device filter options for Web Bluetooth
 */
export const DEVICE_FILTERS = {
    // Primary filter by device name
    name: { name: 'TOWER' },
    // Secondary filter by name prefix
    namePrefix: { namePrefix: 'TOWER' },
    // Service filter (most reliable)
    // Service filter (most reliable)
    service: { services: [BLE_UUIDS.SERVICE, BLE_UUIDS.SERVICE_ALT] }
};

/**
 * Connection configuration
 */
export const CONFIG = {
    // Connection timeout in milliseconds
    CONNECTION_TIMEOUT: 10000,
    // Delay before accessing services after connection (ms)
    SERVICE_ACCESS_DELAY: 1500,
    // Number of retries for service access
    SERVICE_RETRIES: 3,
    // Delay between retries (ms)
    RETRY_DELAY: 1000,
    // MTU size for better throughput
    PREFERRED_MTU: 512
};