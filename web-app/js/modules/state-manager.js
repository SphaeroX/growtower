import { bleManager } from './ble-manager.js';
import { logger } from './logger.js';

/**
 * State Manager
 * Manages application state and syncs with device
 */
class StateManager {
    constructor() {
        this.state = {
            connected: false,
            deviceName: null,
            light: {
                on: false
            },
            fan: {
                speed: 50,
                min: 0,
                max: 100
            },
            schedule: {
                lightOnHour: 18,
                lightOffHour: 14
            }
        };
        
        this.listeners = [];
    }

    /**
     * Subscribe to state changes
     */
    subscribe(callback) {
        this.listeners.push(callback);
        return () => {
            this.listeners = this.listeners.filter(cb => cb !== callback);
        };
    }

    /**
     * Notify all listeners of state change
     */
    notify() {
        this.listeners.forEach(cb => cb(this.state));
    }

    /**
     * Update connection state
     */
    setConnected(connected, deviceName = null) {
        this.state.connected = connected;
        this.state.deviceName = deviceName;
        this.notify();
    }

    /**
     * Read all values from device
     */
    async readAllValues() {
        if (!bleManager.isConnected) {
            throw new Error('Nicht verbunden');
        }

        try {
            logger.info('Lese alle Werte vom Gerät...');

            // Read light state
            const lightValue = await bleManager.readValue('light');
            this.state.light.on = lightValue === 1;

            // Read fan speed
            const fanValue = await bleManager.readValue('fan');
            this.state.fan.speed = fanValue;

            // Read fan min
            const fanMinValue = await bleManager.readValue('fanMin');
            this.state.fan.min = fanMinValue;

            // Read fan max
            const fanMaxValue = await bleManager.readValue('fanMax');
            this.state.fan.max = fanMaxValue;

            // Read light on hour
            const lightOnValue = await bleManager.readValue('lightOn');
            this.state.schedule.lightOnHour = lightOnValue;

            // Read light off hour
            const lightOffValue = await bleManager.readValue('lightOff');
            this.state.schedule.lightOffHour = lightOffValue;

            logger.success('Alle Werte erfolgreich gelesen');
            this.notify();
            
            return true;
        } catch (error) {
            logger.error(`Fehler beim Lesen: ${error.message}`);
            throw error;
        }
    }

    /**
     * Set light state
     */
    async setLight(on) {
        try {
            await bleManager.writeValue('light', on ? 1 : 0);
            this.state.light.on = on;
            logger.success(`Licht ${on ? 'EIN' : 'AUS'}`);
            this.notify();
            return true;
        } catch (error) {
            logger.error(`Fehler beim Licht schalten: ${error.message}`);
            throw error;
        }
    }

    /**
     * Set fan speed
     */
    async setFanSpeed(speed) {
        try {
            speed = Math.max(0, Math.min(100, speed));
            await bleManager.writeValue('fan', speed);
            this.state.fan.speed = speed;
            logger.success(`Lüfter auf ${speed}% gesetzt`);
            this.notify();
            return true;
        } catch (error) {
            logger.error(`Fehler beim Lüfter setzen: ${error.message}`);
            throw error;
        }
    }

    /**
     * Set fan min/max range
     */
    async setFanRange(min, max) {
        try {
            min = Math.max(0, Math.min(100, min));
            max = Math.max(0, Math.min(100, max));
            
            await bleManager.writeValue('fanMin', min);
            await bleManager.writeValue('fanMax', max);
            
            this.state.fan.min = min;
            this.state.fan.max = max;
            
            logger.success(`Lüfter Bereich: ${min}% - ${max}%`);
            this.notify();
            return true;
        } catch (error) {
            logger.error(`Fehler beim Bereich setzen: ${error.message}`);
            throw error;
        }
    }

    /**
     * Set light schedule
     */
    async setSchedule(onHour, offHour) {
        try {
            onHour = Math.max(0, Math.min(23, onHour));
            offHour = Math.max(0, Math.min(23, offHour));
            
            await bleManager.writeValue('lightOn', onHour);
            await bleManager.writeValue('lightOff', offHour);
            
            this.state.schedule.lightOnHour = onHour;
            this.state.schedule.lightOffHour = offHour;
            
            logger.success(`Zeitplan: AN um ${onHour}:00, AUS um ${offHour}:00`);
            this.notify();
            return true;
        } catch (error) {
            logger.error(`Fehler beim Zeitplan setzen: ${error.message}`);
            throw error;
        }
    }

    /**
     * Get current state
     */
    getState() {
        return { ...this.state };
    }
}

// Export singleton instance
export const stateManager = new StateManager();