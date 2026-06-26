import React, { useEffect, useRef, useState } from 'react';
import { View, Text, StyleSheet, TouchableOpacity } from 'react-native';

const ACCENT = 'rgb(140,190,7)';
const MAX_POINTS = 60;
const CHART_HEIGHT = 80;

type DataPoint = {
    value: number;
    timestamp: number;
};

type Series = {
    label: string;
    color: string;
    data: DataPoint[];
};

type ChartTab = 'imu' | 'env';

interface Props {
    temperature: number | null;
    humidity: number | null;
    imu: {
        acc_x: number; acc_y: number; acc_z: number;
        gyro_x: number; gyro_y: number; gyro_z: number;
    } | null;
    timestamp: number | null;
    isConnected: boolean;
}

function MiniLineChart({ series, height = CHART_HEIGHT, width }: {
    series: Series[];
    height?: number;
    width: number;
}) {
    const seriesWithEnoughData = series.filter(s => s.data.length >= 2);

    if (seriesWithEnoughData.length === 0) {
        return (
            <View style={[styles.chartPlaceholder, { height, width }]}>
                <Text style={styles.placeholderText}>waiting for data...</Text>
            </View>
        );
    }

    const allValues = seriesWithEnoughData.flatMap(s => s.data.map(d => d.value));
    const allTs = seriesWithEnoughData.flatMap(s => s.data.map(d => d.timestamp));
    const validValues = allValues.filter(v => isFinite(v));
    const validTs = allTs.filter(t => isFinite(t));

    if (validValues.length < 2 || validTs.length < 2) {
        return (
            <View style={[styles.chartPlaceholder, { height, width }]}>
                <Text style={styles.placeholderText}>waiting for data...</Text>
            </View>
        );
    }

    const rawMin = Math.min(...validValues);
    const rawMax = Math.max(...validValues);
    const padding = (rawMax - rawMin) * 0.1 || 1;
    const min = rawMin - padding;
    const max = rawMax + padding;
    const range = max - min || 1;

    const tMin = Math.min(...validTs);
    const tMax = Math.max(...validTs);
    const tRange = tMax - tMin || 1;

    const toX = (t: number) => ((t - tMin) / tRange) * width;
    const toY = (v: number) => height - ((v - min) / range) * height;

    return (
        <View style={{ width, height, position: 'relative', overflow: 'hidden' }}>
            {seriesWithEnoughData.map(s => {
                const segments = [];
                for (let i = 0; i < s.data.length - 1; i++) {
                    const x1 = toX(s.data[i].timestamp);
                    const y1 = toY(s.data[i].value);
                    const x2 = toX(s.data[i + 1].timestamp);
                    const y2 = toY(s.data[i + 1].value);

                    const dx = x2 - x1;
                    const dy = y2 - y1;
                    const length = Math.sqrt(dx * dx + dy * dy) || 1;
                    const angle = Math.atan2(dy, dx) * (180 / Math.PI);

                    segments.push(
                        <View
                            key={`${s.label}-${i}`}
                            style={{
                                position: 'absolute',
                                left: x1,
                                top: y1,
                                width: length,
                                height: 2,
                                backgroundColor: s.color,
                                transform: [{ rotate: `${angle}deg` }],
                                transformOrigin: '0 50%',
                            }}
                        />
                    );
                }
                const last = s.data[s.data.length - 1];
                const dotX = toX(last.timestamp);
                const dotY = toY(last.value);
                segments.push(
                    <View
                        key={`${s.label}-dot`}
                        style={{
                            position: 'absolute',
                            left: dotX - 4,
                            top: dotY - 4,
                            width: 8,
                            height: 8,
                            borderRadius: 4,
                            backgroundColor: s.color,
                        }}
                    />
                );

                return segments;
            })}
        </View>
    );
}

function Legend({ series }: { series: Series[] }) {
    return (
        <View style={styles.legend}>
            {series.map(s => (
                <View key={s.label} style={styles.legendItem}>
                    <View style={[styles.legendDot, { backgroundColor: s.color }]} />
                    <Text style={styles.legendLabel}>{s.label}</Text>
                </View>
            ))}
        </View>
    );
}

function useRollingBuffer(value: number | null, trigger: number | null, maxPoints: number) {
    const bufferRef = useRef<DataPoint[]>([]);
    const [, forceRender] = useState(0);

    useEffect(() => {
        if (value === null || trigger === null) return;
        const now = Date.now();
        const last = bufferRef.current[bufferRef.current.length - 1];
        const ts = last ? Math.max(now, last.timestamp + 1) : now;
        bufferRef.current = [
            ...bufferRef.current.slice(-(maxPoints - 1)),
            { value, timestamp: ts },
        ];
        forceRender(n => n + 1);
    }, [trigger]);

    return bufferRef.current;
}

