type ChartCardProps = Readonly<{
  title: string;
  children: React.ReactNode;
}>;

export function ChartCard({ title, children }: ChartCardProps) {
  return (
    <div className="chart-card">
      <h2>{title}</h2>
      <div className="chart-wrapper">{children}</div>
    </div>
  );
}
