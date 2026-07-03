export function formatValue(value: null | number, unit = "") {
  if (value === null) return "—";
  return `${value}${unit}`;
}

export function formatBleStatus(status: string): string {
  switch (status) {
    case "idle":
      return "Nicht verbunden";
    case "unsupported":
      return "Bluetooth nicht unterstützt";
    case "selecting":
      return "Gerät auswählen";
    case "connecting":
      return "Verbinde";
    case "subscribing":
      return "Abonniere Daten";
    case "receiving":
      return "Empfange Daten";
    case "disconnecting":
      return "Trenne Verbindung";
    case "done":
      return "Reload abgeschlossen";
    case "error":
      return "Fehler";
    default:
      return status;
  }
}

export function formatBleFinishReason(reason: string): string {
  switch (reason) {
    case "idle":
      return "keine weiteren Daten";
    case "first-packet-timeout":
      return "keine Daten empfangen";
    case "max-timeout":
      return "Zeitlimit erreicht";
    default:
      return reason;
  }
}