export default function SensorChart({ temperature, humidity, imu, timestamp, isConnected }: Props) {
    const [tab, setTab] = useState<ChartTab>('imu');

    const accXBuf = useRollingBuffer(imu?.acc_x ?? null, timestamp, MAX_POINTS);
    const accYBuf = useRollingBuffer(imu?.acc_y ?? null, timestamp, MAX_POINTS);
    const accZBuf = useRollingBuffer(imu?.acc_z ?? null, timestamp, MAX_POINTS);
    const gyroXBuf = useRollingBuffer(imu?.gyro_x ?? null, timestamp, MAX_POINTS);
    const gyroYBuf = useRollingBuffer(imu?.gyro_y ?? null, timestamp, MAX_POINTS);
    const gyroZBuf = useRollingBuffer(imu?.gyro_z ?? null, timestamp, MAX_POINTS);
    const tempBuf = useRollingBuffer(
        temperature,
        timestamp,
        MAX_POINTS
    ); const humBuf = useRollingBuffer(humidity, timestamp, MAX_POINTS);

    if (!isConnected) return null;

    const accSeries: Series[] = [
        { label: 'X', color: '#e74c3c', data: accXBuf },
        { label: 'Y', color: ACCENT, data: accYBuf },
        { label: 'Z', color: '#3498db', data: accZBuf },
    ];

    const gyroSeries: Series[] = [
        { label: 'X', color: '#e74c3c', data: gyroXBuf },
        { label: 'Y', color: ACCENT, data: gyroYBuf },
        { label: 'Z', color: '#3498db', data: gyroZBuf },
    ];

    const tempSeries: Series[] = [
        { label: '°C', color: '#e67e22', data: tempBuf },
    ];

    const humSeries: Series[] = [
        { label: '%', color: '#3498db', data: humBuf },
    ];

    console.log('timestamp', timestamp);
    console.log('temperature', temperature);
    console.log('tempBuf', tempBuf.length);

    return (
        <View style={styles.wrapper}>
            <View style={styles.tabBar}>
                <TouchableOpacity
                    style={[styles.tab, tab === 'imu' && styles.tabActive]}
                    onPress={() => setTab('imu')}
                >
                    <Text style={[styles.tabText, tab === 'imu' && styles.tabTextActive]}>IMU</Text>
                </TouchableOpacity>
                <TouchableOpacity
                    style={[styles.tab, tab === 'env' && styles.tabActive]}
                    onPress={() => setTab('env')}
                >
                    <Text style={[styles.tabText, tab === 'env' && styles.tabTextActive]}>Environment</Text>
                </TouchableOpacity>
            </View>

            {tab === 'imu' && (
                <>
                    <ChartBlock title="Acceleration" series={accSeries} latestValues={[
                        imu?.acc_x ?? null, imu?.acc_y ?? null, imu?.acc_z ?? null
                    ]} unit="" />
                    <ChartBlock title="Gyroscope" series={gyroSeries} latestValues={[
                        imu?.gyro_x ?? null, imu?.gyro_y ?? null, imu?.gyro_z ?? null
                    ]} unit="°/s" />
                </>
            )}

            {tab === 'env' && (
                <>
                    <ChartBlock title="Temperature" series={tempSeries} latestValues={[temperature]} unit="°C" />
                    <ChartBlock title="Humidity" series={humSeries} latestValues={[humidity]} unit="%" />
                </>
            )}
        </View>
    );
}

function ChartBlock({ title, series, latestValues, unit }: {
    title: string;
    series: Series[];
    latestValues: (number | null)[];
    unit: string;
}) {
    const latest = latestValues.find(v => v !== null);
    const [chartWidth, setChartWidth] = useState(0);

    return (

        <View style={styles.block}>
            <View style={styles.blockHeader}>
                <Text style={styles.blockTitle}>{title}</Text>
                {latest !== null && latest !== undefined && (
                    <Text style={styles.blockValue}>{latest}{unit}</Text>
                )}
            </View>
            <Legend series={series} />
            <View
                style={{ width: '100%', height: CHART_HEIGHT }}
                onLayout={e => {
                    const w = e.nativeEvent.layout.width;
                    if (w > 0 && w !== chartWidth) setChartWidth(w);
                }}
            >
                {chartWidth > 0 && (
                    <MiniLineChart series={series} height={CHART_HEIGHT} width={chartWidth} />
                )}
            </View>
        </View>
    );
}

const styles = StyleSheet.create({
    wrapper: {
        marginTop: 8,
        marginBottom: 8,
    },
    tabBar: {
        flexDirection: 'row',
        marginBottom: 8,
        borderRadius: 8,
        overflow: 'hidden',
        backgroundColor: '#e8e8e8',
    },
    tab: {
        flex: 1,
        paddingVertical: 6,
        alignItems: 'center',
    },
    tabActive: {
        backgroundColor: ACCENT,
        borderRadius: 8,
    },
    tabText: {
        fontSize: 13,
        fontWeight: '600',
        color: '#888',
    },
    tabTextActive: {
        color: '#fff',
    },
    block: {
        marginBottom: 16,
    },
    blockHeader: {
        flexDirection: 'row',
        justifyContent: 'space-between',
        alignItems: 'baseline',
        marginBottom: 4,
    },
    blockTitle: {
        fontSize: 13,
        fontWeight: '600',
        color: '#333',
        textTransform: 'uppercase',
        letterSpacing: 0.5,
    },
    blockValue: {
        fontSize: 13,
        color: '#555',
    },
    legend: {
        flexDirection: 'row',
        marginBottom: 4,
        gap: 12,
    },
    legendItem: {
        flexDirection: 'row',
        alignItems: 'center',
        gap: 4,
    },
    legendDot: {
        width: 8,
        height: 8,
        borderRadius: 4,
    },
    legendLabel: {
        fontSize: 11,
        color: '#666',
    },
    chartPlaceholder: {
        justifyContent: 'center',
        alignItems: 'center',
        backgroundColor: '#f7f7f7',
        borderRadius: 6,
    },
    placeholderText: {
        fontSize: 12,
        color: '#aaa',
    },
});