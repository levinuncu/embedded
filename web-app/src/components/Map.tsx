import {
  MapContainer,
  Marker,
  Polyline,
  Popup,
  TileLayer,
  useMap,
} from "react-leaflet";
import { useEffect, useMemo } from "react";
import type { SensorReading } from "../types/readings";
import "leaflet/dist/leaflet.css";
import { HHN_LAT, HHN_LNG } from "../utils/createFakeSensorReadings";

type LatLng = [number, number];

type MapProps = Readonly<{
  readings: SensorReading[];
}>;

const FALLBACK_CENTER: LatLng = [HHN_LAT, HHN_LNG];
const DEFAULT_ZOOM = 15;

export function Map({ readings }: MapProps) {
  const gpsPoints = useMemo(() => {
    return readings
      .filter((reading) => {
        const lat = reading.gnss.latitude;
        const lon = reading.gnss.longitude;

        return (
          lat !== null &&
          lon !== null &&
          Number.isFinite(lat) &&
          Number.isFinite(lon) &&
          lat >= -90 &&
          lat <= 90 &&
          lon >= -180 &&
          lon <= 180
        );
      })
      .map((reading) => {
        return {
          position: [
            reading.gnss.latitude as number,
            reading.gnss.longitude as number,
          ] satisfies LatLng,
          timestamp: reading.gnss.timestamp,
        };
      });
  }, [readings]);

  const latestPoint = gpsPoints.at(-1) ?? null;
  const center = latestPoint?.position ?? FALLBACK_CENTER;

  return (
    <div className="map-card">
      <h2>GPS Position</h2>

      <div className="map-wrapper">
        <MapContainer
          center={center}
          zoom={DEFAULT_ZOOM}
          scrollWheelZoom={true}
          className="sensor-map"
        >
          <TileLayer
            attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
            url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
          />

          <MapAutoCenter position={latestPoint?.position ?? null} />

          {gpsPoints.length > 1 && (
            <Polyline positions={gpsPoints.map((point) => point.position)} />
          )}

          {latestPoint && (
            <Marker position={latestPoint.position}>
              <Popup>
                Letzte Position
                <br />
                Lat: {latestPoint.position[0].toFixed(6)}
                <br />
                Lon: {latestPoint.position[1].toFixed(6)}
                <br />
                Zeit:{" "}
                {latestPoint.timestamp
                  ? latestPoint.timestamp.toLocaleString("de-DE")
                  : "—"}
              </Popup>
            </Marker>
          )}
        </MapContainer>
      </div>
    </div>
  );
}

function MapAutoCenter({ position }: { position: LatLng | null }) {
  const map = useMap();

  useEffect(() => {
    if (!position) {
      return;
    }

    map.setView(position, DEFAULT_ZOOM, {
      animate: true,
    });
  }, [map, position]);

  return null;
}
