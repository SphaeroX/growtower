import { bleManager } from './ble-manager.js';
import { stateManager } from './state-manager.js';
import { uiController } from './ui-controller.js';
import { logger } from './logger.js';
import { DEVICE_FILTERS } from './config.js';

/**
 * Main Application Controller
 * Orchestrates all components
 */
class App {
    constructor() {
        this.initialized = false;
    }

    /**
     * Initialize application
     */
    async init() {
        if (this.initialized) return;

        // Check Web Bluetooth support
        if (!bleManager.isSupported()) {
            logger.error('Web Bluetooth wird nicht unterstützt!');
            logger.error('Bitte verwenden Sie Chrome, Edge oder einen anderen Chromium-basierten Browser.');
            this.disableConnectButtons();
            return;
        }

        logger.info('GrowTower Controller v2.0 wird initialisiert...');
        logger.info('Web Bluetooth wird unterstützt ✓');

        // Initialize UI
        uiController.init();

        // Setup BLE callbacks
        bleManager.onConnect(() => {
            logger.success('Verbindung hergestellt!');
            this.onConnect();
        });

        bleManager.onDisconnect(() => {
            logger.error('Verbindung getrennt');
            stateManager.setConnected(false);
        });

        // Bind button events
        this.bindEvents();

        this.initialized = true;
        logger.success('Initialisierung abgeschlossen. Bereit für Verbindung.');
    }

    /**
     * Bind all event listeners
     */
    bindEvents() {
        // Connect buttons
        document.getElementById('connectBtn')?.addEventListener('click', () => {
            this.connect(false);
        });

        document.getElementById('scanAllBtn')?.addEventListener('click', () => {
            this.connect(true);
        });

        document.getElementById('disconnectBtn')?.addEventListener('click', () => {
            this.disconnect();
        });

        document.getElementById('refreshBtn')?.addEventListener('click', () => {
            this.refreshValues();
        });

        // Light switch
        document.getElementById('lightSwitch')?.addEventListener('change', async (e) => {
            try {
                await stateManager.setLight(e.target.checked);
            } catch (error) {
                logger.error(`Fehler: ${error.message}`);
                e.target.checked = !e.target.checked; // Revert
            }
        });

        // Fan set button
        document.getElementById('setFanBtn')?.addEventListener('click', async () => {
            const values = uiController.getInputValues();
            try {
                await stateManager.setFanSpeed(values.fanSpeed);
            } catch (error) {
                logger.error(`Fehler: ${error.message}`);
            }
        });

        // Save fan range
        document.getElementById('saveFanRangeBtn')?.addEventListener('click', async () => {
            const values = uiController.getInputValues();
            
            if (values.fanMin > values.fanMax) {
                logger.error('Fehler: Minimum darf nicht größer als Maximum sein!');
                return;
            }
            
            try {
                uiController.showLoading(document.getElementById('saveFanRangeBtn'), 'Speichern...');
                await stateManager.setFanRange(values.fanMin, values.fanMax);
                uiController.hideLoading(document.getElementById('saveFanRangeBtn'));
            } catch (error) {
                logger.error(`Fehler: ${error.message}`);
                uiController.hideLoading(document.getElementById('saveFanRangeBtn'));
            }
        });

        // Save schedule
        document.getElementById('saveScheduleBtn')?.addEventListener('click', async () => {
            const values = uiController.getInputValues();
            
            try {
                uiController.showLoading(document.getElementById('saveScheduleBtn'), 'Speichern...');
                await stateManager.setSchedule(values.lightOnHour, values.lightOffHour);
                uiController.hideLoading(document.getElementById('saveScheduleBtn'));
            } catch (error) {
                logger.error(`Fehler: ${error.message}`);
                uiController.hideLoading(document.getElementById('saveScheduleBtn'));
            }
        });
    }

    /**
     * Connect to device
     */
    async connect(scanAll = false) {
        try {
            const btn = scanAll ? document.getElementById('scanAllBtn') : document.getElementById('connectBtn');
            uiController.showLoading(btn, 'Suche...');

            // Request device
            if (scanAll) {
                logger.info('Suche nach allen BLE Geräten...');
                await bleManager.requestDevice([]);
            } else {
                logger.info('Suche nach GrowTower...');
                await bleManager.requestDevice([
                    DEVICE_FILTERS.name,
                    DEVICE_FILTERS.namePrefix,
                    DEVICE_FILTERS.service
                ]);
            }

            uiController.showLoading(btn, 'Verbinde...');

            // Connect
            await bleManager.connect();

            uiController.hideLoading(btn);

        } catch (error) {
            uiController.hideLoading(btn);
            
            if (error.message.includes('cancelled') || error.message.includes('User')) {
                logger.info('Verbindung vom Benutzer abgebrochen');
            } else {
                logger.error(`Verbindungsfehler: ${error.message}`);
                
                if (error.message.includes('getrännt') || error.message.includes('disconnected')) {
                    logger.info('Tipp: Falls das Gerät bereits gekoppelt war, entfernen Sie es in den Windows Bluetooth Einstellungen und versuchen Sie es erneut.');
                }
            }
        }
    }

    /**
     * Handle successful connection
     */
    async onConnect() {
        const status = bleManager.getStatus();
        stateManager.setConnected(true, status.deviceName);

        try {
            // Read all values from device
            await stateManager.readAllValues();
        } catch (error) {
            logger.error(`Fehler beim Lesen der Werte: ${error.message}`);
        }
    }

    /**
     * Disconnect from device
     */
    async disconnect() {
        try {
            logger.info('Trenne Verbindung...');
            await bleManager.disconnect();
            stateManager.setConnected(false);
        } catch (error) {
            logger.error(`Fehler beim Trennen: ${error.message}`);
        }
    }

    /**
     * Refresh all values
     */
    async refreshValues() {
        try {
            const btn = document.getElementById('refreshBtn');
            uiController.showLoading(btn, 'Aktualisiere...');
            
            await stateManager.readAllValues();
            
            uiController.hideLoading(btn);
        } catch (error) {
            logger.error(`Fehler beim Aktualisieren: ${error.message}`);
            uiController.hideLoading(document.getElementById('refreshBtn'));
        }
    }

    /**
     * Disable connect buttons if BLE not supported
     */
    disableConnectButtons() {
        document.getElementById('connectBtn')?.classList.add('hidden');
        document.getElementById('scanAllBtn')?.classList.add('hidden');
    }
}

// Create and export app instance
const app = new App();

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    app.init();
});

export default app;