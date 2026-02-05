import { stateManager } from './state-manager.js';
import { logger } from './logger.js';

/**
 * UI Controller
 * Manages all UI interactions and updates
 */
class UIController {
    constructor() {
        this.elements = {};
        this.initialized = false;
    }

    /**
     * Initialize UI controller
     */
    init() {
        // Cache DOM elements
        this.elements = {
            // Connection
            connectBtn: document.getElementById('connectBtn'),
            disconnectBtn: document.getElementById('disconnectBtn'),
            scanAllBtn: document.getElementById('scanAllBtn'),
            connectionStatus: document.getElementById('connectionStatus'),
            deviceName: document.getElementById('deviceName'),
            
            // Cards
            controlsCard: document.getElementById('controlsCard'),
            configCard: document.getElementById('configCard'),
            valuesCard: document.getElementById('valuesCard'),
            
            // Light control
            lightSwitch: document.getElementById('lightSwitch'),
            lightStatus: document.getElementById('lightStatus'),
            
            // Fan control
            fanSlider: document.getElementById('fanSlider'),
            fanValue: document.getElementById('fanValue'),
            setFanBtn: document.getElementById('setFanBtn'),
            
            // Fan range
            fanMinSlider: document.getElementById('fanMinSlider'),
            fanMinValue: document.getElementById('fanMinValue'),
            fanMaxSlider: document.getElementById('fanMaxSlider'),
            fanMaxValue: document.getElementById('fanMaxValue'),
            saveFanRangeBtn: document.getElementById('saveFanRangeBtn'),
            
            // Schedule
            lightOnHour: document.getElementById('lightOnHour'),
            lightOffHour: document.getElementById('lightOffHour'),
            saveScheduleBtn: document.getElementById('saveScheduleBtn'),
            
            // Values display
            currentLight: document.getElementById('currentLight'),
            currentFan: document.getElementById('currentFan'),
            currentFanMin: document.getElementById('currentFanMin'),
            currentFanMax: document.getElementById('currentFanMax'),
            currentOnHour: document.getElementById('currentOnHour'),
            currentOffHour: document.getElementById('currentOffHour'),
            
            // Log
            clearLogBtn: document.getElementById('clearLogBtn'),
            refreshBtn: document.getElementById('refreshBtn')
        };

        // Bind event listeners
        this.bindEvents();
        
        // Subscribe to state changes
        stateManager.subscribe((state) => this.updateUI(state));
        
        this.initialized = true;
        logger.info('UI Controller initialisiert');
    }

    /**
     * Bind event listeners
     */
    bindEvents() {
        // Sliders - update display value
        this.elements.fanSlider?.addEventListener('input', (e) => {
            this.elements.fanValue.textContent = e.target.value + '%';
        });
        
        this.elements.fanMinSlider?.addEventListener('input', (e) => {
            this.elements.fanMinValue.textContent = e.target.value + '%';
        });
        
        this.elements.fanMaxSlider?.addEventListener('input', (e) => {
            this.elements.fanMaxValue.textContent = e.target.value + '%';
        });

        // Clear log
        this.elements.clearLogBtn?.addEventListener('click', () => {
            logger.clear();
            logger.info('Log geleert');
        });
    }

    /**
     * Update UI based on state
     */
    updateUI(state) {
        // Update connection status
        this.updateConnectionUI(state.connected, state.deviceName);
        
        // Update controls
        if (state.connected) {
            // Light
            this.elements.lightSwitch.checked = state.light.on;
            this.elements.lightStatus.textContent = state.light.on ? 'AN' : 'AUS';
            this.elements.currentLight.textContent = state.light.on ? 'AN' : 'AUS';
            
            // Fan
            this.elements.fanSlider.value = state.fan.speed;
            this.elements.fanValue.textContent = state.fan.speed + '%';
            this.elements.currentFan.textContent = state.fan.speed + '%';
            
            // Fan range
            this.elements.fanMinSlider.value = state.fan.min;
            this.elements.fanMinValue.textContent = state.fan.min + '%';
            this.elements.fanMaxSlider.value = state.fan.max;
            this.elements.fanMaxValue.textContent = state.fan.max + '%';
            this.elements.currentFanMin.textContent = state.fan.min + '%';
            this.elements.currentFanMax.textContent = state.fan.max + '%';
            
            // Schedule
            this.elements.lightOnHour.value = state.schedule.lightOnHour;
            this.elements.lightOffHour.value = state.schedule.lightOffHour;
            this.elements.currentOnHour.textContent = state.schedule.lightOnHour + ':00';
            this.elements.currentOffHour.textContent = state.schedule.lightOffHour + ':00';
        }
    }

    /**
     * Update connection UI
     */
    updateConnectionUI(connected, deviceName) {
        const status = this.elements.connectionStatus;
        const controls = this.elements.controlsCard;
        const config = this.elements.configCard;
        const values = this.elements.valuesCard;
        
        if (connected) {
            status.textContent = 'Verbunden';
            status.className = 'status-badge status-connected';
            this.elements.deviceName.textContent = deviceName || 'TOWER';
            
            controls?.classList.remove('hidden');
            config?.classList.remove('hidden');
            values?.classList.remove('hidden');
            
            this.elements.connectBtn?.classList.add('hidden');
            this.elements.scanAllBtn?.classList.add('hidden');
            this.elements.disconnectBtn?.classList.remove('hidden');
            this.elements.refreshBtn?.classList.remove('hidden');
        } else {
            status.textContent = 'Getrennt';
            status.className = 'status-badge status-disconnected';
            this.elements.deviceName.textContent = '-';
            
            controls?.classList.add('hidden');
            config?.classList.add('hidden');
            values?.classList.add('hidden');
            
            this.elements.connectBtn?.classList.remove('hidden');
            this.elements.scanAllBtn?.classList.remove('hidden');
            this.elements.disconnectBtn?.classList.add('hidden');
            this.elements.refreshBtn?.classList.add('hidden');
        }
    }

    /**
     * Get current input values
     */
    getInputValues() {
        return {
            lightOn: this.elements.lightSwitch?.checked,
            fanSpeed: parseInt(this.elements.fanSlider?.value || 0),
            fanMin: parseInt(this.elements.fanMinSlider?.value || 0),
            fanMax: parseInt(this.elements.fanMaxSlider?.value || 100),
            lightOnHour: parseInt(this.elements.lightOnHour?.value || 18),
            lightOffHour: parseInt(this.elements.lightOffHour?.value || 14)
        };
    }

    /**
     * Show loading state
     */
    showLoading(button, text = 'Laden...') {
        button.dataset.originalText = button.textContent;
        button.textContent = text;
        button.disabled = true;
    }

    /**
     * Hide loading state
     */
    hideLoading(button) {
        button.textContent = button.dataset.originalText || button.textContent;
        button.disabled = false;
    }
}

// Export singleton instance
export const uiController = new UIController();