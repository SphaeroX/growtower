/**
 * Logger module for debug output
 */
class Logger {
    constructor() {
        this.logContainer = document.getElementById('logContainer');
        this.maxEntries = 100;
        this.entries = [];
    }

    /**
     * Log a message
     * @param {string} message - Message to log
     * @param {string} type - Log type: 'info', 'success', 'error', 'warning'
     */
    log(message, type = 'info') {
        const timestamp = new Date().toLocaleTimeString();
        const entry = {
            timestamp,
            message,
            type
        };
        
        this.entries.unshift(entry);
        
        // Keep only last N entries
        if (this.entries.length > this.maxEntries) {
            this.entries.pop();
        }
        
        this.render();
        
        // Also log to console
        console.log(`[${type.toUpperCase()}] ${message}`);
    }

    /**
     * Log with info styling
     */
    info(message) {
        this.log(message, 'info');
    }

    /**
     * Log with success styling
     */
    success(message) {
        this.log(message, 'success');
    }

    /**
     * Log with error styling
     */
    error(message) {
        this.log(message, 'error');
    }

    /**
     * Log with warning styling
     */
    warning(message) {
        this.log(message, 'warning');
    }

    /**
     * Clear all log entries
     */
    clear() {
        this.entries = [];
        this.render();
    }

    /**
     * Render log entries to DOM
     */
    render() {
        if (!this.logContainer) return;
        
        this.logContainer.innerHTML = this.entries.map(entry => `
            <div class="log-entry ${entry.type}">
                <span class="log-timestamp">[${entry.timestamp}]</span>
                <span class="log-message">${this.escapeHtml(entry.message)}</span>
            </div>
        `).join('');
    }

    /**
     * Escape HTML special characters
     */
    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
}

// Export singleton instance
export const logger = new Logger();