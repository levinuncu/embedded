type StatCardProps = Readonly<{
  title: string;
  value: null | number;
  unit?: string;
}>;

export function StatCard({ title, value, unit }: StatCardProps) {
  return (
    <div className="stat-card">
      <div className="stat-title">{title}</div>
      <div className="stat-value">
        {value === null ? "—" : value}
        {value !== null && unit ? <span>{unit}</span> : null}
      </div>
    </div>
  );
}
