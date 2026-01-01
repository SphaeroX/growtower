package com.growtower.control

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import java.util.UUID

class MainActivity : AppCompatActivity() {

    private lateinit var tvStatus: TextView
    private lateinit var btnConnect: Button
    private lateinit var btnToggleLight: Button
    private lateinit var etFanSpeed: EditText
    private lateinit var btnSetFan: Button
    private lateinit var tvLog: TextView

    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    private var bluetoothGatt: BluetoothGatt? = null
    private var isConnected = false

    // UUIDs
    private val SERVICE_UUID = UUID.fromString("4fafc201-1fb5-459e-8fcc-c5c9c331914b")
    private val LIGHT_CHAR_UUID = UUID.fromString("beb5483e-36e1-4688-b7f5-ea07361b26a8")
    private val FAN_CHAR_UUID = UUID.fromString("82563452-9477-4b78-953e-38ec6f43e592")

    // Permissions
    private val requiredPermissions =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT)
            } else {
                arrayOf(Manifest.permission.ACCESS_FINE_LOCATION)
            }

    private val permissionLauncher =
            registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) {
                    permissions ->
                val allGranted = permissions.entries.all { it.value }
                if (allGranted) {
                    startScan()
                } else {
                    log("Permissions denied")
                    Toast.makeText(this, "Permissions required for BLE", Toast.LENGTH_SHORT).show()
                }
            }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tvStatus = findViewById(R.id.tvStatus)
        btnConnect = findViewById(R.id.btnConnect)
        btnToggleLight = findViewById(R.id.btnToggleLight)
        etFanSpeed = findViewById(R.id.etFanSpeed)
        btnSetFan = findViewById(R.id.btnSetFan)
        tvLog = findViewById(R.id.tvLog)

        btnConnect.setOnClickListener {
            if (isConnected) {
                disconnect()
            } else {
                checkPermissionsAndStart()
            }
        }

        btnToggleLight.setOnClickListener { toggleLight() }

        btnSetFan.setOnClickListener { setFanSpeed() }
    }

    private fun checkPermissionsAndStart() {
        if (!hasPermissions()) {
            permissionLauncher.launch(requiredPermissions)
        } else {
            startScan()
        }
    }

    private fun hasPermissions(): Boolean {
        for (permission in requiredPermissions) {
            if (ActivityCompat.checkSelfPermission(this, permission) !=
                            PackageManager.PERMISSION_GRANTED
            ) {
                return false
            }
        }
        return true
    }

    @SuppressLint("MissingPermission")
    private fun startScan() {
        if (bluetoothAdapter == null || !bluetoothAdapter!!.isEnabled) {
            log("Bluetooth not enabled")
            return
        }

        tvStatus.text = getString(R.string.status_scanning)
        btnConnect.isEnabled = false
        log("Scanning for TOWER...")

        val scanner = bluetoothAdapter!!.bluetoothLeScanner
        val settings =
                ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build()

        // val filters =
        // listOf(ScanFilter.Builder().setServiceUuid(ParcelUuid(SERVICE_UUID)).build())
        // Scanning without filters to ensure we find the device even if UUID advertisement is
        // tricky
        val filters = emptyList<ScanFilter>()

        scanner.startScan(filters, settings, scanCallback)

        // Stop scan after 10 seconds
        Handler(Looper.getMainLooper())
                .postDelayed(
                        {
                            if (!isConnected) {
                                scanner.stopScan(scanCallback)
                                runOnUiThread {
                                    if (!isConnected) {
                                        tvStatus.text = getString(R.string.status_disconnected)
                                        btnConnect.isEnabled = true
                                        log("Scan timeout. Ensure GPS is ON and device is close.")
                                    }
                                }
                            }
                        },
                        10000
                )
    }

    private val scanCallback =
            object : ScanCallback() {
                @SuppressLint("MissingPermission")
                override fun onScanResult(callbackType: Int, result: ScanResult) {
                    super.onScanResult(callbackType, result)

                    val device = result.device
                    val deviceName = device.name
                    val scanRecordName = result.scanRecord?.deviceName
                    val rssi = result.rssi
                    val address = device.address

                    // Log every device found to debug visibility
                    log(
                            "Found: [${deviceName ?: "NULL"} / ${scanRecordName ?: "NULL"}] ($address) $rssi dBm"
                    )

                    // Check both cached Name and Advertising Packet Name
                    if (deviceName == "TOWER" || scanRecordName == "TOWER") {
                        log(">>> TARGET FOUND: TOWER <<<")
                        // Stop scanning and connect
                        bluetoothAdapter!!.bluetoothLeScanner.stopScan(this)
                        connectToDevice(device)
                    }
                }

                override fun onScanFailed(errorCode: Int) {
                    super.onScanFailed(errorCode)
                    log("Scan failed: $errorCode")
                    runOnUiThread {
                        tvStatus.text = getString(R.string.status_error)
                        btnConnect.isEnabled = true
                    }
                }
            }

    @SuppressLint("MissingPermission")
    private fun connectToDevice(device: BluetoothDevice) {
        runOnUiThread { tvStatus.text = getString(R.string.status_connecting) }
        log("Connecting to ${device.address}")
        bluetoothGatt = device.connectGatt(this, false, gattCallback)
    }

    @SuppressLint("MissingPermission")
    private fun disconnect() {
        bluetoothGatt?.disconnect()
    }

    // Helper to log
    private fun log(message: String) {
        Log.d("GrowTower", message)
        runOnUiThread { tvLog.append("$message\n") }
    }

    private val gattCallback =
            object : BluetoothGattCallback() {
                @SuppressLint("MissingPermission")
                override fun onConnectionStateChange(
                        gatt: BluetoothGatt,
                        status: Int,
                        newState: Int
                ) {
                    if (newState == BluetoothProfile.STATE_CONNECTED) {
                        log("Connected to GATT server.")
                        isConnected = true
                        runOnUiThread {
                            tvStatus.text = getString(R.string.status_connected)
                            btnConnect.text = getString(R.string.btn_disconnect)
                            btnConnect.isEnabled = true
                            btnToggleLight.isEnabled = true
                            btnSetFan.isEnabled = true
                        }
                        gatt.discoverServices()
                    } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                        log("Disconnected from GATT server.")
                        isConnected = false
                        runOnUiThread {
                            tvStatus.text = getString(R.string.status_disconnected)
                            btnConnect.text = getString(R.string.btn_connect)
                            btnConnect.isEnabled = true
                            btnToggleLight.isEnabled = false
                            btnSetFan.isEnabled = false
                        }
                        bluetoothGatt?.close()
                        bluetoothGatt = null
                    }
                }

                override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
                    if (status == BluetoothGatt.GATT_SUCCESS) {
                        log("Services discovered")
                    } else {
                        log("onServicesDiscovered received: $status")
                    }
                }

                override fun onCharacteristicWrite(
                        gatt: BluetoothGatt,
                        characteristic: BluetoothGattCharacteristic,
                        status: Int
                ) {
                    if (status == BluetoothGatt.GATT_SUCCESS) {
                        log("Write successful: ${characteristic.uuid}")
                    } else {
                        log("Write failed: $status")
                    }
                }

                // Correctly implementing onCharacteristicRead inside the callback object
                @SuppressLint("MissingPermission")
                override fun onCharacteristicRead(
                        gatt: BluetoothGatt,
                        characteristic: BluetoothGattCharacteristic,
                        value: ByteArray,
                        status: Int
                ) {
                    if (status == BluetoothGatt.GATT_SUCCESS) {
                        if (characteristic.uuid == LIGHT_CHAR_UUID) {
                            val currentValue = value.firstOrNull()?.toInt() ?: 0
                            val newValue = if (currentValue == 1) 0 else 1

                            log("Light is $currentValue, switching to $newValue")

                            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                                gatt.writeCharacteristic(
                                        characteristic,
                                        byteArrayOf(newValue.toByte()),
                                        BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                                )
                            } else {
                                characteristic.value = byteArrayOf(newValue.toByte())
                                characteristic.writeType =
                                        BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                                gatt.writeCharacteristic(characteristic)
                            }
                        } else {
                            log("Read value: ${value.contentToString()}")
                        }
                    } else {
                        log("Read failed: $status")
                    }
                }
            }

    @SuppressLint("MissingPermission")
    private fun toggleLight() {
        val service = bluetoothGatt?.getService(SERVICE_UUID)
        val characteristic = service?.getCharacteristic(LIGHT_CHAR_UUID)

        if (characteristic != null) {
            // Initiate read to trigger toggle logic in callback
            gattRead(characteristic)
        } else {
            log("Light characteristic not found")
        }
    }

    @SuppressLint("MissingPermission")
    private fun setFanSpeed() {
        val speedStr = etFanSpeed.text.toString()
        if (speedStr.isNotEmpty()) {
            val speed = speedStr.toIntOrNull()
            if (speed != null && speed in 0..100) {
                val service = bluetoothGatt?.getService(SERVICE_UUID)
                val characteristic = service?.getCharacteristic(FAN_CHAR_UUID)

                if (characteristic != null) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                        bluetoothGatt?.writeCharacteristic(
                                characteristic,
                                byteArrayOf(speed.toByte()),
                                BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                        )
                    } else {
                        characteristic.value = byteArrayOf(speed.toByte())
                        characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                        bluetoothGatt?.writeCharacteristic(characteristic)
                    }
                    log("Setting fan to $speed%")
                } else {
                    log("Fan characteristic not found")
                }
            } else {
                Toast.makeText(this, "Speed must be 0-100", Toast.LENGTH_SHORT).show()
            }
        }
    }

    @SuppressLint("MissingPermission")
    private fun gattRead(characteristic: BluetoothGattCharacteristic) {
        bluetoothGatt?.readCharacteristic(characteristic)
    }
}
