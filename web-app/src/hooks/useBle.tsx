import { useCallback, useEffect, useRef, useState } from "react";
import type { SensorReading } from "../types/readings";
import { parseSensorReadings } from "../utils/parseSensorReadings";

const TARGET_NAME = "piezo_fly";
const SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb";
const CHARACTERISTIC_UUID = "0000fff1-0000-1000-8000-00805f9b34fb";

type BleReloadStatus =
  | "idle"
  | "unsupported"
  | "selecting"
  | "connecting"
  | "subscribing"
  | "receiving"
  | "disconnecting"
  | "done"
  | "error";

type ReloadFinishReason =
  | "idle"
  | "first-packet-timeout"
  | "max-timeout"
  | null;

type UseBleSensorReloadOptions = Readonly<{
  onReadings: (readings: SensorReading[]) => void;
  idleTimeoutMs?: number;
  firstPacketTimeoutMs?: number;
  maxReloadMs?: number;
}>;

export function useBleSensorReload({
  onReadings,
  idleTimeoutMs = 800,
  firstPacketTimeoutMs = 3000,
  maxReloadMs = 10000,
}: UseBleSensorReloadOptions) {
  const [status, setStatus] = useState<BleReloadStatus>("idle");
  const [deviceName, setDeviceName] = useState<string | null>(null);
  const [lastError, setLastError] = useState<string | null>(null);
  const [lastFinishReason, setLastFinishReason] =
    useState<ReloadFinishReason>(null);
  const [receivedPackets, setReceivedPackets] = useState(0);
  const [receivedReadings, setReceivedReadings] = useState(0);

  const deviceRef = useRef<BluetoothDevice | null>(null);
  const onReadingsRef = useRef(onReadings);
  const isReloadingRef = useRef(false);

  useEffect(() => {
    onReadingsRef.current = onReadings;
  }, [onReadings]);

  const reload = useCallback(async () => {
    if (isReloadingRef.current) {
      return;
    }

    isReloadingRef.current = true;

    setLastError(null);
    setLastFinishReason(null);
    setReceivedPackets(0);
    setReceivedReadings(0);

    if (!("bluetooth" in navigator)) {
      setStatus("unsupported");
      setLastError("Web Bluetooth wird von diesem Browser nicht unterstützt.");
      isReloadingRef.current = false;
      return;
    }

    let characteristic: BluetoothRemoteGATTCharacteristic | null = null;
    let notificationHandler: ((event: Event) => void) | null = null;

    let firstPacketTimer: number | null = null;
    let idleTimer: number | null = null;
    let maxTimer: number | null = null;

    try {
      let device = deviceRef.current;

      if (!device) {
        setStatus("selecting");

        device = await navigator.bluetooth.requestDevice({
          filters: [{ namePrefix: TARGET_NAME }],
          optionalServices: [SERVICE_UUID],
        });

        deviceRef.current = device;
        setDeviceName(device.name ?? "Unbekanntes Gerät");
      }

      setStatus("connecting");

      const server = await device.gatt?.connect();

      if (!server) {
        throw new Error("Could not connect to GATT server.");
      }

      const service = await server.getPrimaryService(SERVICE_UUID);
      characteristic = await service.getCharacteristic(CHARACTERISTIC_UUID);

      setStatus("subscribing");

      const waitForReloadDone = new Promise<ReloadFinishReason>((resolve) => {
        let finished = false;
        let hasReceivedFirstPacket = false;

        function finish(reason: ReloadFinishReason) {
          if (finished) {
            return;
          }

          finished = true;
          resolve(reason);
        }

        function resetIdleTimer() {
          if (idleTimer !== null) {
            window.clearTimeout(idleTimer);
          }

          idleTimer = window.setTimeout(() => {
            finish("idle");
          }, idleTimeoutMs);
        }

        notificationHandler = (event: Event) => {
          const source =
            event.target as BluetoothRemoteGATTCharacteristic | null;

          const dataView = source?.value;

          if (!dataView) {
            return;
          }

          try {
            const readings = parseSensorReadings(dataView);

            hasReceivedFirstPacket = true;

            if (firstPacketTimer !== null) {
              window.clearTimeout(firstPacketTimer);
              firstPacketTimer = null;
            }

            setStatus("receiving");
            setReceivedPackets((oldValue) => oldValue + 1);
            setReceivedReadings((oldValue) => oldValue + readings.length);

            onReadingsRef.current(readings);

            resetIdleTimer();
          } catch (error) {
            console.error("Could not parse BLE packet:", error);
            setLastError(
              error instanceof Error
                ? error.message
                : "Could not parse BLE packet",
            );
          }
        };

        characteristic?.addEventListener(
          "characteristicvaluechanged",
          notificationHandler,
        );

        firstPacketTimer = window.setTimeout(() => {
          if (!hasReceivedFirstPacket) {
            finish("first-packet-timeout");
          }
        }, firstPacketTimeoutMs);

        maxTimer = window.setTimeout(() => {
          finish("max-timeout");
        }, maxReloadMs);
      });

      await characteristic.startNotifications();

      const finishReason = await waitForReloadDone;
      setLastFinishReason(finishReason);
    } catch (error) {
      console.error("BLE reload error:", error);

      setStatus("error");
      setLastError(
        error instanceof Error ? error.message : "Unknown BLE error",
      );
      return;
    } finally {
      if (firstPacketTimer !== null) {
        window.clearTimeout(firstPacketTimer);
      }

      if (idleTimer !== null) {
        window.clearTimeout(idleTimer);
      }

      if (maxTimer !== null) {
        window.clearTimeout(maxTimer);
      }

      setStatus((currentStatus) =>
        currentStatus === "error" ? "error" : "disconnecting",
      );

      if (characteristic && notificationHandler) {
        characteristic.removeEventListener(
          "characteristicvaluechanged",
          notificationHandler,
        );

        await characteristic.stopNotifications().catch(() => {
          // Ignorieren, falls die Verbindung schon getrennt ist.
        });
      }

      const device = deviceRef.current;

      if (device?.gatt?.connected) {
        device.gatt.disconnect();
      }

      setStatus((currentStatus) =>
        currentStatus === "error" ? "error" : "done",
      );

      isReloadingRef.current = false;
    }
  }, [firstPacketTimeoutMs, idleTimeoutMs, maxReloadMs]);

  const forgetDevice = useCallback(() => {
    if (deviceRef.current?.gatt?.connected) {
      deviceRef.current.gatt.disconnect();
    }

    deviceRef.current = null;
    setDeviceName(null);
    setStatus("idle");
    setLastError(null);
    setLastFinishReason(null);
    setReceivedPackets(0);
    setReceivedReadings(0);
  }, []);

  useEffect(() => {
    return () => {
      if (deviceRef.current?.gatt?.connected) {
        deviceRef.current.gatt.disconnect();
      }
    };
  }, []);

  const isReloading =
    status === "selecting" ||
    status === "connecting" ||
    status === "subscribing" ||
    status === "receiving" ||
    status === "disconnecting";

  return {
    status,
    deviceName,
    lastError,
    lastFinishReason,
    receivedPackets,
    receivedReadings,
    isSupported: "bluetooth" in navigator,
    isReloading,
    reload,
    forgetDevice,
  };
}
