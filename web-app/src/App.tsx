import { useState } from "react";
import "./App.css";
import { type SensorReading } from "./types/readings";
import { StatCard } from "./components/StatCard";
import { ChartCard } from "./components/ChartCard";
import {
  CartesianGrid,
  Line,
  LineChart,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from "recharts";
import { createFakeSensorReadings } from "./utils/createFakeSensorReadings";
import { Map } from "./components/Map";
import { useBleSensorReload } from "./hooks/useBle";
import {
  formatBleFinishReason,
  formatBleStatus,
  formatValue,
} from "./utils/formatter";
import { fillMissingSensorReadings } from "./utils/fillMissingSensorReadings";

function App() {
  const [readings, setReadings] = useState<SensorReading[]>(
    createFakeSensorReadings([], 30),
  );
  const latestReading = readings.at(-1) ?? null;

  const ble = useBleSensorReload({
    onReadings: (newReadings) => {
      setReadings((oldReadings) => {
        const filledReadings = fillMissingSensorReadings(
          oldReadings,
          newReadings,
        );

        return [...oldReadings, ...filledReadings].slice(-1000);
      });
    },
  });

  const chartData = () => {
    return readings
      .filter((reading) => reading.gnss.timestamp !== null)
      .map((reading) => {
        const timestamp = reading.gnss.timestamp as Date;

        return {
          timestampMs: timestamp.getTime(),

          temperature: reading.temperature.temperature,
          humidity: reading.temperature.humidity,

          accelerationX: reading.imu.accelerationX,
          accelerationY: reading.imu.accelerationY,
          accelerationZ: reading.imu.accelerationZ,

          gyroscopeX: reading.imu.gyroscopeX,
          gyroscopeY: reading.imu.gyroscopeY,
          gyroscopeZ: reading.imu.gyroscopeZ,

          current: reading.current.milliAmpere,
        };
      })
      .sort((a, b) => a.timestampMs - b.timestampMs);
  };

  return (
    <main className="dashboard">
      <header className="dashboard-header">
        <div className="dashboard-title">
          <h1>Piezo Fly Dashboard</h1>
          <p>
            {readings.length} Messwerte geladen
            {ble.deviceName ? ` · ${ble.deviceName}` : ""}
          </p>
        </div>

        <div className="header-controls">
          <div className="ble-status-card">
            <div className={`ble-status-dot ble-status-${ble.status}`} />

            <div>
              <strong>{formatBleStatus(ble.status)}</strong>

              <span>
                {ble.isReloading
                  ? "Daten werden geladen..."
                  : ble.lastFinishReason
                    ? `Letzter Reload: ${formatBleFinishReason(ble.lastFinishReason)}`
                    : "Bereit"}
              </span>
            </div>
          </div>

          <div className="ble-stats">
            <span>Pakete: {ble.receivedPackets}</span>
            <span>Neu: {ble.receivedReadings}</span>
          </div>

          <div className="actions">
            <button
              onClick={() => {
                setReadings((oldReadings) => {
                  const fakeReadings = createFakeSensorReadings(
                    oldReadings,
                    20,
                  );
                  return [...oldReadings, ...fakeReadings].slice(-1000);
                });
              }}
            >
              Fake Messwerte hinzufügen
            </button>

            <button onClick={ble.reload} disabled={ble.isReloading}>
              {ble.isReloading ? "Lade..." : "Verbinden / Reload"}
            </button>

            <button onClick={ble.forgetDevice} disabled={ble.isReloading}>
              Gerät neu wählen
            </button>

            <button
              className="button-secondary"
              onClick={() => setReadings([])}
              disabled={ble.isReloading}
            >
              Messwerte löschen
            </button>
          </div>

          {ble.lastError && <div className="ble-error">{ble.lastError}</div>}
        </div>
      </header>

      <section className="stat-grid">
        <StatCard
          title="Temperatur"
          value={latestReading?.temperature.temperature ?? null}
          unit=" °C"
        />
        <StatCard
          title="Luftfeuchtigkeit"
          value={latestReading?.temperature.humidity ?? null}
          unit=" %"
        />
      </section>

      <section className="map-section">
        <Map readings={readings} />
      </section>

      <section className="chart-grid">
        <ChartCard title="Temperatur & Luftfeuchtigkeit">
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={chartData()}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis
                dataKey="timestampMs"
                type="number"
                scale="time"
                domain={["dataMin", "dataMax"]}
                tickFormatter={(value) =>
                  new Date(value).toLocaleTimeString("de-DE", {
                    hour: "2-digit",
                    minute: "2-digit",
                    second: "2-digit",
                  })
                }
                minTickGap={30}
              />
              <YAxis />
              <Tooltip
                labelFormatter={(value) =>
                  new Date(Number(value)).toLocaleString("de-DE")
                }
              />
              <Line
                type="monotone"
                dataKey="temperature"
                name="Temperatur °C"
                dot={false}
                strokeWidth={2}
              />
              <Line
                type="monotone"
                dataKey="humidity"
                name="Luftfeuchte %"
                dot={false}
                strokeWidth={2}
              />
            </LineChart>
          </ResponsiveContainer>
        </ChartCard>

        <ChartCard title="Beschleunigung">
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={chartData()}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis
                dataKey="timestampMs"
                type="number"
                scale="time"
                domain={["dataMin", "dataMax"]}
                tickFormatter={(value) =>
                  new Date(value).toLocaleTimeString("de-DE", {
                    hour: "2-digit",
                    minute: "2-digit",
                    second: "2-digit",
                  })
                }
                minTickGap={30}
              />
              <YAxis />
              <YAxis />
              <Tooltip
                labelFormatter={(value) =>
                  new Date(Number(value)).toLocaleString("de-DE")
                }
              />
              <Line
                type="monotone"
                dataKey="accelerationX"
                name="X m/s²"
                dot={false}
                strokeWidth={2}
              />
              <Line
                type="monotone"
                dataKey="accelerationY"
                name="Y m/s²"
                dot={false}
                strokeWidth={2}
              />
              <Line
                type="monotone"
                dataKey="accelerationZ"
                name="Z m/s²"
                dot={false}
                strokeWidth={2}
              />
            </LineChart>
          </ResponsiveContainer>
        </ChartCard>

        <ChartCard title="Gyroskop">
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={chartData()}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis
                dataKey="timestampMs"
                type="number"
                scale="time"
                domain={["dataMin", "dataMax"]}
                tickFormatter={(value) =>
                  new Date(value).toLocaleTimeString("de-DE", {
                    hour: "2-digit",
                    minute: "2-digit",
                    second: "2-digit",
                  })
                }
                minTickGap={30}
              />
              <YAxis />
              <YAxis />
              <Tooltip
                labelFormatter={(value) =>
                  new Date(Number(value)).toLocaleString("de-DE")
                }
              />
              <Line
                type="monotone"
                dataKey="gyroscopeX"
                name="X °/s"
                dot={false}
                strokeWidth={2}
              />
              <Line
                type="monotone"
                dataKey="gyroscopeY"
                name="Y °/s"
                dot={false}
                strokeWidth={2}
              />
              <Line
                type="monotone"
                dataKey="gyroscopeZ"
                name="Z °/s"
                dot={false}
                strokeWidth={2}
              />
            </LineChart>
          </ResponsiveContainer>
        </ChartCard>
      </section>

      <section className="latest-card">
        <h2>Letzter Messwert</h2>

        {latestReading ? (
          <div className="latest-grid">
            <div>
              <strong>GNSS</strong>
              <p>
                Longitude: {formatValue(latestReading.gnss.longitude)}
                <br />
                Latitude: {formatValue(latestReading.gnss.latitude)}
                <br />
                Timestamp:{" "}
                {latestReading.gnss.timestamp
                  ? latestReading.gnss.timestamp.toLocaleString("de-DE")
                  : "—"}
              </p>
            </div>

            <div>
              <strong>IMU</strong>
              <p>
                Acc X: {formatValue(latestReading.imu.accelerationX, " m/s²")}
                <br />
                Acc Y: {formatValue(latestReading.imu.accelerationY, " m/s²")}
                <br />
                Acc Z: {formatValue(latestReading.imu.accelerationZ, " m/s²")}
                <br />
                Gyro X: {formatValue(latestReading.imu.gyroscopeX, " °/s")}
                <br />
                Gyro Y: {formatValue(latestReading.imu.gyroscopeY, " °/s")}
                <br />
                Gyro Z: {formatValue(latestReading.imu.gyroscopeZ, " °/s")}
              </p>
            </div>

            <div>
              <strong>Umgebung</strong>
              <p>
                Temperatur:{" "}
                {formatValue(latestReading.temperature.temperature, " °C")}
                <br />
                Luftfeuchte:{" "}
                {formatValue(latestReading.temperature.humidity, " %")}
                <br />
                Strom: {formatValue(latestReading.current.milliAmpere, " mA")}
              </p>
            </div>
          </div>
        ) : (
          <p>Noch keine Messwerte vorhanden.</p>
        )}
      </section>
    </main>
  );
}

export default App;
