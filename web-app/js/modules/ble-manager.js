import { BLE_UUIDS, CONFIG } from './config.js';
import { logger } from './logger.js';

/**
 * BLE Connection Manager
 * Handles all Bluetooth Low Energy operations
 */
class BLEManager {
    constructor() {
        this.device = null;
        this.server = null;
        this.service = null;
        this.characteristics = {};
        this.isConnected = false;
        this.connectionCallbacks = [];
        this.disconnectionCallbacks = [];
    }

    /**
     * Check if Web Bluetooth is supported
     */
    isSupported() {
        return 'bluetooth' in navigator;
    }

    /**
     * Register connection callback
     */
    onConnect(callback) {
        this.connectionCallbacks.push(callback);
    }

    /**
     * Register disconnection callback
     */
    onDisconnect(callback) {
        this.disconnectionCallbacks.push(callback);
    }

    /**
     * Request device with filters
     */
    async requestDevice(filters = []) {
        if (!this.isSupported()) {
            throw new Error('Web Bluetooth wird nicht unterstützt. Bitte Chrome oder Edge verwenden.');
        }

        const options = {
            filters: filters.length > 0 ? filters : undefined,
            acceptAllDevices: filters.length === 0,
            optionalServices: [BLE_UUIDS.SERVICE, BLE_UUIDS.SERVICE_ALT]
        };

        logger.info('Suche nach BLE Gerät...');
        this.device = await navigator.bluetooth.requestDevice(options);

        logger.success(`Gerät gefunden: ${this.device.name || 'Unnamed'} (${this.device.id})`);

        // Setup disconnect handler
        this.device.addEventListener('gattserverdisconnected', (e) => {
            this.handleDisconnect(e);
        });

        return this.device;
    }

    /**
     * Connect to the selected device
     */
    async connect() {
        if (!this.device) {
            throw new Error('Kein Gerät ausgewählt');
        }

        try {
            logger.info('Verbinde mit GATT Server...');

            // Connect with timeout
            const connectPromise = this.device.gatt.connect();
            const timeoutPromise = new Promise((_, reject) => {
                setTimeout(() => reject(new Error('Verbindungs-Timeout')), CONFIG.CONNECTION_TIMEOUT);
            });

            this.server = await Promise.race([connectPromise, timeoutPromise]);

            logger.success('GATT Verbindung hergestellt');

            // Wait for connection to stabilize
            logger.info('Warte auf stabile Verbindung...');
            await this.delay(CONFIG.SERVICE_ACCESS_DELAY);

            if (!this.server.connected) {
                throw new Error('Verbindung wurde unerwartet getrennt');
            }

            // Get primary service with retries
            this.service = await this.getServiceWithRetry();

            // Get all characteristics
            await this.getAllCharacteristics();

            this.isConnected = true;
            logger.success('BLE Verbindung vollständig hergestellt!');

            // Notify callbacks
            this.connectionCallbacks.forEach(cb => cb());

            return true;

        } catch (error) {
            logger.error(`Verbindungsfehler: ${error.message}`);
            this.cleanup();
            throw error;
        }
    }

    /**
     * Get service with retry logic
     */
    async getServiceWithRetry() {
        let lastError;

        for (let attempt = 1; attempt <= CONFIG.SERVICE_RETRIES; attempt++) {
            try {
                logger.info(`Hole Service (Versuch ${attempt}/${CONFIG.SERVICE_RETRIES})...`);
                try {
                    // Try official service UUID first
                    logger.info(`Versuche Service ${BLE_UUIDS.SERVICE}...`);
                    const service = await this.server.getPrimaryService(BLE_UUIDS.SERVICE);
                    logger.success('Service gefunden (Standard UUID)');
                    return service;
                } catch (e1) {
                    logger.warning(`Standard UUID nicht gefunden, versuche Alternative...`);
                    // Try alternative UUID (raw bytes mismatch)
                    const service = await this.server.getPrimaryService(BLE_UUIDS.SERVICE_ALT);
                    logger.success('Service gefunden (Alternative UUID)');
                    return service;
                }
            } catch (error) {
                lastError = error;
                logger.warning(`Service-Abruf fehlgeschlagen: ${error.message}`);

                if (attempt < CONFIG.SERVICE_RETRIES) {
                    logger.info(`Warte ${CONFIG.RETRY_DELAY}ms vor erneutem Versuch...`);
                    await this.delay(CONFIG.RETRY_DELAY);
                }
            }
        }

        throw new Error(`Konnte Service nicht abrufen nach ${CONFIG.SERVICE_RETRIES} Versuchen: ${lastError.message}`);
    }

    /**
     * Get all characteristics from service
     */
    async getAllCharacteristics() {
        const chars = [
            { name: 'light', uuid: BLE_UUIDS.LIGHT },
            { name: 'fan', uuid: BLE_UUIDS.FAN },
            { name: 'fanMin', uuid: BLE_UUIDS.FAN_MIN },
            { name: 'fanMax', uuid: BLE_UUIDS.FAN_MAX },
            { name: 'lightOn', uuid: BLE_UUIDS.LIGHT_ON },
            { name: 'lightOff', uuid: BLE_UUIDS.LIGHT_OFF }
        ];

        logger.info('Lade Characteristics...');

        for (const char of chars) {
            try {
                this.characteristics[char.name] = await this.service.getCharacteristic(char.uuid);
                logger.info(`✓ ${char.name} characteristic geladen`);
            } catch (error) {
                logger.error(`✗ Konnte ${char.name} characteristic nicht laden: ${error.message}`);
                throw error;
            }
        }

        logger.success('Alle Characteristics geladen');
    }

    /**
     * Read value from characteristic
     */
    async readValue(charName) {
        const characteristic = this.characteristics[charName];
        if (!characteristic) {
            throw new Error(`Characteristic ${charName} nicht verfügbar`);
        }

        const value = await characteristic.readValue();
        return value.getUint8(0);
    }

    /**
     * Write value to characteristic
     */
    async writeValue(charName, value) {
        const characteristic = this.characteristics[charName];
        if (!characteristic) {
            throw new Error(`Characteristic ${charName} nicht verfügbar`);
        }

        const data = new Uint8Array([value]);
        await characteristic.writeValue(data);
    }

    /**
     * Handle disconnection
     */
    handleDisconnect(event) {
        logger.error('Verbindung getrennt');
        this.isConnected = false;
        this.cleanup();
        this.disconnectionCallbacks.forEach(cb => cb());
    }

    /**
     * Disconnect from device
     */
    async disconnect() {
        if (this.device && this.device.gatt.connected) {
            await this.device.gatt.disconnect();
        }
        this.cleanup();
    }

    /**
     * Cleanup resources
     */
    cleanup() {
        this.server = null;
        this.service = null;
        this.characteristics = {};
        this.isConnected = false;
    }

    /**
     * Delay helper
     */
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    /**
     * Get connection status
     */
    getStatus() {
        return {
            isConnected: this.isConnected,
            hasDevice: !!this.device,
            deviceName: this.device?.name || null
        };
    }
}

// Export singleton instance
export const bleManager = new BLEManager();